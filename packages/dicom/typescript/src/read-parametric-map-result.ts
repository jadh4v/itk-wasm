// Generated file. To retain edits, remove this comment.

import { Image, JsonCompatible, WorkerPoolFunctionResult } from 'itk-wasm'

interface ReadParametricMapResult extends WorkerPoolFunctionResult {
  /** Parametric map as an output ITK image */
  paramImage: Image

  /** Output meta information as JSON */
  metaInfo: JsonCompatible

}

export default ReadParametricMapResult
