
import React, { useRef, useEffect, useReducer } from 'react';
import { Chess } from 'chess.js';
import Board from './Board';
import Results from './Results';
import { getBotMove, fmt } from './App';
import './style.css';

function Game({ cfg, onQuit }) {
    const R = useRef(null);
    const [, refresh] = useReducer(x => x + 1, 0);

    // Create game data once.
    if (!R.current) {
        const userColor = cfg.col === 'r'
            ? (Math.random() < 0.5 ? 'w' : 'b')
            : cfg.col;

        const startingTime = cfg.mins * 60000;

        R.current = {
            g: new Chess(),
            plies: [],
            sel: null,
            uc: userColor,
            bc: userColor === 'w' ? 'b' : 'w',

            clock: {
                w: startingTime,
                b: startingTime
            },

            t0: performance.now(),
            tmr: null,
            over: false,
            busy: false,
            last: null,
            msg: '',
            result: null
        };
    }

    const S = R.current;
    const activeColor = S.g.turn();

    const names = {
        [S.uc]: cfg.uname || 'Me',
        [S.bc]: cfg.bname || 'MyBot'
    };

    // Finish the game.
    function finish(result, why) {
        if (S.over) return;

        S.over = true;
        S.busy = false;

        if (S.tmr) {
            clearInterval(S.tmr);
            S.tmr = null;
        }

        S.result = {
            plies: S.plies,
            result,
            why,
            uc: S.uc,
            names,
            cfg
        };

        refresh();
    }

    // Play a move from the user or bot.
    function play(uci, who) {
        if (S.over) return false;

        const legalMoves = S.g.moves({ verbose: true });
        const moveText = String(uci);

        // Find a move in UCI format, such as e2e4.
        let move = legalMoves.find(m =>
            (m.from + m.to + (m.promotion || '')) ===
            moveText.toLowerCase()
        );

        // Also support SAN notation, such as Nf3.
        if (!move) {
            move = legalMoves.find(m =>
                m.san.replace(/[+#]/g, '') ===
                moveText.replace(/[+#]/g, '')
            );
        }

        // Default promotion to a queen when unspecified.
        if (!move) {
            move = legalMoves.find(m =>
                m.from + m.to ===
                    moveText.slice(0, 4).toLowerCase() &&
                m.promotion === 'q' &&
                !moveText[4]
            );
        }

        if (!move) return false;

        const mover = S.g.turn();
        const now = performance.now();

        // Account for the time used by the moving player.
        S.clock[mover] -= now - S.t0;
        S.t0 = now;

        if (S.clock[mover] <= 0) {
            S.clock[mover] = 0;

            finish(
                mover === 'w' ? '0-1' : '1-0',
                (who === 'user' ? 'You' : 'The bot') +
                    ' ran out of time'
            );

            return true;
        }

        // Add the increment after a valid move.
        S.clock[mover] += cfg.inc * 1000;

        const previousFen = S.g.fen();

        S.g.move(move);

        S.plies.push({
            san: move.san,
            fen: previousFen,
            mover,
            who,
            clk: S.clock[mover]
        });

        S.last = move;
        S.sel = null;

        // Check for checkmate, stalemate, or a draw.
        if (S.g.isGameOver()) {
            const checkmate = S.g.isCheckmate();

            const result = checkmate
                ? (mover === 'w' ? '1-0' : '0-1')
                : '1/2-1/2';

            const reason = checkmate
                ? 'Checkmate'
                : S.g.in_stalemate()
                    ? 'Stalemate'
                    : S.g.insufficient_material()
                        ? 'Insufficient material'
                        : S.g.in_threefold_repetition()
                            ? 'Threefold repetition'
                            : 'Fifty-move rule';

            finish(result, reason);
            return true;
        }

        refresh();

        // Let the bot move when it is its turn.
        if (S.g.turn() === S.bc) {
            botTurn();
        }

        return true;
    }

    // Ask the selected bot to make a move.
    async function botTurn() {
        if (S.over || S.busy) return;

        S.busy = true;
        refresh();

        // Small pause for a natural playing experience.
        await new Promise(resolve => setTimeout(resolve, 60));

        try {
            const move = await getBotMove(S.g, cfg.level);

            if (S.over) return;

            S.busy = false;

            if (!move) {
                S.msg = 'This bot level is not available yet.';
                finish('1/2-1/2', S.msg);
                return;
            }

            if (!play(String(move), 'bot')) {
                S.msg = 'The bot returned an illegal move: ' + move;
                finish('1/2-1/2', S.msg);
                return;
            }
        } catch (error) {
            S.busy = false;
            S.msg = 'Bot error: ' + error.message;
            finish('1/2-1/2', S.msg);
        }
    }

    // Start and update the clocks.
    useEffect(() => {
        S.t0 = performance.now();

        S.tmr = setInterval(() => {
            if (S.over) return;

            const now = performance.now();
            const color = S.g.turn();

            S.clock[color] -= now - S.t0;
            S.t0 = now;

            if (S.clock[color] <= 0) {
                S.clock[color] = 0;

                finish(
                    color === 'w' ? '0-1' : '1-0',
                    (color === S.uc ? 'You' : 'The bot') +
                        ' ran out of time'
                );

                return;
            }

            refresh();
        }, 100);

        // If the user plays Black, the bot plays White first.
        if (S.bc === 'w') {
            botTurn();
        }

        return () => {
            if (S.tmr) {
                clearInterval(S.tmr);
                S.tmr = null;
            }
        };
    }, []);

    // Handle clicks on board squares.
    function click(square) {
        if (
            S.over ||
            S.busy ||
            S.g.turn() !== S.uc
        ) {
            return;
        }

        if (S.sel) {
            const move = S.g.moves({
                square: S.sel,
                verbose: true
            }).find(m => m.to === square);

            if (move) {
                play(
                    move.from +
                    move.to +
                    (move.promotion ? 'q' : ''),
                    'user'
                );

                return;
            }
        }

        const piece = S.g.get(square);

        S.sel = piece && piece.color === S.uc
            ? square
            : null;

        refresh();
    }

    // Handle dragging a piece.
    function drop(from, to) {
        if (
            S.over ||
            S.busy ||
            S.g.turn() !== S.uc
        ) {
            return false;
        }

        return play(from + to, 'user');
    }

    // Clock component.
    function Clock({ color, name }) {
        const isActive = !S.over && activeColor === color;
        const isLow = S.clock[color] < 20000;

        return (
            <div
                className={[
                    'board-clock',
                    isActive ? 'active' : '',
                    isLow ? 'low' : ''
                ].filter(Boolean).join(' ')}
            >
                <div className="clock-player">
                    <div className="clock-avatar">
                        {name.charAt(0).toUpperCase()}
                    </div>

                    <span>{name}</span>

                    {isActive && (
                        <span className="clock-dot" />
                    )}
                </div>

                <div className="clock-time">
                    {fmt(Math.max(0, S.clock[color]))}
                </div>
            </div>
        );
    }

    // Display the move list.
    const moveList = [];

    S.plies.forEach((move, i) => {
        if (i % 2 === 0) {
            moveList.push(
                <span className="n" key={'n' + i}>
                    {Math.floor(i / 2) + 1}.
                </span>
            );
        }

        moveList.push(
            <span key={i}>{move.san}</span>
        );
    });

    return (
        <div className="wrap game">

            {/* Board with a clock above and below */}
            <div className="board-column">

                <Clock
                    color={S.bc}
                    name={names[S.bc]}
                />

                <div className="board-frame">
                    <Board
                        g={S.g}
                        flip={S.uc === 'b'}
                        sel={S.sel}
                        last={S.last}
                        onSq={click}
                        onDrop={drop}
                        disabled={
                            S.over ||
                            S.busy ||
                            S.g.turn() !== S.uc
                        }
                    />
                </div>

                <Clock
                    color={S.uc}
                    name={names[S.uc]}
                />

            </div>

            {/* Game information and controls */}
            <div className="game-sidebar">

                {S.result ? (
                    <Results
                        d={S.result}
                        onNew={onQuit}
                    />
                ) : (
                    <div className="card">

                        <div className="stat">
                            {S.msg ||
                                (S.over
                                    ? 'Game over'
                                    : S.busy
                                        ? 'Bot is thinking…'
                                        : activeColor === S.uc
                                            ? 'Your move'
                                            : 'Waiting…')}
                        </div>

                        <div className="moves">
                            {moveList}
                        </div>

                        <div
                            className="row"
                            style={{ marginTop: 10 }}
                        >
                            <button
                                onClick={() => {
                                    if (!S.over) {
                                        finish(
                                            S.uc === 'w'
                                                ? '0-1'
                                                : '1-0',
                                            'You resigned'
                                        );
                                    }
                                }}
                            >
                                Resign
                            </button>

                            <button onClick={onQuit}>
                                Quit
                            </button>
                        </div>

                    </div>
                )}

            </div>

        </div>
    );
}

export default Game;