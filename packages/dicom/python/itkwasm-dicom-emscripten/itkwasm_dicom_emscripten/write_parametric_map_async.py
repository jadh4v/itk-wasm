# Generated file. To retain edits, remove this comment.

from pathlib import Path
import os
from typing import Dict, Tuple, Optional, List, Any

from .js_package import js_package

from itkwasm.pyodide import (
    to_js,
    to_py,
    js_resources
)
from itkwasm import (
    InterfaceTypes,
    Image,
    BinaryFile,
)

async def write_parametric_map_async(
    paramap_image: Image,
    meta_info: Any,
    output_dicom_file: str,
    ref_dicom_series: List[os.PathLike] = [],
) -> os.PathLike:
    """Convert ITK image to DICOM Parametric Map.

    :param paramap_image: parametric map as an ITK image
    :type  paramap_image: Image

    :param meta_info: JSON containing metadata attributes.
    :type  meta_info: Any

    :param output_dicom_file: File name of the DICOM Parametricmap object with the result of the conversion.
    :type  output_dicom_file: str

    :param ref_dicom_series:  Source DICOM images that were used to generate the parametric map.
    :type  ref_dicom_series: os.PathLike
    """
    js_module = await js_package.js_module
    web_worker = js_resources.web_worker

    kwargs = {}
    if ref_dicom_series is not None:
        kwargs["refDicomSeries"] = to_js(BinaryFile(ref_dicom_series))

    outputs = await js_module.writeParametricMap(to_js(paramap_image), to_js(meta_info), to_js(output_dicom_file), webWorker=web_worker, noCopy=True, **kwargs)

    output_web_worker = None
    output_list = []
    outputs_object_map = outputs.as_object_map()
    for output_name in outputs.object_keys():
        if output_name == 'webWorker':
            output_web_worker = outputs_object_map[output_name]
        else:
            output_list.append(to_py(outputs_object_map[output_name]))

    js_resources.web_worker = output_web_worker

    if len(output_list) == 1:
        return output_list[0]
    return tuple(output_list)
