import React from "react"
import StarRating from "./star-rating"

type Props = {
    title: string
    color: string
    rating: number
    fullmark: number
    onRatingChange: (newRating: number) => void
    onColorRemove: (_: void) => void
}

const Color = (props: Props) => {
    const {
        title,
        color,
        rating,
        fullmark,
        onRatingChange,
        onColorRemove,
    } = props
    return (
        <section>
            <h1>{title}</h1>
            <button onClick={(_) => onColorRemove()}>X</button>
            <div style={{ backgroundColor: color, height: "100px" }}></div>
            <div>
                <StarRating
                    fullmark={fullmark}
                    rating={rating}
                    onRatingChange={(newRating) => onRatingChange(newRating)}
                />
            </div>
        </section>
    )
}

export default Color
