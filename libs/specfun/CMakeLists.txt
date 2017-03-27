project (specfun)

# We only need this one file
set (SPECFUN_LIBRARY_C_SRC
  mpbdv.c
  )

plm_add_static_library (specfun "${SPECFUN_LIBRARY_C_SRC}" "" "" "")
