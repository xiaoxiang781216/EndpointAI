/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_cmplx_mult_cmplx_q15.c
 * Description:  Q15 complex-by-complex multiplication
 *               using Helium assembly kernels
 *               This version allows boosting Q.15 complex-by-complex performance when using compilers having suboptimal
 *               Helium intrinsic code generation.
 *
 * $Date:        Jan 2022
 * $Revision:    V1.0.0
 *
 * Target Processor: Cortex-M cores with Helium
 * -------------------------------------------------------------------- */
/*
 * Copyright (C) 2010-2022 ARM Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "dsp/complex_math_functions.h"

/**
  @ingroup groupCmplxMath
 */

/**
  @addtogroup CmplxByCmplxMult
  @{
 */

/**
  @brief         Q15 complex-by-complex multiplication.
  @param[in]     pSrcA       points to first input vector
  @param[in]     pSrcB       points to second input vector
  @param[out]    pDst        points to output vector
  @param[in]     numSamples  number of samples in each vector
  @return        none

  @par           Scaling and Overflow Behavior
                   The function implements 1.15 by 1.15 multiplications and finally output is converted into 3.13 format.
 */

#if defined(ARM_MATH_MVEI)

void arm_cmplx_mult_cmplx_q15_mve(
  const q15_t * pSrcA,
  const q15_t * pSrcB,
        q15_t * pDst,
        uint32_t numSamples)
{
#ifdef USE_ASM
    uint32_t  tail = numSamples & 7;

    /* Core loop is unrolled to optimize load / multiplication interleaving */
    __asm volatile (
        "curCpx           .req q2                       \n"

        /* preload */
        "   vldrh.16        q0, [%[pA]], #16            \n"
        "   vldrh.16        q1, [%[pB]], #16            \n"

        /*
         * Complex vector Multiplication loop
         * Produces 8 complex elements
         */
        ".p2align 2                                     \n"
        "   wls             lr, %[cnt], 1f              \n"
        /* low overhead loop start */
        "2:                                             \n"
        "   vqdmlsdh.s16    curCpx, q0, q1              \n"
        "   vldrh.16        q3, [%[pA]], 16             \n"
        "   vqdmladhx.s16   curCpx, q0, q1              \n"
        "   vldrh.16        q1, [%[pB]], 16             \n"
        "   vshr.s16        curCpx, curCpx, #2          \n"
        /* store 4 complex pairs */
        "   vstrh.16        curCpx, [%[pD]], 16         \n"

        "   vqdmlsdh.s16    curCpx, q3, q1              \n"
        "   vldrh.16        q0, [%[pA]], 16             \n"
        "   vqdmladhx.s16   curCpx, q3, q1              \n"
        "   vldrh.16        q1, [%[pB]], 16             \n"
        "   vshr.s16        curCpx, curCpx, #2          \n"
        /* store 4 complex pairs */
        "   vstrh.16        curCpx, [%[pD]], 16         \n"

        /* low overhead loop end */
        "   le              lr, 2b                      \n"
        "1:                                             \n"

        /* tail handling */
        "   wlstp.16        lr, %[tail], 1f             \n"
        "2:                                             \n"
        "   vqdmlsdh.s16    curCpx, q0, q1              \n"
        "   vqdmladhx.s16   curCpx, q0, q1              \n"
        "   vshr.s16        curCpx, curCpx, #2          \n"
        "   vstrh.16        curCpx, [%[pD]], 16         \n"
        "   vldrh.16        q0, [%[pA]], 16             \n"
        "   vldrh.16        q1, [%[pB]], 16             \n"
        "   letp            lr, 2b                      \n"
        "1:                                             \n"

        " .unreq curCpx                                 \n"

        :[pD] "+r"(pDst), [pA] "+r"(pSrcA), [pB] "+r"(pSrcB)
        :[cnt] "r"(numSamples / 8), [tail] "r" (tail * CMPLX_DIM)
        : "q0", "q1", "q2", "q3",
          "memory", "r14");

#else
    /* CMSIS DSP MVE intrinsic reference */

   int32_t         blkCnt;
    q15x8_t         vecSrcA, vecSrcB;
    q15x8_t         vecSrcC, vecSrcD;
    q15x8_t         vecDst;

    blkCnt = (numSamples >> 3);
    blkCnt -= 1;
    if (blkCnt > 0)
    {
        /* should give more freedom to generate stall free code */
        vecSrcA = vld1q(pSrcA);
        vecSrcB = vld1q(pSrcB);
        pSrcA += 8;
        pSrcB += 8;

        while (blkCnt > 0)
        {

            /* C[2 * i] = A[2 * i] * B[2 * i] - A[2 * i + 1] * B[2 * i + 1].  */
            vecDst = vqdmlsdhq(vuninitializedq_s16(), vecSrcA, vecSrcB);
            vecSrcC = vld1q(pSrcA);
            pSrcA += 8;

            /* C[2 * i + 1] = A[2 * i] * B[2 * i + 1] + A[2 * i + 1] * B[2 * i].  */
            vecDst = vqdmladhxq(vecDst, vecSrcA, vecSrcB);
            vecSrcD = vld1q(pSrcB);
            pSrcB += 8;

            vstrhq_s16(pDst, vshrq(vecDst, 2));
            pDst += 8;

            vecDst = vqdmlsdhq(vuninitializedq_s16(), vecSrcC, vecSrcD);
            vecSrcA = vld1q(pSrcA);
            pSrcA += 8;

            vecDst = vqdmladhxq(vecDst, vecSrcC, vecSrcD);
            vecSrcB = vld1q(pSrcB);
            pSrcB += 8;

            vstrhq_s16(pDst, vshrq(vecDst, 2));
            pDst += 8;

            /*
             * Decrement the blockSize loop counter
             */
            blkCnt--;
        }

        /* process last elements out of the loop avoid the armclang breaking the SW pipeline */
        vecDst = vqdmlsdhq(vuninitializedq_s16(), vecSrcA, vecSrcB);
        vecSrcC = vld1q(pSrcA);

        vecDst = vqdmladhxq(vecDst, vecSrcA, vecSrcB);
        vecSrcD = vld1q(pSrcB);

        vstrhq_s16(pDst, vshrq(vecDst, 2));
        pDst += 8;

        vecDst = vqdmlsdhq(vuninitializedq_s16(), vecSrcC, vecSrcD);
        vecDst = vqdmladhxq(vecDst, vecSrcC, vecSrcD);

        vstrhq_s16(pDst, vshrq(vecDst, 2));
        pDst += 8;

        /*
         * tail
         */
        blkCnt = CMPLX_DIM * (numSamples & 7);
        do
        {
            mve_pred16_t    p = vctp16q(blkCnt);

            pSrcA += 8;
            pSrcB += 8;

            vecSrcA = vldrhq_z_s16(pSrcA, p);
            vecSrcB = vldrhq_z_s16(pSrcB, p);

            vecDst = vqdmlsdhq_m(vuninitializedq_s16(), vecSrcA, vecSrcB, p);
            vecDst = vqdmladhxq_m(vecDst, vecSrcA, vecSrcB, p);

            vecDst = vshrq_m(vuninitializedq_s16(), vecDst, 2, p);
            vstrhq_p_s16(pDst, vecDst, p);
            pDst += 8;

            blkCnt -= 8;
        }
        while ((int32_t) blkCnt > 0);
    }
    else
    {
        blkCnt = numSamples * CMPLX_DIM;
        while (blkCnt > 0) {
            mve_pred16_t    p = vctp16q(blkCnt);

            vecSrcA = vldrhq_z_s16(pSrcA, p);
            vecSrcB = vldrhq_z_s16(pSrcB, p);

            vecDst = vqdmlsdhq_m(vuninitializedq_s16(), vecSrcA, vecSrcB, p);
            vecDst = vqdmladhxq_m(vecDst, vecSrcA, vecSrcB, p);

            vecDst = vshrq_m(vuninitializedq_s16(), vecDst, 2, p);
            vstrhq_p_s16(pDst, vecDst, p);

            pDst += 8;
            pSrcA += 8;
            pSrcB += 8;

            blkCnt -= 8;
    }
  }
#endif
}
#endif
/**
  @} end of CmplxByCmplxMult group
 */
