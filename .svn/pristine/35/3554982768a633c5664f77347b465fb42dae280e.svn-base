--------------------------------------------------

Directory organization:

1) Original files go into "new-data" directory
2) Derivative files go into "processed"

Derivative directories:

mha		Unaltered or subsampled mha
masks		Mask files
masked		Masked mha files

Mask/vector field abbreviations

p		Patient mask
m		Moving
n		Non-moving
g		GTV
c		Combined moving & non-moving

For example, t0p means that processing was done 
using a patient mask.

--------------------------------------------------

Naming conventions for output files.

Example:

  t_0_5_p_221_vf.mha

  0:    Fixed image
  1:    Moving image
  p:    Patient mask
  221:  Subsampled 2x2x1
  vf:   Vector field

This vector field is defined in the coordinate frame 
of the fixed image.  It can be used to deform the 
moving image into the fixed image.

--------------------------------------------------
How to anonymize

1)  Transfer to SHARP or STASH

2)  Gather cross-reference data (except ExamID).  I use ConquestDicom 
    GUI for this.  Write into notebook

3)  Anonymize data

3a) Using cygwin, run:
        perl ./list_struct_names.pl /g/ConquestDicom/data/????

3b) Edit c:/mirc/DicomEditor/dicom-anonymizer.properties
    Change:
        ID (set to new id)
        If MINDATE != 29991231
          Fill in DATEOFFSET into tags [3006,0008] and [300A,0006]
        If patient name or MD name found in StructureSet/RTPlan label/name
          Fill in regular expression matching name into 
          [3006,0002], [3006,0004], [300A,0002], and [300A,0003]

3c) Run DicomEditor.  Make sure that "Change names of modified files" 
    is checked.

3d) If there were more than one regular expression with patient name, 
    change dicom-anonymizer.properties, and re-run DicomEditor in 
    output directory

4)  Copy files from conquest dicom

4a) Edit conquest_split.pl, set $indir and $outdir

4b) Using windows, run:
        perl conquest_dicom.pl

4c) Copy down StudyID into notebook

5)  Clean up junk
    Delete patient in ConquestDicom
    Delete directory in ConquestDicom
    Delete temp-anon directory

6)  Copy over RPM file
    <anonymize>

--------------------------------------------------
How to run registrations

0)  Run set_path.bat to set the paths

1)  Convert dicom to mha
    Edit and run convert_4dct.pl

2)  Fix the problem with RPI -> RAI orientation in the header.
    Edit and run fix_orientation.pl
    (ITK 2.4.1 defaults to ???)
    (ITK 3.4.0 defaults to RAI, skip this step)

3a) Make masks from cms contours
    Edit and run render_cms_contours.m
    You have to manually rename these for dp-series sets

3b) Make masks from ge contours
    Edit and run render_dicom_contours.m

3c) Make masks using new C program
    find_patient_contours.pl

4)  Subsample mha & mask separately
    Edit and run subsample_mhas.pl
    Then, edit and run subsample_mhas.bat

5)  Apply masks
    Edit and run apply_mask.pl
    Then, edit and run apply_mask.bat

6)  Run registration
    Edit and run bspline.pl
    Check bspline??.bat, and parms??_??.bat
