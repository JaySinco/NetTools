import React from 'react';

const Welcome = () => (
    <div id="welcome">
        <h1>Hello world</h1>
    </div>
);

export class App extends React.Component {
    render() {
        return (
            <Welcome />
        );
    }
}
