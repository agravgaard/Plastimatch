landmark_warp
=============

Synopsis
--------

``landmark_warp [options]``

Description
-----------
The landmark_warp executable performs registration by matching 
fiducials on reference and test images. 
The list of possible options can be seen by typing::

  landmark_warp --help

The command line usage is given as follows::

 Usage: landmark_warp [options]
 Options:
  -a, --algorithm <arg>         RBF warping algorithm {tps,gauss, 
                                 wendland} 
  -d, --default-value <arg>     Value to set for pixels with unknown 
                                 value 
      --dim <arg>               Size of output image 
                                 in voxels "x [y z]" 
  -F, --fixed <arg>             Fixed image (match output size to this 
      	      			 image) 
  -f, --fixed-landmarks <arg>   Input fixed landmarks 
  -h, --help                    Display this help message 
  -I, --input-image <arg>       Input image to warp 
  -v, --input-vf <arg>          Input vector field (applied prior to 
                                 landmark warping) 
  -m, --moving-landmarks <arg>   
                                Output moving landmarks 
  -N, --numclusters <arg>       Number of clusters of landmarks 
      --origin <arg>            Location of first image voxel 
                                 in mm "x y z" 
  -O, --output-image <arg>      Output warped image 
  -L, --output-landmarks <arg>   
                                Output warped landmarks 
  -V, --output-vf <arg>         Output vector field 
  -r, --radius <arg>            Radius of radial basis function 
                                 (in mm)
      --spacing <arg>           Voxel spacing in mm "x [y z]" 
  -Y, --stiffness <arg>         Young modulus (default = 0.0) 

Options "-a", "-r", "-Y", "-d" are set by default to::

      -a=gauss		Gaussian RBFs with infinite support
      -r=50.0		Gaussian width 50 mm
      -Y=0.0		No regularization of vector field
      -d=-1000		Air

You may want to choose different algorithm::

      -a=tps		Thin-plate splines (for global registration)
      -a=wendland	Wendland RBFs with compact support (for 
                         local registration)

In the case of Wendland RBFs "-r" option sets the radius of support.

Regularization of vector field is available for "gauss"  and "wendland" algorithms. To regularize the output vector field increase "-Y" to '0.1' and up with increment '0.1'.
	




