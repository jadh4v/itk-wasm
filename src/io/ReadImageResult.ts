import Image from '../core/Image.js'

interface ReadImageResult {
  image: Image
  webWorker: Worker
  orderedFileNames?: Array<string>
}

export default ReadImageResult
