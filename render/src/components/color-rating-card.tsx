import React from "react"
import StarRating from "./star-rating"

type Props = {
    id: string
    title: string
    color: string
    rating: number
    fullmark: number
    onRatingChange: (id: string, newRating: number) => void
}

const ColorRatingCard = (props: Props) => {
    return (
        <section>
            <h1>{props.title}</h1>
            <div style={{ backgroundColor: props.color }}></div>
            <div>
                <StarRating
                    fullmark={props.fullmark}
                    rating={props.rating}
                    onRatingChange={(newRating) =>
                        props.onRatingChange(props.id, newRating)
                    }
                />
            </div>
        </section>
    )
}

export default ColorRatingCard
