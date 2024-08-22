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
    BinaryFile,
    Image,
)

async def read_parametric_map_async(
    dicom_file: os.PathLike,
) -> Tuple[Image, Any]:
    """Read and convert DICOM Parametric Map into ITK image, and generate a JSON file holding meta information.

    :param dicom_file: File name of the DICOM Parametric map image.
    :type  dicom_file: os.PathLike

    :return: Parametric map as an output ITK image
    :rtype:  Image

    :return: Output meta information as JSON
    :rtype:  Any
    """
    js_module = await js_package.js_module
    web_worker = js_resources.web_worker

    kwargs = {}

    outputs = await js_module.readParametricMap(to_js(BinaryFile(dicom_file)), webWorker=web_worker, noCopy=True, **kwargs)

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
