// Generated file. To retain edits, remove this comment.

import {
  BinaryFile,
  Image,
  JsonCompatible,
  InterfaceTypes,
  PipelineOutput,
  PipelineInput,
  runPipeline
} from 'itk-wasm'

import ReadParametricMapOptions from './read-parametric-map-options.js'
import ReadParametricMapResult from './read-parametric-map-result.js'

import { getPipelinesBaseUrl } from './pipelines-base-url.js'
import { getPipelineWorkerUrl } from './pipeline-worker-url.js'

import { getDefaultWebWorker } from './default-web-worker.js'

/**
 * Read and convert DICOM Parametric Map into ITK image, and generate a JSON file holding meta information.
 *
 * @param {File | BinaryFile} dicomFile - File name of the DICOM Parametric map image.
 * @param {ReadParametricMapOptions} options - options object
 *
 * @returns {Promise<ReadParametricMapResult>} - result object
 */
async function readParametricMap(
  dicomFile: File | BinaryFile,
  options: ReadParametricMapOptions = {}
) : Promise<ReadParametricMapResult> {

  const desiredOutputs: Array<PipelineOutput> = [
    { type: InterfaceTypes.Image },
    { type: InterfaceTypes.JsonCompatible },
  ]

  let dicomFileFile = dicomFile
  if (dicomFile instanceof File) {
    const dicomFileBuffer = await dicomFile.arrayBuffer()
    dicomFileFile = { path: dicomFile.name, data: new Uint8Array(dicomFileBuffer) }
  }
  const inputs: Array<PipelineInput> = [
    { type: InterfaceTypes.BinaryFile, data: dicomFileFile as BinaryFile },
  ]

  const args = []
  // Inputs
  const dicomFileName = (dicomFileFile as BinaryFile).path
  args.push(dicomFileName)

  // Outputs
  const paramImageName = '0'
  args.push(paramImageName)

  const metaInfoName = '1'
  args.push(metaInfoName)

  // Options
  args.push('--memory-io')

  const pipelinePath = 'read-parametric-map'

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
    paramImage: outputs[0]?.data as Image,
    metaInfo: outputs[1]?.data as JsonCompatible,
  }
  return result
}

export default readParametricMap
