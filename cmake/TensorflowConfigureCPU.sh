set -e

# Setup environment variables so that configure command will not ask user for input by keyboard
export CC_OPT_FLAGS="-march=core-avx2"
export TF_NEED_GCP=0
export TF_NEED_HDFS=0
export TF_NEED_OPENCL=0
export TF_NEED_OPENCL_SYCL=0
export TF_NEED_COMPUTECPP=0
export TF_NEED_TENSORRT=0
export TF_NEED_KAFKA=0
export TF_NEED_JEMALLOC=1
export TF_NEED_VERBS=0
export TF_NEED_MKL=0
export TF_NEED_ROCM=0
export TF_SET_ANDROID_WORKSPACE=0
export TF_DOWNLOAD_MKL=0
export TF_DOWNLOAD_CLANG=0
export TF_NEED_MPI=0
export TF_NEED_S3=0
export TF_NEED_GDR=0
export TF_ENABLE_XLA=0
export TF_CUDA_CLANG=0
export TF_NEED_CUDA=0
export TF_NCCL_VERSION=" "
export PYTHON_BIN_PATH="$(which python3)"
export PYTHON_LIB_PATH="$($PYTHON_BIN_PATH -c 'import site; print(site.getsitepackages()[0])')"

./configure
