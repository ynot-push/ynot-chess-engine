import React from 'react';
import { Chessboard } from 'react-chessboard';
import './style.css';

function Board({
    g,
    flip = false,
    sel = null,
    last = null,
    onSq = () => {},
    onDrop = () => false,
    disabled = false
}) {
    const squareStyles = {};

    if (last) {
        squareStyles[last.from] = {
            backgroundColor: 'rgba(255, 235, 59, 0.45)'
        };
        squareStyles[last.to] = {
            backgroundColor: 'rgba(255, 235, 59, 0.45)'
        };
    }

    if (sel) {
        squareStyles[sel] = {
            ...squareStyles[sel],
            backgroundColor: 'rgba(255, 235, 59, 0.65)'
        };

        for (const move of g.moves({ square: sel, verbose: true })) {
            const occupied = g.get(move.to);

            squareStyles[move.to] = {
                ...squareStyles[move.to],
                background: occupied
                    ? 'radial-gradient(transparent 52%, rgba(0,0,0,.25) 53%)'
                    : 'radial-gradient(rgba(0,0,0,.25) 19%, transparent 20%)'
            };
        }
    }

    if (g.isCheck()) {
        g.board().forEach((row, r) => {
            row.forEach((piece, c) => {
                if (piece?.type === 'k' && piece.color === g.turn()) {
                    const square = String.fromCharCode(97 + c) + (8 - r);

                    squareStyles[square] = {
                        ...squareStyles[square],
                        background: 'radial-gradient(circle, #ff5555, #ff000099 70%, transparent 72%)'
                    };
                }
            });
        });
    }

    function handleSquareClick(value) {
        if (disabled || g.isGameOver()) return;

        // Some versions pass "e2"; others may pass an object.
        const square = typeof value === 'string'
            ? value
            : value?.square;

        if (square) onSq(square);
    }

    function handleDrop(from, to) {
        if (disabled || g.isGameOver()) return false;
        return onDrop(from, to);
    }

    return (
        <div className="board">
            <Chessboard
                options={{
                    position: g.fen(),
                    boardOrientation: flip ? 'black' : 'white',
                    onPieceDrop: handleDrop,
                    onSquareClick: handleSquareClick,
                    squareStyles,
                    lightSquareStyle: { backgroundColor: '#eeeed2' },
                    darkSquareStyle: { backgroundColor: '#769656' },
                    boardStyle: {
                        borderRadius: '4px',
                        overflow: 'hidden'
                    },
                    boardWidth: 450,
                    arePiecesDraggable: !disabled && !g.isGameOver()
                }}
            />
        </div>
    );
}

export default Board;