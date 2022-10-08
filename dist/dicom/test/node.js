import fs from 'fs'
import test from 'ava'
import { structuredReportToTextNode } from '../dist/itk-dicom.node.js'
import { structuredReportToHtmlNode } from '../dist/itk-dicom.node.js'
import { readDicomEncapsulatedPdfNode } from '../dist/itk-dicom.node.js'
import { applyPstateToImageNode } from '../dist/itk-dicom.node.js'
import { apply } from 'async'

test('structuredReportToText', async t => {

  const fileName = '88.33-comprehensive-SR.dcm'
  const testFilePath = `../../build-emscripten/ExternalData/test/Input/${fileName}`

  const dicomFileBuffer = fs.readFileSync(testFilePath)
  const dicomFile = new Uint8Array(dicomFileBuffer)

  const { outputText } = await structuredReportToTextNode(dicomFile)

  t.assert(outputText.includes('Comprehensive SR Document'))

  const { outputText: outputTextNoHeader } = await structuredReportToTextNode(dicomFile, { noDocumentHeader: true })
  t.assert(!outputTextNoHeader.includes('Comprehensive SR Document'))
  t.assert(outputTextNoHeader.includes('Breast Imaging Report'))
})

test('structuredReportToHtml', async t => {

  const fileName = '88.33-comprehensive-SR.dcm'
  const testFilePath = `../../build-emscripten/ExternalData/test/Input/${fileName}`

  const dicomFileBuffer = fs.readFileSync(testFilePath)
  const dicomFile = new Uint8Array(dicomFileBuffer)

  const { outputText } = await structuredReportToHtmlNode(dicomFile)

  t.assert(outputText.includes('Comprehensive SR Document'))
  t.assert(outputText.includes('Breast Diagnosis 010001 (female, #BreastDx-01-0001)'))
  t.assert(outputText.includes('PixelMed (XSLT from di3data csv extract)'))

  const { outputText: outputTextNoHeader } = await structuredReportToHtmlNode(dicomFile, { noDocumentHeader: true })

  t.assert(!outputTextNoHeader.includes('Breast Diagnosis 010001 (female, #BreastDx-01-0001)'))
  t.assert(!outputTextNoHeader.includes('PixelMed (XSLT from di3data csv extract)'))

  const { outputText: outputTextRenderAllCodes } = await structuredReportToHtmlNode(dicomFile, { renderAllCodes: true })

  t.assert(outputTextRenderAllCodes.includes('Overall Assessment (111413, DCM)'))

})

test('read Radiation Dose SR', async t => {

  const fileName = '88.67-radiation-dose-SR.dcm'
  const testFilePath = `../../build-emscripten/ExternalData/test/Input/${fileName}`

  const dicomFileBuffer = fs.readFileSync(testFilePath)
  const dicomFile = new Uint8Array(dicomFileBuffer)

  const { outputText } = await structuredReportToHtmlNode(dicomFile)

  t.assert(outputText.includes('<title>X-Ray Radiation Dose SR Document</title>'))
  t.assert(outputText.includes('<h2>CT Accumulated Dose Data</h2>'))
})

test('readDicomEncapsulatedPdfNode', async t => {

  const fileName = '104.1-SR-printed-to-pdf.dcm'
  const testFilePath = `../../build-emscripten/ExternalData/test/Input/${fileName}`
  const dicomFileBuffer = fs.readFileSync(testFilePath)
  const dicomFile = new Uint8Array(dicomFileBuffer)
  const { pdfBinaryOutput: outputBinaryStream } = await readDicomEncapsulatedPdfNode(dicomFile)
  t.assert(outputBinaryStream != null)
  t.assert(outputBinaryStream.length === 91731)
})

test('read Key Object Selection SR', async t => {

  const fileName = '88.59-KeyObjectSelection-SR.dcm'
  const testFilePath = `../../build-emscripten/ExternalData/test/Input/${fileName}`
  const dicomFileBuffer = fs.readFileSync(testFilePath)
  const dicomFile = new Uint8Array(dicomFileBuffer)

  const { outputText } = await structuredReportToHtmlNode(
    dicomFile, {
      urlPrefix: 'http://my-custom-dicom-server/dicom.cgi',
      cssReference: "https://css-host/dir/subdir/my-first-style.css",
    }
  )

  t.assert(outputText.includes('http://my-custom-dicom-server/dicom.cgi'))
  t.assert(!outputText.includes('http://localhost/dicom.cgi'))
  t.assert(outputText.includes(`<link rel="stylesheet" type="text/css" href="https://css-host/dir/subdir/my-first-style.css">`))

  const cssfileName = 'test-style.css'
  const testCssFilePath = `../../build-emscripten/ExternalData/test/Input/${cssfileName}`
  const cssFileBuffer = fs.readFileSync(testCssFilePath)

  const { outputText: outputWithCSSFile } = await structuredReportToHtmlNode(
    dicomFile, { cssFile: cssFileBuffer })

  t.assert(outputWithCSSFile.includes('<style type="text/css">'))
  t.assert(outputWithCSSFile.includes('background-color: lightblue;'))
  t.assert(outputWithCSSFile.includes('margin-left: 20px;'))
  t.assert(outputWithCSSFile.includes('</style>'))
  t.assert(!outputWithCSSFile.includes('http://my-custom-dicom-server/dicom.cgi'))
  t.assert(outputWithCSSFile.includes('http://localhost/dicom.cgi'))
})

test('Apply presentation state to dicom image.', async t => {

  // Read the input image file
  const inputFile = 'gsps-pstate-test-input-image.dcm'
  const inputFilePath = `../../build-emscripten/ExternalData/test/Input/${inputFile}`
  const dicomFileBuffer = fs.readFileSync(inputFilePath)
  const inputImage = new Uint8Array(dicomFileBuffer)

  // Read the presentation state file (that references the above image internally using its SOPInstanceUID).
  const pstateFile = 'gsps-pstate-test-input-pstate2.dcm'
  const pstateFilePath = `../../build-emscripten/ExternalData/test/Input/${pstateFile}`
  const pstateFileBuffer = fs.readFileSync(pstateFilePath)
  const inputPState = new Uint8Array(pstateFileBuffer)

  const { pstateOutStream, bitmapOutStream } = await applyPstateToImageNode(inputImage, {pstateFile: inputPState})

  t.assert(pstateOutStream != null)
  t.assert(bitmapOutStream != null)

  const baselineJson = 'gsps-pstate2-baseline.json'
  const baselineJsonFilePath = `../../build-emscripten/ExternalData/test/Input/${baselineJson}`
  const baselineJsonFileBuffer = fs.readFileSync(baselineJsonFilePath)
  // the slice operation removes the last EOF char from the baseline file.
  const baselineJsonString = baselineJsonFileBuffer.toString().slice(0, -1)
  t.assert(baselineJsonString === pstateOutStream)
  t.assert(baselineJsonString.length === pstateOutStream.length)

  const baselineImage = 'gsps-pstate2-image-baseline.pgm'
  const baselineImageFilePath = `../../build-emscripten/ExternalData/test/Input/${baselineImage}`
  const baselineImageFileBuffer = fs.readFileSync(baselineImageFilePath)
  t.assert(baselineImageFileBuffer.length === bitmapOutStream.length)
  t.assert(Buffer.compare(baselineImageFileBuffer, bitmapOutStream) === 0)

})
