/*!
 ******************************************************************************
 *
 * \file
 *
 * \brief   Implementation file for Stream kernel DOT.
 *
 ******************************************************************************
 */

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Copyright (c) 2017, Lawrence Livermore National Security, LLC.
//
// Produced at the Lawrence Livermore National Laboratory
//
// LLNL-CODE-xxxxxx
//
// All rights reserved.
//
// This file is part of the RAJA Performance Suite.
//
// For additional details, please read the file LICENSE.
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//


#include "DOT.hpp"

#include "common/DataUtils.hpp"

#include "RAJA/RAJA.hpp"
#include "RAJA/policy/cuda.hpp"

#include <iostream>

#define USE_THRUST
//#undef USE_THRUST


#if defined(RAJA_ENABLE_CUDA) && defined(USE_THRUST)
#include <thrust/device_vector.h>
#include <thrust/inner_product.h>
#endif

namespace rajaperf 
{
namespace stream
{

#define DOT_DATA \
  ResReal_ptr a = m_a; \
  ResReal_ptr b = m_b;

#define DOT_BODY  \
  dot += a[i] * b[i] ;


#if defined(RAJA_ENABLE_CUDA)

  //
  // Define thread block size for CUDA execution
  //
  const size_t block_size = 256;


#define DOT_DATA_SETUP_CUDA \
  Real_ptr a; \
  Real_ptr b; \
\
  allocAndInitCudaDeviceData(a, m_a, iend); \
  allocAndInitCudaDeviceData(b, m_b, iend);

#define DOT_DATA_TEARDOWN_CUDA \
  deallocCudaDeviceData(a); \
  deallocCudaDeviceData(b);

#if defined(USE_THRUST)
// Nothing to do here...
#else
__global__ void dot(Real_ptr a, Real_ptr b,
                    Real_ptr dprod, Real_type dprod_init,
                    Index_type iend) 
{
  extern __shared__ Real_type pdot[ ];

  Index_type i = blockIdx.x * blockDim.x + threadIdx.x;

  pdot[ threadIdx.x ] = dprod_init; 
  for ( ; i < iend ; i += gridDim.x * blockDim.x ) {
    pdot[ threadIdx.x ] += a[ i ] * b[i];
  }
  __syncthreads();

  for ( i = blockDim.x / 2; i > 0; i /= 2 ) {
    if ( threadIdx.x < i ) {
      pdot[ threadIdx.x ] += pdot[ threadIdx.x + i ];
    }
     __syncthreads();
  }

#if 1 // serialized access to shared data;
  if ( threadIdx.x == 0 ) {
    RAJA::_atomicAdd( dprod, pdot[ 0 ] );
  }
#else // this doesn't work due to data races
  if ( threadIdx.x == 0 ) {
    *dprod += pdot[ 0 ];
  }
#endif

}
#endif

#endif // if defined(RAJA_ENABLE_CUDA)


DOT::DOT(const RunParams& params)
  : KernelBase(rajaperf::Stream_DOT, params)
{
   setDefaultSize(1000000);
   setDefaultReps(1000);
}

DOT::~DOT() 
{
}

void DOT::setUp(VariantID vid)
{
  allocAndInitData(m_a, getRunSize(), vid);
  allocAndInitData(m_b, getRunSize(), vid);

  m_dot = 0.0;
  m_dot_init = 0.0;
}

void DOT::runKernel(VariantID vid)
{
  const Index_type run_reps = getRunReps();
  const Index_type ibegin = 0;
  const Index_type iend = getRunSize();

  switch ( vid ) {

    case Base_Seq : {

      DOT_DATA;

      startTimer();
      for (RepIndex_type irep = 0; irep < run_reps; ++irep) {

        Real_type dot = m_dot_init;

        for (Index_type i = ibegin; i < iend; ++i ) {
          DOT_BODY;
        }

         m_dot += dot;

      }
      stopTimer();

      break;
    }

    case RAJA_Seq : {

      DOT_DATA;

      startTimer();
      for (RepIndex_type irep = 0; irep < run_reps; ++irep) {

        RAJA::ReduceSum<RAJA::seq_reduce, Real_type> dot(m_dot_init);

        RAJA::forall<RAJA::simd_exec>(ibegin, iend, [=](Index_type i) {
          DOT_BODY;
        });

        m_dot += static_cast<Real_type>(dot.get());

      }
      stopTimer();

      break;
    }

#if defined(_OPENMP)
    case Base_OpenMP : {

      DOT_DATA;

      startTimer();
      for (RepIndex_type irep = 0; irep < run_reps; ++irep) {

        Real_type dot = m_dot_init;

        #pragma omp parallel for reduction(+:dot)
        for (Index_type i = ibegin; i < iend; ++i ) {
          DOT_BODY;
        }

        m_dot += dot;

      }
      stopTimer();

      break;
    }

    case RAJALike_OpenMP : {
      // case is not defined...
      break;
    }

    case RAJA_OpenMP : {

      DOT_DATA;

      startTimer();
      for (RepIndex_type irep = 0; irep < run_reps; ++irep) {

        RAJA::ReduceSum<RAJA::omp_reduce, Real_type> dot(m_dot_init);

        RAJA::forall<RAJA::omp_parallel_for_exec>(ibegin, iend, 
          [=](Index_type i) {
          DOT_BODY;
        });

        m_dot += static_cast<Real_type>(dot.get());

      }
      stopTimer();

      break;
    }
#endif

#if defined(RAJA_ENABLE_CUDA)
    case Base_CUDA : {

#if defined(USE_THRUST)

      thrust::device_vector<Real_type> va(m_a, m_a+iend);
      thrust::device_vector<Real_type> vb(m_b, m_b+iend);

      startTimer();
      for (RepIndex_type irep = 0; irep < run_reps; ++irep) {

        Real_type dprod = thrust::inner_product(va.begin(), va.end(), 
                                                vb.begin(), m_dot_init);

        m_dot += dprod;

      }
      stopTimer();
      
#else
      DOT_DATA_SETUP_CUDA;
      Real_ptr dprod;
      allocAndInitCudaDeviceData(dprod, &m_dot_init, 1);

      startTimer();
      for (RepIndex_type irep = 0; irep < run_reps; ++irep) {

        initCudaDeviceData(dprod, &m_dot_init, 1);

        const size_t grid_size = RAJA_DIVIDE_CEILING_INT(iend, block_size);
        dot<<<grid_size, block_size, 
              sizeof(Real_type)*block_size>>>( a, b, 
                                               dprod, m_dot_init,
                                               iend ); 

        Real_type lprod;
        Real_ptr plprod = &lprod;
        getCudaDeviceData(plprod, dprod, 1);
        m_dot += lprod;  

      }
      stopTimer();

      DOT_DATA_TEARDOWN_CUDA;
      deallocCudaDeviceData(dprod);
#endif

      break; 
    }

    case RAJA_CUDA : {

      DOT_DATA_SETUP_CUDA;

      startTimer();
      for (RepIndex_type irep = 0; irep < run_reps; ++irep) {

         RAJA::ReduceSum<RAJA::cuda_reduce<block_size>, Real_type> dot(m_dot_init);

         RAJA::forall< RAJA::cuda_exec<block_size, true /*async*/> >(
           ibegin, iend, 
           [=] __device__ (Index_type i) {
           DOT_BODY;
         });

         m_dot += static_cast<Real_type>(dot.get());

      }
      stopTimer();

      DOT_DATA_TEARDOWN_CUDA;

      break;
    }
#endif

#if 0
    case Base_OpenMP4x :
    case RAJA_OpenMP4x : {
      // Fill these in later...you get the idea...
      break;
    }
#endif

    default : {
      std::cout << "\n  Unknown variant id = " << vid << std::endl;
    }

  }

}

void DOT::updateChecksum(VariantID vid)
{
  checksum[vid] += m_dot;
}

void DOT::tearDown(VariantID vid)
{
  (void) vid;
  deallocData(m_a);
  deallocData(m_b);
}

} // end namespace stream
} // end namespace rajaperf
