import React, { useState, useRef, useEffect, useReducer } from 'react';
import './style.css';


function Landing({ onStart, onFlip }) {
    const [pre, setPre] = useState('3+2');
    const [mins, setM] = useState(3);
    const [inc, setI] = useState(2);
    const [col, setCol] = useState('w');

    const [un, setUn] = useState('Me');
    const [bn, setBn] = useState('GgYnot');


    const [level, setLevel] = useState('beginner');

    const presets = ['1+0', '3+2', '10+5', '30+0'];

    function pickTime(preset) {
        setPre(preset);

        const [minutes, increment] = preset.split('+');

        setM(Number(minutes));
        setI(Number(increment));
    }

    function startGame() {
        onStart({
            mins: Math.max(0.5, Number(mins) || 5),
            inc: Math.max(0, Number(inc) || 0),
            col,
            uname: un || 'Me',
            bname: bn || 'MyBot',
            level
        });
    }

    return (
        <div className="card">
            <h2>Play my bot</h2>

            <label>Time control (minutes + increment)</label>

            <div className="pre">
                {presets.map(preset => (
                    <button key={preset} className={pre === preset ? 'on' : ''}
                             onClick={() => pickTime(preset)}
                    >
                        <b>{preset}</b>
                    </button>
                ))}
            </div>

            <div className="row">
                <div>
                    <label>Minutes</label>
                    <input type="number" step="0.5" min="0.5" value={mins}
                         onChange={e => {
                            setM(e.target.value);
                            setPre('');
                        }}
                    />
                </div>

                <div>
                    <label>Increment (s)</label>
                    <input type="number" min="0" value={inc}
                        onChange={e => {
                            setI(e.target.value);
                            setPre('');
                        }}
                    />
                </div>
            </div>

            <label>You play</label>
            <div className="seg">
                <button
                    className={col === 'w' ? 'on' : ''}
                    onClick={() => {
                        setCol('w');
                        onFlip(false);
                    }}
                >
                    White
                </button>

                <button
                    className={col === 'b' ? 'on' : ''}
                    onClick={() => {
                        setCol('b');
                        onFlip(true);
                    }}
                >
                    Black
                </button>

                <button
                    className={col === 'r' ? 'on' : ''}
                    onClick={() => {
                        setCol('r');
                        onFlip(false);
                    }}
                >
                    Random
                </button>
            </div>

            <div className="row">
                <div>
                    <label>Your name</label>
                    <input
                        value={un}
                        onChange={e => setUn(e.target.value)}
                    />
                </div>

                <div>
                    <label>Bot name</label>
                    <input
                        value={bn}
                        onChange={e => setBn(e.target.value)}
                    />
                </div>
            </div>

            <label>Bot difficulty</label>

            <div className="seg">
                <button
                    className={level === 'beginner' ? 'on' : ''}
                    onClick={() => setLevel('beginner')}
                >
                    Beginner
                </button>

               <button
                    className={level === 'gm' ? 'on' : ''}
                    onClick={() => setLevel('gm')}
                >
                    Grandmaster
                </button>
            </div>

            <p className="mu">
                Beginner uses the current built-in bot.
                Grandmaster will use your own NNUE model later.
            </p>

            <button
                className="go"
                style={{ width: '100%' }}
                onClick={startGame}
            >
                Start game
            </button>
        </div>
    );
}

export default Landing;