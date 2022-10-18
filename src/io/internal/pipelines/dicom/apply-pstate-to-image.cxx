/*=========================================================================
 *
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
/*
 *
 *  Copyright (C) 1998-2018, OFFIS e.V.
 *  All rights reserved.  See COPYRIGHT file for details.
 *
 *  This software and supporting documentation were developed by
 *
 *    OFFIS e.V.
 *    R&D Division Health
 *    Escherweg 2
 *    D-26121 Oldenburg, Germany
 *
 *
 *  Module:  dcmpstat
 *
 *  Authors: Joerg Riesmeier, Marco Eichelberg
 *
 *  Purpose
 *    sample application that reads a DICOM image and (optionally)
 *    a presentation state and creates a PGM bitmap using the settings
 *    of the presentation state. Non-grayscale transformations are
 *    ignored. If no presentation state is loaded, a default is created.
 *
 */

#include "itkPipeline.h"
#include "itkOutputTextStream.h"
#include "itkOutputBinaryStream.h"
#include "itkOutputImage.h"

#include "dcmtk/config/osconfig.h"    /* make sure OS specific configuration is included first */

#include "dcmtk/ofstd/ofstream.h"
// Fix warning for redefinition of __STDC_FORMAT_MACROS in the header include tree for dviface.h
#ifdef __STDC_FORMAT_MACROS
  #undef __STDC_FORMAT_MACROS
#endif
#include "dcmtk/dcmpstat/dviface.h"
#include "dcmtk/dcmpstat/dvpstx.h"    /* for DVPSTextObject */
#include "dcmtk/dcmpstat/dvpsgr.h"    /* for DVPSGraphicObject */
#include "dcmtk/dcmpstat/dvpscu.h"    /* for DVPSCurve */
#include "dcmtk/dcmimgle/dcmimage.h"
#include "dcmtk/dcmdata/cmdlnarg.h"
#include "dcmtk/ofstd/ofcmdln.h"
#include "dcmtk/ofstd/ofconapp.h"
#include "dcmtk/dcmdata/dcuid.h"      /* for dcmtk version name */
#include "dcmtk/dcmdata/dcjson.h"     /* JSON format */

#include "cpp-base64/base64.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "rapidjson/prettywriter.h"
using namespace rapidjson;

#include <cstring>
#include <array>

#ifdef WITH_ZLIB
#include "itk_zlib.h"     /* for zlibVersion() */
#endif

#define OFFIS_CONSOLE_APPLICATION "dcmp2pgm"

static OFLogger dcmp2pgmLogger = OFLog::getLogger("dcmtk.apps." OFFIS_CONSOLE_APPLICATION);

static char rcsid[] = "$dcmtk: " OFFIS_CONSOLE_APPLICATION " v"
  OFFIS_DCMTK_VERSION " " OFFIS_DCMTK_RELEASEDATE " $";

template<typename Iteratorable>
rapidjson::Value getArrayJson(Iteratorable container, rapidjson::Document::AllocatorType& allocator)
{
  rapidjson::Value value(kArrayType);
  for(auto iter = container.begin(); iter != container.end(); ++iter)
  {
    value.PushBack(rapidjson::Value(*iter), allocator);
  }
  return value;
}

static void dumpPresentationState(STD_NAMESPACE ostream &out, DVPresentationState &ps)
{
  size_t i, j, max;

  // START JSON structure
  // Using short names such as 'd' and 'a' for Document and Allocator deliberately,
  // to minimize verbosity of subsequent lines of code.
  Document d(kObjectType);
  Document::AllocatorType& a = d.GetAllocator();

  d.AddMember("PresentationLabel",
    Value(StringRef(ps.getPresentationLabel())), a);
  d.AddMember("PresentationDescription",
    Value(StringRef(ps.getPresentationDescription())), a);
  d.AddMember("PresentationCreatorsName",
    Value(StringRef(ps.getPresentationCreatorsName())), a);

  if (ps.haveActiveVOIWindow())
  {
    double width=0.0, center=0.0;
    ps.getCurrentWindowWidth(width);
    ps.getCurrentWindowCenter(center);
    d.AddMember("CurrentWindowCenter", Value(center), a);
    d.AddMember("CurrentWindowWidth", Value(width), a);
    d.AddMember("CurrentVOIDescription", Value(StringRef(ps.getCurrentVOIDescription())), a);
  }
  else if (ps.haveActiveVOILUT())
  {
    d.AddMember("CurrentVOIDescription", Value(StringRef(ps.getCurrentVOIDescription())), a);
  }

  d.AddMember("Flip", Value(ps.getFlip()), a);
  int rotation = 0;
  switch (ps.getRotation())
  {
    case DVPSR_0_deg:
      rotation = 0;
      break;
    case DVPSR_90_deg:
      rotation = 90;
      break;
    case DVPSR_180_deg:
      rotation = 180;
      break;
    case DVPSR_270_deg:
      rotation = 270;
      break;
  }
  d.AddMember("Rotation", Value(rotation), a);

  DVPSPresentationSizeMode sizemode = ps.getDisplayedAreaPresentationSizeMode();
  double factor=1.0;
  std::string presentationSizeMode;
  switch (sizemode)
  {
    case DVPSD_scaleToFit:
      presentationSizeMode = "scaleToFit";
      break;
    case DVPSD_trueSize:
      presentationSizeMode = "trueSize";
      break;
    case DVPSD_magnify:
      presentationSizeMode = "magnify";
      ps.getDisplayedAreaPresentationPixelMagnificationRatio(factor);
      break;
  }
  d.AddMember("PresentationSizeMode", Value(presentationSizeMode.c_str(), a), a);
  d.AddMember("DisplayedAreaPresentationPixelMagnificationRatio", Value(factor), a);

  std::array<int, 4> displayArea{0, 0, 0, 0};
  ps.getStandardDisplayedArea(displayArea[0], displayArea[1], displayArea[2], displayArea[3]);
  auto dav = getArrayJson(displayArea, a);
  d.AddMember("StandardDisplayedArea", dav, a);

  std::array<double, 2> pixelSpacing{0.0, 0.0};
  if (EC_Normal == ps.getDisplayedAreaPresentationPixelSpacing(pixelSpacing[0], pixelSpacing[1]))
  {
    auto psv = getArrayJson(pixelSpacing, a);
    d.AddMember("DisplayedAreaPresentationPixelSpacing", psv, a);
  } else {
    auto aspectRatio = ps.getDisplayedAreaPresentationPixelAspectRatio();
    d.AddMember("DisplayedAreaPresentationPixelAspectRatio", Value(aspectRatio), a);
  }

  if (ps.haveShutter(DVPSU_rectangular))
  {
    Value rectShutter(kObjectType);
    rectShutter.AddMember("RectShutterLV", Value(ps.getRectShutterLV()), a);
    rectShutter.AddMember("RectShutterRV", Value(ps.getRectShutterRV()), a);
    rectShutter.AddMember("RectShutterUH", Value(ps.getRectShutterUH()), a);
    rectShutter.AddMember("RectShutterLH", Value(ps.getRectShutterLH()), a);
    d.AddMember("RectangularShutter", rectShutter, a);
  }

  if (ps.haveShutter(DVPSU_circular))
  {
    Value circularShutter(kObjectType);
    circularShutter.AddMember("CenterOfCircularShutter_x", Value(ps.getCenterOfCircularShutter_x()), a);
    circularShutter.AddMember("CenterOfCircularShutter_y", Value(ps.getCenterOfCircularShutter_y()), a);
    circularShutter.AddMember("RadiusOfCircularShutter", Value(ps.getRadiusOfCircularShutter()), a);
    d.AddMember("CircularShutter", circularShutter, a);
  }

  if (ps.haveShutter(DVPSU_polygonal))
  {
    j = ps.getNumberOfPolyShutterVertices();
    Sint32 polyX, polyY;
    std::vector<Sint32> vertices;
    vertices.reserve(j*2);
    for (i=0; i<j; i++)
    {
      if (EC_Normal == ps.getPolyShutterVertex(i, polyX, polyY))
      {
        vertices.push_back(polyX);
        vertices.push_back(polyY);
      }
    }
    auto coordinates = getArrayJson(vertices, a);
    Value polyShutter(kObjectType);

    // rapidjson::Value doesn't have a default constructor for size_t (aka unsigned long).
    // However, we can safely type-cast this value into a 32-bit unsigned integer (uint32_t)
    // based on DICOM standards value representation "Integer String" (IS).
    polyShutter.AddMember("NumberOfPolyShutterVertices", Value(static_cast<uint32_t>(ps.getNumberOfPolyShutterVertices())), a);

    polyShutter.AddMember("Coordinates", coordinates, a);
    d.AddMember("PolygonalShutter", polyShutter, a);
  }

  // TODO: add support for Bitmap shutter (bitmap masking).
  // if (ps.haveShutter(DVPSU_bitmap))
  // {
  //   JSON_OPEN_OBJ("BitmapShutter");
  // }
  d.AddMember("ShutterPresentationValue", Value(ps.getShutterPresentationValue()), a);

  ps.sortGraphicLayers();  // to order of display
  Value graphicsLayersJsonArray(kArrayType);
  for (size_t layer=0; layer<ps.getNumberOfGraphicLayers(); layer++)
  {
    Value layerJson(kObjectType);
    layerJson.AddMember("Name", Value(StringRef(ps.getGraphicLayerName(layer))), a);
    layerJson.AddMember("Description", Value(StringRef(ps.getGraphicLayerDescription(layer))), a);
    if (ps.haveGraphicLayerRecommendedDisplayValue(layer))
    {
      Uint16 r, g, b;
      if (EC_Normal == ps.getGraphicLayerRecommendedDisplayValueGray(layer, g))
      {
        layerJson.AddMember("RecommendedDisplayValueGray", Value(g), a);
      }

      if (EC_Normal == ps.getGraphicLayerRecommendedDisplayValueRGB(layer, r, g, b))
      {
        const std::array<Uint16, 3> rgb{r, g, b};
        layerJson.AddMember("RecommendedDisplayValueRGB", getArrayJson(rgb, a), a);
      }
    }

    // text objects
    max = ps.getNumberOfTextObjects(layer);
    DVPSTextObject *ptext = NULL;

    // TextObjects[]
    Value textObjectsJsonArray(kArrayType);

    for (size_t textidx=0; textidx<max; textidx++)
    {
      ptext = ps.getTextObject(layer, textidx);
      if (ptext)
      {
        Value textObjectJson(kObjectType);
        // display contents of text object
        textObjectJson.AddMember("Text", Value(StringRef(ptext->getText())), a);
        if (ptext->haveAnchorPoint())
        {
          const std::array<double, 2> anchorPoint{ptext->getAnchorPoint_x(), ptext->getAnchorPoint_y()};
          auto apv = getArrayJson(anchorPoint, a);
          textObjectJson.AddMember("AnchorPoint", apv, a);
          textObjectJson.AddMember("AnchorPointUnits", Value(StringRef((ptext->getAnchorPointAnnotationUnits()==DVPSA_display? "display" : "pixel"))), a);
          textObjectJson.AddMember("AnchorPointVisible", Value(ptext->anchorPointIsVisible()), a);
        }

        if (ptext->haveBoundingBox())
        {
          const std::array<double, 4> box{ptext->getBoundingBoxTLHC_x(), ptext->getBoundingBoxTLHC_y(), ptext->getBoundingBoxBRHC_x(), ptext->getBoundingBoxBRHC_y()};
          auto bv = getArrayJson(box, a);
          textObjectJson.AddMember("BoundingBox", getArrayJson(box, a), a);
          textObjectJson.AddMember("BoundingBoxUnits", Value(StringRef(ptext->getBoundingBoxAnnotationUnits()==DVPSA_display ? "display" : "pixel")), a);

          DVPSTextJustification justification = ptext->getBoundingBoxHorizontalJustification();
          std::string horizontalJustification;
          switch (justification)
          {
            case DVPSX_left:
              horizontalJustification = "left";
              break;
            case DVPSX_right:
              horizontalJustification = "right";
              break;
            case DVPSX_center:
              horizontalJustification = "center";
              break;
          }
          textObjectJson.AddMember("BoundingBoxHorizontalJustification", Value(horizontalJustification.c_str(), a), a);
        }
        textObjectsJsonArray.PushBack(textObjectJson, a);
      }
    }

    layerJson.AddMember("TextObjects", textObjectsJsonArray, a);

    // graphic objects
    max = ps.getNumberOfGraphicObjects(layer);
    DVPSGraphicObject *pgraphic = NULL;

    // GraphicObjects[]
    Value graphicObjectsJsonArray(kArrayType);

    for (size_t graphicidx=0; graphicidx<max; graphicidx++)
    {
      pgraphic = ps.getGraphicObject(layer, graphicidx);
      if (pgraphic)
      {
        Value graphicJson(kObjectType);
        std::string graphicType = "none";
        switch (pgraphic->getGraphicType())
        {
          case DVPST_polyline: graphicType = "polyline"; break;
          case DVPST_interpolated: graphicType = "interpolated"; break;
          case DVPST_circle: graphicType = "circle"; break;
          case DVPST_ellipse: graphicType = "ellipse"; break;
          case DVPST_point: graphicType = "point"; break;
        }
        graphicJson.AddMember("GraphicType", Value(graphicType.c_str(), a), a);
        graphicJson.AddMember("IsFilled", Value(pgraphic->isFilled()), a);
        graphicJson.AddMember("Units", Value(StringRef(pgraphic->getAnnotationUnits()==DVPSA_display? "display" : "pixel")), a);

        j = pgraphic->getNumberOfPoints();
        Float32 fx=0.0, fy=0.0;
        std::vector<float> points;
        for (i=0; i<j; i++)
        {
          if (EC_Normal==pgraphic->getPoint(i,fx,fy))
          {
            points.push_back(fx);
            points.push_back(fy);
          }
        }
        graphicJson.AddMember("Points", getArrayJson(points, a), a);
        graphicObjectsJsonArray.PushBack(graphicJson, a);
      }
    }
    layerJson.AddMember("GraphicObjects", graphicObjectsJsonArray, a); // GraphicObjects[]

    // curve objects
    max = ps.getNumberOfCurves(layer);
    DVPSCurve *pcurve = NULL;

    // Curves[]
    Value curvesJsonArray(kArrayType);

    for (size_t curveidx=0; curveidx<max; curveidx++)
    {
      pcurve = ps.getCurve(layer, curveidx);
      if (pcurve)
      {
        Value curveJson(kObjectType); // curve{}
        std::string type;
        switch (pcurve->getTypeOfData())
        {
          case DVPSL_roiCurve: type = "roiCurve"; break;
          case DVPSL_polylineCurve: type = "polylineCurve"; break;
        }
        curveJson.AddMember("Type", Value(type.c_str(), a), a);
        curveJson.AddMember("AxisUnitsX", Value(pcurve->getCurveAxisUnitsX(), a), a);
        curveJson.AddMember("AxisUnitsY", Value(pcurve->getCurveAxisUnitsY(), a), a);
        curveJson.AddMember("Label", Value(StringRef(pcurve->getCurveLabel())), a);
        curveJson.AddMember("Description", Value(StringRef(pcurve->getCurveDescription())), a);

        j = pcurve->getNumberOfPoints();
        double dx=0.0, dy=0.0;
        std::vector<double> points;
        points.reserve(j*2);
        for (i=0; i<j; i++)
        {
          if (EC_Normal==pcurve->getPoint(i,dx,dy))
          {
            points.push_back(dx);
            points.push_back(dy);
          }
        }
        curveJson.AddMember("Points", getArrayJson(points, a), a);
        curvesJsonArray.PushBack(curveJson, a);
      }
    }

    layerJson.AddMember("Curves", curvesJsonArray, a);

    // overlay objects
    const void *overlayData=NULL;
    unsigned int overlayWidth=0, overlayHeight=0, overlayLeft=0, overlayTop=0;
    OFBool overlayROI=OFFalse;
    Uint16 overlayTransp=0;
    char overlayfile[100];
    FILE *ofile=NULL;

    max = ps.getNumberOfActiveOverlays(layer);

    // Overlays[]
    Value overlaysJsonArray(kArrayType);

    for (size_t ovlidx=0; ovlidx<max; ovlidx++)
    {
      Value overlayJson(kObjectType);
      overlayJson.AddMember("Index", Value(static_cast<uint32_t>(ovlidx)), a);
      std::stringstream value;
      value << "0x" << STD_NAMESPACE hex << ps.getActiveOverlayGroup(layer, ovlidx) << STD_NAMESPACE dec;
      overlayJson.AddMember("Group", Value(value.str().c_str(), a), a);
      overlayJson.AddMember("Label", Value(StringRef(ps.getActiveOverlayLabel(layer, ovlidx))), a);
      overlayJson.AddMember("Description", Value(StringRef(ps.getActiveOverlayDescription(layer, ovlidx))), a);
      overlayJson.AddMember("Type", Value(StringRef(ps.activeOverlayIsROI(layer, ovlidx) ? "ROI" : "graphic")), a);

      /* get overlay data */
      if (EC_Normal == ps.getOverlayData(layer, ovlidx, overlayData, overlayWidth, overlayHeight,
          overlayLeft, overlayTop, overlayROI, overlayTransp))
      {
        overlayJson.AddMember("Width", Value(overlayWidth), a);
        overlayJson.AddMember("Height", Value(overlayHeight), a);
        overlayJson.AddMember("Left", Value(overlayLeft), a);
        overlayJson.AddMember("Top", Value(overlayTop), a);

        std::stringstream buff;
        buff << "P5\n" << overlayWidth << " " << overlayHeight << " " << "255\n";
        buff.write((const char*)overlayData, overlayWidth * overlayHeight);

        constexpr bool urlFriendly = false;
        overlayJson.AddMember("OverlayData", Value(base64_encode(buff.str(), urlFriendly).c_str(), a), a);
      } else {
        OFLOG_ERROR(dcmp2pgmLogger, "unable to access overlay data!");
      }
      overlaysJsonArray.PushBack(overlayJson, a);
    }
    layerJson.AddMember("Overlays", overlaysJsonArray, a); // Overlays[]
    graphicsLayersJsonArray.PushBack(layerJson, a);
  }
  d.AddMember("GraphicsLayers", graphicsLayersJsonArray.Move(), a); // GraphicsLayers[]

  // Pretty print the json into output stream.
  StringBuffer buffer;
  PrettyWriter<StringBuffer> writer(buffer);
  d.Accept(writer);
  out << buffer.GetString();
}


int main(int argc, char *argv[])
{
  itk::wasm::Pipeline pipeline("apply-pstate-to-image", "Apply a presentation state to a given DICOM image and render output as pgm bitmap or dicom file.", argc, argv);
  // Inputs
  std::string imageIn;
  pipeline.add_option("image-in", imageIn, "Input DICOM file")->required()->check(CLI::ExistingFile)->type_name("INPUT_BINARY_FILE");

  // Outputs
  // Metadata output regarding overlays
  itk::wasm::OutputTextStream pstateOutStream;
  pipeline.add_option("pstate-out-stream", pstateOutStream, "Output overlay information")->type_name("OUTPUT_TEXT_STREAM");
  // Processed output image
  itk::wasm::OutputBinaryStream bitmapOutStream;
  pipeline.add_option("bitmap-out-stream", bitmapOutStream, "Output image")->type_name("OUTPUT_BINARY_STREAM");

  // Parameters
  // addGroup "processing options:"
  std::string pstateFile;
  pipeline.add_option("--pstate-file", pstateFile, "[f]ilename: string, process using presentation state file")->required()->check(CLI::ExistingFile)->type_name("INPUT_BINARY_FILE");
  std::string configFile;
  pipeline.add_option("--config-file", configFile, "[f]ilename: string, process using settings from configuration file");
  // process a specific frame within the input dicom:
  int frame = 1;
  pipeline.add_option("--frame", frame, "[f]rame: integer, process using image frame f (default: 1)");

  // addGroup "output format:"
  bool pstateOutput{true};
  pipeline.add_flag("--pstate-output", pstateOutput, "get presentation state information in text stream (default: ON).");
  bool bitmapOutput{true};
  pipeline.add_flag("--bitmap-output", bitmapOutput, "get resulting image as bitmap output stream (default: ON).");

  bool outputFormatPGM{true};
  pipeline.add_flag("--pgm", outputFormatPGM, "save image as PGM (default)");
  bool outputFormatDICOM{false};
  pipeline.add_flag("--dicom", outputFormatDICOM, "save image as DICOM secondary capture");

  /* evaluate command line */
  ITK_WASM_PARSE(pipeline);

  // OFBool opt_dump_pstate     = OFFalse;              /* default: do not dump presentation state */
  OFBool opt_dicom_mode      = OFFalse;                 /* default: create PGM, not DICOM SC */
  OFCmdUnsignedInt opt_frame = frame;                   /* default: first frame */
  const char *opt_pstName    = NULL;                    /* pstate read file name */
  const char *opt_imgName    = imageIn.c_str();         /* image read file name */
  const char *opt_pgmName    = NULL;                    /* pgm save file name */
  const char *opt_cfgName    = NULL;                    /* config read file name */

  /* command line parameters and options */
  if (!pstateOutput && !bitmapOutput)
  {
    OFLOG_FATAL(dcmp2pgmLogger, "No output form requested. Specify either --pstate-output, --bitmap-output or both.");
  }

  if(!pstateFile.empty()) opt_pstName = pstateFile.c_str();
  if(!configFile.empty()) opt_cfgName = configFile.c_str();

  if (outputFormatPGM)         opt_dicom_mode = OFFalse;
  if (outputFormatDICOM)       opt_dicom_mode = OFTrue;

  /* print resource identifier */
  OFLOG_DEBUG(dcmp2pgmLogger, rcsid << OFendl);

  if (opt_cfgName)
  {
    FILE *cfgfile = fopen(opt_cfgName, "rb");
    if (cfgfile) fclose(cfgfile); else
    {
      OFLOG_FATAL(dcmp2pgmLogger, "can't open configuration file '" << opt_cfgName << "'");
      return 10;
    }
  }
  DVInterface dvi(opt_cfgName);
  OFCondition status = EC_Normal;

  if (opt_pstName == NULL)
  {
    OFLOG_DEBUG(dcmp2pgmLogger, "reading DICOM image file: " << opt_imgName);
    status = dvi.loadImage(opt_imgName);
  } else {
    OFLOG_DEBUG(dcmp2pgmLogger, "reading DICOM pstate file: " << opt_pstName);
    OFLOG_DEBUG(dcmp2pgmLogger, "reading DICOM image file: " << opt_imgName);
    status = dvi.loadPState(opt_pstName, opt_imgName);
  }

  if (status == EC_Normal)
  {
    if (pstateOutput) dumpPresentationState(pstateOutStream.Get(), dvi.getCurrentPState());
    if (bitmapOutput)
    {
      const void *pixelData = NULL;
      unsigned long width = 0;
      unsigned long height = 0;
      OFLOG_DEBUG(dcmp2pgmLogger, "creating pixel data");
      if ((opt_frame > 0) && (dvi.getCurrentPState().selectImageFrameNumber(opt_frame) != EC_Normal))
        OFLOG_ERROR(dcmp2pgmLogger, "cannot select frame " << opt_frame);
      if ((dvi.getCurrentPState().getPixelData(pixelData, width, height) == EC_Normal) && (pixelData != NULL))
      {
        if (opt_dicom_mode)
        {
          OFLOG_ERROR(dcmp2pgmLogger, "DICOM output format is currently not supported.");
        }
        else
        {
          OFLOG_DEBUG(dcmp2pgmLogger, "writing PGM file: " << opt_pgmName);
          bitmapOutStream.Get() << "P5\n" << width << " " << height << " " << "255\n";
          if (bitmapOutStream.Get().write((const char*)pixelData, OFstatic_cast(std::streamsize, width * height)).good())
          {
            OFLOG_FATAL(dcmp2pgmLogger, "Can't write output data to file.");
          }
        }
      }
      else
      {
        OFLOG_FATAL(dcmp2pgmLogger, "Can't create output data.");
        return 10;
      }
    }
  }
  else
  {
    OFLOG_FATAL(dcmp2pgmLogger, "Can't open input file(s).");
    return 10;
  }

#ifdef DEBUG
  dcmDataDict.clear();  /* useful for debugging with dmalloc */
#endif

  return (status != EC_Normal);
}
