# Generated file. To retain edits, remove this comment.

from pathlib import Path, PurePosixPath
import os
from typing import Dict, Tuple, Optional, List, Any

from importlib_resources import files as file_resources

_pipeline = None

from itkwasm import (
    InterfaceTypes,
    PipelineOutput,
    PipelineInput,
    Pipeline,
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
    global _pipeline
    if _pipeline is None:
        _pipeline = Pipeline(file_resources('itkwasm_dicom_wasi').joinpath(Path('wasm_modules') / Path('write-parametric-map.wasi.wasm')))

    pipeline_outputs: List[PipelineOutput] = [
        PipelineOutput(InterfaceTypes.BinaryFile, BinaryFile(PurePosixPath(output_dicom_file))),
    ]

    pipeline_inputs: List[PipelineInput] = [
        PipelineInput(InterfaceTypes.Image, paramap_image),
        PipelineInput(InterfaceTypes.JsonCompatible, meta_info),
    ]

    args: List[str] = ['--memory-io',]
    # Inputs
    args.append('0')
    args.append('1')
    # Outputs
    output_dicom_file_name = str(PurePosixPath(output_dicom_file))
    args.append(output_dicom_file_name)

    # Options
    input_count = len(pipeline_inputs)
    if len(ref_dicom_series) < 1:
       raise ValueError('"ref-dicom-series" kwarg must have a length > 1')
    if len(ref_dicom_series) > 0:
        args.append('--ref-dicom-series')
        for value in ref_dicom_series:
            input_file = str(PurePosixPath(value))
            pipeline_inputs.append(PipelineInput(InterfaceTypes.BinaryFile, BinaryFile(value)))
            args.append(input_file)


    outputs = _pipeline.run(args, pipeline_outputs, pipeline_inputs)


