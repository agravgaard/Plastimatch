DICOM conformance
=================

This section is under development.

The DICOM prefix for plastimatch is 1.2.826.0.1.3680043.8.274.1.  This
is a subrange of the prefix 1.2.826.0.1.3680043.8.274 which was
assigned by Medical Connections. 

Plastimatch defines UIDs the following sub-subrange(s).  All other
uses are reserved. 

1.2.826.0.1.3680043.8.274.1.Y.ZZZZZZZZZZZZ

+---------------+---------------------------------------+
|Y = 1          |Reserved for non-clinical data         |
|               |(phantoms, special projects, software  |
|               |development)                           |
+---------------+---------------------------------------+
|Y = 2          |Clinical data                          |
+---------------+---------------------------------------+
|Y = 3          |Anonymized data                        |
+---------------+---------------------------------------+
|Y = 101        |ImplementationClassUID in meta         |
|               |information header                     |
+---------------+---------------------------------------+
|Z              |Up to 33 digits and dots               |
+---------------+---------------------------------------+

Mondoshot uses the following scheme:

+--------------------------------+---------------------------+
|suffix = 1.200.ZZZZZZZZZZZ      |Non-clinical               |
+--------------------------------+---------------------------+
|suffix = 2.200.ZZZZZZZZZZZ      |Clinical                   |
+--------------------------------+---------------------------+

For the subrange used by the ImplementationClassUID tag, 
i.e. DICOM tag (0002, 0010) in the meta-information header, 
the string ZZZZ is used to mark the plastimatch version number.
If th string ZZZZ is blank, the version is unknown.

NOTE: Plastimatch uses two different dicom engines.  GDCM for itk-related
routines, and DCMTK for non-itk routines (dicom_uid.exe,
mondoshot.exe, perl scripts, etc.).  These engines have different
default conformance properties.  

CT export
---------

CT Images exported by plastimatch will contain the following modules.

+-------------------------+-------------------------+-------------------------+
|IE                       |Module                   |Reference (Dicom 2004)   |
+=========================+=========================+=========================+
|Patient                  |Patient                  |C.7.1.1                  |
+-------------------------+-------------------------+-------------------------+
|Study                    |General Study            |C.7.2.1                  |
+-------------------------+-------------------------+-------------------------+
|Series                   |General Series           |C.7.3.1                  |
+-------------------------+-------------------------+-------------------------+
|Frame of Reference       |Frame of Reference       |C.7.4.1                  |
+-------------------------+-------------------------+-------------------------+
|Equipment                |General Equipment        |C.7.5.1                  |
+-------------------------+-------------------------+-------------------------+
|Image                    |General Image            |C.7.6.1                  |
+-------------------------+-------------------------+-------------------------+
|                         |Image Plane              |C.7.6.2                  |
+-------------------------+-------------------------+-------------------------+
|                         |Image Pixel              |C.7.6.3                  |
+-------------------------+-------------------------+-------------------------+
|                         |CT Image                 |C.8.2.1                  |
+-------------------------+-------------------------+-------------------------+
|                         |SOP Common               |C.12.1                   |
+-------------------------+-------------------------+-------------------------+

Patient Module

+-------------------+---------------+---------------+--------------------+
|Attribute Name     |Tag            |Type           |Notes               |
+===================+===============+===============+====================+
|Patient Name       |(0010,0010)    |2              |                    |
+-------------------+---------------+---------------+--------------------+
|Patient ID         |(0010,0020)    |2              |                    |
+-------------------+---------------+---------------+--------------------+
|Patient's Birth    |(0010,0030)    |2              |No value assigned   |
|Date               |               |               |                    |
+-------------------+---------------+---------------+--------------------+
|Patient's Sex      |(0010,0040)    |2              |No value assigned   |
|                   |               |               |                    |
+-------------------+---------------+---------------+--------------------+

Frame of Reference Module

+-------------------+---------------+---------------+--------------------+
|Attribute Name     |Tag            |Type           |Notes               |
+===================+===============+===============+====================+
|Frame of Reference |(0020,0052)    |1              |                    |
|UID                |               |               |                    |
+-------------------+---------------+---------------+--------------------+
|Position Reference |(0020,1040)    |2              |No value assigned   |
|Indicator          |               |               |                    |
+-------------------+---------------+---------------+--------------------+

DICOM RT structure sets
-----------------------

To be written.

DICOM RT dose
-------------

To be written.
