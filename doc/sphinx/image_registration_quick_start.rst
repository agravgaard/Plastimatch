
If you want to register image_2.mha to match image_1.mha using 
B-spline registration, create a command file like this::

  # command_file.txt
  [GLOBAL]
  fixed=image_1.mha
  moving=image_2.mha
  img_out=warped_2.mha
  xform_out=bspline_coefficients.txt

  [STAGE]
  xform=bspline
  impl=plastimatch
  threading=openmp
  max_its=30
  regularization_lambda=0.005
  grid_spac=100 100 100
  res=4 4 2

Then, run the registration like this::

  plastimatch register command_file.txt

The above example only performs a single registration stage.  If you 
want to do multi-stage registration, use multiple [STAGE] sections.  
Like this::

  # command_file.txt
  [GLOBAL]
  fixed=image_1.mha
  moving=image_2.mha
  img_out=warped_2.mha
  xform_out=bspline_coefficients.txt

  [STAGE]
  xform=bspline
  impl=plastimatch
  threading=openmp
  max_its=30
  regularization_lambda=0.005
  grid_spac=100 100 100
  res=4 4 2

  [STAGE]
  max_its=30
  grid_spac=80 80 80
  res=2 2 1

  [STAGE]
  max_its=30
  grid_spac=60 60 60
  res=1 1 1

