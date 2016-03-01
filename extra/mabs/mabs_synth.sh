# Make synthetic images
plastimatch synth --pattern lung --dim "200 200 75" --volume-size "500 500 200" --lung-tumor-pos -10 --output-dicom mabs_synth/atlas/lung_m10
plastimatch synth --pattern lung --dim "200 200 75" --volume-size "500 500 200" --lung-tumor-pos -6 --output-dicom mabs_synth/atlas/lung_m6
#plastimatch synth --pattern lung --dim "200 200 75" --volume-size "500 500 200" --lung-tumor-pos -2 --output-dicom mabs_synth/atlas/lung_m2
plastimatch synth --pattern lung --dim "200 200 75" --volume-size "500 500 200" --lung-tumor-pos 10 --output-dicom mabs_synth/atlas/lung_p10
plastimatch synth --pattern lung --dim "200 200 75" --volume-size "500 500 200" --lung-tumor-pos 6 --output-dicom mabs_synth/atlas/lung_p6
#plastimatch synth --pattern lung --dim "200 200 75" --volume-size "500 500 200" --lung-tumor-pos 2 --output-dicom mabs_synth/atlas/lung_p2
plastimatch synth --pattern lung --dim "200 200 75" --volume-size "500 500 200" --lung-tumor-pos 0 --output-dicom mabs_synth/testcase

plastimatch mabs --convert mabs_synth.cfg
plastimatch mabs --pre-align mabs_synth.cfg

#It is now ok to delete the convert directory
rm -rf mabs_synth/training/convert

plastimatch mabs --train-registration mabs_synth.cfg
plastimatch mabs --train mabs_synth.cfg

