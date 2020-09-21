import React from "react"
import colorRatingStore from "./color-rating-store"

class AddColorForm extends React.Component {
    textRef: React.RefObject<HTMLInputElement> = React.createRef()
    colorRef: React.RefObject<HTMLInputElement> = React.createRef()

    submit = (e: React.FormEvent) => {
        const { onNewColor } = colorRatingStore
        e.preventDefault()
        onNewColor(this.textRef.current!.value, this.colorRef.current!.value)
        this.textRef.current!.value = ""
        this.colorRef.current!.value = "#000000"
        this.textRef.current!.focus()
    }

    render() {
        return (
            <form onSubmit={this.submit}>
                <input
                    ref={this.textRef}
                    type="text"
                    placeholder="color title..."
                />
                <input ref={this.colorRef} type="color" />
                <button>ADD</button>
            </form>
        )
    }
}

export default AddColorForm
