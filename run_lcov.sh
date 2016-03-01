#! /bin/sh

lcov --directory . --zerocounters 
ctest 
lcov --directory . --capture --output-file app.info 
genhtml app.info 
firefox ./index.html
