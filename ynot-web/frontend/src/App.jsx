
import React, { useState, useRef, useEffect, useReducer } from 'react';
import Landing from './landing';
import Board from './Board';
import Game from './Game';

import { Chess } from 'chess.js';
import './style.css';


const V = {
    p: 100,
    n: 320,
    b: 330,
    r: 500,
    q: 900,
    k: 0
};

//eval function(only material)
function ev(game) {
    let score = 0;

    for (const row of game.board()) {
        for (const piece of row) {
            if (piece) {
                if (piece.color === 'w') {
                    score += V[piece.type];
                } else {
                    score -= V[piece.type];
                }
            }
        }
    }

    return game.turn() === 'w' ? score : -score;
}
//negamax
function nm(game, depth, alpha, beta) {
    if (game.in_checkmate()) {
        return -99999 - depth;
    }

    if (game.in_draw()) {
        return 0;
    }

    if (depth === 0) {
        return ev(game);
    }

    let bestScore = -Infinity;

    for (const move of game.moves()) {
        game.move(move);

        const score = -nm(game,depth - 1,-beta,-alpha);
        game.undo();

        if (score > bestScore) {
            bestScore = score;
        }

        if (score > alpha) {
            alpha = score;
        }

        if (alpha >= beta) {
            break;
        }
    }
    return bestScore;
}

// beginner engine 
function builtin(game) {
    let bestMove = null;
    let bestScore = -Infinity;

    const moves = game.moves();

    for (const move of moves) {
        game.move(move);

        const score =
            -nm(game, 1, -Infinity, Infinity)
            + Math.random() * 30;

        game.undo();

        if (score > bestScore) {
            bestScore = score;
            bestMove = move;
        }
    }

    return bestMove;
}


export async function getBotMove(game, level) {
    if (level === 'beginner') {
        return builtin(game);
    }

    const  response= await fetch('/api/bestmove',{
        method:'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify({
            fen: game.fen()
        })
    }) 
    if (!response.ok) {
        const error = await response.json();
        throw new Error(error.detail || 'Engine request failed');
    }

    const data = await response.json();
    return data.move;
}


// TIME FORMATTING

export function fmt(ms) {
    const seconds = Math.ceil(Math.max(0, ms) / 1000);

    const minutes = Math.floor(seconds / 60);
    const remaining = String(seconds % 60).padStart(2, '0');

    return minutes + ':' + remaining;
}





function App() {
    const [screen, setScreen] = useState({ v: 'idle' });
    const [flip, setFlip] = useState(false);
 
    const startingGame = useRef(null);
 
    if (!startingGame.current) {
        startingGame.current = new Chess();
    }
 
 
    if (screen.v === 'idle') {
        return (
            <div className="wrap game">
                <Board
                    g={startingGame.current}
                    flip={flip}
                    sel={null}
                    last={null}
                    onSq={() => {}}
                    disabled={true}
                />
 
                <Landing
                    onStart={cfg => {
                        setScreen({
                            v: 'play',
                            cfg,
                            k: Date.now()
                        });
                    }}
                    onFlip={setFlip}
                />
            </div>
        );
    }
 
    return (
        <Game
            key={screen.k}
            cfg={screen.cfg}
            onQuit={() => {
                setFlip(false);
                setScreen({ v: 'idle' });
            }}
        />
    );
}
 
export default App;
