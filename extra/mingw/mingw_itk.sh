if test -f /c/Program\ Files\ \(x86\)/CMake\ 2.8/bin/cmake; then
    CMAKE_EXE=/c/Program\ Files\ \(x86\)/CMake\ 2.8/bin/cmake
elif test -f /c/Program\ Files/CMake\ 2.8/bin/cmake; then
    CMAKE_EXE=/c/Program\ Files/CMake\ 2.8/bin/cmake
else
    echo "CMake not found!"
    return 1
fi

"${CMAKE_EXE}" \
	-G"MSYS Makefiles" \
	-DBUILD_EXAMPLES=OFF \
	-DBUILD_TESTING=OFF \
	-DCMAKE_BUILD_TYPE=Release \
	-DCMAKE_USE_PTHREADS=OFF \
	-DITK_USE_REVIEW=ON \
	-DITK_USE_OPTIMIZED_REGISTRATION_METHODS=ON \
	/c/gcs6/build/src/InsightToolkit-3.20.0/

