.. _image_registration_guidebook:

Image registration guidebook
============================

Quick start guide
-----------------

You must create a command file to do registration.  

.. include:: image_registration_quick_start.rst

For more examples, read on!

3-DOF registration (translation)
--------------------------------
Sometimes it is convenient to register only with translations.  
You can do this with the following example::

  # command_file.txt
  [GLOBAL]
  fixed=image_1.mha
  moving=image_2.mha
  img_out=warped_2.mha

  [STAGE]
  xform=translation
  optim=rsg
  max_its=30
  res=4 4 2

6-DOF registration (rigid)
--------------------------
The following example performs a rigid registration::

  # command_file.txt
  [GLOBAL]
  fixed=image_1.mha
  moving=image_2.mha
  img_out=warped_2.mha

  [STAGE]
  xform=rigid
  optim=versor
  max_its=30
  res=4 4 2

12-DOF registration (affine)
----------------------------
The following example performs an affine registration::

  # command_file.txt
  [GLOBAL]
  fixed=image_1.mha
  moving=image_2.mha
  img_out=warped_2.mha
  xform_out=affine_coefficients.txt

  [STAGE]
  xform=affine
  optim=rsg
  max_its=30
  res=4 4 2

Demons registration
-------------------
The following example performs a demons registration::

  # command_file.txt
  [GLOBAL]
  fixed=image_1.mha
  moving=image_2.mha
  img_out=warped_2.mha
  xform_out=demons_vf.mha

  [STAGE]
  xform=vf
  optim=demons
  max_its=30
  res=4 4 2

The demons code has several parameters which can be optimized.
The following example illustrates their use::

  [STAGE]
  xform=vf
  optim=demons
  max_its=200
  res=4 4 2
  # Std dev of smoothing kernel (in mm)
  demons_std=10
  # "Gain" factor, higher gains are faster but less robust
  demons_acceleration=5
  # Homogenezation is the tradeoff between gradient 
  # and image difference.  Values should increase for larger 
  # voxel sizes, going down to about 1 for 1mm voxels.
  demons_homogenization=30
  # This is the size of the filter (in voxels)
  demons_filter_width=5 5 5
  
  
Currently there are also 4 different versions of itk demons available in plastimatch:

- fast symmetric forces (=fsf) demons 
- difffeomorphic demons
- log-domain demons(~diffeopmorphic demons with directly accessible inverted deformation field)
- symmetric log domain diffeomorphic demons (symetric log-domain demons computing forward and backward update) 

More information on these demons variants can be found here: http://hdl.handle.net/10380/3060


The following example illustrates the use of the ITK demons::

  [STAGE]
  xform=vf
  impl=itk
  optim=demons
  # Demons implementation that should be used: 
  # fsf(default),diffeomorphic,log_domain,sym_log_domain
  optim_subtype=sym_log_domain
 
  # Type of gradient that will be used to compute update force: 
  # symmetric(compute gradient in fixed and moving image),fixed,
  # warped_moving,mapped_moving(=not warped moving image) 
  demons_gradient_type=symmetric
  
  # Set whether the update field is smoothed
  # (regularized). Smoothing the update field yields a solution
  # viscous in nature. If demons_smooth_update_field is on, then the
  # update field is smoothed with a Gaussian whose standard
  # deviations are specified with demons_std_update_field
  demons_smooth_update_field=0
  demons_std_update_field=1
  
  #Set whether the deformation field should be smoothed
  #(regularized). Smoothing the deformation field yields a solution
  #elastic in nature. If demons_smooth_deformation_field is on, then the
  #deformation field is smoothed with a Gaussian whose standard
  #deviations are specified with demons_std_deformation_field
  demons_smooth_deformation_field=1
  demons_std_deformation_field=2

  #Set the maximum update step length. In Thirion this is 0.5.
  #Setting it to 0 implies no restriction (beware of numerical
  #instability in this case. 
  demons_step_length=1;

  #Flag to turn on/off histogram equalization
  histo_equ=0
  #Number of histogram levels used for histogram equalization
  num_hist_levels=100
  #Number of matching points used for histogram equalization
  num_matching_points=10
  #Flag to turn on /off threshold at mean intensity. If true, only source (reference) pixels which are
  #greater than the mean source (reference) intensity is used in the histogram matching. If false, all 
  #pixels are used.
  thresh_mean_intensity=1

  max_its=30
  res=4 4 2


The ITK demons implementation is also capable of using fixed and moving image masks. 
When using masks the deformation field will only be updated for the region where the masks overlap.
If only one mask is used the calculation of the update field is restricted to this region. 
More information on ITK demons registration with masks can be found here:  http://hdl.handle.net/10380/3105


B-spline registration
---------------------
The following example performs a B-spline registration::

  # command_file.txt
  [GLOBAL]
  fixed=image_1.mha
  moving=image_2.mha
  img_out=warped_2.mha
  xform_out=bspline_coefficients.txt

  [STAGE]
  xform=bspline
  optim=lbfgsb
  # Run for at most 30 iterations
  max_its=30
  # Subsample the image 4x4x2 voxels
  res=4 4 2
  # B-spline grid spacing (in mm)
  grid_spac=30 30 30
  # Smoothness term
  regularization_lambda=0.005

Just like demons, b-spline has several options.  The most important ones 
are shown above:
res is used to subsample both input volumes prior to running the registration; 
max_its is used to determine how many iterations to run; 
grid_spac defines how far apart the control points are spaced; 
and regularization_lambda is used to increase the smoothness of the 
registration. 
The following example illustrates some additional options::

  [STAGE]
  xform=bspline
  optim=lbfgsb
  # Run for at most 30 iterations
  max_its=30
  # Subsample the image 4x4x2 voxels
  res=4 4 2
  # B-spline grid spacing (in mm)
  grid_spac=30 30 30
  # Smoothness term
  regularization_lambda=0.005
  # Quit if change in score differs by less than 3
  convergence_tol=3
  # Quit if gradient norm is less than 0.1
  grad_tol=0.1

Using ITK algorithms
--------------------
The default is to use plastimatch native implementations where available.  
When a native implementation is not available, the ITK implementation is used.
Native implementations are available for demons and bspline methods.  

If you want to use an ITK method, you can use the "impl=itk" parameter.
For example, the following command file will use the ITK demons 
implementation::

  # command_file.txt
  [GLOBAL]
  fixed=image_1.mha
  moving=image_2.mha
  img_out=warped_2.mha
  xform_out=bspline_coefficients.txt

  [STAGE]
  xform=vf
  optim=demons
  impl=itk
  max_its=30
  res=4 4 2


Mutual information
------------------
The default metric is mean squared error, which is useful for 
registration of CT with CT.  For other registration problems, mutual 
information is better.  The following example uses the Mattes 
mutual information metric with the B-spline transform::

  # command_file.txt
  [GLOBAL]
  fixed=image_1.mha
  moving=image_2.mha
  img_out=warped_2.mha
  xform_out=bspline_coefficients.txt

  [STAGE]
  xform=bspline
  impl=plastimatch
  regularization_lambda=0.005
  metric=mi
  max_its=30
  res=4 4 2

Image masking
-------------
Not all algorithms support masking.  But for those that do, you can 
specify a mask image for either the fixed image, the moving image, 
or both.  The mask image must be the same size as the input image.
When a mask image is used, only voxels of the moving or fixed image 
which have non-zero corresponding voxels in their mask image 
will contribute to the matching score.  Masks are specified as follows::

  [GLOBAL]
  fixed=image_1.mha
  moving=image_2.mha
  fixed_mask=image_1_mask.mha
  moving_mask=image_2_mask.mha

At this time, only the plastimatch B-spline transform 
with mutual information cost function supports masking.

Output options
--------------
Outputs can be generated at the end of the registration, by putting 
the appropriate file names in the "[GLOBAL]" section.  The 
file formats of the output files are selected automatically based 
on the file extension.  

In addition to generating files at the end of registration, intermediate 
results can be generated at the end of each stage.  The following 
example shows the range of output files which can be created::

  [GLOBAL]
  # These are the inputs
  fixed=t0p_221.mha
  moving=t5p_221.mha
  xform_in=my_bsp.txt

  # These are the final outputs.  They will be rendered at full resolution.
  vf_out=my_output_vf.mha
  xform_out=my_output_bsp.txt
  img_out=my_output_img.mha

  [STAGE]
  xform=rigid
  max_its=20
  res=4 4 2

  # These are the outputs from the first stage
  xform_out=stage_1_rigid.txt
  vf_out=stage_1_rigid.mha
  img_out=stage_1_img.mha

  [STAGE]
  xform=vf
  optim=demons
  res=2 2 1

  # These are the outputs from the second stage.
  # They will be similar to the final outputs, but at lower resolution.  
  # The resolution of the stage outputs match the resolution of the stage.
  vf_out=stage_1_rigid.mha
  img_out=stage_1_img.mha

