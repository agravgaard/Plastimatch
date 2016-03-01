# Make synthetic image
plastimatch synth --pattern lung --dim "100 100 40" --volume-size "500 500 200" --lung-tumor-pos 0 --output-dicom lung-dicom

# Convert to mha
plastimatch convert --input lung-dicom --output-img lung_mha/ct.mha --output-prefix lung_mha

# Delete original dicom
rm -rf lung-dicom

# Use wed command to make aperture, range compensator
## wed --segdepth wed.cfg

# Compute dose
proton_dose dose_2.cfg

