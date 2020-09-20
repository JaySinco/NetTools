import React from "react"
import { observer } from "mobx-react"
import Color from "./color"
import colorRatingStore from "./color-rating-store"

const ColorList = observer(() => {
    const { colors, fullmark, onRatingChange, onColorRemove } = colorRatingStore
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
})

export default ColorList
