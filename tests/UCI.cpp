// UCI.cpp
//
// Minimal UCI protocol wrapper around the existing Board/search engine.
// This lets the engine be driven by any standard UCI GUI or tournament
// tool (Arena, fastchess, cutechess-cli, etc.) for strength testing.
//
// IMPORTANT: Board::findBestMove(depth) has no internal clock awareness --
// it always searches to completion of the requested fixed depth. Since UCI
// time controls (wtime/btime/winc/binc, movetime) require the engine to
// respect a time budget, this wrapper implements basic time management
// itself: it repeatedly calls findBestMove(depth) at increasing depths
// (iterative deepening from the outside) and stops once the allocated
// time budget for the move is used up, returning the move from the last
// depth that finished in time. This adds real time-control support
// without touching the search internals in Board.cpp.

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cctype>
#include <chrono>
#include <algorithm>

#include "Board.h"
#include "NNUE.h"
#include "square.h"
#include "PieceType.h"

using Clock = std::chrono::steady_clock;

namespace
{
    Piece promotionPieceFromChar(char c, bool white)
    {
        switch (std::tolower(static_cast<unsigned char>(c)))
        {
            case 'q': return white ? WQUEEN  : BQUEEN;
            case 'r': return white ? WROOK   : BROOK;
            case 'b': return white ? WBISHOP : BBISHOP;
            case 'n': return white ? WKNIGHT : BKNIGHT;
            default:  return EMPTY;
        }
    }

    char promotionCharFromPiece(Piece p)
    {
        switch (p)
        {
            case WKNIGHT: case BKNIGHT: return 'n';
            case WBISHOP: case BBISHOP: return 'b';
            case WROOK:   case BROOK:   return 'r';
            case WQUEEN:  case BQUEEN:  return 'q';
            default: return '\0';
        }
    }

    std::string moveToUci(const Move& m)
    {
        std::string s = indexToNotation(m.from) + indexToNotation(m.to);
        char promoChar = promotionCharFromPiece(static_cast<Piece>(m.promotion));
        if (promoChar) s += promoChar;
        return s;
    }

    // Applies a single UCI move string (e.g. "e2e4", "e7e8q") to the board
    // by matching it against the current legal move list. Returns false if
    // no matching legal move is found (shouldn't happen with a correct GUI).
    bool applyUciMove(Board& board, const std::string& uciMove)
    {
        if (uciMove.size() < 4) return false;

        int from = notationToIndex(uciMove.substr(0, 2));
        int to   = notationToIndex(uciMove.substr(2, 2));
        if (from < 0 || to < 0) return false;

        Piece promo = EMPTY;
        if (uciMove.size() >= 5)
            promo = promotionPieceFromChar(uciMove[4], board.isWhiteToMove());

        MoveList moves = board.generateLegalMoves();
        for (const Move& m : moves)
        {
            const bool promoMatches = (promo == EMPTY)
                ? (m.promotion == EMPTY)
                : (m.promotion == promo);

            if (m.from == from && m.to == to && promoMatches)
            {
                board.makeMove(m);
                return true;
            }
        }
        return false;
    }

    constexpr const char* START_FEN =
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

    void handlePosition(Board& board, std::istringstream& iss)
    {
        std::string token;
        iss >> token;

        if (token == "startpos")
        {
            board.loadFEN(START_FEN);
            iss >> token; // consumes "moves" if present
        }
        else if (token == "fen")
        {
            std::vector<std::string> fenParts;
            std::string part;
            while (iss >> part && part != "moves")
                fenParts.push_back(part);

            std::string fen;
            for (size_t i = 0; i < fenParts.size(); i++)
            {
                fen += fenParts[i];
                if (i + 1 < fenParts.size()) fen += " ";
            }
            board.loadFEN(fen);
            token = part; // "moves" or empty if input ended
        }
        else
        {
            return; // malformed "position" command; ignore
        }

        if (token == "moves")
        {
            std::string mv;
            while (iss >> mv)
                applyUciMove(board, mv);
        }
    }

    // Very simple time allocation: budget a fraction of remaining time plus
    // most of the increment. This is intentionally conservative to avoid
    // ever losing on time, at the cost of not being maximally efficient.
    long long allocateTimeMs(bool whiteToMove, long long wtime, long long btime,
                              long long winc, long long binc, int movesToGo)
    {
        long long myTime = whiteToMove ? wtime : btime;
        long long myInc  = whiteToMove ? winc  : binc;

        if (myTime <= 0) myTime = 1000; // fallback if GUI didn't send times

        int divisor = movesToGo > 0 ? std::min(movesToGo, 30) : 30;
        long long budget = myTime / divisor + (myInc * 8) / 10;

        // Never spend more than ~40% of remaining time on one move,
        // and leave a small safety margin so we don't lose on time.
        budget = std::min(budget, (myTime * 4) / 10);
        budget = std::max<long long>(budget, 20);
        budget = std::min(budget, myTime - 50 > 0 ? myTime - 50 : budget);

        return budget;
    }

    void handleGo(Board& board, std::istringstream& iss)
    {
        std::string token;
        int fixedDepth = -1;
        long long movetime = -1;
        long long wtime = -1, btime = -1, winc = 0, binc = 0;
        int movesToGo = -1;

        while (iss >> token)
        {
            if      (token == "depth")    iss >> fixedDepth;
            else if (token == "movetime") iss >> movetime;
            else if (token == "wtime")    iss >> wtime;
            else if (token == "btime")    iss >> btime;
            else if (token == "winc")     iss >> winc;
            else if (token == "binc")     iss >> binc;
            else if (token == "movestogo") iss >> movesToGo;
            // "infinite", "ponder", "nodes", "mate" are not supported;
            // silently ignored rather than misbehaving.
        }

        constexpr int MAX_SEARCH_DEPTH = 64;
        Move bestMove{};
        bool haveMove = false;

        if (fixedDepth > 0)
        {
            // Fixed-depth mode (e.g. depth-matched testing): just search
            // to exactly that depth, no time budget involved.
            bestMove = board.findBestMove(fixedDepth);
            haveMove = true;
        }
        else
        {
            long long budgetMs = (movetime > 0)
                ? movetime
                : allocateTimeMs(board.isWhiteToMove(), wtime, btime, winc, binc, movesToGo);

            auto start = Clock::now();

            for (int depth = 1; depth <= MAX_SEARCH_DEPTH; depth++)
            {
                Move candidate = board.findBestMove(depth);
                bestMove = candidate;
                haveMove = true;

                long long elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                    Clock::now() - start).count();

                std::cout << "info depth " << depth
                          << " nodes " << board.getNodeCount()
                          << " time " << elapsedMs << "\n" << std::flush;

                // Stop if we've used up the budget, or if we're already
                // past half of it (next depth typically costs several
                // times more, so starting it would likely blow the budget).
                if (elapsedMs >= budgetMs || elapsedMs * 3 >= budgetMs * 2)
                    break;
            }
        }

        if (!haveMove)
        {
            // Should not happen in a legal position, but never send nothing.
            bestMove = board.findBestMove(1);
        }

        std::cout << "bestmove " << moveToUci(bestMove) << "\n" << std::flush;
    }
}

int main(int argc, char** argv)
{
    // NNUE weights path defaults to "tools/nnue.bin" (relative to whatever
    // working directory the process is launched from), but can be
    // overridden with an absolute path as the first argument. This matters
    // for tournament tools (fastchess/cutechess-cli), which don't
    // necessarily launch the engine with the project root as the working
    // directory -- pass an absolute path there to avoid silently falling
    // back to the much weaker material-only eval.
    std::string nnuePath = (argc > 1) ? argv[1] : "tools/nnue.bin";

    if (!NNUE::load(nnuePath))
        std::cout << "info string failed to load NNUE weights from '"
                   << nnuePath << "' -- falling back to material-only eval\n"
                   << std::flush;

    Board board;
    board.loadFEN(START_FEN);

    std::string line;
    while (std::getline(std::cin, line))
    {
        std::istringstream iss(line);
        std::string cmd;
        iss >> cmd;

        if (cmd == "uci")
        {
            std::cout << "id name ChessEngine\n";
            std::cout << "id author kerha\n";
            std::cout << "uciok\n" << std::flush;
        }
        else if (cmd == "isready")
        {
            std::cout << "readyok\n" << std::flush;
        }
        else if (cmd == "ucinewgame")
        {
            board.loadFEN(START_FEN);
        }
        else if (cmd == "position")
        {
            handlePosition(board, iss);
        }
        else if (cmd == "go")
        {
            handleGo(board, iss);
        }
        else if (cmd == "stop")
        {
            // No background search thread to stop in this simple wrapper;
            // "go" always returns synchronously before this could arrive.
        }
        else if (cmd == "quit")
        {
            break;
        }
        // Unknown/unsupported commands (setoption, ponderhit, debug, etc.)
        // are silently ignored, per the UCI spec's tolerance for this.
    }

    return 0;
}
