DICOM anonymizer comparison
---------------------------
There are a number of good and not-so-good DICOM libraries and toolkits 
available.  However, it is not so easy to choose between them.  
For example, most DICOM anonymizers have bugs, which cause them to be 
unusable in some fashion or another.

Over the years, I've used a lot of DICOM anonymizers.  The following 
is a list of common problems that I have encountered:

- Some date fields cannot be changed
  (duh)
- It is not possible to preserve relative dates (for example, we would 
  like to set StudyDate to Jan 1, 2000, but 
  preserve the fact that StructureSetDate occurred 35 days later)
- Private tags are always deleted (on our GE scanner, these contain 
  important acquisition details)
- Comments, diagnosis descriptions, or other fields cannot be reset
  (may contain protected information)
- It is not possible to modify strings (for example, the physician 
  might type "smith-final" into the StructureSetName field, and we 
  would like to delete "smith" but preserve "final" )
- UIDs are not changed, or relationships between UIDs are not preserved
  (duh)

Anonymize IJ DICOM
~~~~~~~~~~~~~~~~~~
  http://rsbweb.nih.gov/ij/plugins/anonymize-ij-dicom/index.html

  Looks doubtful.

Conquest DICOM
~~~~~~~~~~~~~~
  http://www.xs4all.nl/~ingenium/dicom.html

  License: BSD-style

  Version: 1.4.15

CTN
~~~

DICOM Anonymizer
~~~~~~~~~~~~~~~~
  http://sourceforge.net/projects/dicomanonymizer/

  Looks doubtful.

DICOM Rewriter
~~~~~~~~~~~~~~
  http://rsbweb.nih.gov/ij/plugins/dicom-rewriter.html

  Looks doubtful.

DVTk
~~~~

GDCM v2
~~~~~~~

LONI Inspector
~~~~~~~~~~~~~~

MIRC DicomEditor
~~~~~~~~~~~~~~~~
  http://mircwiki.rsna.org/index.php?title=DicomEditor

  License: MIRC license

  Version: 22

  A pretty nice tool for batch editing of DICOM files.   You can modify 
  each tag using separate rules, and all fields are accessible.

  Private fields can be preserved.

  Dates are properly anonymized.  There is a feature which allows date 
  offsets, but it only partially works (@incrementdate works fine, 
  but I could not get @offsetdate to work).

  It includes a regular expression substitution tool for strings.

  It includes a UID remapper, based on UID hashing.  However, UIDs within 
  Sequences don't seem to work (they are not remapped).

MIview
~~~~~~
  http://www.nitrc.org/projects/miview/

Tudor DICOM
~~~~~~~~~~~


Other information
~~~~~~~~~~~~~~~~~
The following link includes a comparison of different DICOM 
toolkits in terms of their APIs:

http://www.vph-noe.eu/vph-repository/doc_download/141-dicom-survey

Here are more links of DICOM toolkits:

http://www.goomedic.com/development-dicom-libraries-frameworks-and-toolkits-for-developers.html?wpmp_switcher=desktop

http://www.uiowa.edu/~mihpclab/mias.html
