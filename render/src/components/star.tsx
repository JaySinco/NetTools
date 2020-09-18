import React from "react"

type Props = {
    selected: boolean
    onClick: (ev: React.MouseEvent) => void
}

const Star = (props: Props) => {
    const { selected, onClick } = props
    return (
        <div
            className={selected ? "star selected" : "star"}
            onClick={onClick}
        ></div>
    )
}

export default Star
