import React, { useState, useEffect } from 'react';
import { Chess } from 'chess.js';
import './style.css';

const STOCKFISH_WORKER_URL =
    '/stockfish/stockfish-19-lite-single.js';

const STOCKFISH_DEPTH = 12;

// Convert centipawns to the Lichess Win% scale.
function wp(cp) {
    cp = Math.max(-1000, Math.min(1000, cp));

    return 100 / (1 + Math.exp(-0.00368208 * cp));
}

// Calculate accuracy for one move.
// before and after are Win% from the MOVING PLAYER'S perspective.
function moveAccuracy(before, after) {
    if (after >= before) return 100;

    const loss = before - after;

    const accuracy =
        103.1668100711649 *
        Math.exp(-0.04354415386753951 * loss) -
        3.166924740191411 + 1;

    return Math.max(0, Math.min(100, accuracy));
}

function clk(ms) {
    const seconds = Math.max(0, Math.round(ms / 1000));
    const hours = Math.floor(seconds / 3600);
    const minutes = Math.floor(seconds / 60) % 60;
    const remaining = seconds % 60;

    return (
        hours + ':' +
        String(minutes).padStart(2, '0') + ':' +
        String(remaining).padStart(2, '0')
    );
}
function standardDeviation(values) {
    if (!values.length) return 0;

    const mean =
        values.reduce((sum, value) => sum + value, 0) /
        values.length;

    const variance =
        values.reduce(
            (sum, value) => sum + (value - mean) ** 2,
            0
        ) / values.length;

    return Math.sqrt(variance);
}

function harmonicMean(values) {
    if (!values.length) return null;
    if (values.some(value => value <= 0)) return 0;

    return values.length /
        values.reduce((sum, value) => sum + 1 / value, 0);
}

// Create a Stockfish UCI worker.
function createStockfish() {
    const worker = new Worker(STOCKFISH_WORKER_URL);

    let readyResolve;
    let readyReject;
    let currentResolve = null;
    let currentScore = null;
    let stopped = false;

    const ready = new Promise((resolve, reject) => {
        readyResolve = resolve;
        readyReject = reject;
    });

    worker.onmessage = event => {
        const message = String(event.data);

        if (message.includes('uciok')) {
            worker.postMessage('isready');
            return;
        }

        if (message.includes('readyok')) {
            readyResolve();
            return;
        }

        // Keep the latest evaluation reported by Stockfish.
        const scoreMatch = message.match(
            /\bscore (cp|mate) (-?\d+)/
        );

        if (scoreMatch) {
            const type = scoreMatch[1];
            const value = Number(scoreMatch[2]);

            if (type === 'cp') {
                currentScore = value;
            } else {
                // Approximate forced mates as extreme evaluations.
                currentScore = value > 0 ? 1000 : -1000;
            }
        }

        if (message.startsWith('bestmove')) {
            if (currentResolve) {
                currentResolve(currentScore);
                currentResolve = null;
            }
        }
    };

    worker.onerror = error => {
        if (!stopped) {
            readyReject(error);

            if (currentResolve) {
                currentResolve(null);
                currentResolve = null;
            }
        }
    };

    worker.postMessage('uci');

    return {
        async evaluate(fen) {
            await ready;

            return new Promise(resolve => {
                currentScore = null;
                currentResolve = resolve;

                worker.postMessage(`position fen ${fen}`);
                worker.postMessage(`go depth ${STOCKFISH_DEPTH}`);
            }).then(score => {
                if (score === null) return null;

                // UCI score is from the side-to-move perspective.
                // Convert it to White's perspective.
                const sideToMove = fen.split(' ')[1];

                return sideToMove === 'b' ? -score : score;
            });
        },

        stop() {
            stopped = true;
            worker.postMessage('stop');
            worker.terminate();
        }
    };
}

// Reproduce Lichess's game-level weighting approach.
function calculateGameAccuracy(moves, winPercents, firstColor) {
    if (!moves.length) return null;

    const moveCount = moves.length;

    // Lichess uses a window size based on game length.
    const windowSize = Math.max(
        2,
        Math.min(8, Math.floor(moveCount / 10))
    );

    // Match the reference implementation's window alignment.
    const windows = [];

    for (let i = 0; i < windowSize - 2; i++) {
        windows.push(winPercents.slice(0, windowSize));
    }

    for (
        let i = 0;
        i + windowSize <= winPercents.length;
        i++
    ) {
        windows.push(winPercents.slice(i, i + windowSize));
    }

    // Each position transition gets a volatility weight.
    const weights = windows.map(window => {
        const deviation = standardDeviation(window);

        return Math.max(0.5, Math.min(12, deviation));
    });

    // Associate each move with the volatility of its window.
    const moveData = moves.map((move, index) => {
        const weight = weights[index] ?? 0.5;

        return { ...move, weight };
    });

    function forColor(color) {
        const selected = moveData.filter(
            move => move.color === color
        );

        if (!selected.length) return null;

        const weightTotal = selected.reduce(
            (sum, move) => sum + move.weight,
            0
        );

        const weightedMean = selected.reduce(
            (sum, move) => sum + move.acc * move.weight,
            0
        ) / weightTotal;

        const hMean = harmonicMean(
            selected.map(move => move.acc)
        );

        if (hMean === null) return null;

        return (weightedMean + hMean) / 2;
    }

    return {
        white: forColor('w'),
        black: forColor('b')
    };
}

function Results({ d, onNew }) {
    const [analysis, setAnalysis] = useState(null);
    const [progress, setProgress] = useState(0);
    const [copyMessage, setCopyMessage] = useState('');
    const [error, setError] = useState('');

    const {
        plies,
        result,
        why,
        uc,
        names,
        cfg
    } = d;

    const botColor = uc === 'w' ? 'b' : 'w';

    const date = new Date();

    const dateString =
        date.getFullYear() + '.' +
        String(date.getMonth() + 1).padStart(2, '0') + '.' +
        String(date.getDate()).padStart(2, '0');

    let pgn =
        `[Event "Casual game vs bot"]\n` +
        `[Date "${dateString}"]\n` +
        `[White "${names.w}"]\n` +
        `[Black "${names.b}"]\n` +
        `[Result "${result}"]\n` +
        `[TimeControl "${Math.round(cfg.mins * 60)}+${cfg.inc}"]\n\n`;

    plies.forEach((move, i) => {
        if (i % 2 === 0) {
            pgn += (Math.floor(i / 2) + 1) + '. ';
        }

        pgn += move.san + ' {[%clk ' + clk(move.clk) + ']} ';
    });

    pgn += result;

    useEffect(() => {
        let cancelled = false;
        let engine = null;

        async function analyseGame() {
            try {
                setError('');
                setAnalysis(null);
                setProgress(0);

                if (!plies.length) {
                    setAnalysis({
                        moves: [],
                        game: { white: null, black: null }
                    });
                    return;
                }

                engine = createStockfish();

                // Collect every position, including the final one.
                const positions = [];

                for (const move of plies) {
                    positions.push(move.fen);
                }

                const finalGame = new Chess(
                    plies[plies.length - 1].fen
                );

                finalGame.move(plies[plies.length - 1].san);
                positions.push(finalGame.fen());

                // Evaluate each position with Stockfish.
                const evaluations = [];

                for (let i = 0; i < positions.length; i++) {
                    if (cancelled) return;

                    const score = await engine.evaluate(
                        positions[i]
                    );

                    if (score === null) {
                        throw new Error(
                            'Stockfish did not return an evaluation.'
                        );
                    }

                    evaluations.push(score);

                    setProgress(
                        Math.round(100 * (i + 1) / positions.length)
                    );
                }

                const winPercents = evaluations.map(wp);

                const moveResults = plies.map((move, i) => {
                    const moverColor = i % 2 === 0 ? 'w' : 'b';

                    // Convert White's Win% into the mover's Win%.
                    const before = moverColor === 'w'
                        ? winPercents[i]
                        : 100 - winPercents[i];

                    const after = moverColor === 'w'
                        ? winPercents[i + 1]
                        : 100 - winPercents[i + 1];

                    const beforeCp = moverColor === 'w'
                        ? evaluations[i]
                        : -evaluations[i];

                    const afterCp = moverColor === 'w'
                        ? evaluations[i + 1]
                        : -evaluations[i + 1];

                    const drop = Math.max(0, before - after);

                    const accuracy = moveAccuracy(before, after);

                    const loss = Math.max(0, beforeCp - afterCp);

                    return {
                        i,
                        san: move.san,
                        who: move.who,
                        color: moverColor, // This was missing!
                        loss,
                        drop,
                        acc: accuracy
                    };
                });

                const gameAccuracy = calculateGameAccuracy(
                    moveResults,
                    winPercents,
                    'w'
                );

                if (!cancelled) {
                    setAnalysis({
                        moves: moveResults,
                        game: gameAccuracy
                    });

                    setProgress(100);
                }
            } catch (err) {
                if (!cancelled) {
                    console.error(err);
                    setError(
                        'Could not analyse the game. Check that Stockfish is installed at ' +
                        STOCKFISH_WORKER_URL +
                        ' and that its WASM files load correctly.'
                    );
                }
            } finally {
                if (engine) engine.stop();
            }
        }

        analyseGame();

        return () => {
            cancelled = true;

            if (engine) engine.stop();
        };
    }, [plies]);

    function average(color, type) {
        if (!analysis) return 0;

        const moves = analysis.moves.filter(
            move => move.who === color
        );

        if (!moves.length) return 0;


            if (type === 'acc') {
                const chessColor = color === 'user' ? uc : botColor;

                const resultKey = chessColor === 'w' ? 'white' : 'black';
                const score = analysis.game?.[resultKey];

                return Number.isFinite(score) ? score : 0;
            }


        return moves.reduce(
            (sum, move) => sum + Math.min(move.loss, 1000),
            0
        ) / moves.length;
    }

    async function copyPGN() {
        try {
            await navigator.clipboard.writeText(pgn);
            setCopyMessage('Copied ✓');
        } catch (error) {
            const textarea = document.getElementById('pgn');

            textarea.select();

            try {
                document.execCommand('copy');
                setCopyMessage('Copied ✓');
            } catch (copyError) {
                setCopyMessage('Press Ctrl/Cmd+C');
            }
        }
    }

    function AccuracyBox({ color, name }) {
        return (
            <div>
                <span>{name}</span>

                <b>
                    {average(color, 'acc').toFixed(1)}%
                </b>

                <span className="mu">
                    avg loss {average(color, 'loss').toFixed(0)} cp
                </span>
            </div>
        );
    }

    return (
        <div>
            <div className="card">
                <h2>
                    {result === '1/2-1/2'
                        ? 'Draw ½–½'
                        : result === '1-0'
                            ? 'White wins 1–0'
                            : 'Black wins 0–1'}
                </h2>

                <div className="mu">
                    {why} · {plies.length} plies
                </div>

            
                {!analysis ? (
                    <div className="analysis-progress">
                        <div className="analysis-progress-info">
                            <span>
                                {error ? 'Analysis failed' : 'Analysing with Stockfish…'}
                            </span>
                            <strong>{progress}%</strong>
                        </div>

                        <div
                            className="analysis-progress-track"
                            role="progressbar"
                            aria-valuemin={0}
                            aria-valuemax={100}
                            aria-valuenow={progress}
                        >
                            <div
                                className="analysis-progress-fill"
                                style={{ width: `${progress}%` }}
                            />
                        </div>

                        {error && (
                            <p className="analysis-progress-status">{error}</p>
                        )}
                    </div>
                ) : (

                    <div>
                        <div className="acc">
                            <AccuracyBox
                                color="user"
                                name={names[uc] + ' (you)'}
                            />

                            <AccuracyBox
                                color="bot"
                                name={names[botColor] + ' (bot)'}
                            />
                        </div>

                        <div className="tw">
                            <table>
                                <thead>
                                    <tr>
                                        <th>Ply</th>
                                        <th>Move</th>
                                        <th>Player</th>
                                        <th>Loss (cp)</th>
                                        <th>Accuracy</th>
                                        <th>Verdict</th>
                                    </tr>
                                </thead>

                                <tbody>
                                    {analysis.moves.map(move => {
                                        let verdict = '';
                                        let className = '';

                                        if (move.drop >= 30) {
                                            verdict = 'Blunder';
                                            className = 'c-b';
                                        } else if (move.drop >= 20) {
                                            verdict = 'Mistake';
                                            className = 'c-m';
                                        } else if (move.drop >= 10) {
                                            verdict = 'Inaccuracy';
                                            className = 'c-i';
                                        } else {
                                            verdict = 'Good';
                                        }

                                        return (
                                            <tr key={move.i}>
                                                <td>
                                                    {Math.floor(move.i / 2) + 1}
                                                    {move.i % 2 ? '…' : '.'}
                                                </td>

                                                <td>{move.san}</td>

                                                <td>
                                                    {move.who === 'user'
                                                        ? 'You'
                                                        : 'Bot'}
                                                </td>

                                                <td>
                                                    {move.loss.toFixed(0)}
                                                </td>

                                                <td>
                                                    {move.acc.toFixed(1)}%
                                                </td>

                                                <td className={className}>
                                                    {verdict}
                                                </td>
                                            </tr>
                                        );
                                    })}
                                </tbody>
                            </table>
                        </div>
                    </div>
                )}

                <label>PGN (with clock times)</label>

                <textarea
                    id="pgn"
                    rows={7}
                    readOnly
                    value={pgn}
                />

                <div
                    className="row"
                    style={{
                        marginTop: 10,
                        alignItems: 'center'
                    }}
                >
                    <div style={{ flex: '0 0 auto' }}>
                        <button
                            className="go"
                            style={{ margin: 0 }}
                            onClick={copyPGN}
                        >
                            Copy PGN
                        </button>
                    </div>

                    <span className="mu">{copyMessage}</span>

                    <div
                        style={{
                            flex: '0 0 auto',
                            marginLeft: 'auto'
                        }}
                    >
                        <button onClick={onNew}>
                            New game
                        </button>
                    </div>
                </div>

                <p className="mu">
                    Accuracy uses Stockfish evaluations and a
                    Lichess-style win-percentage formula. Results
                    may differ from Lichess because engine version,
                    depth, and analysis settings affect evaluations.
                </p>
            </div>
        </div>
    );
}

export default Results;
