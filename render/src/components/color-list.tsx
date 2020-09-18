import React from "react"
import Color from "./color"

type Props = {
    colors: Array<{
        id: string
        title: string
        color: string
        rating: number
    }>
    fullmark: number
    onRatingChange: (id: string, newRating: number) => void
    onColorRemove: (id: string) => void
}

const ColorList = (props: Props) => {
    const { colors, fullmark, onRatingChange, onColorRemove } = props
    return (
        <div>
            {colors.length <= 0 ? (
                <p>No Colors Listed. (Add a Color)</p>
            ) : (
                colors.map((color) => (
                    <Color
                        {...color}
                        fullmark={fullmark}
                        onRatingChange={(newRating) =>
                            onRatingChange(color.id, newRating)
                        }
                        onColorRemove={(_) => onColorRemove(color.id)}
                    />
                ))
            )}
        </div>
    )
}

export default ColorList
