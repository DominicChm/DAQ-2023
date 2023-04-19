// https://stackoverflow.com/a/33397682/16238567 
export function dotNotationLookup(obj: Object, notation: string) {
    return notation.split('.').reduce((a, b) => a[b], obj);
}

export function mapDotNotations(obj: Object, notations: string[]) {
    return notations.map(n => dotNotationLookup(obj, n));
}