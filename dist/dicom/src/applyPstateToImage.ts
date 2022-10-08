import {
  TextStream,
  BinaryStream,
  InterfaceTypes,
  PipelineInput,
  runPipeline
} from 'itk-wasm'

import ApplyPstateToImageOptions from './ApplyPstateToImageOptions.js'
import ApplyPstateToImageResult from './ApplyPstateToImageResult.js'

/**
 * Apply a presentation state to a given DICOM image and render output as pgm bitmap or dicom file.
 *
 * @param {Uint8Array} imageIn - Input DICOM file
 *
 * @returns {Promise<ApplyPstateToImageResult>} - result object
 */
async function applyPstateToImage(
  webWorker: null | Worker,
  imageIn: Uint8Array,
  options: ApplyPstateToImageOptions = {})
    : Promise<ApplyPstateToImageResult> {

  const desiredOutputs = [
    { type: InterfaceTypes.TextStream },
    { type: InterfaceTypes.BinaryStream },
  ]
  const inputs: [ PipelineInput ] = [
    { type: InterfaceTypes.BinaryFile, data: { data: imageIn, path: "file0" }  },
  ]

  const args = []
  // Inputs
  args.push('file0')
  // Outputs
  args.push('0')
  args.push('1')
  // Options
  args.push('--memory-io')
  if (options.pstateFile) {
    const inputFile = 'file' + inputs.length.toString()
    inputs.push({ type: InterfaceTypes.BinaryFile, data: { data: options.pstateFile, path: inputFile } })
    args.push('--pstate-file', inputFile)
  }
  if (options.configFile) {
    args.push('--config-file', options.configFile.toString())
  }
  if (options.frame) {
    args.push('--frame', options.frame.toString())
  }
  if (options.pstateOutput) {
    args.push('--pstate-output')
  }
  if (options.bitmapOutput) {
    args.push('--bitmap-output')
  }
  if (options.pgm) {
    args.push('--pgm')
  }
  if (options.dicom) {
    args.push('--dicom')
  }

  const pipelinePath = 'apply-pstate-to-image'

  const {
    webWorker: usedWebWorker,
    returnValue,
    stderr,
    outputs
  } = await runPipeline(webWorker, pipelinePath, args, desiredOutputs, inputs)
  if (returnValue !== 0) {
    throw new Error(stderr)
  }

  const result = {
    webWorker: usedWebWorker as Worker,
    pstateOutStream: (outputs[0].data as TextStream).data,
    bitmapOutStream: (outputs[1].data as BinaryStream).data,
  }
  return result
}

export default applyPstateToImage
