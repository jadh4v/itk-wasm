/*=========================================================================

 *  Copyright NumFOCUS
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         https://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/

// DCMQI includes
#undef HAVE_SSTREAM // Avoid redefinition warning
#include "dcmqi/ParaMapConverter.h"
#include "dcmqi/internal/VersionConfigure.h"

// DCMTK includes
#include "dcmtk/oflog/configrt.h"

#include "itkPipeline.h"
#include "itkInputImage.h"
#include "itkOutputBinaryStream.h"
#include "itkSupportInputImageTypes.h"
#include "itkSmartPointer.h"
#include "itkInputTextStream.h"

typedef dcmqi::Helper helper;
constexpr unsigned int Dimension = 3;
using PixelType = float;
using FloatImageType = itk::Image<PixelType, Dimension>;

int runPipeline(
  const itk::wasm::InputImage<FloatImageType>& inputImage,
  itk::wasm::InputTextStream& metaInfoStream,
  const std::vector<std::string> & dicomImageFiles,
  const std::string & outputDicomFile)
{
  #if !defined(NDEBUG) || defined(_DEBUG)
    // Display DCMTK debug, warning, and error logs in the console
    // For some reason, this code has no effect if it is called too early (e.g., directly after PARSE_ARGS)
    // therefore we call it here.
    dcmtk::log4cplus::BasicConfigurator::doConfigure();
  #endif

  //FloatImageType::ConstPointer parametricMapImage(inputImage.Get());
  FloatImageType::Pointer parametricMapImage(const_cast<FloatImageType*>(inputImage.Get()));

  const std::string metaInfo((std::istreambuf_iterator<char>(metaInfoStream.Get())),
                                      std::istreambuf_iterator<char>());

  // Pipeline code goes here
  if(dicomImageFiles.empty() || !helper::pathsExist(dicomImageFiles))
  {
    std::cerr << "Error: No reference DICOM files specified!" << std::endl;
    return EXIT_FAILURE;
  }

  std::vector<DcmDataset*> dcmDatasets = helper::loadDatasets(dicomImageFiles);

  if(dcmDatasets.empty()){
    std::cerr << "Error: no DICOM could be loaded from the specified list/directory" << std::endl;
    return EXIT_FAILURE;
  }

  if(helper::isUndefined(outputDicomFile, "Output DICOM file")) {
    std::cerr << "Output DICOM filename not provided." << std::endl;
    return EXIT_FAILURE;
  }

/*
  Json::Value metaRoot;
  istringstream metainfoisstream(metaInfo);
  metainfoisstream >> metaRoot;
*/
  try
  {
    DcmDataset* result = dcmqi::ParaMapConverter::itkimage2paramap(parametricMapImage, dcmDatasets, metaInfo);

    if (result == NULL)
    {
      std::cerr << "ERROR: Conversion failed." << std::endl;
      return EXIT_FAILURE;
    }
    else
    {
      DcmFileFormat segdocFF(result);
      CHECK_COND(segdocFF.saveFile(outputDicomFile.c_str(), EXS_LittleEndianExplicit));

      std::cout << "Saved parametric map as " << outputDicomFile << std::endl;
      return EXIT_SUCCESS;
    }
  }
  catch (const std::exception& e)
  {
    std::cerr << "Fatal error encountered: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

int main(int argc, char * argv[])
{
  itk::wasm::Pipeline pipeline("write-parametric-map", "Convert ITK image to DICOM Parametric Map.", argc, argv);

  itk::wasm::InputImage<FloatImageType> inputImage;
  pipeline.add_option("paramap-image", inputImage, "parametric map as an ITK image")->required()->type_name("INPUT_IMAGE");

  itk::wasm::InputTextStream metaInfo;
  pipeline.add_option("meta-info", metaInfo, "JSON containing metadata attributes.")->required()->type_name("INPUT_JSON");

  std::string outputDicomFile;
  pipeline.add_option("output-dicom-file", outputDicomFile, "File name of the DICOM Parametric" \
    "map object with the result of the conversion.")->required()->type_name("OUTPUT_BINARY_FILE");

  std::vector<std::string> refDicomSeriesFiles;
  pipeline.add_option("-r,--ref-dicom-series", refDicomSeriesFiles, " Source DICOM images that were used to generate the parametric map.")
    ->required()->check(CLI::ExistingFile)->expected(1,-1)->type_name("INPUT_BINARY_FILE");

  ITK_WASM_PARSE(pipeline);

  return runPipeline(
    inputImage,
    metaInfo,
    refDicomSeriesFiles,
    outputDicomFile
  );
}
