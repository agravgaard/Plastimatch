#!/usr/bin/env python

"""
This script analyzes the seg_dice.csv mabs file and compute statistic on it.
Author: Paolo Zaffino  (p.zaffino@unicz.it)
Rev 4

Usage examples:
./mabs_stats.py --input seg_dice.csv
./mabs_stats.py --input seg_dice.csv --structures "brainstem"
./mabs_stats.py --input seg_dice.csv --thresholds "gaussian_0.4"
./mabs_stats.py --input seg_dice.csv --structures "brainstem" --thresholds "gaussian_0.4"
./mabs_stats.py --input seg_dice.csv --structures "brainstem" --thresholds "staple_0.5"
./mabs_stats.py --input seg_dice.csv --structures "brainstem" --thresholds "staple*"
./mabs_stats.py --input seg_dice.csv --structures "brainstem left_protid" --thresholds "gaussian_0.4 gaussian_0.5 staple*"
"""

import argparse
from copy import deepcopy
import numpy as np

# Utility function
def str_or_float(s):
    try: return float(s)
    except ValueError: return s

# Parser settings
parser = argparse.ArgumentParser(description='MABS seg_dice.csv analyzer')
parser.add_argument('--input', help='Mabs seg_dice.csv file', type=str, required=True)
parser.add_argument('--structures', help='Subset of structures to analyze. String as "stru1 stru2"', type=str, required=False)
parser.add_argument('--thresholds', help='Subset of thresholds to analyze. String as "fusiontype_thr1 fusiontype_thr2 fusiontype*"', type=str, required=False)
args = parser.parse_args()

# Read data from file
seg_dice = open(args.input, "r")
raw_lines = [l.strip().split(",") for l in seg_dice.readlines()]
seg_dice.close()

# Make a list of all the patients
patients=[]
for line in raw_lines:
    patient=line[0].split("=")[1] if line[0].split("=")[0] == "target" else None
    if patient not in patients and patient is not None: patients.append(patient)

# Organize the data
data=dict()
for patient in patients:
    data[patient]=[]

for line in raw_lines:
    patient = line[0].split("=")[1] if line[0].split("=")[0] == "target" else None
    if patient == None: continue
    line_dict = dict()
    for field in line[1:]:
        key, value = field.split("=")
        key, value = key.strip(), str_or_float(value.strip())
        line_dict[key]=value
    data[patient].append(deepcopy(line_dict))

# Extract structures and thresholds
structures = []
thresholds = []

for patient in patients:
    patient_data = data[patient]
    for entry in patient_data:
        if entry["struct"] not in structures:
            structures.append(entry["struct"])
        if "thresh" in entry and "gaussian_%f" % entry["thresh"] not in thresholds:
            thresholds.append("gaussian_%f" % entry["thresh"])
        if "confidence_weight" in entry and "staple_%f" % entry["confidence_weight"] not in thresholds:
             thresholds.append("staple_%.9f" % entry["confidence_weight"])
            
structures = set(structures)
thresholds = set(thresholds)

# Filter data using input parameters
if args.thresholds != None:
    args_thresholds_splitted = args.thresholds.split(" ")
    filtered_thresholds = []
    
    # Check for "full range" thresholds (something like "staple*" or "gaussian*")
    full_range_thresholds = []
    for arg in args_thresholds_splitted:
        if "*" in arg:
            fusion_criterion = arg.split("*")[0]
            if fusion_criterion not in full_range_thresholds: full_range_thresholds.append(fusion_criterion)
    
    # Set the "well defined" thresholds
    for input_threshold in args_thresholds_splitted:
        if len(input_threshold.split("_")) == 2:
            fusion, thr = input_threshold.split("_")[0], float(input_threshold.split("_")[1])
            filtered_thresholds.append("%s_%.9f" % (fusion, thr))
    
    # Set the "full range" thresholds
    for full_range_of_threshold in full_range_thresholds:
        for threshold in thresholds:
            if full_range_of_threshold in threshold: filtered_thresholds.append(threshold)
    
    selected_thresholds = thresholds.intersection(filtered_thresholds)
else:
    selected_thresholds = thresholds

if args.structures != None:
    selected_structures = structures.intersection(args.structures.split(" "))
else:
    selected_structures = structures

# Create data structures
# eg:
# stats_left_parotid (dict) -
#                            | --> dice_04 (list)
#                            | --> dice_05 (list)
#                            | --> b_avg_dist_04 (list)
#                            | --> b_avg_dist_05 (list)
#                            | --> b_95_dist_04 (list)
#                            | --> b_95_dist_05 (list)
for structure in structures:
    vars()["stats_%s" % structure] = {}
    for threshold in thresholds:
        thr = str(threshold).replace(".", "")
        vars()["stats_%s" % structure]["dice_%s" % thr] = []
        vars()["stats_%s" % structure]["b_avg_dist_%s" % thr] = []
        vars()["stats_%s" % structure]["b_95_dist_%s" % thr] = []

# Insert data inside the structure
for patient in patients:
    patient_data = data[patient]
    for entry in patient_data:
        if "thresh" in entry:
            thr = "gaussian_%f" % entry["thresh"]
        elif "confidence_weight" in entry:
            thr = "staple_%.9f" % entry["confidence_weight"]
        thr = thr.replace(".", "")
        stru = entry["struct"]
        dice = entry["dice"]
        b_avg_dist = entry["abhd"]
        b_95_dist = entry["95bhd"]
        
        vars()["stats_%s" % stru]["dice_%s" % thr].append(dice)
        vars()["stats_%s" % stru]["b_avg_dist_%s" % thr].append(b_avg_dist)
        vars()["stats_%s" % stru]["b_95_dist_%s" % thr].append(b_95_dist)

# Medians and percentiles
for structure in structures:
    for threshold in thresholds:
        thr = str(threshold).replace(".", "")
        vars()["median_dice_%s_%s" % (structure, thr)] = np.median(vars()["stats_%s" % structure]["dice_%s" % thr])
        vars()["95th_perc_dice_%s_%s" % (structure, thr)] = np.percentile(vars()["stats_%s" % structure]["dice_%s" % thr], 95)
        vars()["5th_perc_dice_%s_%s" % (structure, thr)] = np.percentile(vars()["stats_%s" % structure]["dice_%s" % thr], 5)

        vars()["median_b_avg_dist_%s_%s" % (structure, thr)] = np.median(vars()["stats_%s" % structure]["b_avg_dist_%s" % thr])
        vars()["95th_perc_b_avg_dist_%s_%s" % (structure, thr)] = np.percentile(vars()["stats_%s" % structure]["b_avg_dist_%s" % thr], 95)
        vars()["5th_perc_b_avg_dist_%s_%s" % (structure, thr)] = np.percentile(vars()["stats_%s" % structure]["b_avg_dist_%s" % thr], 5)

        vars()["median_b_95_dist_%s_%s" % (structure, thr)] = np.median(vars()["stats_%s" % structure]["b_95_dist_%s" % thr])
        vars()["95th_perc_b_95_dist_%s_%s" % (structure, thr)] = np.percentile(vars()["stats_%s" % structure]["b_95_dist_%s" % thr], 95)
        vars()["5th_perc_b_95_dist_%s_%s" % (structure, thr)] = np.percentile(vars()["stats_%s" % structure]["b_95_dist_%s" % thr], 5)

# Print results
for structure in selected_structures:
    for threshold in selected_thresholds:
        thr = str(threshold).replace(".", "")
        print("Structure = %s" % structure)
        print("  threshold = %s" % threshold)
        dice_string = "    dice = median %f  5th_perc %f  95th_perc %f" % (vars()["median_dice_%s_%s" % (structure, thr)],
            vars()["5th_perc_dice_%s_%s" % (structure, thr)],
            vars()["95th_perc_dice_%s_%s" % (structure, thr)])
        print(dice_string)
        avg_distance_string = "    average boundary distance = median %f  5th_perc %f  95th_perc %f" % (vars()["median_b_avg_dist_%s_%s" % (structure, thr)],
            vars()["5th_perc_b_avg_dist_%s_%s" % (structure, thr)],
            vars()["95th_perc_b_avg_dist_%s_%s" % (structure, thr)])
        print(avg_distance_string)
        max_distance_string = "    95th percentile boundary distance = median %f  5th_perc %f  95th_perc %f" % (vars()["median_b_95_dist_%s_%s" % (structure, thr)],
            vars()["5th_perc_b_95_dist_%s_%s" % (structure, thr)],
            vars()["95th_perc_b_95_dist_%s_%s" % (structure, thr)])
        print(max_distance_string)

