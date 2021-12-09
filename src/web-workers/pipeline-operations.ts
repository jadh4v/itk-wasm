import registerWebworker from 'webworker-promise/lib/register.js'

import loadEmscriptenModule from '../core/internal/loadEmscriptenModuleWebWorker.js'
import runPipelineEmscripten from '../pipeline/internal/runPipelineEmscripten.js'
import IOTypes from '../core/IOTypes.js'
import getTransferable from '../core/getTransferable.js'

import PipelineEmscriptenModule from '../pipeline/PipelineEmscriptenModule.js'
import PipelineInput from '../pipeline/PipelineInput.js'
import PipelineOutput from '../pipeline/PipelineOutput.js'

import InterfaceTypes from '../core/InterfaceTypes.js'
import TextStream from '../core/TextStream.js'
import BinaryStream from '../core/BinaryStream.js'
import TextFile from '../core/TextFile.js'
import BinaryFile from '../core/BinaryFile.js'
import Image from '../core/Image.js'
import Mesh from '../core/Mesh.js'
import PolyData from '../core/vtkPolyData.js'
import TypedArray from '../core/TypedArray.js'

interface ITKConfig {
  pipelinesUrl: string
  polydataIOUrl: string
}

interface Input {
  operation: 'runPipeline' | 'runPolyDataIOPipeline'
  config: ITKConfig
}

export interface RunPipelineInput extends Input {
  pipelinePath: string | object
  args: string[]
  outputs: PipelineOutput[]
  inputs: PipelineInput[]
}

// To cache loaded pipeline modules
const pipelineToModule: Map<string,PipelineEmscriptenModule> = new Map()

export async function loadPipelineModule (pipelinePath: string | object, baseUrl: string) {
  let moduleRelativePathOrURL: string | URL = pipelinePath as string
  let pipeline = pipelinePath as string
  let pipelineModule = null
  if (typeof pipelinePath !== 'string') {
    moduleRelativePathOrURL = new URL((pipelinePath as URL).href)
    pipeline = moduleRelativePathOrURL.href
  }
  if (pipelineToModule.has(pipeline)) {
    pipelineModule = pipelineToModule.get(pipeline) as PipelineEmscriptenModule
  } else {
    pipelineToModule.set(pipeline, await loadEmscriptenModule(moduleRelativePathOrURL, baseUrl) as PipelineEmscriptenModule)
    pipelineModule = pipelineToModule.get(pipeline) as PipelineEmscriptenModule
  }
  return pipelineModule
}

export async function runPipeline(pipelineModule: PipelineEmscriptenModule, args: string[], outputs: PipelineOutput[], inputs: PipelineInput[]) {
  const result = runPipelineEmscripten(pipelineModule, args, outputs, inputs)

  const transferables: ArrayBuffer[] = []
  if (result.outputs) {
    result.outputs.forEach(function (output) {
      if (output.type === InterfaceTypes.BinaryStream || output.type === InterfaceTypes.BinaryFile) {
        // Binary data
        const binary = output.data as Uint8Array
        const transferable = getTransferable(binary)
        if (transferable) {
          transferables.push(transferable)
        }
      } else if (output.type === IOTypes.Binary) {
        // Binary data
        const binary = output.data as Uint8Array
        const transferable = getTransferable(binary)
        if (transferable) {
          transferables.push(transferable)
        }
      } else if (output.type === IOTypes.Image) {
        // Image data
        const image = output.data as Image
        let transferable = getTransferable(image.data)
        if (transferable) {
          transferables.push(transferable)
        }
        transferable = getTransferable(image.direction)
        if (transferable) {
          transferables.push(transferable)
        }
      } else if (output.type === IOTypes.Mesh) {
        // Mesh data
        const mesh = output.data as Mesh
        if (mesh.points) {
          const transferable = getTransferable(mesh.points)
          if (transferable) {
            transferables.push(transferable)
          }
        }
        if (mesh.pointData) {
          const transferable = getTransferable(mesh.pointData)
          if (transferable) {
            transferables.push(transferable)
          }
        }
        if (mesh.cells) {
          const transferable = getTransferable(mesh.cells)
          if (transferable) {
            transferables.push(transferable)
          }
        }
        if (mesh.cellData) {
          const transferable = getTransferable(mesh.cellData)
          if (transferable) {
            transferables.push(transferable)
          }
        }
      } else if (output.type === IOTypes.vtkPolyData) {
        // vtkPolyData data
        const polyData = output.data as PolyData
        const cellTypes = ['points', 'verts', 'lines', 'polys', 'strips']
        cellTypes.forEach((cellName) => {
          // @ts-ignore: error TS7053: Element implicitly has an 'any' type
          // because expression of type 'string' can't be used to index type
          // 'vtkPolyData'.
          if (polyData[cellName]) {
            // @ts-ignore: error TS7053: Element implicitly has an 'any' type
            // because expression of type 'string' can't be used to index type
            // 'vtkPolyData'.
            const transferable = getTransferable(polyData[cellName])
            if (transferable) {
              transferables.push(transferable)
            }
          }
        })

        const dataSetType = ['pointData', 'cellData', 'fieldData']
        dataSetType.forEach((dataName) => {
          // @ts-ignore: error TS7053: Element implicitly has an 'any' type
          // because expression of type 'string' can't be used to index type
          // 'vtkPolyData'.
          if (polyData[dataName]) {
            // @ts-ignore: error TS7053: Element implicitly has an 'any' type
            // because expression of type 'string' can't be used to index type
            // 'vtkPolyData'.
            const data = polyData[dataName]
            data.arrays.forEach((array: { data: TypedArray }) => {
              const transferable = getTransferable(array.data)
              if (transferable) {
                transferables.push(transferable)
              }
            })
          }
        })
      }
    })
  }

  return new registerWebworker.TransferableResponse(result, transferables)
}
