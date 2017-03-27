set target_file_a=C:\Windows\system32\cmd.exe
@echo off
set SCRIPT="%TEMP%\%RANDOM%-%RANDOM%-%RANDOM%-%RANDOM%.vbs"
echo Set oWS = WScript.CreateObject("WScript.Shell") >> %SCRIPT%
echo sLinkFile = "%ALLUSERSPROFILE%\Desktop\plastimatch32_cmd.lnk" >> %SCRIPT%
echo Set oLink = oWS.CreateShortcut(sLinkFile) >> %SCRIPT%
echo oLink.TargetPath =  "%target_file_a%" >> %SCRIPT%
echo oLink.WorkingDirectory = "%~dp0" >> %SCRIPT%
echo oLink.Save >> %SCRIPT%
cscript /nologo %SCRIPT%
del %SCRIPT%

set SCRIPT2="%TEMP%\%RANDOM%-%RANDOM%-%RANDOM%-%RANDOM%.vbs"
echo Set oWS = WScript.CreateObject("WScript.Shell") >> %SCRIPT2%
echo sLinkFile = "%~dp0\plastimatch32_cmd.lnk" >> %SCRIPT2%
echo Set oLink = oWS.CreateShortcut(sLinkFile) >> %SCRIPT2%
echo oLink.TargetPath =  "%target_file_a%" >> %SCRIPT2%
echo oLink.WorkingDirectory = "%~dp0" >> %SCRIPT2%
echo oLink.Save >> %SCRIPT2%
cscript /nologo %SCRIPT2%
del %SCRIPT2%

start "" http://plastimatch.org/contents.html
%SystemRoot%\explorer.exe "%~dp0"
