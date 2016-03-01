project (specfun)

# For plm_fortran.h
include_directories (BEFORE ${CMAKE_BINARY_DIR})

# We only need this one
set (SPECFUN_LIBRARY_FORTRAN_SRC
  mpbdv.for
  )
set (SPECFUN_LIBRARY_C_SRC
  mpbdv.c
  )

if (PLM_PREFER_NO_FORTRAN_NO_F2C OR PLM_USE_F2C)
  plm_add_static_library (specfun "${SPECFUN_LIBRARY_C_SRC}" 
    "${PLM_FORTRAN_LIBRARIES}" "" "")
else ()
  plm_add_static_library (specfun "${SPECFUN_LIBRARY_FORTRAN_SRC}" 
    "${PLM_FORTRAN_LIBRARIES}" "" "")
endif ()
