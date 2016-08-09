.. _registration_command_file_reference:

Image registration command file reference
-----------------------------------------
The registration 
command file uses the "ini file" format.  There are two 
possible sections: GLOBAL, and STAGE.  There should be exactly 
one GLOBAL section, and there can be multiple STAGE sections.

In general, the GLOBAL section defines the input files and 
output files for a single registration.  Each STAGE section 
defines a single processing stage within a registration 
pipeline.  

Global options
==============
The GLOBAL section has only a limited set of allowed parameters.
However, some GLOBAL parameters are also allowed in a STAGE section, 
as noted below.

.. list-table::
   :widths: 20 20 60
   :header-rows: 1

   * - option
     - type
     - value
   * - fixed
     - GLOBAL
     - Filename of fixed (reference) image
   * - moving
     - GLOBAL
     - Filename of moving (target) image
   * - fixed_roi
     - GLOBAL, STAGE
     - Filename of a binary mask for the fixed image; 
       only pixels which are non-zero in this image will contribute 
       to the registration result
   * - moving_roi
     - GLOBAL, STAGE
     - Filename of a binary mask for the moving image;
       only pixels which are non-zero in this image will contribute 
       to the registration result
   * - fixed_landmarks
     - GLOBAL, STAGE
     - Filename of a list of landmark locations within the fixed image
       which can be used to guide the registration
   * - moving_landmarks
     - GLOBAL, STAGE
     - Filename of a list of landmark locations within the moving image
       which can be used to guide the registration
   * - warped_landmarks
     - GLOBAL, STAGE
     - Filename of output landmarks, warped by the registration result
   * - xform_in
     - GLOBAL
     - Initial guess for transform
   * - xform_out
     - GLOBAL, STAGE
     - Filename of output transform
   * - vf_out
     - GLOBAL, STAGE
     - Filename of output transform, as vector field
   * - img_out
     - GLOBAL, STAGE
     - Filename of warped image
   * - img_out_fmt
     - GLOBAL, STAGE
     - Output format, which must be either “auto” (default), 
       which means the filename extenstion is used to determine
       the file format, or “dicom”, which interprets img_out 
       as a directory name to output the dicom files
   * - img_out_type
     - GLOBAL, STAGE
     - Data type of the output image.  Either “auto” (default), or 
       an image type string: char, uchar, short, ushort, int, uint, 
       float, or double
   * - logfile
     - GLOBAL
     - Filename for output log of registration progress

Stage options
=============
Each STAGE section generates a computational stage within a 
computational sequence.  Therefore, a command file with 
three STAGE sections will have three registration stages.
The stages are executed in the same order as they appear in the 
command file.  The registration result from a previous stage 
is passed as the starting point to the next stage.
After all three stages are complete, the output files are generated.

As a general rule, all parameters are optional.  When they are specified, 
they are used.  When they are not specified, they are set automatically.
There are different ways they are automatically set.

* Any parameters not specified in the first stage are given default values.
* Any parameters not specified in subsequent stages are given the 
  same value they had in the previous stage.
* Some parameters can have a value of "auto."  That means the value 
  can be chosen dynamically based on registration inputs 
  or optimization results.

A registration stage is classified according to its transform (xform), 
its implementation, and its optimizer.  Only certain combinations 
are possible, as shown in the following table.

.. list-table::
   :widths: 20 40 40
   :header-rows: 1

   * - xform
     - impl
     - optim
   * - align_center
     - itk
     - N/A
   * - translation
     - itk
     - amoeba, rsg
   * - 
     - plastimatch
     - grid_search
   * - rigid (default)
     - itk
     - amoeba, versor (default)
   * - affine
     - itk
     - amoeba, rsg
   * - bspline
     - itk
     - lbfgs, lbfgsb
   * - 
     - plastimatch (default)
     - lbfgsb (default), steepest
   * - vf
     - itk
     - demons
   * - 
     - plastimatch
     - demons

The following specific parameters are used to refine the optimization.
Depending on the choice of xform, impl, and optim a different set of
specific parameters are available. 

.. list-table::
   :widths: 20 15 10 10 45
   :header-rows: 1

   * - option
     - xform+optim+impl
     - default
     - units
     - description
   * - background_max
     - any+any+any
     - -999.0
     - image intensity
     - This is a threshold value that is used to automatically 
       determine the registration region of interest.
   * - convergence_tol
     - any+not demons+any
     - 1e-6
     - score
     - Stop optimization if change in score between iterations 
       falls below this value
   * - default_value
     - any+any+any
     - -999.0
     - image intensity
     - (needs description)
   * - demons_acceleration
     - vf+demons+plastimatch
     - 1.0
     - unitless
     - (needs description)
   * - demons_filter_width
     - vf+demons+plastimatch
     - [3 3 3]
     - voxels
     - (needs description)
   * - demons_homogenization
     - vf+demons+plastimatch
     - 1.0
     - unitless
     - (needs description)
   * - demons_std
     - vf+demons+any
     - 6.0
     - mm
     - width of demons smoothing kernel
   * - demons_gradient_type
     - vf+demons+itk
     - symmetric
     - enumeration
     - Type of gradient that will be used to compute update force, choose 
       from {symmetric, fixed, warped_moving, mapped_moving}
   * - demons_smooth_update_field
     - vf+demons+itk
     - false
     - bool
     - Set whether the update field is smoothed
   * - demons_std_update_field
     - vf+demons+itk
     - 1
     - std-dev.
     - Width of Gaussian used to smooth update field
   * - demons_smooth_deformation_field
     - vf+demons+itk
     - true
     - bool
     - Set whether the deformation field is smoothed
   * - demons_std_deformation_field
     - vf+demons+itk
     - 1
     - std-dev.
     - Width of Gaussian used to smooth deformation field
   * - demons_step_length
     - vf+demons+itk
     - 1
     - mm
     - maximum update step length. 0 implies no restriction 
   * - grad_tol
     - any+{lbfgs}+itk
     - 1.5
     - score per unit parameter
     - Gradient convergence tolerance for LBFGS optimizer.
       The optimizer can be asked to stop when the gradient
       magnitude is below this number.
   * - grid_spac
     - bspline+any+any
     - [20 20 20]
     - mm
     - Spacing between control points in B-spline grid. 
       The minimum spacing is 4*(Pixel Size); if a smaller size is 
       specified, it will be adjusted upward.
   * - gridsearch_min_overlap
     - translation+grid_search +plastimatch
     - [0.5 0.5 0.5]
     - percent
     - Minimum amount of overlap required during grid search.  
       The smaller of the two images must overlap the larger image 
       by at least this amount in three dimensions.
   * - histoeq
     - vf+demons+itk
     - 0
     - boolean
     - specify whether or not to equalize intensity histograms before 
       registration
   * - landmark_stiffness
     - bspline+any+plastimatch
     - 1.0
     - float
     - Relative contribution of landmark distance in cost function
   * - mattes_fixed_minVal, mattes_fixed_maxVal
     - bspline+any+itk
     - 0
     - image intensity
     - Min and max intensity values of intensity range for fixed image 
       used for MI calculation.
       If values are not set by user min and max values will be calculated 
       from images. Only used for optimized version of itk implementation.
   * - mattes_moving_minVal, mattes_moving_maxVal
     - bspline+any+itk
     - 0
     - image intensity
     - Min and max intensity values of intensity range for moving image 
       used for MI calculation.
       If values are not set by user min and max values will be calculated 
       from images. Only used for optimized version of itk implementation.
   * - max_its
     - any+any+any
     - 25
     - iterations
     - Maximum number of iterations (or sometimes function evaluations) 
       performed within a stage.
   * - max_step
     - any+{versor, rsg}+itk
     - 10.0
     - scaled parameters
     - (needs description)
   * - metric
     - any+not demons+any
     - mse
     - string
     - Cost function metric to be optimized.  
       The choices are {mse, mi, nmi, mattes, viola-wells} when impl=itk, 
       and {gm, mse, mi} when impl=plastimatch.
   * - mi_histogram_bins
     - any+any+any
     - 20
     - number of histogram bins
     - Only used for plastimatch mi metric, and itk mattes metric.
   * - min_its
     - any+any+any
     - 2
     - iterations
     - (needs description)
   * - min_step
     - any+{versor, rsg}+itk
     - 0.5
     - scaled parameters
     - (needs description)
   * - num_hist_levels_equal
     - vf+demons+itk
     - 1000
     - unsigned int
     - set number of histogram levels for histogram equalization
   * - num_matching_points
     - vf+demons+itk
     - 500
     - unsigned int
     - set number of histogram levels for histogram equalization
   * - num_samples
     - any+any+itk
     - -1
     - voxels
     - Number of voxels to randomly sample to score the cost function. 
       Only used for itk mattes metric.  If this parameter is not 
       specified, num_samples_pct will be used instead.
   * - num_samples_pct
     - any+any+itk
     - 0.3
     - percent
     - Percent of voxels to randomly sample to score the cost function. 
       Only used for itk mattes metric.
   * - num_substages
     - translation+grid_search +plastimatch
     - 1
     - stages
     - Number of times to refine the grid search.  By default, the 
       first search is global, and the subsequent searches refine the 
       result within a local region.
   * - optim_subtype
     - vf+demons+itk
     - fsf
     - string
     - Demons algorithm subtype used in ITK implementation.
       Values are {fsf(default), diffeomorphic, log_domain, sym_log_domain}.
   * - pgtol
     - any+{lbfgsb}+any
     - 1e-5
     - score per unit parameter
     - Projected gradient tolerance for LBFGSB optimizer.
       The optimizer can be asked to stop when the projected gradient
       is below this number.  The projected gradient is defined 
       as max{proj g_i | i = 1, ..., n} 
       where proj g_i is the ith component of the projected gradient.
   * - regularization
     - bspline+any+plastimatch
     - analytic
     - string
     - Implmentation variant for plastimatch B-spline regularization.
       Choices are { analytic, numeric, semi_analytic }.
   * - regularization_lambda
     - bspline+any+plastimatch
     - 0
     - unitless
     - Relative contribution of second derivative regularization 
       as compared to metric.  A typical value would range between 0.005 
       and 0.1.
   * - res
     -
     -
     -
     - Alias for "res_vox"
   * - res_mm
     - any+any+any
     - automatic
     - mm
     - Subsampling rate (in mm) for fixed and moving images.  
       This can be either "automatic", 
       a single integer (for isotropic subsampling), 
       or three integers (for anisotropic subsampling).
       For example, "3 3 3" would have voxels sampled once every 3 mm.
       In automatic mode, image is subsampled to the maximum rate 
       which yields less than 100 voxels in each dimension. 
   * - res_mm_fixed
     - any+any+any
     - automatic
     - mm
     - Equivalent to res_mm, but only applied to the fixed image.
   * - res_mm_moving
     - any+any+any
     - automatic
     - mm
     - Equivalent to res_mm, but only applied to the moving image.
   * - res_vox
     - any+any+any
     - automatic
     - voxels
     - Subsampling rate (in voxels) for fixed and moving images.  
       This can be either "automatic", 
       a single integer (for isotropic subsampling), 
       or three integers (for anisotropic subsampling).
       For example, "3 3 3" would have one voxel for
       every 3 voxels in the input image.
       In automatic mode, image is subsampled to the maximum rate 
       which yields less than 100 voxels in each dimension. 
   * - res_vox_fixed
     - any+any+any
     - automatic
     - voxels
     - Equivalent to res_vox, but only applied to the fixed image.
   * - res_vox_moving
     - any+any+any
     - automatic
     - voxels
     - Equivalent to res_vox, but only applied to the moving image.
   * - rsg_grad_tol
     - any+{rsg, versor}+itk
     - 0.001
     - score per unit parameter
     - Gradient magnitude tolerance for RSG and Versor optimizers.
       The optimizer can be asked to stop when the cost function is 
       in a stable region where the gradient magnitude is smaller 
       than this value.
   * - ss
     -
     -
     -
     - Alias for "res_vox"
   * - ss_fixed
     - any+any+any
     - automatic
     - voxels
     - Alias for "res_vox_fixed"
   * - ss_moving
     - any+any+any
     - automatic
     - voxels
     - Alias for "res_vox_moving"
   * - threading
     - any+any+plastimatch
     - openmp
     - string
     - Threading method used for parallel cost and gradient computations. 
       The choices are {cuda, opencl, openmp, single}.  
       If an unsupported threading choice is made (such as cuda with 
       demons), the nearest valid choice will be used.
   * - thresh_mean_intensity
     - vf+demons+itk
     - 0
     - boolean
     - Set the threshold at mean intensity flag. If true, only source 
       (reference) pixels which are greater than the mean source 
       (reference) intensity is used in the histogram matching. 
       If false, all pixels are used.
   * - translation_scale_factor
     - any+{rigid, affine}+itk
     - 1000
     - ratio
     - Sets the relative scale of translation when compared to 
       rotation, scaling, and shearing.
