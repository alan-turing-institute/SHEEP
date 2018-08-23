find_path(LP_INCLUDE_DIR NAMES paillier.h)
find_library(LP_LIBRARY NAMES libpaillier.a) 

#set(LP_FOUND TRUE)
set(LP_INCLUDE_DIRS ${LP_INCLUDE_DIR})
set(LP_LIBRARIES ${LP_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LP DEFAULT_MSG
                                  LP_LIBRARY LP_INCLUDE_DIR)

mark_as_advanced(LP_LIBRARY LP_INCLUDE_DIR)

