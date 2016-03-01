.. _segmentation_command_file_reference:

Segmentation command file reference
===================================

The mabs parameter file can have seven sections: PREALIGNMENT, ATLAS-SELECTION, 
TRAINING, REGISTRATION, STRUCTURES, LABELING and OPTIMIZATION_RESULT.
Each section contatins specific entries that can not be mixed.
All the possible parameters are listed below:


.. list-table::
   :widths: 20 20 20 20 60
   :header-rows: 1

   * - Option
     - Section
     - Default value
     - Possible values
     - Description
   * - mode
     - PREALIGNMENT
     - disabled
     - disabled, default, custom
     - Set the method for prealign the images
   * - reference
     - PREALIGNMENT
     - not set
     - 
     - Set the reference image
   * - spacing
     - PREALIGNMENT
     - not set
     - 
     - Set the reference spacing
   * - registration_config
     - PREALIGNMENT
     - not set
     - plastimatch registration file without the GLOABL section
     - Set the registration parmameters for prealign if "custom" mode is chosen
   * - prealign_struct
     - PREALIGNMENT
     - not set
     - name of an atlas structure
     - Use the specified structure to execute center of gravity initialization 
   * -
     -
     -
     -
     -
   * - enable_atlas_selection
     - ATLAS-SELECTION
     - false
     - true, false
     - Enable atlas selection process
   * - atlas_selection_criteria
     - ATLAS-SELECTION
     - nmi
     - nmi, nmi-post, nmi-ratio, mse, mse-post, mse-ratio, random, precomputed
     - Set the criterion to use for ranking the atlases (see at the bottom of this page for further explanations)
   * - similarity_percent_threshold
     - ATLAS-SELECTION
     - 0.40
     - Ranging from 0.0 to 1.0
     - Percentage threshold used to pick the atlases from the ranking obtained on the basis of a nmi/mse similarity value.
       All the atlases having a similarity value greater equal to the following value will be selected.
       minimum_similarity_percentage_value = similarity_min + (threshold * (similarity_max - similarity_min))
   * - atlases_from_ranking
     - ATLAS-SELECTION
     - -1 (means disabled)
     - ranging from 0 to the whole number of atlases
     - Number of atlases to pick from the ranking (nmi/mse based or precomputed).
   * - mi_histogram_bins
     - ATLAS-SELECTION
     - 100
     - as usual for mi histogram bins setting
     - Set the number of histogram bins to use for nmi computation
   * - percentage_nmi_random_sample
     - ATLAS-SELECTION
     - not set by plastimatch, itk default value is used
     - between 0 (not included) and 1 (included)
     - Set the number of random voxels to use for NMI computation.
       The final value is equal to the number of voxels of fixe images time the set value.
   * - roi_mask
     - ATLAS-SELECTION
     - not set
     - image file name (.mha/.mhd/.nii.gz/.nrrd/ecc)
     - Set the mask to reduce the volume of the subject/atlas images where the nmi/mse will be computed
   * - selection_reg_parms
     - ATLAS-SELECTION
     - not set
     - plastimatch registration file without the GLOABL section
     - File where are stored the registration parameters (without GLOBAL stage) that will be used for the nmi-post, nmi-ratio mse-post and mse-ratio selection
   * - lower_mi_value_subject
     - ATLAS-SELECTION
     - not set
     - in the range of the intesities of the subject image. Anyway lower than the upper_mi_value_subject
     - Lower intensity bound on the histogram of the subject image. Only the values greater than this threshold will be used for nmi computation
   * - upper_mi_value_subject
     - ATLAS-SELECTION
     - not set
     - in the range of the intesities of the subject image. Anyway higher than the lower_mi_value_subject
     - Upper intensity bound on the histogram of the subject image. Only the values lower than this threshold will be used for nmi computation
   * - lower_mi_value_atlas
     - ATLAS-SELECTION
     - not set
     - in the range of the intesities of the atlas image. Anyway lower than the upper_mi_value_atlas
     - Lower intensity bound on the histogram of the atlas image. Only the values greater than this threshold will be used for nmi computation
   * - upper_mi_value_atlas
     - ATLAS-SELECTION
     - not set
     - in the range of the intesities of the atlas image. Anyway higher than the lower_mi_value_atlas
     - Upper intensity bound on the histogram of the atlas image. Only the values lower than this threshold will be used for nmi computation
   * - min_random_atlases
     - ATLAS-SELECTION
     - 6
     - ranging from 1 to the whole number of atlases. Anyway equal/lower than max_random_atlases
     - Minimum number on atlases to extract when random selection is choosen
   * - max_random_atlases
     - ATLAS-SELECTION
     - 14
     - ranging from 1 to the whole number of atlases. Anyway equal/greater than min_random_atlases
     - Maximim number on atlases to extract when random selection is choosen
   * - precomputed_ranking
     - ATLAS-SELECTION
     - note set
     - text file where a precomputed ranking was stored
     - Text file containing the precomputed ranking. For each line there is a ranking for an patient.
       The style is: 
       patient1: atl1 atl2 atl3 atl4 
   * -
     -
     -
     -
     -
   * - training_dir
     - TRAINING
     - not set
     - folder path
     - Folder that contains the data for the training
   * - fusion_criteria
     - TRAINING
     - gaussian
     - "gaussian", "staple", "gaussian,staple" and "none"
     - Labels fusion criterion
   * - distance_map_algorithm
     - TRAINING
     - not set
     -
     - Implementation for distance map computation
   * - minsim_values
     - TRAINING
     - L 0.0001:1:0.0001
     -
     - Minimum similarity values for gaussian labels fusion
   * - rho_values
     - TRAINING
     - 1:1:1
     - 
     - Rho values for gaussian labels fusion
   * - sigma_values
     - TRAINING
     - L 1.7:1:1.7
     - 
     - Sigma values for gaussian labels fusion
   * - threshold_values
     - TRAINING
     - 0.5
     - 
     - Threshold values for gaussian labels fusion
   * - write_distance_map_files
     - TRAINING
     - 1 (true)
     - 0 (false) or 1 (true)
     - Write distance map files on the disk (only for gaussian fusion)
   * - write_thresholded_files
     - TRAINING
     - 1 (true)
     - 0 (false) or 1 (true)
     - Write thresholded files on the disk (only for gaussian fusion)
   * - write_weight_files
     - TRAINING
     - 1 (true)
     - 0 (false) or 1 (true)
     - Write weight files on the disk (only for gaussian fusion)
   * - write_warped_images
     - TRAINING
     - 1 (true)
     - 0 (false) or 1 (true)
     - Write warped images on the disk
   * - write_warped_structures
     - TRAINING
     - 1 (true)
     - 0 (false) or 1 (true)
     - Write warped structures on the disk
   * - confidence_weight 
     - TRAINING
     - 1.0
     -
     - Multiplicative factor for the prior probability that any pixel would be classified as inside the structure (only for staple fusion)
   * -
     -
     -
     -
     -
   * - registration_config
     - REGISTRATION
     - not set
     - plastimatch registration file without the GLOABL section
     - Set the registration parmameters for the deformable registration
   * -
     -
     -
     -
     -
   * - no predefined entries, just the list of the structures to segment (without extension file)
     - STRUCTURES
     - not set
     - 
     -
   * -
     -
     -
     -
     -
   * - input
     - LABELING
     - not set
     -
     -
   * - output
     - LABELING
     - not set
     -
     -
   * -
     -
     -
     -
     -
   * - registration
     - OPTIMIZATION_RESULT
     - not set
     -
     -
   * - gaussian_weighting_voting_rho
     - OPTIMIZATION_RESULT
     - 1.0
     -
     -
   * - gaussian_weighting_voting_sigma
     - OPTIMIZATION_RESULT
     - 30.0
     -
     -
   * - gaussian_weighting_voting_minsim
     - OPTIMIZATION_RESULT
     - 0.3
     -
     -
   * - optimization_result_confidence_weight
     - OPTIMIZATION_RESULT
     - 1.0
     -
     -
   * - gaussian_weighting_voting_thresh
     - OPTIMIZATION_RESULT
     - 0.4
     -
     -

The possible atlas selection criteria are nmi, nmi-post, nmi-ratio, mse, mse-post, mse-ratio, random, precomputed.

.. list-table::
   :widths: 20 60
   :header-rows: 1
   
   * - Criterion
     - Description
   * - nmi
     - Normalized mutual information computed between prealigned (ridgly) images
   * - nmi-post
     - Normalized mutual information computed between images after a deformable registration
   * - nmi-ratio
     - Score calculated using the normalized mutual information computed before and after a deformable registration
   * - mse
     - Root mean square error computed between prealigned (ridgly) images
   * - mse-post
     - Root mean square error computed between images after a deformable registration
   * - mse-ratio
     - Score calculated using the root mean square error computed before and after a deformable registration
   * - random
     - Random selection of a random number of atlases
   * - precomputed
     - Ranking read from a text file containing a precomputed list of atlases



Structure name mapping
----------------------
It is required to put the name of each structure to be segmented into the
STRUCTURES section.  If structure names are uniformly defined, then 
you simply need to list them::

  [STRUCTURES]
  Prostate
  Bladder
  Rectum

If the structure names are not uniform, you can use this section to 
map multiple names (within an atlas) to the same target name.
To do so, put the target name to the left of the equal sign, and the 
original name to the right of the equal sign.  For example, 
the following would map three different spellings to the same target name::

  [STRUCTURES]
  Prostate
  Prostate=prostate
  Prostate=Pros_GTV

