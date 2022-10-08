interface ApplyPstateToImageOptions {
  /** [f]ilename: string, process using presentation state file */
  pstateFile?: Uint8Array

  /** [f]ilename: string, process using settings from configuration file */
  configFile?: string

  /** [f]rame: integer, process using image frame f (default: 1) */
  frame?: number

  /** get presentation state information in text stream (default: ON). */
  pstateOutput?: boolean

  /** get resulting image as bitmap output stream (default: ON). */
  bitmapOutput?: boolean

  /** save image as PGM (default) */
  pgm?: boolean

  /** save image as DICOM secondary capture */
  dicom?: boolean

}

export default ApplyPstateToImageOptions
