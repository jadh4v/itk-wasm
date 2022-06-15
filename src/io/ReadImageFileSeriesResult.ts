import Image from '../core/Image.js'
import WorkerPool from '../core/WorkerPool.js'

interface ReadImageFileSeriesResult {
  image: Image
  webWorkerPool: WorkerPool
  orderedFileNames?: Array<string>
}

export default ReadImageFileSeriesResult
