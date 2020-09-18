import React from "react"
import { v4 as uuid } from "uuid"
import AddColorForm from "./add-color-form"
import ColorList from "./color-list"

type State = {
    colors: Array<{
        id: string
        title: string
        color: string
        rating: number
    }>
}

class ColorRating extends React.Component<{}, State> {
    state = {
        colors: [
            {
                id: "0175d1f0-a8c6-41bf-8d02-df5734d829a4",
                title: "ocean at dusk",
                color: "#00c4e2",
                rating: 5,
            },
            {
                id: "83c7ba2f-7392-4d7d-9e23-35adbe186046",
                title: "lawn",
                color: "#26ac56",
                rating: 3,
            },
            {
                id: "a11e3995-b0bd-4d58-8c48-5e49ae7f7f23",
                title: "bright red",
                color: "#ff0000",
                rating: 0,
            },
        ],
    }

    onNewColor = (title: string, color: string) => {
        if (title.length <= 0) return
        let colors = [
            ...this.state.colors,
            { id: uuid(), title, color, rating: 0 },
        ]
        this.setState({
            ...this.state,
            colors,
        })
    }

    onColorRemove = (id: string) => {
        let colors = this.state.colors.filter((c) => c.id !== id)
        this.setState({
            ...this.state,
            colors,
        })
    }

    onRatingChange = (id: string, newRating: number) => {
        let colors = this.state.colors.map((c) =>
            c.id === id ? { ...c, rating: newRating } : c
        )
        this.setState({
            ...this.state,
            colors,
        })
    }

    render() {
        const { onNewColor, onRatingChange, onColorRemove } = this
        return (
            <div>
                <AddColorForm onNewColor={onNewColor} />
                <ColorList
                    colors={this.state.colors}
                    fullmark={5}
                    onRatingChange={onRatingChange}
                    onColorRemove={onColorRemove}
                />
            </div>
        )
    }
}

export default ColorRating
