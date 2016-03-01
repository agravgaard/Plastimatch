# Plastimatch
A git repository of Plastimatch ___modified___ to be used with [CBCTrecon](https://github.com/agravgaard/cbctrecon)

## Modifications:
### Major:
Added Files   | Modified files
------------- | -------------
dcmtk_rtplan.cxx/.h  | dcmtk_loader.cxx/.h
rtplan.cxx/.h  | dcmtk_loader_p.h
rtplan_beam.cxx/.h  | dcmtk_rt_study.cxx/.h
rtplan_control_pt.cxx/.h  | dcmtk_rt_study_p.h
  | dicom_probe.cxx/.h
  | plm_file_format.cxx/.h
  | rt_study.cxx/.h
  | CMakelist.txt
  
### Minor:
Addition of DEBUG switch to turn standard output off from water equivalent pathlength related functions:
proj_volume.cxx | rpl_volume.cxx
Due to standard output being the bottleneck of heavy for-looping these functions.
