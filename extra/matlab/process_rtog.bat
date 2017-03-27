@echo off
rem ---------------------------------------------------------------
rem           Test run for RTOG format registration
rem ---------------------------------------------------------------
rem mkdir set1a
rem mkdir set1b
rem mkdir set2a
rem mkdir set2b

set SKIP_STUFF=0
set SKIP_STUFF=1

set INDIR_1A=c:\gsharp\idata\rtog-samples\set1\12387.RTOG
set INDIR_1B=c:\gsharp\idata\rtog-samples\set1\12911.RTOG
set INDIR_2A=c:\gsharp\idata\rtog-samples\set2\13558.RTOG
set INDIR_2B=c:\gsharp\idata\rtog-samples\set2\13556.RTOG

set TMPDIR_1A=c:\gsharp\idata\rtog-process\set1a
set TMPDIR_1B=c:\gsharp\idata\rtog-process\set1b
set TMPDIR_2A=c:\gsharp\idata\rtog-process\set2a
set TMPDIR_2B=c:\gsharp\idata\rtog-process\set2b
set TMPDIR_3B=c:\gsharp\idata\rtog-process\12911.RTOG.ALT

rem ---------------------------------------------------------------
rem set1a/set2a is moving
rem set1b/set2b is fixed
rem This means, we'll warp moving image (1a/2a) dose onto the fixed 
rem image (1b/2b) reference system.  Therefore, (1a/2a) should be 
rem the earlier scan, and (1b/2b) should be the later scan
rem ---------------------------------------------------------------

if %SKIP_STUFF%==1 goto :skipped_stuff
:skipped_stuff
rtog_to_mha -d %INDIR_1A% -o %TMPDIR_1A%
rtog_to_mha -d %INDIR_1B% -o %TMPDIR_1B%
rtog_to_mha -d %INDIR_2A% -o %TMPDIR_2A%
rtog_to_mha -d %INDIR_2B% -o %TMPDIR_2B%

move %TMPDIR_1A%\dose.mha %TMPDIR_2A%
move %TMPDIR_1B%\dose.mha %TMPDIR_2B%
rmdir /s/q %TMPDIR_1A%
rmdir /s/q %TMPDIR_1B%

resample_mha --input_type=mask --output_type=mask --input=%TMPDIR_2A%\mask.mha --output=%TMPDIR_2A%\mask_221.mha --subsample="2 2 1"
resample_mha --input_type=mask --output_type=mask --input=%TMPDIR_2B%\mask.mha --output=%TMPDIR_2B%\mask_221.mha --subsample="2 2 1"
resample_mha --input_type=short --output_type=short --input=%TMPDIR_2A%\ct.mha --output=%TMPDIR_2A%\ct_221.mha --subsample="2 2 1"
resample_mha --input_type=short --output_type=short --input=%TMPDIR_2B%\ct.mha --output=%TMPDIR_2B%\ct_221.mha --subsample="2 2 1"

mask_mha %TMPDIR_2A%\ct_221.mha %TMPDIR_2A%\mask_221.mha -1200 %TMPDIR_2A%\masked_221.mha
mask_mha %TMPDIR_2B%\ct_221.mha %TMPDIR_2B%\mask_221.mha -1200 %TMPDIR_2B%\masked_221.mha

ra_registration rtog_test_parms_1.txt

warp_mha --output_type=float --input=%TMPDIR_2A%\dose.mha --output=%TMPDIR_2B%\dose_2a.mha --vf=%TMPDIR_2B%\vf_221.mha

rem HANDJOB HERE -*- Origin, Spacing, and Size -*- Should match fixed
resample_mha.exe --input_type=float --output_type=float --origin="0.446500 0.446500 0.446500" --spacing="0.893000 0.893000 0.893000" --size="512 303 421" --input=%TMPDIR_2B%\dose_2a.mha --output=%TMPDIR_2B%\dose_2a_fullres.mha --default_val=0.0

rem HANDJOB HERE -*- Scale -*- Should match fixed
rem   (scale for sample set is 0.2621 for scan1, 0.0847 for scan2)
mha_to_rtog_dose.exe --input=%TMPDIR_2B%\dose_2a_fullres.mha --output=%TMPDIR_2B%\dose_2a_rtog.raw --scale=0.0847

rem HANDJOB HERE -*- Need "aapm number" for dose image
rem   (aapm0139 for scan1, aapm0151 for scan2)
xcopy /s/e/i/y %INDIR_1B% %TMPDIR_3B%
copy %TMPDIR_2B%\dose_2a_rtog.raw %TMPDIR_3B%\aapm0151

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@rem Resample dose onto ct grid (as sanity check)
@rem resample_mha.exe --input_type=float --output_type=short --origin="0.908 1.36199 1.25" --spacing="1.816 1.816 2.5" --size="256 166 138" --input=set2b\dose_2a_fullres.mha --output=set2b\dose_2a_221.mha --default_val=0.0
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
