// Generated file. To retain edits, remove this comment.

import { BinaryFile, WorkerPoolFunctionOption } from 'itk-wasm'

interface WriteParametricMapOptions extends WorkerPoolFunctionOption {
  /**  Source DICOM images that were used to generate the parametric map. */
  refDicomSeries: string[] | File[] | BinaryFile[]

}

export default WriteParametricMapOptions
