include_directories(${amba_arm_BINARY_DIR})
include_directories(${amba_arm_SOURCE_DIR})

SET(AMBARELLA_DIR /home/edge/cv2x_linux_sdk_3.0/ambarella)

#fakeroot
link_directories(${AMBARELLA_DIR}/out/cv25m_pinenut/fakeroot/usr/lib)

#amba bsp
include_directories(${AMBARELLA_DIR}/include)
include_directories(${AMBARELLA_DIR}/include/arch_v5)
include_directories(${AMBARELLA_DIR}/boards/cv25m_pinenut)

#utils
include_directories(${AMBARELLA_DIR}/packages/utils)
link_directories(${AMBARELLA_DIR}/out/cv25m_pinenut/packages/utils)

#nnctrl
include_directories(${AMBARELLA_DIR}/packages/nnctrl/inc)
link_directories(${AMBARELLA_DIR}/out/cv25m_pinenut/packages/nnctrl)

#cavalry
include_directories(${AMBARELLA_DIR}/include/cavalry)
include_directories(${AMBARELLA_DIR}/packages/cavalry_mem/inc)
link_directories(${AMBARELLA_DIR}/out/cv25m_pinenut/packages/cavalry_mem)

#smartfb
include_directories(${AMBARELLA_DIR}/packages/smartfb)
link_directories(${AMBARELLA_DIR}/out/cv25m_pinenut/packages/smartfb)

#vproc
include_directories(${AMBARELLA_DIR}/packages/vproc/inc)
link_directories(${AMBARELLA_DIR}/out/cv25m_pinenut/packages/vproc)

#textinsert
include_directories(${AMBARELLA_DIR}/packages/textinsert)
link_directories(${AMBARELLA_DIR}/out/cv25m_pinenut/packages/textinsert)

#data_process
include_directories(${AMBARELLA_DIR}/packages/data_process)
link_directories(${AMBARELLA_DIR}/out/cv25m_pinenut/packages/data_process)

#eazyai
include_directories(${AMBARELLA_DIR}/packages/eazyai/inc)
include_directories(${AMBARELLA_DIR}/packages/eazyai/src)
link_directories(${AMBARELLA_DIR}/out/cv25m_pinenut/packages/eazyai)

#data_transfer
include_directories(${AMBARELLA_DIR}/packages/data_transfer)
link_directories(${AMBARELLA_DIR}/out/cv25m_pinenut/packages/data_transfer)

#third-party
link_directories(${AMBARELLA_DIR}/prebuild/oss/armv8-a/libjpeg-turbo/usr/lib)
link_directories(${AMBARELLA_DIR}/prebuild/oss/armv8-a/libpng/usr/lib)
link_directories(${AMBARELLA_DIR}/prebuild/oss/armv8-a/zlib/usr/lib)
link_directories(${AMBARELLA_DIR}/prebuild/oss/armv8-a/bzip2/usr/lib)
link_directories(${AMBARELLA_DIR}/prebuild/oss/armv8-a/freetype/usr/lib)
link_directories(${AMBARELLA_DIR}/prebuild/oss/armv8-a/tbb/usr/lib)
link_directories(${AMBARELLA_DIR}/prebuild/oss/armv8-a/protobuf/usr/lib)

include_directories(${AMBARELLA_DIR}/prebuild/commercial/armv8-a/liteon_phase/include)
link_directories(${AMBARELLA_DIR}/prebuild/commercial/armv8-a/liteon_phase/lib)
include_directories(${AMBARELLA_DIR}/prebuild/ambarella/library/cvwarp/inc)
include_directories(${AMBARELLA_DIR}/prebuild/ambarella/library/ai_cam/cvlib/include)
link_directories(${AMBARELLA_DIR}/prebuild/ambarella/library/ai_cam/cvlib/lib/armv8-a)
include_directories(${AMBARELLA_DIR}/prebuild/oss/armv8-a/eigen/include/eigen3)

#opencv
#set(OpenCV_DIR ${AMBARELLA_DIR}/prebuild/oss/armv8-a/opencv4/usr/lib/OpenCV)
#find_package(OpenCV REQUIRED CONFIG)
#message(STATUS "Found OpenCV ${OpenCV_VERSION}")
#message(STATUS "${OpenCV_INCLUDE_DIRS}, ${OpenCV_LIBS}")
include_directories(${AMBARELLA_DIR}/prebuild/oss/armv8-a/opencv/include)
link_directories(${AMBARELLA_DIR}/prebuild/oss/armv8-a/opencv/usr/lib)

#gflags
include_directories(${AMBARELLA_DIR}/prebuild/oss/armv8-a/gflags/include)
link_directories(${AMBARELLA_DIR}/prebuild/oss/armv8-a/gflags/usr/lib)

#glog
include_directories(${AMBARELLA_DIR}/prebuild/oss/armv8-a/glog/include)
link_directories(${AMBARELLA_DIR}/prebuild/oss/armv8-a/glog/usr/lib)

OPTION(USE_OpenMP "Use OpenMP" ON)
IF(USE_OpenMP)
   FIND_PACKAGE(OpenMP)
   IF(OPENMP_FOUND)
      SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
      SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
   ENDIF()
ENDIF()
