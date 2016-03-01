#!/usr/bin/env python

## rev 2
## Usage: Unix based OS: ./mha-mhd.py file_to_convert
##        Windows: python mha-mhd.py file_to_convert
##
## This script converts a mha file in a mhd file and vice versa.
## The input file can be an image or a vector field.
##
## Author: Paolo Zaffino (p.zaffino@unicz.it)
##
## NOT TESTED ON PYTHON 3

import sys

file_name=sys.argv[1]

if file_name.endswith('.mha'):  # mha to mhd
	
	mhd_name = file_name[:-4]+'.mhd'
	raw_name = file_name[:-4]+'.raw'
	
	mha_file = open(file_name, 'r')
	mhd_file = open (mhd_name, 'w')
	raw_file = open(raw_name, 'w')
	
	end_header = 0

	for i, line in enumerate(mha_file):
		if line.startswith('ElementDataFile ='):
			end_header = i
			break
		if i > 40:
			break
	
	mha_file.seek(0)
	for k, line in enumerate(mha_file):
		if k < end_header:
			mhd_file.write(line)
		elif k == end_header:
			mhd_file.write('ElementDataFile = '+raw_name+'\n')
		elif k > end_header:
			raw_file.write(line)
			
	raw_file.close()
	mhd_file.close()
	mha_file.close()

	
elif file_name.endswith('.mhd'): # mhd to mha
	
	mha_name = file_name[:-4]+'.mha'
	raw_name = file_name[:-4]+'.raw'
	
	header = open(file_name,'r')
	mha_file = open(mha_name,'w')
	
	
	for line in header:
		if not line.startswith('ElementDataFile ='):
			mha_file.write(line)
		elif line.startswith('ElementDataFile ='):
			mha_file.write('ElementDataFile = LOCAL\n')
	
	header.close()
	
	raw = open(raw_name, 'r')
	r=raw.readlines()
	mha_file.writelines(r)
	
	raw.close()
	mha_file.close()
	
	
elif not file_name.endswith('.mha') and not file_name.endswith('.mhd'):  # no mha/mhd file
	print ("The input file isn't a mha/mhd file!")
