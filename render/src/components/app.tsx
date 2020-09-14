import React from 'react';

interface Ingredient_t {
    name: string,
    amount: number,
    measurement: string,
}

interface Recipe_t {
    name: string,
    ingredients: Ingredient_t[],
    steps: string[],
}

const data: Recipe_t[] = [
{
    "name": "Baked Salmon",
    "ingredients": [
        { "name": "Salmon", "amount": 1, "measurement": "l lb" },
        { "name": "Pine Nuts", "amount": 1, "measurement": "cup" },
        { "name": "Butter Lettuce", "amount": 2, "measurement": "cups" },
        { "name": "Yellow Squash", "amount": 1, "measurement": "med" },
        { "name": "Olive Oil", "amount": 0.5, "measurement": "cup" },
        { "name": "Garlic", "amount": 3, "measurement": "cloves" }
    ],
    "steps": [
        "Preheat the oven to 350 degrees.",
        "Spread the olive oil around a glass baking dish.",
        "Add the salmon, garlic, and pine nuts to the dish.",
        "Bake for 15 minutes.",
        "Add the yellow squash and put back in the oven for 30 mins.",
        "Remove from oven and let cool for 15 minutes. Add the lettuce and serve."
    ]
},
{
    "name": "Fish Tacos",
    "ingredients": [
        { "name": "Whitefish", "amount": 1, "measurement": "l lb" },
        { "name": "Cheese", "amount": 1, "measurement": "cup" },
        { "name": "Iceberg Lettuce", "amount": 2, "measurement": "cups" },
        { "name": "Tomatoes", "amount": 2, "measurement": "large"},
        { "name": "Tortillas", "amount": 3, "measurement": "med" }
    ],
    "steps": [
        "Cook the fish on the grill until hot.",
        "Place the fish on the 3 tortillas.",
        "Top them with lettuce, tomatoes, and cheese."
    ]
}
];

const Recipe = ({name, ingredients, steps}: Recipe_t) => (
    <div>
    <h1>{name}</h1>
    <ul>
        {ingredients.map(
            ({name}) => <li>{name}</li>
        )}
    </ul>
    <h2>steps</h2>
    <section>
        {steps.map(
            s => <p>{s}</p>
        )}
    </section>
    </div>
);

const Menu = ({data}: {data: Recipe_t[]}) => (
    <div>
        <h1>Delicious Recipes</h1>
        {data.map(
            v => <Recipe {...v} />
        )}
    </div>
);

export class App extends React.Component {
    render() {
        return <Menu data={data} />;
    }
}
