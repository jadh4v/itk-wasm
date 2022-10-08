import {
  TextStream,
  BinaryStream,
  InterfaceTypes,
  PipelineInput,
  runPipelineNode
} from 'itk-wasm'

import ApplyPstateToImageOptions from './ApplyPstateToImageOptions.js'
import ApplyPstateToImageNodeResult from './ApplyPstateToImageNodeResult.js'


import path from 'path'

/**
 * Apply a presentation state to a given DICOM image and render output as pgm bitmap or dicom file.
 *
 * @param {Uint8Array} imageIn - Input DICOM file
 *
 * @returns {Promise<ApplyPstateToImageNodeResult>} - result object
 */
async function applyPstateToImageNode(  imageIn: Uint8Array,
  options: ApplyPstateToImageOptions = {})
    : Promise<ApplyPstateToImageNodeResult> {

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

  const pipelinePath = path.join(path.dirname(import.meta.url.substring(7)), 'pipelines', 'apply-pstate-to-image')

  const {
    returnValue,
    stderr,
    outputs
  } = await runPipelineNode(pipelinePath, args, desiredOutputs, inputs)
  if (returnValue !== 0) {
    throw new Error(stderr)
  }

  const result = {
    pstateOutStream: (outputs[0].data as TextStream).data,
    bitmapOutStream: (outputs[1].data as BinaryStream).data,
  }
  return result
}

export default applyPstateToImageNode
