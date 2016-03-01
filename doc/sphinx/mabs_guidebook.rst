.. _mabs_guidebook:

Image segmentation (MABS) guidebook
===================================
.. container:: twocol

   .. container:: rightside

      .. image:: ../figures/mabs_1.png
        :width: 100 %
        :align: right

   .. container:: leftside

	  MABS (Multi Atlas Based Segmentation) is a flexible 
	  system for performing automatic segmentation of medical images. 
	  This guidebook explains how to prepare an atlas for
	  segmentation, how to perform a segmentation, 
	  and how to tune MABS for optimal accuracy.

So far, MABS has only been tested on linux, but 
it probably can work on other platforms.  Please contact 
the email list if you desire to run MABS on other platforms.

Step 1: Creating a master configuration file
--------------------------------------------
First, you should create a new directory for 
holding your configuration files and training data.
You can start with the following layout::

  --+------- mabs/
    +------- mabs/task01.cfg

In this context, "task01" refers to a segmentation task 
with an associated atlas.  You can have multiple tasks, 
such as one task for head and neck, one for prostate, and so on. 
You don't have to call it "task01", you can call it anything.
The file "mabs/task01.cfg" is a master configuration file that 
controls MABS for task01.  For purposes of the guidebook, we will use 
the configuration file specified below::

  [TRAINING]
  atlas_dir=task01-atlas
  training_dir=task01

  rho_values=0.75:0.25:1.25
  sigma_values=L 0.75:0.25:1.5
  minimum_similarity=L -0.7:0.2:-0.3
  threshold_values=0.2:0.1:0.5

  [REGISTRATION]
  registration_config=task01-reg

  [STRUCTURES]
  brainstem
  right_parotid
  left_parotid

The meaning of each of these parameters will be described 
as we proceed through the guidebook.

Step 2: Preparing the atlas data
--------------------------------
If your input is in DICOM format, 
you should organize your data like this::

  --+------- mabs/
    +------- mabs/task01.cfg
    +---+--- mabs/task01-atlas/
    |   +--- mabs/task01-atlas/subject-01
    |   +--- mabs/task01-atlas/subject-02
    |   +--- mabs/task01-atlas/subject-03

The directory "mabs/task01-atlas" contains the atlas data, 
and each subject must be placed in a separate subdirectory, 
but you can name the directories whatever you like.  
Each subject subdirectory should contain one image (in DICOM format), 
and one structure set (in DICOM-RT format).  

Next, the atlas data should be converted from DICOM-RT into nrrd format 
using the following command::

  plastimatch mabs --convert task01.cfg

After this command completes, you will see newly created directories, 
containing converted images and structures.  The layout is as follows::

  --+------- mabs/
    +------- mabs/task01.cfg
    +--+---- mabs/task01-atlas/
    |  +---- mabs/task01-atlas/subject-01
    |  +---- ...
    +--+---- mabs/task01/
    |  +---- mabs/task01/convert/
    |  +--+- mabs/task01/convert/subject-01/img.nrrd
    |     +- mabs/task01/convert/subject-01/structures/brainstem
    |     +- mabs/task01/convert/subject-01/structures/right_parotid
    |     +- ...

Finally, you must create a prealign directory.  At this time, the 
prealignment procedure is still under development, so you may simply 
rename or copy the converted data directory.  Here is how to do this 
on linux::

  mv task01/convert task01/prealign

If your input data is not DICOM, you must manually convert them 
into nrrd, and then put them into the prealign directory as described 
above.  

Step 3: Choose a registration strategy
--------------------------------------
Next, you must choose a registration strategy for your atlas-based 
segmentation task.  Create the directory "task01-reg", as specified 
in the "registration_config" line of the master config file.
Within that directory, create one or more registration configuration files.
For example::

  --+------- mabs/
    +--+---- mabs/task01-reg/
    |  +---- mabs/task01-reg/reg01.txt
    |  +---- mabs/task01-reg/reg02.txt

During the registration optimization phase, each registration 
configuration file will evaluated against the atlas image.
The optimal strategy will be chosen to maximize the 
Average Dice score over structures defined 
in the master configuration file.

The format of the registration configuration files follows the 
format specified in the :ref:`image_registration_guidebook`
and the :ref:`registration_command_file_reference`.
However, a GLOBAL section is not needed, nor should one be specified.
The following example is a bare-bones configuration::

  # == reg01.txt ==
  # A single B-spline stage, with 10 cm grid spacing
  [STAGE]
  xform=bspline
  impl=plastimatch
  grid_spac=100 100 100
  regularization_lambda=10
  max_its=30
  res=4 4 2

Here is another, more complicated example, which may or may not give 
better results::

  # == reg02.txt ==
  # First, truncate HU values to range [-1000,1000]
  [PROCESS]
  action=adjust
  parms=-inf,0,-1000,-1000,1000,1000,inf,0
  images=fixed,moving

  # Next, do a grid search to find good global translation
  [STAGE]
  xform=translation
  impl=plastimatch
  gridsearch_min_overlap=0.8 0.8 0.8
  res=4 4 2

  # Next, do a local search to improve translation
  [STAGE]
  xform=translation
  impl=itk
  optim=rsg
  res=4 4 2

  # Finally, a single B-spline stage, with 10 cm grid spacing
  [STAGE]
  xform=bspline
  impl=plastimatch
  grid_spac=100 100 100
  regularization_lambda=10
  max_its=30
  res=4 4 2

Once you have created one or more registration parameter file, you can 
run a training routine to evaluate them, as follows::

  plastimatch mabs --train-registration task01.cfg

This will take a long time to run.  If you have a large atlas and 
you want to evaluate several strategies, it may run for several days.
In the end, you will get a directory layout which looks like this::

  --+----------- mabs/
    +--+-------- mabs/task01/
    +--+--+----- mabs/task01/mabs-train/
    +--+--+--+-- mabs/task01/mabs-train/subject-01/...
    +--+--+--+-- mabs/task01/mabs-train/subject-02/...

The "mabs-train" directory contains results from exhaustive testing 
of all pairs of atlas members on all registration strategies.  
These results are analyzed by running a script in the plastimatch 
source code directory::

  plastimatch-source/extra/perl/digest_mabs_stats.pl task01/mabs-train

You will see something like this::

  reg01.txt,0.718458,4.91699
  reg02.txt,0.769172,3.26388

Which means that the first registration strategy (reg01.txt) 
had an average Dice of 0.72 and an average 95-boundary Hausdorff of 4.9.  
The second strategy (reg02.txt) was better, and therefore was selected.
The script writes another file which confirms this choice to MABS.::

  --+----------- mabs/
    +--+-------- mabs/task01/
    +--+--+----- mabs/task01/mabs-train/
    +--+--+----- mabs/task01/mabs-train/optimization_result_reg.txt

Step 4: Choose a segmentation strategy
--------------------------------------
Next, you must optimize the voting parameters.  
This is easier than optimizing the registration strategy, because 
there are a fixed set of parameters to be optimized.
The search range is specified in the master configuration 
file, for example, like this::

  rho_values=0.75:0.25:1.25
  sigma_values=L 0.75:0.25:1.5
  minimum_similarity=L -0.7:0.2:-0.3
  threshold_values=0.2:0.1:0.5

To run the segmentation optimization, do this::

  plastimatch mabs --train task01.cfg

This will also take a long time to run.  If you have a large atlas, 
it may run for several days.  In the end, you will get additional 
files and directories like this::

  --+----------- mabs/
    +----------- mabs/seg_dice.csv
    +--+-------- mabs/task01/mabs-train/subject-01/segmentations/...

Once again, run the analysis script::

  plastimatch-source/extra/perl/digest_mabs_stats.pl task01/mabs-train

Which should give something like this::

  reg01.txt,0.718458,4.91699
  reg02.txt,0.769172,3.26388
  seg: 0.750000,31.622776,0.199526,0.200000,0.777639

This tells you that the optimal segmentation parameters are 
rho=0.75, sigma=31.6, minsim=0.20, and thresh=0.20.
The average Dice over all structures using these parameters is 0.77.
The script writes yet another file which confirms these choices
for future use with MABS.::

  --+----------- mabs/
    +--+--+----- mabs/task01/mabs-train/optimization_result_seg.txt

Step 5: Running a segmentation
------------------------------
Whew!  That was a lot of work.  But now you are ready to run segmentations.
If your images are in directory "input-dicom", you can do this::

  plastimatch mabs --input dicom-in --output result-directory task01.cfg

This will segment the input image, and create an output directory
which contains the segmented structures (and a lot of other files too).
