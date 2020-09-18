import React from "react"

const Star = (props: {
    selected: boolean
    onClick: (ev: React.MouseEvent) => void
}) => {
    return (
        <div
            className={props.selected ? "star selected" : "star"}
            onClick={props.onClick}
        ></div>
    )
}

type Props = {
    fullmark: number
    rating: number
    onRatingChange: (newRating: number) => void
}

const StarRating = (props: Props) => {
    const { fullmark, rating, onRatingChange } = props
    return (
        <div>
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
