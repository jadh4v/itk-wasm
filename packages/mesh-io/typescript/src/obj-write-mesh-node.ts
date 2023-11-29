// Generated file. To retain edits, remove this comment.

import {
  Mesh,
  JsonCompatible,
  InterfaceTypes,
  PipelineOutput,
  PipelineInput,
  runPipelineNode
} from 'itk-wasm'

import ObjWriteMeshOptions from './obj-write-mesh-options.js'
import ObjWriteMeshNodeResult from './obj-write-mesh-node-result.js'

import path from 'path'

/**
 * Write an itk-wasm file format converted to an mesh file format
 *
 * @param {Mesh} mesh - Input mesh
 * @param {string} serializedMesh - Output mesh
 * @param {ObjWriteMeshOptions} options - options object
 *
 * @returns {Promise<ObjWriteMeshNodeResult>} - result object
 */
async function objWriteMeshNode(
  mesh: Mesh,
  serializedMesh: string,
  options: ObjWriteMeshOptions = {}
) : Promise<ObjWriteMeshNodeResult> {

  const mountDirs: Set<string> = new Set()

  const desiredOutputs: Array<PipelineOutput> = [
    { type: InterfaceTypes.JsonCompatible },
  ]

  const inputs: Array<PipelineInput> = [
    { type: InterfaceTypes.Mesh, data: mesh },
  ]

  const args = []
  // Inputs
  const meshName = '0'
  args.push(meshName)

  // Outputs
  const couldWriteName = '0'
  args.push(couldWriteName)

  const serializedMeshName = serializedMesh
  args.push(serializedMeshName)
  mountDirs.add(path.dirname(serializedMeshName))

  // Options
  args.push('--memory-io')
  if (typeof options.informationOnly !== "undefined") {
    options.informationOnly && args.push('--information-only')
  }
  if (typeof options.useCompression !== "undefined") {
    options.useCompression && args.push('--use-compression')
  }
  if (typeof options.binaryFileType !== "undefined") {
    options.binaryFileType && args.push('--binary-file-type')
  }

  const pipelinePath = path.join(path.dirname(import.meta.url.substring(7)), 'pipelines', 'obj-write-mesh')

  const {
    returnValue,
    stderr,
    outputs
  } = await runPipelineNode(pipelinePath, args, desiredOutputs, inputs, mountDirs)
  if (returnValue !== 0 && stderr !== "") {
    throw new Error(stderr)
  }

  const result = {
    couldWrite: outputs[0]?.data as JsonCompatible,
  }
  return result
}

export default objWriteMeshNode
