project (nkidecompress)

set (NKIDECOMPRESS_SRC
    nkidecompress.cxx
    )

plm_add_static_library (nkidecompress "${NKIDECOMPRESS_SRC}" "" "" "")
