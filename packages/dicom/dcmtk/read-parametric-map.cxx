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

// CLP includes
//#include "paramap2itkimageCLP.h"

// DCMQI includes
// #undef HAVE_SSTREAM // Avoid redefinition warning
#include "dcmqi/ParaMapConverter.h"
#include "dcmqi/internal/VersionConfigure.h"

// DCMTK includes
//#include "dcmtk/oflog/configrt.h"

// ITK includes
//#include "itkComposeImageFilter.h"
//#include "itkVectorImage.h"

// ITK-wasm includes
#include "itkPipeline.h"
#include "itkOutputImage.h"
#include "itkOutputTextStream.h"

typedef dcmqi::Helper helper;

int runPipeline(
  const std::string & dicomFile,
  itk::wasm::OutputImage<FloatImageType>& outputImage,
  itk::wasm::OutputTextStream& outputMetaInfoJSON)
{
#if !defined(NDEBUG) || defined(_DEBUG)
  // Display DCMTK debug, warning, and error logs in the console
  dcmtk::log4cplus::BasicConfigurator::doConfigure();
#endif

  if(helper::isUndefinedOrPathDoesNotExist(dicomFile, "Input DICOM file"))
  {
    std::cerr << "ERROR: " << dicomFile.c_str() << " is undefined or path does not exist." << std::endl;
    return EXIT_FAILURE;
  }

  DcmFileFormat sliceFF;
  std::cout << "Opening input file " << dicomFile.c_str() << std::endl;
  CHECK_COND(sliceFF.loadFile(dicomFile.c_str()));
  DcmDataset* dataset = sliceFF.getDataset();

  try
  {
    std::pair <FloatImageType::Pointer, string> result =  dcmqi::ParaMapConverter::paramap2itkimage(dataset);
    outputImage.Set(result.first);
    outputMetaInfoJSON.Get() << result.second;
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
  itk::wasm::Pipeline pipeline(
    "read-parametric-map",
    "Read and convert DICOM Parametric Map into ITK image, and generate a JSON file holding meta information.",
    argc,
    argv
  );

  std::string dicomFile;
  pipeline.add_option("dicom-file", dicomFile, "File name of the DICOM Parametric map image.")->required()->check(CLI::ExistingFile)->type_name("INPUT_BINARY_FILE");

  itk::wasm::OutputImage<FloatImageType> outputImage;
  pipeline.add_option("param-image", outputImage, "Parametric map as an output ITK image")->required()->type_name("OUTPUT_IMAGE");

  itk::wasm::OutputTextStream outputMetaInfoJSON;
  pipeline.add_option("meta-info", outputMetaInfoJSON, "Output meta information as JSON")->type_name("OUTPUT_JSON");

  ITK_WASM_PARSE(pipeline);

  // Pipeline code goes here
  runPipeline(dicomFile, outputImage, outputMetaInfoJSON);

  return EXIT_SUCCESS;
}
  
