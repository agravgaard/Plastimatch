"""
This is the default viewer for the pypla project (a subproject of Plastimatch).
It can show a single image or an image overlayed on another one
(in the checkerboard mode, diff mode, color mode or trasparency mode).
A vector field and/or binary structures (maximum two) can be also added.

FUNCTION STILL IN TESTING

Author: Paolo Zaffino (p.zaffino@unicz.it)

rev 7

Required libraries:
1) numpy (http://numpy.scipy.org)
2) matplotlib (http://matplotlib.org)
3) mha.py file (another file of the Pypla project)

NOT TESTED ON PYTHON 3

USAGE EXAMPLES:

import pypla_viewer as pv
pv.show_img(img_in='foo.mha')

	OR

import mha
import pypla_viewer as pv
foo_obj=mha.new(input_file='foo.mha')
pv.show_img(img_in=foo_obj)

BRIEF INPUTS EXPLANATION:

img_in = Input image, it can be a mha file or a mha object (from mha.py)
slice_n = Slice number, default slice is the middle one
view = View, default is coronal, choices='a','c' or 's'
overlay_img = Overlay image, it can be a mha file or a mha object (from mha.py)
overlay_img_gain = Gain of overlay image
overlay_img_trasparency = Image trasparency of the overlap image (0.0 transparent, 1.0 opaque)
img_windowing = Windowing interval as "-100 100" (for the main image)
overlay_img_windowing = Windowing interval for the overlay image
vf = Input vector field in order to plot the phase map, it can be a mha file or a mha object (from mha.py)
vf_trasparency = Vector field trasparency in overlap mode (0.0 transparent, 1.0 opaque)
structure = Binary structure file name, it can be a mha file or a mha object (from mha.py)
structure_color = Structure 1 color, default is 'red', choise='red','green','blue' or 'yellow'
structure_alpha = Structure 1 alpha value, default is 1.0
structure2 = Binary structure file name, it can be a mha file or a mha object (from mha.py)
structure2_color = Structure 2 color, default is 'blue', choise='red','green','blue' or 'yellow'
structure2_alpha = Structure 2 alpha value, default is 0.8
checkerboard = Checkerboard mode, to enable set on True
check-size = Size of the check (in voxel), default is 100
diff = Shows the difference between the input image and the overlay one. To enable set on True
colors = Allows the color in the overlay mode. To enable set on True
axes = Show the axes. To enable set on True
screenshot_filename = file name where will be saved the screenshot
"""


########################################################################


from copy import deepcopy
import matplotlib.cm as cm
from matplotlib.pyplot import axis, gcf, hold, imshow, savefig, show
import mha
import numpy as np
import types
import warnings


########################################################################

warnings.filterwarnings(action='ignore', module='numpy')

def show_img(img_in='', slice_n=None, view='c', overlay_img=None, overlay_img_gain=1, overlay_img_trasparency=0.5,
img_windowing='', overlay_img_windowing='', vf=None, vf_trasparency=0.5, structure=None, structure_color='red',
structure_alpha=1.0, structure2=None, structure2_color='blue', structure2_alpha=0.8, checkerboard=False,
check_size=100, diff=False, colors=False, axes=False, screenshot_filename=None):
	
	"""
	This is the function that shows the images/vf/structure
	"""
	
	## Read the input image
	
	img=_scan_input(img_in)
	figure_info='Basic img'
		
	## Read the overlay image
	if overlay_img != None:
		img2=_scan_input(overlay_img)
		_check_data_parameters(img, img2, 'overlay image')
		figure_info+=' + overlay img'
		
	## Read the vector field
	if vf != None:
		vf_data=_scan_input(vf)
		_check_data_parameters(img, vf_data, 'vector field')
		figure_info+=' + vf'
			
	## Read the structure
	if structure != None:
		stru=_scan_input(structure)
		_check_data_parameters(img, stru, 'structure')
		figure_info+=' + first stru'	

	## Read the structure 2
	if structure2 != None:
		stru2=_scan_input(structure2)
		_check_data_parameters(img, stru2, 'structure 2')
		figure_info+=' + second stru'
	
	## Slice number settings
	if slice_n == None and (view == 'c' or view == 's'):
		slice_number=np.rint(img.size[1]/2)
	elif slice_n == None and view == 'a':
		slice_number=np.rint(img.size[2]/2)
	else:
		slice_number=deepcopy(slice_n)
	figure_info+= ' -- slice ' + str(int(slice_number))
	
	## View settings
	if view == 'c':
		slice_img=img.data[:,slice_number].T
		pixel_ratio=img.spacing[2]/img.spacing[0]
		if overlay_img != None:
			slice2_img=img2.data[:,slice_number].T
		if vf != None:
			phase_vf=np.arctan(vf_data.data[:,slice_number,:,0], vf_data.data[:,slice_number,:,2]).T
		if structure != None:
			slice_stru=stru.data[:,slice_number].T
		if structure2 != None:
			slice_stru2=stru2.data[:,slice_number].T
		
	elif view == 's':
		slice_img=img.data[slice_number,:].T
		pixel_ratio=img.spacing[2]/img.spacing[1]
		if overlay_img != None:
			slice2_img=img2.data[slice_number,:].T
		if vf != None:
			phase_vf=np.arctan(vf_data.data[slice_number,:,:,1], vf_data.data[slice_number,:,:,2]).T
		if structure != None:
			slice_stru=stru.data[slice_number,:].T
		if structure2 != None:
			slice_stru2=stru2.data[slice_number,:].T
		
	elif view == 'a':
		slice_img=np.rot90(img.data[:,:,slice_number],1)
		pixel_ratio=img.spacing[1]/img.spacing[0]
		if overlay_img != None:
			slice2_img=np.rot90(img2.data[:,:,slice_number],1)
		if vf != None:
			phase_vf=np.rot90(np.arctan(vf_data.data[:,:,slice_number,0], vf_data.data[:,:,slice_number,1]), 1)
		if structure != None:
			slice_stru=np.rot90(stru.data[:,:,slice_number],1)
		if structure2 != None:
			slice_stru2=np.rot90(stru2.data[:,:,slice_number],1)
			
	if structure != None:
		slice_stru = np.ma.masked_where(slice_stru == 0, slice_stru)
		
	if structure2 != None:
		slice_stru2 = np.ma.masked_where(slice_stru2 == 0, slice_stru2)
		
	if type(vf)==str:
		del vf_data.data
	
	if type(img_in)==str:
		del img.data
		
	if type(overlay_img)==str:
		del img2.data
		
	if type(structure)==str:
		del stru.data
		
	if type(structure2)==str:
		del stru2.data
	
	## Windowing settings
	if img_windowing != '':
		windowing_low_value=int(img_windowing.split(' ')[0])
		windowing_hi_value=int(img_windowing.split(' ')[1])
		slice_img = _windowing_img(slice_img, windowing_low_value, windowing_hi_value)
	
	if overlay_img_windowing != '' and overlay_img != None:
		slice2_img = _windowing_img(slice2_img, windowing_low_value, windowing_hi_value)
	
	## Structure colormap
	stru_colormap=_set_colormap(structure_color, 1)
	
	## Structure 2 colormap
	stru2_colormap=_set_colormap(structure2_color, 2)
	
	## Show options
	if diff == False and checkerboard == False: ## NO options
		imshow(slice_img, cmap=cm.gray, aspect=pixel_ratio, origin='lower')
		
		if colors == True and overlay_img != None and vf == None and checkerboard == False: ## Plus overlay images in color mode (NO checkerboard, NO vf)
			imshow(slice2_img, aspect=pixel_ratio, origin='lower', alpha=overlay_img_trasparency)
			
		elif colors == False and overlay_img == None and vf != None and checkerboard == False: ## Plus vector field
			imshow(phase_vf, aspect=pixel_ratio, origin='lower', alpha=vf_trasparency)

	elif diff == False and colors == False and overlay_img != None and vf == None and checkerboard == False: ## Overlay images (NO checkerboard, NO color, NO vf)
		slice_sum=np.add(np.multiply(slice2_img, overlay_img_gain), slice_img)
		del slice_img, slice2_img
		imshow(slice_sum, cmap=cm.gray, aspect=pixel_ratio, origin='lower')

	elif diff == True and colors == False and overlay_img != None and vf == None and checkerboard == False: ## Two images in diff mode
		diff_slice=np.subtract(slice_img, np.multiply(slice2_img, overlay_img_gain))
		del slice_img, slice2_img
		imshow(diff_slice, cmap=cm.gray, aspect=pixel_ratio, origin='lower')
	
	elif diff == False and overlay_img != None and checkerboard == True: ## Overlay images in checkerboard mode		
		slice_img, slice2_img = _img_for_checkerboard(slice_img, slice2_img, check_size, pixel_ratio)
		
		if colors == True: ## Checkerboad color mode
			imshow(slice_img, cmap=cm.get_cmap('bone'), aspect=pixel_ratio, origin='lower')
			imshow(slice2_img, cmap=cm.get_cmap('reds'), aspect=pixel_ratio, origin='lower')
		elif colors == False: ## Checkerboad, NO color mode
			imshow(slice_img, cmap=cm.gray, aspect=pixel_ratio, origin='lower')
			imshow(slice2_img, cmap=cm.gray, aspect=pixel_ratio, origin='lower')
			if vf != None: ## Checkerboad, NO color mode + vf
				imshow(phase_vf, aspect=pixel_ratio, origin='lower', alpha=vf_trasparency)
	else:
		raise NameError('Unrecognized view options combination!')
				
	if structure != None: ## Plus structure
		imshow(slice_stru, cmap=stru_colormap, aspect=pixel_ratio, origin='lower', interpolation="nearest", alpha=structure_alpha)
	
	if structure2 != None: ## Plus structure 2
		imshow(slice_stru2, cmap=stru2_colormap, aspect=pixel_ratio, origin='lower', interpolation="nearest", alpha=structure2_alpha)
	
	## Print the result on the screen or into a file
	if axes == False:
		axis('off')
	
	if screenshot_filename==None:
		gcf().canvas.set_window_title(figure_info)
		show()
	else:
		savefig(screenshot_filename, bbox_inches="tight", transparent = True)
		print ("Screenshot saved into the file: " + screenshot_filename)


########################################################################
######## Private utility function, NOT FOR PUBLIC USE - START - ########
########################################################################


def _scan_input(input_par):
	
	"""
	This private function scans the input data parameter.
	If it is a file name (string) it returns the mha object,
	else if it is already a mha object it returns the unchanged input parameter
	"""
	
	if type(input_par)==str: return mha.new(input_file=input_par)
	elif type(input_par)==types.InstanceType: return input_par


def _check_data_parameters(im1, im2, im_type):
	
	"""
	This private function checks if the second image (or vf or structure) has the same dimension, spacing and offset
	of the main input image
	"""
	
	if (im1.size != im2.size):
		raise NameError("The " + im_type + " doesn't have the same dimensions of the input image!")
	if (im1.spacing != im2.spacing):
		raise NameError("The " + im_type + " doesn't have the same spacing of the input image!")
	if (im1.offset != im2.offset):
		print "Warning: the " + im_type + " doesn't have the same offset of the input image!"
	if (im1.direction_cosines != im2.direction_cosines):
		print "Warning: the " + im_type + " doesn't have the same direction cosines of the input image!"

def _set_colormap(cm_in, stru_number):
	
	"""
	This private function sets the chosen colormap
	"""
	
	if cm_in == "red": return cm.autumn
	elif cm_in == "blue": return cm.winter
	elif cm_in == "yellow": return cm.autumn_r
	elif cm_in == "green": return cm.brg_r
	else: raise NameError('Unknown structure ' + str(stru_number) +' colormap')


def _windowing_img (img, low_threshold, hi_threshold):
	
	"""
	This private function sets a low and a high threshold on the image voxels
	"""
	
	img[img < low_threshold]=0
	img[img > hi_threshold]=0
	
	return img


def _img_for_checkerboard(slice_img, slice2_img, check_size, pixel_ratio):

	"""
	This private function builds the two images for the checkerboard mode
	"""
	
	check_white=np.ones((np.rint(check_size/pixel_ratio), check_size))
	check_black=np.zeros((np.rint(check_size/pixel_ratio), check_size))
	check_number_x=slice_img.shape[1]/check_size
	check_number_y=slice_img.shape[0]/np.rint(check_size/pixel_ratio)
	
	x=y=0
	while x <= check_number_x:
		if x == 0:
			row=np.concatenate((check_white, check_black), axis=1)
			row_neg=np.concatenate((check_black, check_white), axis=1)
		else:
			row=np.concatenate((row, check_white), axis=1)
			row=np.concatenate((row, check_black), axis=1)
		x=x+2
	
	row_neg=np.ones(row.shape)
	row_neg=np.subtract(row_neg, row)
	
	while y <= check_number_y:
		if y == 0:
			checkerboard=np.concatenate((row, row_neg), axis=0)
		else:
			checkerboard=np.concatenate((checkerboard, row), axis=0)
			checkerboard=np.concatenate((checkerboard, row_neg), axis=0)
		y=y+2
	
	checkerboard=np.delete(checkerboard, np.s_[slice_img.shape[0]:checkerboard.shape[0]], axis=0)
	checkerboard=np.delete(checkerboard, np.s_[slice_img.shape[1]:checkerboard.shape[1]], axis=1)
	
	checkerboard_neg=np.ones(checkerboard.shape)
	checkerboard_neg=np.subtract(checkerboard_neg, checkerboard)
	checkerboard=np.ma.masked_where(checkerboard == 0, checkerboard)
	checkerboard_neg=np.ma.masked_where(checkerboard_neg == 0, checkerboard_neg)
		
	slice_img=np.multiply(slice_img, checkerboard)
	slice2_img=np.multiply(slice2_img, checkerboard_neg)
	
	return slice_img, slice2_img

########################################################################
######### Private utility function, NOT FOR PUBLIC USE - END - #########
########################################################################
