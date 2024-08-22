// Generated file. To retain edits, remove this comment.

import {
  Image,
  JsonCompatible,
  InterfaceTypes,
  PipelineOutput,
  PipelineInput,
  runPipelineNode
} from 'itk-wasm'

import WriteParametricMapNodeOptions from './write-parametric-map-node-options.js'
import WriteParametricMapNodeResult from './write-parametric-map-node-result.js'

import path from 'path'
import { fileURLToPath } from 'url'

/**
 * Convert ITK image to DICOM Parametric Map.
 *
 * @param {Image} paramapImage - parametric map as an ITK image
 * @param {JsonCompatible} metaInfo - JSON containing metadata attributes.
 * @param {string} outputDicomFile - File name of the DICOM Parametricmap object with the result of the conversion.
 * @param {WriteParametricMapNodeOptions} options - options object
 *
 * @returns {Promise<WriteParametricMapNodeResult>} - result object
 */
async function writeParametricMapNode(
  paramapImage: Image,
  metaInfo: JsonCompatible,
  outputDicomFile: string,
  options: WriteParametricMapNodeOptions = { refDicomSeries: [] as string[], }
) : Promise<WriteParametricMapNodeResult> {

  const mountDirs: Set<string> = new Set()

  const desiredOutputs: Array<PipelineOutput> = [
  ]

  const inputs: Array<PipelineInput> = [
    { type: InterfaceTypes.Image, data: paramapImage },
    { type: InterfaceTypes.JsonCompatible, data: metaInfo as JsonCompatible  },
  ]

  const args = []
  // Inputs
  const paramapImageName = '0'
  args.push(paramapImageName)

  const metaInfoName = '1'
  args.push(metaInfoName)

  // Outputs
  const outputDicomFileName = outputDicomFile
  args.push(outputDicomFileName)
  mountDirs.add(path.dirname(outputDicomFileName))

  // Options
  args.push('--memory-io')
  if (options.refDicomSeries) {
    if(options.refDicomSeries.length < 1) {
      throw new Error('"ref-dicom-series" option must have a length > 1')
    }
    args.push('--ref-dicom-series')

    options.refDicomSeries.forEach((value) => {
      mountDirs.add(path.dirname(value as string))
      args.push(value as string)
    })
  }

  const pipelinePath = path.join(path.dirname(fileURLToPath(import.meta.url)), 'pipelines', 'write-parametric-map')

  const {
    returnValue,
    stderr,
  } = await runPipelineNode(pipelinePath, args, desiredOutputs, inputs, mountDirs)
  if (returnValue !== 0 && stderr !== "") {
    throw new Error(stderr)
  }

  const result = {
  }
  return result
}

export default writeParametricMapNode
