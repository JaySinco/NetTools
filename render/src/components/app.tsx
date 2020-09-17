import React from 'react';

interface AddColorFormProps {
    onNewColor: (title: string, color: string) => void
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

interface StarProps {
    selected: boolean,
    onClick: (ev: React.MouseEvent) => void;
}

const Star = (props: StarProps) => {
    return (
        <div className={props.selected ? "star selected" : "star"}
             onClick={props.onClick}>
        </div>
    )
}

interface StarRatingProps {
    highest: number
    rating: number
    onRatingChange: (newRating: number) => void
}

const StarRating = (props: StarRatingProps) => {
    const { highest, rating, onRatingChange } = props;
    return (
        <div>
            {[...Array(highest)].map(
                (i, n) => (
                    <Star selected={(n+1) <= rating ? true : false}
                          onClick={ev => onRatingChange(n+1)}
                    />)
            )}
        </div>
    )
}

interface ColorRatingProps {

}

interface ColorRatingState {
    rating: number
}

class ColorRating extends React.Component<ColorRatingProps, ColorRatingState> {
    constructor(props: ColorRatingProps) {
        super(props);
        this.state = {
            rating: 0 
        };
    }

    onNewColor = (title: string, color: string) => {
        console.log(`New Color: ${title} ${color}`)
    }

    onRatingChange = (newRating: number) => {
        this.setState({
            ...this.state,
            rating: newRating,
        })
    }

    render() {
        const { rating } = this.state;
        return (
            <div>
                <AddColorForm onNewColor={this.onNewColor} />
                <StarRating highest={5} rating={rating} onRatingChange={this.onRatingChange} />
            </div>
        )
    }
}

export const App = () => <ColorRating />;
