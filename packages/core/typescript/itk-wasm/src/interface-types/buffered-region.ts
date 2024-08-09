class BufferedRegion {
    index: number[]
    size: number[]
    constructor (dimension: number) {
        this.index = new Array(dimension)
        this.index.fill(0)

        this.size = new Array(dimension)
        this.size.fill(0)
    }
}

export default BufferedRegion
