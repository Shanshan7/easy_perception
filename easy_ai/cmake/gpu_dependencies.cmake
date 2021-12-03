
SET(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS};-gencode arch=compute_52,code=sm_52)

#set(CUDA_TOOLKIT_ROOT_DIR /usr/local/cuda)
enable_language(CUDA)
find_package(CUDA REQUIRED)
message(STATUS "CUDA_INCLUDE_DIRS : " ${CUDA_INCLUDE_DIRS})
include_directories(${CUDA_INCLUDE_DIRS})

# set(CUDA_INSTALL_DIR /usr/local/cuda)
# set(TENSORRT_ROOT /usr/local/cuda)
# include_directories(/home/lpj/Software/TensorRT7/include)
# link_directories(/home/lpj/Software/TensorRT7/lib)
# link_directories(/home/lpj/Software/TensorRT7/lib/stubs)