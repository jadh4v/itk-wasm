# Generated file. Do not edit.

import os
from typing import Dict, Tuple, Optional, List, Any

from itkwasm import (
    environment_dispatch,
    Image,
    BinaryFile,
)

def write_parametric_map(
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
    func = environment_dispatch("itkwasm_dicom", "write_parametric_map")
    output = func(paramap_image, meta_info, output_dicom_file, ref_dicom_series=ref_dicom_series)
    return output
