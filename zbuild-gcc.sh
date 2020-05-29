#!/bin/sh

USE_INTEL=false
USE_INTEL=true

if [ -f Makefile ]; then
   make clean
fi
if [ -f CMakeCache.txt ]; then
   echo "Removing CMakeCache.txt"
   rm CMakeCache.txt
fi
if [ -d CMakeFiles ]; then
   echo "Removing CMakeFiles"
   rm -rf CMakeFiles
fi

export PKG_DIR=/home/mth/mth/zpkg
export MKL_LIB_ROOT=/opt/intel/mkl/lib/intel64
export IPP_LIB_ROOT=/opt/intel/ipp/lib/intel64
export EW_DIR=/opt/earthworm/earthworm_svn

		 #-DAPR_INCLUDE_DIR=/usr/include/apr-1 \

if $USE_INTEL; then

cmake ./ -DCMAKE_BUILD_TYPE=DEBUG \
-DCMAKE_INSTALL_PREFIX=./ \
-DCMAKE_C_FLAGS="-g3 -O2 -Wno-reserved-id-macro -Wno-padded -Wno-unknown-pragmas -fopenmp" \
-DCMAKE_CXX_FLAGS="-g3 -O2 -fopenmp" \
-DGFAST_INSTANCE="PNSN" \
-DGFAST_USE_INTEL=TRUE \
-DGFAST_USE_ISCL=TRUE \
-DGFAST_USE_AMQ=TRUE \
-DGFAST_USE_EW=TRUE \
-DLSSL_LIBRARY=/usr/lib64/libssl.so.10 \
-DLCRYPTO_LIBRARY=/usr//lib64/libcrypto.so.10 \
-DLIBAMQ_INCLUDE_DIR=/usr/include/activemq-cpp-3.9.3 \
-DLIBAMQ_LIBRARY=/usr/lib64/libactivemq-cpp.so \
-DMKL_LIBRARY="${MKL_LIB_ROOT}/libmkl_intel_lp64.so;${MKL_LIB_ROOT}/libmkl_core.so;${MKL_LIB_ROOT}/libmkl_sequential.so" \
-DIPP_LIBRARY="${IPP_LIB_ROOT}/libipps.so;${IPP_LIB_ROOT}/libippvm.so;${IPP_LIB_ROOT}/libippcore.so" \
-DH5_C_INCLUDE_DIR=/usr/include \
-DH5_LIBRARY=/usr/lib64/libhdf5_cpp.so \
-DINIPARSER_INCLUDE_DIR=${PKG_DIR}/iniparser/src \
-DINIPARSER_LIBRARY=${PKG_DIR}/iniparser/libiniparser.a \
-DCOMPEARTH_INCLUDE_DIR=${PKG_DIR}/compearth/momenttensor/c_src/include \
-DCOMPEARTH_LIBRARY=${PKG_DIR}/compearth/momenttensor/c_src/lib/libcompearth_shared.so \
-DISCL_INCLUDE_DIR=${PKG_DIR}/iscl/include \
-DISCL_LIBRARY=${PKG_DIR}/iscl/lib/libiscl_shared.so \
-DEW_BUILD_FLAGS="-Dlinux -D_LINUX -D_INTEL -D_USE_SCHED -D_USE_PTHREADS" \
-DEW_INCLUDE_DIR=${EW_DIR}/include \
-DEW_LIBRARY="${EW_DIR}/lib/libew_mt.a;${EW_DIR}/lib/libew_util.a" \
-DLIBXML2_LIBRARY=/usr/lib64/libxml2.so.2 \
-DLIBXML2_INCLUDE_DIR=/usr/include/libxml2 \
-DUW_AMAZON=FALSE

else
	cmake ./ -DCMAKE_BUILD_TYPE=DEBUG \
		 -DCMAKE_INSTALL_PREFIX=./ \
		 -DCMAKE_C_FLAGS="-g3 -O2 -Wno-reserved-id-macro -Wno-padded -Wno-unknown-pragmas -fopenmp" \
		 -DCMAKE_CXX_FLAGS="-g3 -O2 -fopenmp" \
		 -DGFAST_INSTANCE="PNSN" \
		 -DGFAST_USE_INTEL=FALSE \
		 -DGFAST_USE_AMQ=TRUE \
		 -DGFAST_USE_EW=TRUE \
		 -DGFAST_USE_ISCL=TRUE \
		 -DLSSL_LIBRARY=/usr/lib64/libssl.so.10 \
		 -DLCRYPTO_LIBRARY=/usr//lib64/libcrypto.so.10 \
		 -DLIBAMQ_INCLUDE_DIR=/usr/include/activemq-cpp-3.9.3 \
		 -DLIBAMQ_LIBRARY=/usr/lib64/libactivemq-cpp.so \
	 	 -DLAPACKE_INCLUDE_DIR=/usr/include/lapacke \
	 	 -DLAPACKE_LIBRARY=/lib64/liblapacke.so \
	 	 -DLAPACK_LIBRARY=/lib64/liblapack.so \
	 	 -DBLAS_LIBRARY=/lib64/libblas.so \
	 	 -DCBLAS_INCLUDE_DIR=/usr/include \
	 	 -DCBLAS_LIBRARY=/lib64/atlas/libsatlas.so \
		 -DH5_C_INCLUDE_DIR=/usr/include \
		 -DH5_LIBRARY=/usr/lib64/libhdf5_cpp.so \
		 -DINIPARSER_INCLUDE_DIR=${PKG_DIR}/iniparser/src \
		 -DINIPARSER_LIBRARY=${PKG_DIR}/iniparser/libiniparser.a \
		 -DCOMPEARTH_INCLUDE_DIR=${PKG_DIR}/compearth/momenttensor/c_src/include \
		 -DCOMPEARTH_LIBRARY=${PKG_DIR}/compearth/momenttensor/c_src/lib/libcompearth_shared.so \
		 -DISCL_INCLUDE_DIR=${PKG_DIR}/iscl/include \
		 -DISCL_LIBRARY=${PKG_DIR}/iscl/lib/libiscl_shared.so \
		 -DEW_BUILD_FLAGS="-Dlinux -D_LINUX -D_INTEL -D_USE_SCHED -D_USE_PTHREADS" \
		 -DEW_INCLUDE_DIR=${EW_DIR}/include \
		 -DEW_LIBRARY="${EW_DIR}/lib/libew_mt.a;${EW_DIR}/lib/libew_util.a" \
		 -DLIBXML2_LIBRARY=/usr/lib64/libxml2.so.2 \
		 -DLIBXML2_INCLUDE_DIR=/usr/include/libxml2 \
		 -DUW_AMAZON=FALSE 

fi
