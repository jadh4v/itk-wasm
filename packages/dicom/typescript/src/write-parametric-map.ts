// Generated file. To retain edits, remove this comment.

import {
  Image,
  JsonCompatible,
  BinaryFile,
  InterfaceTypes,
  PipelineOutput,
  PipelineInput,
  runPipeline
} from 'itk-wasm'

import WriteParametricMapOptions from './write-parametric-map-options.js'
import WriteParametricMapResult from './write-parametric-map-result.js'

import { getPipelinesBaseUrl } from './pipelines-base-url.js'
import { getPipelineWorkerUrl } from './pipeline-worker-url.js'

import { getDefaultWebWorker } from './default-web-worker.js'

/**
 * Convert ITK image to DICOM Parametric Map.
 *
 * @param {Image} paramapImage - parametric map as an ITK image
 * @param {JsonCompatible} metaInfo - JSON containing metadata attributes.
 * @param {string} outputDicomFile - File name of the DICOM Parametricmap object with the result of the conversion.
 * @param {WriteParametricMapOptions} options - options object
 *
 * @returns {Promise<WriteParametricMapResult>} - result object
 */
async function writeParametricMap(
  paramapImage: Image,
  metaInfo: JsonCompatible,
  outputDicomFile: string,
  options: WriteParametricMapOptions = { refDicomSeries: [] as BinaryFile[] | File[] | string[], }
) : Promise<WriteParametricMapResult> {

  const desiredOutputs: Array<PipelineOutput> = [
    { type: InterfaceTypes.BinaryFile, data: { path: outputDicomFile, data: new Uint8Array() }},
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

  // Options
  args.push('--memory-io')
  if (options.refDicomSeries) {
    if(options.refDicomSeries.length < 1) {
      throw new Error('"ref-dicom-series" option must have a length > 1')
    }
    args.push('--ref-dicom-series')

    await Promise.all(options.refDicomSeries.map(async (value) => {
      let valueFile = value
      if (value instanceof File) {
        const valueBuffer = await value.arrayBuffer()
        valueFile = { path: value.name, data: new Uint8Array(valueBuffer) }
      }
      inputs.push({ type: InterfaceTypes.BinaryFile, data: valueFile as BinaryFile })
      const name = value instanceof File ? value.name : (valueFile as BinaryFile).path
      args.push(name)
    }))
  }

  const pipelinePath = 'write-parametric-map'

  let workerToUse = options?.webWorker
  if (workerToUse === undefined) {
    workerToUse = await getDefaultWebWorker()
  }
  const {
    webWorker: usedWebWorker,
    returnValue,
    stderr,
    outputs
  } = await runPipeline(pipelinePath, args, desiredOutputs, inputs, { pipelineBaseUrl: getPipelinesBaseUrl(), pipelineWorkerUrl: getPipelineWorkerUrl(), webWorker: workerToUse, noCopy: options?.noCopy })
  if (returnValue !== 0 && stderr !== "") {
    throw new Error(stderr)
  }

  const result = {
    webWorker: usedWebWorker as Worker,
    outputDicomFile: outputs[0]?.data as BinaryFile,
  }
  return result
}

export default writeParametricMap
