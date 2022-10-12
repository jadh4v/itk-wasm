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
#include "nlohmann/json.hpp"
#include <cstring>

#ifdef WITH_ZLIB
#include "itk_zlib.h"     /* for zlibVersion() */
#endif

#define OFFIS_CONSOLE_APPLICATION "dcmp2pgm"

static OFLogger dcmp2pgmLogger = OFLog::getLogger("dcmtk.apps." OFFIS_CONSOLE_APPLICATION);

static char rcsid[] = "$dcmtk: " OFFIS_CONSOLE_APPLICATION " v"
  OFFIS_DCMTK_VERSION " " OFFIS_DCMTK_RELEASEDATE " $";

template<typename T>
void PrintJSONProperty(STD_NAMESPACE ostream &out, DcmJsonFormat& format, const char* propName, const T propValue)
{
  out << format.indent() << "\"" << propName << "\": " << propValue << "," << format.newline();
}
template<typename T>
void PrintJSONProperty(STD_NAMESPACE ostream &out, DcmJsonFormat& format, const char* propName, const std::vector<T>& values)
{
  out << format.indent() << "\"" << propName << "\": [";
  for(size_t i=0; i < values.size(); ++i)
  {
    if((i+1) < values.size())
      out << values[i] << ", ";
    else
      out << values[i] << "]," << format.newline();
  }
}
template <>
void PrintJSONProperty<const char*>(STD_NAMESPACE ostream &out, DcmJsonFormat& format, const char* propName, const char* propValue)
{
  const std::string quote = "\"";
  const std::string value = propValue ? std::string(quote + propValue + quote) : "null";
  out << format.indent() << "\"" << propName << "\": " << value.c_str() << "," << format.newline();
}
template<>
void PrintJSONProperty<bool>(STD_NAMESPACE ostream &out, DcmJsonFormat& format, const char* propName, const bool propValue)
{
  out << format.indent() << "\"" << propName << "\": " << (propValue ? "true" : "false") << "," << format.newline();
}

#define JSON_OUT1(property) \
  PrintJSONProperty(out, format, #property, ps.get##property())

#define JSON_OUT2(property, value) \
  PrintJSONProperty(out, format, property, (value))

#define JSON_OPEN() \
  out << format.indent() << "{" << format.newline(); \
  format.increaseIndention()

#define JSON_OPEN_OBJ(obj) \
  out << format.indent() << "\"" << (obj) << "\": {" << format.newline(); \
  format.increaseIndention()

#define JSON_CLOSE() \
  format.decreaseIndention(); \
  out << format.indent() << "}," << format.newline()

#define JSON_OPEN_ARRAY(obj) \
  out << format.indent() << "\"" << (obj) << "\": [" << format.newline(); \
  format.increaseIndention()

#define JSON_CLOSE_ARRAY() \
  format.decreaseIndention(); \
  out << format.indent() << "]," << format.newline()

static void dumpPresentationState(STD_NAMESPACE ostream &out, DVPresentationState &ps)
{
  DcmJsonFormatPretty format(true); // formatted output, use DcmJsonFormatCompact for compact output.
  format.setJsonExtensionEnabled(true);
  OFBool meta = format.printMetaheaderInformation;
  OFCondition status = EC_Normal;

  // START JSON structure
  JSON_OPEN();

  size_t i, j, max;
  const char *c = nullptr;

  OFOStringStream oss;

  JSON_OUT1(PresentationLabel);
  JSON_OUT1(PresentationDescription);
  JSON_OUT1(PresentationCreatorsName);

  if (ps.haveActiveVOIWindow())
  {
    double width=0.0, center=0.0;
    ps.getCurrentWindowWidth(width);
    ps.getCurrentWindowCenter(center);
    JSON_OUT2("CurrentWindowCenter", center);
    JSON_OUT2("CurrentWindowWidth", width);
    JSON_OUT1(CurrentVOIDescription);
  }
  else if (ps.haveActiveVOILUT())
  {
    JSON_OUT1(CurrentVOIDescription);
  }

  JSON_OUT1(Rotation);
  JSON_OUT1(Flip);

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
      // ps.getDisplayedAreaPresentationPixelMagnificationRatio(factor);
      JSON_OUT2("DisplayedAreaPresentationPixelMagnificationRatio", factor);
      break;
  }
  JSON_OUT2("PresentationSizeMode", presentationSizeMode.c_str());

  std::vector<int> displayArea{0, 0, 0, 0};
  ps.getStandardDisplayedArea(displayArea[0], displayArea[1], displayArea[2], displayArea[3]);
  JSON_OUT2("StandardDisplayedArea", displayArea);

  std::vector<double> pixelSpacing{0.0, 0.0};
  if (EC_Normal == ps.getDisplayedAreaPresentationPixelSpacing(pixelSpacing[0], pixelSpacing[1]))
  {
    JSON_OUT2("DisplayedAreaPresentationPixelSpacing", pixelSpacing);
  } else {
    JSON_OUT1(DisplayedAreaPresentationPixelAspectRatio);
  }

  if (ps.haveShutter(DVPSU_rectangular))
  {
    JSON_OPEN_OBJ("RectangularShutter");
    JSON_OUT1(RectShutterLV);
    JSON_OUT1(RectShutterRV);
    JSON_OUT1(RectShutterUH);
    JSON_OUT1(RectShutterLH);
    JSON_CLOSE();
  }

  if (ps.haveShutter(DVPSU_circular))
  {
    JSON_OPEN_OBJ("CircularShutter");
    JSON_OUT1(CenterOfCircularShutter_x);
    JSON_OUT1(CenterOfCircularShutter_y);
    JSON_OUT1(RadiusOfCircularShutter);
    JSON_CLOSE();
  }

  if (ps.haveShutter(DVPSU_polygonal))
  {
    JSON_OPEN_OBJ("PolygonalShutter");
    JSON_OUT1(NumberOfPolyShutterVertices);
    out << format.indent() << "\"coordinates\": [";
    j = ps.getNumberOfPolyShutterVertices();
    Sint32 polyX, polyY;
    for (i=0; i<j; i++)
    {
      if (EC_Normal == ps.getPolyShutterVertex(i, polyX, polyY))
      {
        out << " [" << polyX << ", " << polyY << "],";
      }
    }
    JSON_CLOSE();
  }

  // if (ps.haveShutter(DVPSU_bitmap))
  // {
  //   JSON_OPEN_OBJ("BitmapShutter");
  // }
  JSON_OUT1(ShutterPresentationValue);

  ps.sortGraphicLayers();  // to order of display
  JSON_OPEN_ARRAY("GraphicsLayers"); // GraphicsLayers[]
  for (size_t layer=0; layer<ps.getNumberOfGraphicLayers(); layer++)
  {
    JSON_OPEN(); // Layer{}
    c = ps.getGraphicLayerName(layer);
    JSON_OUT2("Name", c);
    c = ps.getGraphicLayerDescription(layer);
    JSON_OUT2("Description", c);
    if (ps.haveGraphicLayerRecommendedDisplayValue(layer))
    {
      Uint16 r, g, b;
      if (EC_Normal == ps.getGraphicLayerRecommendedDisplayValueGray(layer, g))
      {
        JSON_OUT2("RecommendedDisplayValueGray", g);
        // std::stringstream value;
        // value << "0x" << STD_NAMESPACE hex << g << STD_NAMESPACE dec;
        // JSON_OUT2("RecommendedDisplayValueGray", value.str().c_str());
      }

      if (EC_Normal == ps.getGraphicLayerRecommendedDisplayValueRGB(layer, r, g, b))
      {
        // std::stringstream value;
        // value << "0x" << STD_NAMESPACE hex << r << "\\0x" << g << "\\0x" << b << STD_NAMESPACE dec;
        // JSON_OUT2("RecommendedDisplayValueRGB", value.str().c_str());
        std::vector<Uint16> rgb{r, g, b};
        JSON_OUT2("RecommendedDisplayValueRGB", rgb);
      }
    }

    // text objects
    max = ps.getNumberOfTextObjects(layer);
    DVPSTextObject *ptext = NULL;
    JSON_OPEN_ARRAY("TextObjects"); // TextObjects[]
    for (size_t textidx=0; textidx<max; textidx++)
    {
      ptext = ps.getTextObject(layer, textidx);
      if (ptext)
      {
        JSON_OPEN(); // TextObject{}
        // display contents of text object
        JSON_OUT2("Text", ptext->getText());
        if (ptext->haveAnchorPoint())
        {
          const std::vector<double> anchorPoint{ptext->getAnchorPoint_x(), ptext->getAnchorPoint_y()};
          JSON_OUT2("AnchorPoint", anchorPoint);
          JSON_OUT2("AnchorPointUnits", ptext->getAnchorPointAnnotationUnits()==DVPSA_display? "display" : "pixel");
          JSON_OUT2("AnchorPointVisible", ptext->anchorPointIsVisible());
        }

        if (ptext->haveBoundingBox())
        {
          std::vector<double> box { ptext->getBoundingBoxTLHC_x(), ptext->getBoundingBoxTLHC_y(), ptext->getBoundingBoxBRHC_x(), ptext->getBoundingBoxBRHC_y() };
          JSON_OUT2("BoundingBox", box);
          JSON_OUT2("BoundingBoxUnits", ptext->getBoundingBoxAnnotationUnits()==DVPSA_display ? "display" : "pixel");

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
          JSON_OUT2("BoundingBoxHorizontalJustification", horizontalJustification.c_str());
        }
        JSON_CLOSE(); // TextObject{}
      }
    }
    JSON_CLOSE_ARRAY(); // TextObjects[]

    // graphic objects
    max = ps.getNumberOfGraphicObjects(layer);
    DVPSGraphicObject *pgraphic = NULL;
    JSON_OPEN_ARRAY("GraphicObjects");
    for (size_t graphicidx=0; graphicidx<max; graphicidx++)
    {
      pgraphic = ps.getGraphicObject(layer, graphicidx);
      if (pgraphic)
      {
        JSON_OPEN(); // Graphic{}
        std::string graphicType = "none";
        switch (pgraphic->getGraphicType())
        {
          case DVPST_polyline: graphicType = "polyline"; break;
          case DVPST_interpolated: graphicType = "interpolated"; break;
          case DVPST_circle: graphicType = "circle"; break;
          case DVPST_ellipse: graphicType = "ellipse"; break;
          case DVPST_point: graphicType = "point"; break;
        }
        JSON_OUT2("GraphicType", graphicType.c_str());
        JSON_OUT2("IsFilled", pgraphic->isFilled());
        JSON_OUT2("Units", pgraphic->getAnnotationUnits()==DVPSA_display? "display" : "pixel");

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
        JSON_OUT2("Points", points);
        JSON_CLOSE(); // Graphic{}
      }
    }
    JSON_CLOSE_ARRAY(); // GraphicObjects[]

    // curve objects
    max = ps.getNumberOfCurves(layer);
    DVPSCurve *pcurve = NULL;
    JSON_OPEN_ARRAY("Curves");
    for (size_t curveidx=0; curveidx<max; curveidx++)
    {
      pcurve = ps.getCurve(layer, curveidx);
      if (pcurve)
      {
        JSON_OPEN(); // curve{}
        std::string type;
        switch (pcurve->getTypeOfData())
        {
          case DVPSL_roiCurve: type = "roiCurve"; break;
          case DVPSL_polylineCurve: type = "polylineCurve"; break;
        }
        JSON_OUT2("Type", type.c_str());

        c = pcurve->getCurveAxisUnitsX();
        if (c && (strlen(c)>0)) JSON_OUT2("AxisUnitsX", c);
        c = pcurve->getCurveAxisUnitsY();
        if (c && (strlen(c)>0)) JSON_OUT2("AxisUnitsY", c);
        c = pcurve->getCurveLabel();
        if (c && (strlen(c)>0)) JSON_OUT2("Label", c);
        c = pcurve->getCurveDescription();
        if (c && (strlen(c)>0)) JSON_OUT2("Description", c);

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
        JSON_OUT2("Points", points);
        JSON_CLOSE(); // curve{}
      }
    }
    JSON_CLOSE_ARRAY(); // Curves[]

    // overlay objects
    const void *overlayData=NULL;
    unsigned int overlayWidth=0, overlayHeight=0, overlayLeft=0, overlayTop=0;
    OFBool overlayROI=OFFalse;
    Uint16 overlayTransp=0;
    char overlayfile[100];
    FILE *ofile=NULL;

    max = ps.getNumberOfActiveOverlays(layer);

    JSON_OPEN_ARRAY("Overlays");
    for (size_t ovlidx=0; ovlidx<max; ovlidx++)
    {
      JSON_OPEN(); // overlay{}
      JSON_OUT2("Index", ovlidx);

      std::stringstream value;
      value << "0x" << STD_NAMESPACE hex << ps.getActiveOverlayGroup(layer, ovlidx) << STD_NAMESPACE dec;
      JSON_OUT2("Group", value.str().c_str());

      c=ps.getActiveOverlayLabel(layer, ovlidx);
      if (c) JSON_OUT2("Label", c);

      c=ps.getActiveOverlayDescription(layer, ovlidx);
      if (c) JSON_OUT2("Description", c);

      std::string type;
      if (ps.activeOverlayIsROI(layer, ovlidx)) type = "ROI"; else type = "graphic";
      JSON_OUT2("Type", type.c_str());

      /* get overlay data */
      if (EC_Normal == ps.getOverlayData(layer, ovlidx, overlayData, overlayWidth, overlayHeight,
          overlayLeft, overlayTop, overlayROI, overlayTransp))
      {
        JSON_OUT2("Width", overlayWidth);
        JSON_OUT2("Height", overlayHeight);
        JSON_OUT2("Left", overlayLeft);
        JSON_OUT2("Top", overlayTop);
        std::stringstream buff;
        buff << "P5\n" << overlayWidth << " " << overlayHeight << " " << "255\n";
        buff.write((const char*)overlayData, overlayWidth * overlayHeight);

        constexpr bool urlFriendly = false;
        const std::string outputText = base64_encode(buff.str(), urlFriendly);
        JSON_OUT2("OverlayData", outputText.c_str());
      } else {
        oss << "        unable to access overlay data!" << OFendl;
      }
      JSON_CLOSE(); // overlay{}
    }
    JSON_CLOSE_ARRAY(); // Overlays[]
    JSON_CLOSE(); // Layer{}
  }
  JSON_CLOSE_ARRAY(); // GraphicsLayers[]

  oss << OFStringStream_ends;
  OFSTRINGSTREAM_GETSTR(oss, res)
  OFLOG_INFO(dcmp2pgmLogger, res);
  OFSTRINGSTREAM_FREESTR(res)

  // END JSON structure
  JSON_CLOSE();
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
