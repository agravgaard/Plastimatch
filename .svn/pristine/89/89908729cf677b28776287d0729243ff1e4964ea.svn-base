########################################################################
## This is a test file for the Plastimatch Python wrapper
## Usage: python pypla_test.py
## Author: Paolo Zaffino  (p.zaffino@unicz.it)
## Rev 4
## NOT TESTED ON PYTHON 3
########################################################################

import plastimatch as plm

## EXAMPLE TO ADD TWO (OR MORE) IMAGES
sum=plm.add() #Another way to create this object is: sum=plm.add(log_file='add_log.txt')
sum.log_file='add_log.txt' ## Log file is not indispensable
sum.input_files=['img1.mha','img2.mha'] ## You must define at least two images
sum.output_file='sum_img.mha'
sum.run_add()

## EXAMPLE TO CROP AN IMAGE
crop=plm.crop() #Another way to create this object is: crop=plm.crop(log_file='crop_log.txt')
crop.log_file='crop_log.txt' ## Log file is not indispensable
crop.option['input']='img.mha'
crop.option['voxels']='0 511 0 511 20 50'
crop.option['output']='img_crop.mha'
crop.run_crop()

## EXAMPLE TO CONVERT AN IMAGE
conv=plm.convert() ## Another way to create this object is: conv=plm.convert(log_file='conv_log.txt')
conv.log_file='convert_log.txt' ## Log file is not indispensable
conv.option['input']='dicom_dir'
conv.option['output-img']='img_from_dicom.mha'
conv.run_convert()

## EXAMPLE TO RUN A DICE STATS ON TWO STRUCTURES
dice=plm.dice() #Another way to create this object is: dice=plm.dice(log_file='dice_log.txt')
dice.log_file='dice_log.txt' ## Log file is required
dice.input_files=['reference_stru.mha','test_stru.mha'] ## You must define two structures of the same dimension
dice.run_dice()

## EXAMPLE TO SUBTRACT TWO IMAGES
sub=plm.diff() #Another way to create this object is: sub=plm.diff(log_file='diff_log.txt')
sub.log_file='diff_log.txt' ## Log file is not indispensable
sub.input_files=['img1.mha','img2.mha'] ## You must define two images of the same dimension
sub.output_file='sub_img.mha'
sub.run_diff()

## EXAMPLE TO FILL AN IMAGE
fill=plm.fill() #Another way to create this object is: fill=plm.fill(log_file='fill_log.txt')
fill.log_file='fill_log.txt' ## Log file is not indispensable
fill.option['input']='fix.mha'
fill.option['mask']='mask_fix.mha'
fill.option['mask-value']='-1200'
fill.option['output']='out.mha'
fill.run_fill()

## EXAMPLE TO MASK AN IMAGE
mask=plm.mask() #Another way to create this object is: mask=plm.mask(log_file='mask_log.txt')
mask.log_file='mask_log.txt' ## Log file is not indispensable
mask.option['input']='img.mha'
mask.option['mask']='mask.mha'
mask.option['mask-value']='-1000'
mask.option['output']='out.mha'
mask.run_mask()

## EXAMPLE TO REGISTER AN IMAGE
reg=plm.register()## Another way to create this object is: reg=plm.register(par_file='par.txt', log_file='reg_log.txt')
reg.par_file='par.txt' ## Parameters file is indispensable, it could be also a inexistent file (it will be created)
reg.log_file='reg_log.txt' ## Log file is not indispensable
reg.add_global_stage() ## This section is needed only if the paramaters file does not exist - START - 
reg.stages[0]['fixed']='fix.mha'
reg.stages[0]['moving']='mov.mha'
reg.stages[0]['img_out']='out_img.mha'
reg.add_stage()
reg.stages[1]['xform']='rigid'
reg.stages[1]['optim']='versor'
reg.stages[1]['impl']='itk'
reg.stages[1]['metric']='mse'
reg.stages[1]['max_its']='100'
reg.stages[1]['convergence_tol']='3'
reg.stages[1]['grad_tol']='1'
reg.stages[1]['res']='2 2 2'
reg.add_stage()
reg.stages[2]['xform']='bspline'
reg.stages[2]['optim']='lbfgsb'
reg.stages[2]['impl']='plastimatch'
reg.stages[2]['metric']='mse'
reg.stages[2]['max_its']='100'
reg.stages[2]['grid_spac']='33 35 35'
reg.stages[2]['res']='1 1 1'
reg.add_stage()
reg.stages[3]['xform']='bspline'
reg.stages[3]['optim']='lbfgsb'
reg.stages[3]['impl']='plastimatch'
reg.stages[3]['metric']='mse'
reg.stages[3]['max_its']='100'
reg.stages[3]['grid_spac']='10 10 10'
reg.stages[3]['res']='1 1 1' ## This section is needed only if the paramaters file does not exist - END -
reg.delete_stage(3) ## Deletes the last stage
reg.run_registration()

## EXAMPLE TO RESAMPLE AN IMAGE
res=plm.resample() #Another way to create this object is: res=plm.resample(log_file='res_log.txt')
res.log_file='res_log.txt' ## Log file is not indispensable
res.option['input']='img1.mha'
res.option['fixed']='img2.mha'
res.option['output']='res_img.mha'
res.run_resample()

## EXAMPLE TO SEGMENT AN IMAGE
seg=plm.segment() #Another way to create this object is: seg=plm.segment(log_file='segment_log.txt')
seg.log_file='segmentation_log.txt' ## Log file is not indispensable
seg.option['input']='fix.mha'
seg.option['fast']='Enabled'
seg.option['output-img']='mask.mha'
seg.run_segment()

## EXAMPLE TO WARP AN IMAGE
warp=plm.warp() ## Another way to create this object is: warp=plm.warp(log_file='warp_log.txt')
warp.log_file='warp_log.txt' ## Log file is not indispensable
warp.option['input']='fix.mha'
warp.option['output-img']='out.mha'
warp.option['xf']='vf.mha'
warp.run_warp()

## EXAMPLE TO RUN A XF-CONVERT COMMAND
xfconvert=plm.xfconvert() ## Another way to create this object is: xfconvert=plm.xfconvert(log_file='xfconvert_log.txt')
xfconvert.log_file='xfconvert_log.txt' ## Log file is not indispensable
xfconvert.option['input']='in.mha'
xfconvert.option['output']='out.txt'
xfconvert.option['dim']='512 512 80'
xfconvert.option['output-type']='bspline'
xfconvert.run_xfconvert()
