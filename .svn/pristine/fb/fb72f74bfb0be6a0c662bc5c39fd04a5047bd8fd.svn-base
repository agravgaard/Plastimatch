PROJECT (sqlite3)

SET (SQLITE_LIBRARY_SRC
  sqlite3.c
  sqlite3.h
  sqlite3ext.h
  )

ADD_LIBRARY (sqlite3 STATIC ${SQLITE_LIBRARY_SRC})

## JAS 2011.01.24
## I have disabled this INSTALL since it would be inappropriate
## for our CPack packages to install libsqlite3.a
##
IF (PLM_OLD_INSTALL_TARGETS)
  INSTALL (TARGETS sqlite3 DESTINATION lib)
ENDIF (PLM_OLD_INSTALL_TARGETS)
