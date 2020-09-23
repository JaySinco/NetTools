import React from "react"
import Star from "./star"

type Props = {
    fullmark: number
    rating: number
    onRatingChange: (newRating: number) => void
}

const StarRating = (props: Props) => {
    const { fullmark, rating, onRatingChange } = props
    return (
        <div className="star-rating">
            {[...Array(fullmark).keys()].map((i) => (
                <Star
                    selected={i + 1 <= rating ? true : false}
                    onClick={(_) => onRatingChange(i + 1)}
                />
            ))}
            <p>
                {rating} of {fullmark} stars
            </p>
        </div>
    )
}

export default StarRating
