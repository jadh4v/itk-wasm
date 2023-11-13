// Generated file. To retain edits, remove this comment.

import { JsonCompatible, Image } from 'itk-wasm'

interface VtkReadImageResult {
  /** WebWorker used for computation */
  webWorker: Worker | null

  /** Whether the input could be read. If false, the output image is not valid. */
  couldRead: JsonCompatible

  /** Output image */
  image: Image

}

export default VtkReadImageResult