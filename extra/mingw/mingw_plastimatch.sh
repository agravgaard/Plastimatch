if test -f /c/Program\ Files\ \(x86\)/CMake\ 2.8/bin/cmake; then
    CMAKE_EXE=/c/Program\ Files\ \(x86\)/CMake\ 2.8/bin/cmake
elif test -f /c/Program\ Files/CMake\ 2.8/bin/cmake; then
    CMAKE_EXE=/c/Program\ Files/CMake\ 2.8/bin/cmake
else
    echo "CMake not found!"
    return 1
fi

#ITK_DIR=/c/gcs6/build/itk-3.20.0-mingw
ITK_DIR=/c/gcs6/build/mingw/itk-3.20.0

"${CMAKE_EXE}" \
	-G"MSYS Makefiles" \
	-DCMAKE_BUILD_TYPE=Release \
	-DITK_DIR="${ITK_DIR}" \
	/c/gcs6/work/plastimatch

