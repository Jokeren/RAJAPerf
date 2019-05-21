#!/usr/bin/env bash

###############################################################################
# Copyright (c) 2017-19, Lawrence Livermore National Security, LLC
# and RAJA Performance Suite project contributors.
# See the RAJAPerf/COPYRIGHT file for details.
#
# SPDX-License-Identifier: (BSD-3-Clause)
#################################################################################

BUILD_SUFFIX=nvcc10.1
RAJA_HOSTCONFIG=../nvcc10.1.cmake

rm -rf build_${BUILD_SUFFIX} >/dev/null
mkdir build_${BUILD_SUFFIX} && cd build_${BUILD_SUFFIX}

CUB_HOME=`echo ${CUB_HOME}`

cmake \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -C ${RAJA_HOSTCONFIG} \
  -DENABLE_EXTERNAL_CUB=On \
  -DCUB_INCLUDE_DIRS=${CUB_HOME} \
  -DENABLE_CUDA=On \
  -DCUDA_ARCH=sm_70 \
  -DCMAKE_INSTALL_PREFIX=../install_${BUILD_SUFFIX} \
  "$@" \
  ..
