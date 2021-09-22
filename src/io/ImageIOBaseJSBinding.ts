import IOComponent from './IOComponent.js'
import IOPixel from './IOPixel.js'

interface ImageIOBaseJSBinding {
  IOComponentType: typeof IOComponent,
  IOPixelType: typeof IOPixel,
}

export default ImageIOBaseJSBinding
