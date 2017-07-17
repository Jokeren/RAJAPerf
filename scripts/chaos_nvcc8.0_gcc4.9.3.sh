#!/bin/bash

##
## Copyright (c) 2017, Lawrence Livermore National Security, LLC.
##
## Produced at the Lawrence Livermore National Laboratory.
##
## LLNL-CODE-xxxxxx
##
## All rights reserved.
##
## This file is part of the RAJA Performance Suite.
##
## For more information, see the file LICENSE in the top-level directory.
##

rm -rf build_chaos-nvcc8.0_gcc4.9.3 2>/dev/null
mkdir build_chaos-nvcc8.0_gcc4.9.3 && cd build_chaos-nvcc8.0_gcc4.9.3

PERFSUITE_DIR=$(git rev-parse --show-toplevel)

cmake \
  -DCMAKE_BUILD_TYPE=Release \
  -C ${PERFSUITE_DIR}/host-configs/chaos/nvcc_gcc4_9_3.cmake \
  -DENABLE_OPENMP=On \
  -DENABLE_CUDA=On \
  -DCUDA_TOOLKIT_ROOT_DIR=/opt/cudatoolkit-8.0 \
  -DPERFSUITE_ENABLE_WARNINGS=Off \
  -DENABLE_ALL_WARNINGS=Off \
  -DRAJA_ENABLE_TESTS=Off \
  -DCMAKE_INSTALL_PREFIX=../install_chaos-gcc-4.9.3 \
  "$@" \
  ${PERFSUITE_DIR}
