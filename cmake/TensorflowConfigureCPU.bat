rem Setup environment variables so that configure command will not ask user for input by keyboard
setx CC_OPT_FLAGS "/arch:AVX2"
setx TF_NEED_GCP 0
setx TF_NEED_HDFS 0
setx TF_NEED_OPENCL 0
setx TF_NEED_OPENCL_SYCL 0
setx TF_NEED_COMPUTECPP 0
setx TF_NEED_TENSORRT 0
setx TF_NEED_KAFKA 0
setx TF_NEED_JEMALLOC 1
setx TF_NEED_VERBS 0
setx TF_NEED_MKL 0
setx TF_NEED_ROCM 0
setx TF_SET_ANDROID_WORKSPACE 0
setx TF_DOWNLOAD_MKL 0
setx TF_DOWNLOAD_CLANG 0
setx TF_NEED_MPI 0
setx TF_NEED_S3 0
setx TF_NEED_GDR 0
setx TF_ENABLE_XLA 0
setx TF_CUDA_CLANG 0
setx TF_NEED_CUDA 0
setx TF_NCCL_VERSION " "
setx PYTHON_BIN_PATH "$(which python3)"
setx PYTHON_LIB_PATH "$($PYTHON_BIN_PATH -c 'import site; print(site.getsitepackages()[0])')"

python configure.py
