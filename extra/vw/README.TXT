########################################################################
# Run the scripts in the following order:

########################################################################
# 1.  make_ml_files
make_ml_files.pl --convert ~/gelato_shared/mabs/pddca/convert --training-dir ~/gelato_shared/mabs/pddca/task22/mabs-train --structure BrainStem --execute
make_ml_files.pl --convert ~/gelato_shared/mabs/pddca/convert --training-dir ~/gelato_shared/mabs/pddca/task22/mabs-train --structure OpticNerve_L --execute
make_ml_files.pl --convert ~/gelato_shared/mabs/pddca/convert --training-dir ~/gelato_shared/mabs/pddca/task22/mabs-train --structure Parotid_L --execute


########################################################################
# 2.  sort_ml_files
( cd Brainstem && sort_ml_files.pl )
( cd OpticNerve_L && sort_ml_files.pl )
( cd Parotid_L && sort_ml_files.pl )

########################################################################
# 3.  make_ml_groups.pl
( cd BrainStem && make_ml_groups.pl )
( cd OpticNerve_L && make_ml_groups.pl )
( cd Parotid_L && make_ml_groups.pl )

########################################################################
# 4.  run_vw_groups.pl
( cd BrainStem && run_vw_groups.pl )
( cd OpticNerve_L && run_vw_groups.pl )
( cd Parotid_L && run_vw_groups.pl )
