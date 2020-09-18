import React from "react"

type Props = {
    onNewColor: (title: string, color: string) => void
}

class AddColorForm extends React.Component<Props> {
    textRef: React.RefObject<HTMLInputElement> = React.createRef()
    colorRef: React.RefObject<HTMLInputElement> = React.createRef()

    submit = (e: React.FormEvent) => {
        const { onNewColor } = this.props
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
