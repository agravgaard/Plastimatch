CXT file format
---------------

The cxt file format is an ASCII format that 
mirrors the DICOM-RT structure set format.  The command
"plastimatch convert" converts from CXT to DICOM-RT structure set.

The cxt file format contains the following sections

+--------------------+--------------------------------------------------------+
|Header section      |Various flexible ascii fields                           |
+--------------------+--------------------------------------------------------+
|ROI section         |List of ROI's in the structure set, along with a fixed  |
|                    |set of information on each                              |
|                    |                                                        |
+--------------------+--------------------------------------------------------+
|Contour section     |List of contours, with a fixed set of information about |
|                    |each                                                    |
+--------------------+--------------------------------------------------------+

The header section contains mostly information about the associated volume:

+--------------------+--------------------------------------------------------+
|SERIES_CT_UID *uid* |The uid field specifies the UID for the associated CT   |
|                    |volume                                                  |
+--------------------+--------------------------------------------------------+
|OFFSET *x y z*      |The x y z fields specify the position of the first      |
|                    |(upper left) voxel in the image                         |
+--------------------+--------------------------------------------------------+
|DIMENSION *x y z*   |The x y z fields specify the size of the volume         |
|                    |                                                        |
+--------------------+--------------------------------------------------------+
|SPACING *x y z*     |The x y z fields specify the inter-voxel spacing        |
|                    |                                                        |
+--------------------+--------------------------------------------------------+

Each line of the ROI section contains three fields, separated by the
ASCII space “ ”. 

+--------------------+-----------------------------------+--------------------+
|Field               |Value                              |Example             |
+====================+===================================+====================+
|Structure number    |A positive integer                 |1                   |
+--------------------+-----------------------------------+--------------------+
|Color               |RGB tuple, separated by backslashes|0\255\0             |
|                    |                                   |                    |
+--------------------+-----------------------------------+--------------------+
|Structure name      |ASCII string                       |gtv_primary         |
+--------------------+-----------------------------------+--------------------+

Each line of the contour section has 6 fields, separated by the pipe
symbol “|”. 

+-------------+-------------------+-------------------------------------------+
|Field        |Description        |Example                                    |
+=============+===================+===========================================+
|ROI Number   |A postitive        |1                                          |
|             |integer, referring |                                           |
|             |to the structure   |                                           |
+-------------+-------------------+-------------------------------------------+
|Contour      |This is included by|2.5                                        |
|thickness    |GE dicom export.   |                                           |
|             |Not sure how useful|                                           |
|             |it is              |                                           |
+-------------+-------------------+-------------------------------------------+
|Number of    |How many points in |3                                          |
|points       |the contour?       |                                           |
+-------------+-------------------+-------------------------------------------+
|Slice index  |Which slice does   |20                                         |
|             |this contour belong|                                           |
|             |to within the CT   |                                           |
|             |series of interest?|                                           |
|             |The numbering start|                                           |
|             |with 0 for the     |                                           |
|             |first slice sorted |                                           |
|             |by Z.              |                                           |
+-------------+-------------------+-------------------------------------------+
|Associated   |Which slice does   |2.16.840.1.114362.1.90609.1196125535718.935|
|slice UID    |this contour belong|                                           |
|             |to within the CT   |                                           |
|             |series of interest?|                                           |
+-------------+-------------------+-------------------------------------------+
|Points       |Sequence of x\y\z  |5.4\-63.2\10\8.4\-66.2\10\1.8\-49.0\10     |
|             |tuples, separated  |                                           |
|             |by                 |                                           |
|             |backslashes. Units |                                           |
|             |are mm. z value    |                                           |
|             |should be the same |                                           |
|             |for all of the     |                                           |
|             |points.            |                                           |
+-------------+-------------------+-------------------------------------------+
