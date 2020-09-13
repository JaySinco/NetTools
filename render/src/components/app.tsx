import React from 'react';
import { message } from 'antd';

function Square(props: {value: string|null, onClick: ()=>void}) {
    return (
        <button className="square" onClick={props.onClick}>
            {props.value}
        </button>
    );
}

interface BoardState {
    squares: Array<string|null>,
    xIsNext: boolean,
}

class Board extends React.Component<{}, BoardState> {
    constructor(props: {}) {
        super(props);
        this.state = {
            squares: Array(9).fill(null),
            xIsNext: true,
        }
    }

    renderSquare(i: number) {
        return (
        <Square
            value={this.state.squares[i]}
            onClick={()=>{this.handleClick(i);}}
        />
        );
    }

    handleClick(i: number) {
        message.info("intput!");
        const squares = this.state.squares.slice();
        squares[i] = this.state.xIsNext ? "X" : "O";
        this.setState({
            squares,
            xIsNext: !this.state.xIsNext
        });
    }

    render() {
        const status = 'Next player: ' + (this.state.xIsNext ? 'X' : 'O');
        return (
            <div>
                <div className="status">{status}</div>
                <div className="board-row">
                {this.renderSquare(0)}
                {this.renderSquare(1)}
                {this.renderSquare(2)}
                </div>
                <div className="board-row">
                {this.renderSquare(3)}
                {this.renderSquare(4)}
                {this.renderSquare(5)}
                </div>
                <div className="board-row">
                {this.renderSquare(6)}
                {this.renderSquare(7)}
                {this.renderSquare(8)}
                </div>
            </div>
        );
    }
}

export class App extends React.Component {
    render() {
        return (
            <div className="game">
                <div className="game-board">
                    <Board />
                </div>
                <div className="game-info">
                <div>{/* status */}</div>
                <ol>{/* TODO */}</ol>
                </div>
            </div>
        );
    }
}
