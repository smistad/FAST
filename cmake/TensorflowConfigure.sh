set -e

# Setup environment variables so that configure command will not ask user for input by keyboard
export CC_OPT_FLAGS="-march=native"
export TF_NEED_GCP=0
export TF_NEED_HDFS=0
export TF_NEED_OPENCL=0
export TF_NEED_OPENCL_SYCL=0
export TF_NEED_TENSORRT=0
export TF_NEED_KAFKA=0
export TF_NEED_JEMALLOC=1
export TF_NEED_VERBS=0
export TF_NEED_MKL=0
export TF_DOWNLOAD_MKL=0
export TF_NEED_MPI=0
export TF_NEED_S3=0
export TF_NEED_GDR=0
export TF_ENABLE_XLA=0
export TF_CUDA_CLANG=0
export PYTHON_BIN_PATH="$(which python3)"
export PYTHON_LIB_PATH="$($PYTHON_BIN_PATH -c 'import site; print(site.getsitepackages()[0])')"

# CUDA specific variables
# Check first if CUDA exists at /usr/local/cuda
if [ -e /usr/local/cuda ]; then
    echo "Tensorflow CUDA support enabled"
    export TF_NEED_CUDA=1
    export TF_CUDA_COMPUTE_CAPABILITIES="3.5,5.2,6.1"
    export CUDA_TOOLKIT_PATH=/usr/local/cuda
    export CUDNN_INSTALL_PATH=/usr/local/cuda
    export TF_CUDA_VERSION="$($CUDA_TOOLKIT_PATH/bin/nvcc --version | sed -n 's/^.*release \(.*\),.*/\1/p')"
    export TF_CUDNN_VERSION="$(sed -n 's/^#define CUDNN_MAJOR\s*\(.*\).*/\1/p' $CUDNN_INSTALL_PATH/include/cudnn.h)"
    export GCC_HOST_COMPILER_PATH=/usr/bin/gcc-5
    export CLANG_CUDA_COMPILER_PATH=/usr/bin/clang
    export TF_CUDA_CLANG=0
else
    echo "Tensorflow CUDA support disabled"
    export TF_NEED_CUDA=0
fi

./configure