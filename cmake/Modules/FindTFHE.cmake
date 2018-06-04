find_path(TFHE_INCLUDE_DIR NAMES tfhe.h tfhe/tfhe.h)
find_library(TFHE_LIBRARY NAMES tfhe-fftw libtfhe-fftw)
# Use the following to link against the nayuki fft library
#find_library(TFHE_LIBRARY NAMES tfhe-nayuki-avx libtfhe-nayuki-avx)

#set(TFHE_FOUND TRUE)
set(TFHE_INCLUDE_DIRS ${TFHE_INCLUDE_DIR})
set(TFHE_LIBRARIES ${TFHE_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(TFHE DEFAULT_MSG
                                  TFHE_LIBRARY TFHE_INCLUDE_DIR)

mark_as_advanced(TFHE_LIBRARY TFHE_INCLUDE_DIR)
