########################################################################
## This is a Python wrapper for Plastimatch
## Author: Paolo Zaffino  (p.zaffino@unicz.it)
## Rev 4
## NOT TESTED ON PYTHON 3
########################################################################

import os
import subprocess

####################### PUBLIC CLASSES - START - #######################


print ("WARNING: Plastimatch Python wrapper is still in alpha version!")


class add:
	
	def __init__ (self, log_file=""):
		self.log_file=log_file
	
	input_files=[]
	output_file=""
	
	def run_add(self):
		
		if len(self.input_files) < 2:
			raise NameError("You must define at least two input images!")
		
		if self.log_file == "":
			self.log=open(os.devnull, "w")
		else:
			self.log=open(self.log_file, "w")
		
		input_parms=""
		
		for file_name in self.input_files:
			input_parms+=str(file_name) + " "
		
		subprocess.call("plastimatch add " + input_parms + str(self.output_file),\
		shell=True, stdout=self.log, stderr=self.log)
		
		self.log.close()




class adjust:
	
	def __init__ (self, log_file=""):
		self.log_file=log_file
	
	option={}
	
	_adjust_keys=("input", "output", "output-type", "scale", "ab-scale",\
	"stretch", "truncate-above", "truncate-below")
	
	def run_adjust(self):
		_run_plm_command("adjust", self.option, self._adjust_keys, self.log_file)




class convert:
	
	def __init__ (self, log_file=""):
		self.log_file=log_file
	
	option={}
	
	_convert_keys=("input","default-val","dif","dim","fixed",\
	"input-cxt","input-dose-ast","input-dose-img","input-dose-mc",\
	"input-dose-xio","input-ss-img","input-ss-list","interpolation",\
	"metadata","origin","output-color_map","output-ctx","output-dicom",\
	"output-dij","output-dose_img","output-img","output-labelmap",\
	"output-pointset","output-prefix","output-prefix_fcsv","output-ss_img",\
	"output-ss_list","output-type","output-vf","output-xio","patient-id",\
	"patient-name","patient-pos","prune-empty","referenced-ct","simplify-perc",\
	"spacing","xf","xor-contours")
	
	def run_convert(self):
		_run_plm_command("convert", self.option, self._convert_keys, self.log_file)




class crop:
	
	def __init__ (self, log_file=""):
		self.log_file=log_file
		
	option={}
	
	_crop_keys=("input", "output", "voxels")
	
	def run_crop(self):
		_run_plm_command("crop", self.option, self._crop_keys, self.log_file)




class dice:
	
	def __init__ (self, log_file=""):
		self.log_file=log_file
	
	input_files=[]
	
	def run_dice(self):
		
		if len(self.input_files) != 2:
			raise NameError("You must define two input structures!")
		
		if self.log_file == "":
			raise NameError("You must define a log file!")
		else:
			self.log=open(self.log_file, "w")
		
		subprocess.call("plastimatch dice " + str(self.input_files[0]) + " " + str(self.input_files[1]), \
		shell=True, stdout=self.log, stderr=self.log)
		
		self.log.close()




class diff:
	
	def __init__ (self, log_file=""):
		self.log_file=log_file
	
	input_files=[]
	output_file=""
	
	def run_diff(self):
		
		if len(self.input_files) != 2:
			raise NameError("You must define two input images!")
		
		if self.log_file == "":
			self.log=open(os.devnull, "w")
		else:
			self.log=open(self.log_file, "w")
		
		subprocess.call("plastimatch diff " + str(self.input_files[0]) + " " + str(self.input_files[1])\
		+ " " + str(self.output_file), shell=True, stdout=self.log, stderr=self.log)
		
		self.log.close()




class fill:
	
	def __init__ (self, log_file=""):
		self.log_file=log_file
	
	option={}
	
	_fill_keys=("input", "mask", "mask-value", "output", "output-format", "output-type")
	
	def run_fill(self):
		_run_plm_command("fill", self.option, self._fill_keys, self.log_file)




class mask:
	
	def __init__ (self, log_file=""):
		self.log_file=log_file
	
	option={}
	
	_mask_keys=("input", "mask", "mask-value", "output", "output-format", "output-type")
	
	def run_mask(self):
		_run_plm_command("mask", self.option, self._mask_keys, self.log_file)




class register:
	
	def __init__ (self, par_file="", log_file=""):
		self.par_file=par_file
		self.log_file=log_file
	
	_stage_keys=("xform","optim","impl","background_val","convergence_tol",\
	"demons_acceleration","demons_filter_width","demons_homogenization","demons_std",\
	"histoeq","grad_tol","grid_spac","max_its","max_step","metric","mi_histogram_bins",\
	"min_its","min_step","num_saples","regularization_lambda","res","ss","ss_fixed",\
	"ss_moving","threading","xform_in","xform_out","vf_out","img_out","img_out_fmt","img_out_type")
	
	_global_keys=("fixed","moving","xform_in","xform_out","vf_out","img_out",\
	"img_out_fmt","img_out_type","background_max")
	
	stages=[]
	_global_stage_added=False
	
	def add_global_stage(self):
		if self.par_file=="" or os.path.exists(self.par_file):
			raise NameError("GLOBAL STAGE NOT ADDED! You have to define a new parameters file name")
		else:
			if self._global_stage_added==False:
				self.stages=[{}]+self.stages
				self._global_stage_added=True
			else:
				raise NameError("The global stage already exists!")
	
	def add_stage(self):
		if self.par_file=="" or os.path.exists(self.par_file):
			raise NameError("STAGE NOT ADDED! You have to define a new parameters file name")
		else:
			self.stages+=[{}]
	
	def delete_stage(self, stage_number):
		if self.par_file=="" or os.path.exists(self.par_file):
			raise NameError("STAGE NOT DELETED! You have to define a new parameters file name")
		else:
			if stage_number != 0:
				del self.stages[stage_number]
			else:
				raise NameError("GLOBAL STAGE NOT DELETED! You can not delete the global stage")
	
	def run_registration(self):
		if not os.path.exists(self.par_file) and self.par_file!="":
			f=open(self.par_file, "w")			
			for stage_index, stage in enumerate(self.stages):
				if stage_index==0:
					stage=_clean_parms(stage, self._global_keys)
					f.write("[GLOBAL]\n")
				else:
					stage=_clean_parms(stage, self._stage_keys)
					f.write("\n[STAGE]\n")
				
				for key, value in dict.items(stage):
						f.write(key+"="+value+"\n")	
			f.close()
		
		if self.log_file == "":
			self.log=open(os.devnull, "w")
		else:
			self.log=open(self.log_file, "w")
		
		if self.par_file!="" and os.path.exists(self.par_file):
			print ("Please wait...")
			subprocess.call("plastimatch register " + self.par_file, shell=True, stdout=self.log, stderr=self.log)
			self.log.close()
		else:
			raise NameError("REGISTRATION NOT EXECUTED! You have to define a new parameters file name")




class resample:
	
	def __init__ (self, log_file=""):
		self.log_file=log_file
		
	option={}
	
	_resample_keys=("default-value", "dim", "fixed", "input", "interpolation",\
	"origin", "output", "output-type", "spacing", "subsample")
	
	def run_resample(self):
		_run_plm_command("resample", self.option, self._resample_keys, self.log_file)




class segment:
	
	def __init__ (self, log_file=""):
		self.log_file=log_file
		
	option={}
	
	_segment_keys=("bottom", "debug", "fast", "input", "lower-treshold", "output-img")
	
	def run_segment(self):
		_run_plm_command("segment", self.option, self._segment_keys, self.log_file)




class warp:
	
	def __init__ (self, log_file=""):
		self.log_file=log_file
		
	option={}
	
	_warp_keys=("input","default-val","dif","dim","fixed",\
	"input-cxt","input-dose-ast","input-dose-img","input-dose-mc",\
	"input-dose-xio","input-ss-img","input-ss-list","interpolation",\
	"metadata","origin","output-color_map","output-ctx","output-dicom",\
	"output-dij","output-dose_img","output-img","output-labelmap",\
	"output-pointset","output-prefix","output-prefix_fcsv","output-ss_img",\
	"output-ss_list","output-type","output-vf","output-xio","patient-id",\
	"patient-name","patient-pos","prune-empty","referenced-ct","simplify-perc",\
	"spacing","xf","xor-contours")
	
	def run_warp(self):
		_run_plm_command("warp", self.option, self._warp_keys, self.log_file)




class xfconvert:
	
	def __init__ (self, log_file=""):
		self.log_file=log_file
	
	option={}
	
	_xfconvert_keys=("dim", "grid-spacing", "input", "nobulk", "origin",\
	"output", "output-type", "spacing")
	
	def run_xfconvert(self):
		_run_plm_command("xf-convert", self.option, self._xfconvert_keys, self.log_file)




####################### PUBLIC CLASSES - END - #########################




#################### UTILITY FUNCTION - START - ########################
############ PRIVATE UTILITY FUNCTION, NOT FOR PUBLIC USE ##############

def _clean_parms (d, t):
	
	return dict((k, v) for k, v in d.iteritems() if k in t)


def _run_plm_command(command_type, command_options, command_keys, command_log_file):
	
	if command_log_file == "":
		log=open(os.devnull, "w")
	else:
		log=open(command_log_file, "w")
	
	subprocess.call("plastimatch "+ command_type + _scan_options(command_options, command_keys),\
	shell=True, stdout=log, stderr=log)
	
	log.close()


def _scan_options (d, t):
	
		d=_clean_parms(d, t)
		
		special_keys=("voxels", "scale", "ab-scale", "stretch", "dim",\
		"grid-spacing", "origin", "spacing")
		
		opt_str=""
		
		for key, value in dict.items(d):
			if value!="Enabled" and value!="Disabled" and key not in special_keys:
				opt_str+=" --"+key+"="+value
			elif key in special_keys:
				opt_str+=" --"+key+"="+'"'+value+'"'
			elif value=="Enabled":
				opt_str+=" --"+key
			elif value == "Disabled":
				pass				
		
		return opt_str


############ PRIVATE UTILITY FUNCTION, NOT FOR PUBLIC USE ##############
##################### UTILITY FUNCTION - END - #########################
