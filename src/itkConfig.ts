const version = '1.0.0-b.15'

const itkConfig = {
  webWorkersUrl: undefined,
  // pipelineWorkerUrl: `file://wsl$/Ubuntu/home/jadhav/code/itk-wasm/dist/web-workers/min-bundles/pipeline.worker.js`,
  // pipelineWorkerUrl: `../../web-workers/pipeline.worker.js`,

  /*
  pipelineWorkerUrl: `itk-wasm/dist/web-workers/min-bundles/pipeline.worker.js`,
  imageIOUrl: `itk-wasm/dist/image-io`,
  meshIOUrl: `itk-wasm/dist/mesh-io`,
  pipelinesUrl: `itk-wasm/dist/pipeline`,
  */

  pipelineWorkerUrl: 'http://localhost:8000/web-workers/bundles/pipeline.worker.js',
  imageIOUrl: 'http://localhost:8000/image-io',
  meshIOUrl: 'http://localhost:8000/mesh-io',
  pipelinesUrl: 'http://localhost:8000/pipeline'

  /*
  pipelineWorkerUrl: `https://cdn.jsdelivr.net/npm/itk-wasm@${version}/dist/web-workers/min-bundles/pipeline.worker.js`,
  imageIOUrl: `https://cdn.jsdelivr.net/npm/itk-image-io@${version}`,
  meshIOUrl: `https://cdn.jsdelivr.net/npm/itk-mesh-io@${version}`,
  pipelinesUrl: `https://cdn.jsdelivr.net/npm/itk-wasm@${version}/dist/pipeline`
  */
}

export default itkConfig
