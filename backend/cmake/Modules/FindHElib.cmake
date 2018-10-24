find_path(HElib_INCLUDE_DIR NAMES FHE.h PATHS /Users/tlazauskas/git/Turing/SHEEP/backend/lib/HElib/src/)
find_library(HElib_LIBRARY NAMES fhe.a libfhe.a PATHS /Users/tlazauskas/git/Turing/SHEEP/backend/lib/HElib/build/lib/)

#set(HElib_FOUND TRUE)
set(HElib_INCLUDE_DIRS ${HElib_INCLUDE_DIR})
set(HElib_LIBRARIES ${HElib_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(HElib DEFAULT_MSG
                                  HElib_LIBRARY HElib_INCLUDE_DIR)

mark_as_advanced(HElib_LIBRARY HElib_INCLUDE_DIR)
