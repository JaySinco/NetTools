import React from 'react';

interface AddColorFormProps {
    onNewColor: (title: string, color: string) => void,
}

class AddColorForm extends React.Component<AddColorFormProps> {
    inputTextRef: React.RefObject<HTMLInputElement>;
    inputColorRef: React.RefObject<HTMLInputElement>;

    constructor(props: AddColorFormProps) {
        super(props);
        this.inputTextRef = React.createRef();
        this.inputColorRef = React.createRef();
    }

    submit = (e: React.FormEvent) => {
        e.preventDefault();
        this.props.onNewColor(this.inputTextRef.current!.value, this.inputColorRef.current!.value)
        this.inputTextRef.current!.value = "";
        this.inputColorRef.current!.value = "#000000";
        this.inputTextRef.current!.focus();
    }

    render() {
        return (
            <form onSubmit={this.submit}>
                <input ref={this.inputTextRef} type="text" placeholder="color title..."  />
                <input ref={this.inputColorRef} type="color" />
                <button>ADD</button>
            </form>
        )
    }
}

export class App extends React.Component {
    logColor = (title: string, color: string) => {
        console.log(`New Color: ${title} ${color}`)
    }
    
    render() {
        return <AddColorForm onNewColor={this.logColor} />;
    }
}
