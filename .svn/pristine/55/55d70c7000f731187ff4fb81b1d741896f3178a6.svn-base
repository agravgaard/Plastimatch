project (libf2c)

set (LIBF2C_LIBRARY_SRC
  abort_.c    d_int.c	inquire.c	r_atn2.c  sig_die.c
  d_lg10.c	ftell_.c    i_sign.c	r_cnjg.c  signal_.c
  backspac.c  d_log.c	            lbitbits.c	r_cos.c   signbit.c
  c_abs.c     d_mod.c	getenv_.c   lbitshft.c	r_cosh.c  
  cabs.c      d_nint.c	h_abs.c     l_ge.c	rdfmt.c   s_rnge.c
  c_cos.c     dolio.c	h_dim.c     l_gt.c	r_dim.c   s_stop.c
  c_div.c     d_prod.c	h_dnnt.c    l_le.c	rewind.c  sue.c
  c_exp.c     d_sign.c	h_indx.c    l_lt.c	r_exp.c   system_.c
  c_log.c     d_sin.c	h_len.c     lread.c	r_imag.c  typesize.c
  close.c     d_sinh.c	hl_ge.c     lwrite.c	r_int.c   uio.c
  c_sin.c     d_sqrt.c	hl_gt.c                 r_lg10.c  uninit.c
  c_sqrt.c    d_tan.c	hl_le.c     open.c	r_log.c   util.c
  ctype.c     d_tanh.c	hl_lt.c     pow_ci.c	r_mod.c   wref.c
  d_abs.c     dtime_.c	h_mod.c     pow_dd.c	r_nint.c  wrtfmt.c
  d_acos.c    due.c	h_nint.c    pow_di.c	rsfe.c	  wsfe.c
  d_asin.c    ef1asc_.c	h_sign.c    pow_hh.c	r_sign.c  wsle.c
  d_atan.c    ef1cmc_.c	i77vers.c   pow_ii.c	r_sin.c   wsne.c
  d_atn2.c    endfile.c	i_abs.c     r_sinh.c  xwsne.c
  d_cnjg.c    erf_.c	            pow_ri.c	rsli.c	  z_abs.c
  d_cos.c     erfc_.c	i_dim.c     pow_zi.c	rsne.c	  z_cos.c
  d_cosh.c    err.c	i_dnnt.c    pow_zz.c	r_sqrt.c  z_div.c
  d_dim.c     etime_.c	i_indx.c    r_tan.c   z_exp.c
  derf_.c     exit_.c	iio.c	    r_tanh.c  z_log.c
  derfc_.c    f77_aloc.c i_len.c     r_abs.c	s_cat.c   z_sin.c
  d_exp.c     f77vers.c	ilnw.c	    r_acos.c	s_cmp.c   z_sqrt.c
  dfe.c	      fmt.c	i_mod.c     r_asin.c	s_copy.c
  d_imag.c    fmtlib.c	i_nint.c    r_atan.c	sfe.c
  )
set (LIBF2C_LIBRARY_SRC_UNUSED
  ftell64_.c
  getarg_.c
  iargc_.c
  main.c
  pow_qq.c
  qbitbits.c
  qbitshft.c
  s_paus.c
  )

## Fix for building shared library on MSVC which does not provide erf()
# set (LIBF2C_LIBRARY_SRC
#   ${LIBF2C_LIBRARY_SRC}
#   tesuji_erf.c)

## Need output from arithchk.  Note this operation does not support 
## cross-compiling.
set (ARITHCHK_LINK_LIBRARIES ${MATH_LIB})
try_run (
  RUN_RESULT
  COMPILE_RESULT
  "${CMAKE_CURRENT_BINARY_DIR}/arithchk"
  "${CMAKE_CURRENT_SOURCE_DIR}/arithchk.c"
  CMAKE_FLAGS "-DLINK_LIBRARIES:STRING=${ARITHCHK_LINK_LIBRARIES}"
  COMPILE_DEFINITIONS -DNO_FPINIT
  COMPILE_OUTPUT_VARIABLE ARITHCHK_COMPILE_OUTPUT
  RUN_OUTPUT_VARIABLE ARITHCHK_STDOUT
  )

## Check exit code == 0 for running arithchk
if (NOT RUN_RESULT AND ARITHCHK_STDOUT)
  ## cmake adds extra \r on windows.  This creates a file with lines 
  ## like "Blah blah\r\r\n", which MSVC refuses to compile.
  ## So we strip out the \r, to get lines like "Blah blah\r\n".
  string (REGEX REPLACE "\r" "" ARITHCHK_STDOUT ${ARITHCHK_STDOUT})
  file (WRITE "${CMAKE_CURRENT_BINARY_DIR}/arith.h" ${ARITHCHK_STDOUT})
else ()
  message (STATUS "RUN_RESULT is ${RUN_RESULT}")
  message (STATUS "COMPILE_RESULT is ${COMPILE_RESULT}")
  message (STATUS "COMPILE_OUTPUT is ${ARITHCHK_COMPILE_OUTPUT}")
  message (STATUS "RESULT is ${ARITHCHK_STDOUT}")
  message (FATAL_ERROR "Failed to create arith.h when building libf2c")
endif ()

include_directories (BEFORE ${CMAKE_CURRENT_BINARY_DIR})
include_directories (BEFORE ${CMAKE_CURRENT_SOURCE_DIR})

plm_add_static_library (f2c1 "${LIBF2C_LIBRARY_SRC}" "" "" "")
