# Generated file. Do not edit.

import os
from typing import Dict, Tuple, Optional, List, Any

from itkwasm import (
    environment_dispatch,
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
    func = environment_dispatch("itkwasm_dicom", "read_parametric_map_async")
    output = await func(dicom_file)
    return output
