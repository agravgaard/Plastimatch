.. _mabs_commands:

Image segmentation (MABS) commands
==================================
MABS can be run in several modes.  These modes are described here.

.. list-table::
   :widths: 20 80
   :header-rows: 1

   * - Command
     - Description
   * - convert
     - Convert an atlas data set from DICOM-RT into nrrd for use by MABS.
   * - pre-align
     - Does this work?
   * - train-registration
     - Add description here.
   * - train-atlas-selection
     - Add description here.
   * - train
     - Add description here.
   * - atlas-selection
     - Add description here.
   * - segment
     - Use MABS to segment the specified image.
  
Convert
-------
Convert a set of atlases.  The input images are found in the directory specified by the
"atlas_dir" field in the [TRAINING] section.  The output are placed in the directory
specified by the "convert_dir" field of the [TRAINING] section.

If images are already converted into nrrd format, this command may be omitted.

Pre-align
---------
To run the pre-alignment command, you must set the "mode" field in the [PREALIGN]
section to "custom".

The pre-align command takes images from the directory specified by the "convert_dir"
filed of the [TRAINING] section as input.  The output images are placed into the dirctory
specified by the "training_dir" field of the [TRAINING] section.

Note, pre-alignment is also used in atlas selection.
No documentation on this yet.


