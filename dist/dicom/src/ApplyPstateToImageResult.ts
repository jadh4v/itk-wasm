interface ApplyPstateToImageResult {
  /** WebWorker used for computation */
  webWorker: Worker | null

  /** Output overlay information */
  pstateOutStream: string

  /** Output image */
  bitmapOutStream: Uint8Array

}

export default ApplyPstateToImageResult
