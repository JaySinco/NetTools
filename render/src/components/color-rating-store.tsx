import { observable, action } from "mobx"
import { v4 as uuid } from "uuid"

class ColorRatingStore {
    @observable fullmark = 5
    @observable colors: Array<{
        id: string
        title: string
        color: string
        rating: number
    }> = [
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
    ]

    @action
    onNewColor = (title: string, color: string) => {
        if (title.length <= 0) return
        this.colors.push({ id: uuid(), title, color, rating: 0 })
    }

    @action
    onColorRemove = (id: string) => {
        this.colors = this.colors.filter((c) => c.id !== id)
    }

    @action
    onRatingChange = (id: string, newRating: number) => {
        this.colors = this.colors.map((c) =>
            c.id === id ? { ...c, rating: newRating } : c
        )
    }
}

export default new ColorRatingStore()
