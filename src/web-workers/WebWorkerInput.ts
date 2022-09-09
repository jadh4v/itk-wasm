import ITKConfig from './ITKConfig.js'

interface WebWorkerInput {
  operation: 'runPipeline' | 'runPolyDataIOPipeline' | 'readImage' | 'writeImage' | 'readMesh' | 'writeMesh' | 'meshToPolyData' |  'polyDataToMesh' | 'readDICOMImageSeries' | 'readDICOMTags' | 'structuredReportToText'
  config: ITKConfig
}

export default WebWorkerInput
