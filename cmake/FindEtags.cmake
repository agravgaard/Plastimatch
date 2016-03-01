## From Jan Woetzel, CMAKE email list
## http://www.cmake.org/pipermail/cmake/2006-January/007883.html
IF (UNIX)
  ADD_CUSTOM_TARGET(tags etags --members --declarations  `find ${CMAKE_SOURCE_DIR} -name *.cc -or -name *.cxx -or -name *.hxx -or -name *.hh -or -name *.cpp -or -name *.h -or -name *.c -or -name *.f`)
  ADD_CUSTOM_TARGET(etags DEPENDS tags)
  MESSAGE (STATUS "Etags targets added.")
ENDIF (UNIX)
