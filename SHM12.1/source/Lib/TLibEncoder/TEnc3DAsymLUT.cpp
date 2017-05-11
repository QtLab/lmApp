/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * Copyright (c) 2010-2016, ITU/ISO/IEC
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *  * Neither the name of the ITU/ISO/IEC nor the names of its contributors may
 *    be used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

/** \file     TEnc3DAsymLUT.cpp
    \brief    TEnc3DAsymLUT encoder class
*/

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <algorithm>

#include "TEnc3DAsymLUT.h"

#if CGS_3D_ASYMLUT
TEnc3DAsymLUT::TEnc3DAsymLUT()
{
  m_pColorInfo = NULL;
  m_pColorInfoC = NULL;
  m_pEncCuboid = NULL;

  m_pBestEncCuboid = NULL;
  m_nAccuFrameBit = 0;
  m_nAccuFrameCGSBit = 0;
  m_nPrevFrameCGSPartNumLog2 = 0;
  m_dTotalFrameBit = 0;
  m_nTotalCGSBit = 0;
  m_nPPSBit = 0;
  m_pDsOrigPic = NULL;
#if R0179_ENC_OPT_3DLUT_SIZE
  m_pMaxColorInfo = NULL;
  m_pMaxColorInfoC = NULL;

  
  // fixed m_dDistFactor
  Double dTmpFactor[3];   
  dTmpFactor[I_SLICE] = 1.0;
  dTmpFactor[P_SLICE] = 4./3.;
  dTmpFactor[B_SLICE] = 1.5; 
  for( Int iSliceType = 0; iSliceType < 3; iSliceType++) 
  {
    for(Int iLayer = 0; iLayer < MAX_TLAYER; iLayer++)
    {
      m_dDistFactor[iSliceType][iLayer] = dTmpFactor[iSliceType]*(Double)(1<<iLayer);      
    }
  }
  // initialization with approximate number of bits to code the LUT
  m_nNumLUTBits[0][0] = 200; // 1x1x1
  m_nNumLUTBits[1][0] = 400; // 2x1x1  
  m_nNumLUTBits[1][1] = 1500; // 2x2x2  
  m_nNumLUTBits[2][0] = 800; // 4x1x1
  m_nNumLUTBits[2][1] = 3200; // 4x2x2  
  m_nNumLUTBits[2][2] = 8500; // 4x4x4  
  m_nNumLUTBits[3][0] = 1200; // 8x1x1
  m_nNumLUTBits[3][1] = 4500; // 8x2x2  
  m_nNumLUTBits[3][2] = 10000; // 8x4x4  
  m_nNumLUTBits[3][3] = 12000; // 8x8x8
#endif
}

Void TEnc3DAsymLUT::create( Int nMaxOctantDepth, Int nInputBitDepth, Int nInputBitDepthC, Int nOutputBitDepth, Int nOutputBitDepthC, Int nMaxYPartNumLog2 )
{
  if( m_pColorInfo != NULL )
  {
    destroy();
  }

  TCom3DAsymLUT::create( nMaxOctantDepth, nInputBitDepth, nInputBitDepthC, nOutputBitDepth, nOutputBitDepthC, nMaxYPartNumLog2, 1 << ( nInputBitDepthC - 1 ), 1 << ( nInputBitDepthC - 1 ) );

  xAllocate3DArray( m_pColorInfo , xGetYSize() , xGetUSize() , xGetVSize() );
  xAllocate3DArray( m_pColorInfoC , xGetYSize() , xGetUSize() , xGetVSize() );
  xAllocate3DArray( m_pEncCuboid , xGetYSize() , xGetUSize() , xGetVSize() );
  xAllocate3DArray( m_pBestEncCuboid , xGetYSize() , xGetUSize() , xGetVSize() );
#if R0179_ENC_OPT_3DLUT_SIZE
  xAllocate3DArray( m_pMaxColorInfo , xGetYSize() , xGetUSize() , xGetVSize() );
  xAllocate3DArray( m_pMaxColorInfoC , xGetYSize() , xGetUSize() , xGetVSize() );

  m_pEncCavlc = new TEncCavlc;
  m_pBitstreamRedirect = new TComOutputBitstream;
  m_pEncCavlc->setBitstream(m_pBitstreamRedirect);
#endif
}

Void TEnc3DAsymLUT::destroy()
{
  xFree3DArray( m_pColorInfo );
  xFree3DArray( m_pColorInfoC );
  xFree3DArray( m_pEncCuboid );
  xFree3DArray( m_pBestEncCuboid );
#if R0179_ENC_OPT_3DLUT_SIZE
  xFree3DArray( m_pMaxColorInfo );
  xFree3DArray( m_pMaxColorInfoC );
  delete m_pBitstreamRedirect;
  delete m_pEncCavlc;
#endif 
  TCom3DAsymLUT::destroy();
}

TEnc3DAsymLUT::~TEnc3DAsymLUT()
{
  if( m_dTotalFrameBit != 0 )
  {
    printf( "\nTotal CGS bits: %d, %.2lf%%\n\n" , m_nTotalCGSBit , m_nTotalCGSBit * 100 / m_dTotalFrameBit );
  }

  destroy();
}

Double TEnc3DAsymLUT::xDeriveVertexPerColor( Double N, Double Ys, Double Yy, Double Yu, Double Yv, Double ys, Double us, Double vs, Double yy, Double yu, Double yv, Double uu, Double uv, Double vv, Double YY,
                                             Pel & rP0, Pel & rP1, Pel & rP3, Pel & rP7, Int nResQuantBit )
{
  Int nInitP0 = rP0;
  Int nInitP1 = rP1;
  Int nInitP3 = rP3;
  Int nInitP7 = rP7;

  const Int nOne = xGetNormCoeffOne();
  Double dNorm = (N * yy * vv * uu - N * yy * uv * uv - N * yv * yv * uu - N * vv * yu * yu + 2 * N * yv * uv * yu - yy * vs * vs * uu + 2 * yy * vs * uv * us - yy * vv * us * us - 2 * vs * uv * yu * ys + uv * uv * ys * ys + vs * vs * yu * yu - 2 * yv * vs * us * yu + 2 * yv * vs * ys * uu - 2 * yv * uv * us * ys + 2 * vv * yu * ys * us - vv * uu * ys * ys + yv * yv * us * us);
  if( N > 16 && dNorm != 0 )
  {
    Double dInitA = (-N * uu * yv * Yv + N * uu * Yy * vv - N * Yy * uv * uv + N * yv * uv * Yu - N * yu * Yu * vv + N * yu * uv * Yv + yu * us * Ys * vv - vs * ys * uv * Yu - yu * vs * us * Yv - yv * uv * us * Ys - yv * vs * us * Yu - yu * uv * vs * Ys - ys * us * uv * Yv + ys * us * Yu * vv + 2 * Yy * vs * uv * us + uu * yv * vs * Ys - uu * ys * Ys * vv + uu * vs * ys * Yv + ys * Ys * uv * uv - Yy * vv * us * us + yu * Yu * vs * vs + yv * Yv * us * us - uu * Yy * vs * vs) / dNorm;
    Double dInitB = (N * yy * Yu * vv - N * yy * uv * Yv - N * Yu * yv * yv - N * yu * Yy * vv + N * uv * yv * Yy + N * yv * yu * Yv - yy * us * Ys * vv + yy * uv * vs * Ys - yy * Yu * vs * vs + yy * vs * us * Yv - uv * vs * ys * Yy - yv * yu * vs * Ys + yu * Yy * vs * vs + yu * ys * Ys * vv - uv * yv * ys * Ys + 2 * Yu * yv * vs * ys + us * ys * Yy * vv - vs * ys * yu * Yv + uv * ys * ys * Yv + us * Ys * yv * yv - Yu * ys * ys * vv - yv * ys * us * Yv - vs * us * yv * Yy) / dNorm;
    Double dInitC = -(-N * yy * Yv * uu + N * yy * uv * Yu - N * yv * yu * Yu - N * uv * yu * Yy + N * Yv * yu * yu + N * yv * Yy * uu - yy * uv * us * Ys + yy * Yv * us * us + yy * vs * Ys * uu - yy * vs * us * Yu + yv * ys * us * Yu - vs * Ys * yu * yu - yv * ys * Ys * uu + vs * us * yu * Yy + vs * ys * yu * Yu - uv * Yu * ys * ys + Yv * uu * ys * ys - yv * Yy * us * us - 2 * Yv * yu * ys * us - vs * ys * Yy * uu + uv * us * ys * Yy + uv * yu * ys * Ys + yv * yu * us * Ys) / dNorm;
    nInitP0 = ( Int )( dInitA * nOne + 0.5 ) >> nResQuantBit << nResQuantBit;
    nInitP1 = ( Int )( dInitB * nOne + 0.5 ) >> nResQuantBit << nResQuantBit;
    nInitP3 = ( Int )( dInitC * nOne + 0.5 ) >> nResQuantBit << nResQuantBit;
  }

  Int nMin = - ( 1 << ( m_nLUTBitDepth - 1 ) );
  Int nMax = - nMin - ( 1 << nResQuantBit  );
  Int nMask = ( 1 << nResQuantBit ) - 1;

  Double dMinError = MAX_DOUBLE;
  Int nTestRange = 2;
  Int nStepSize = 1 << nResQuantBit;
  for( Int i = - nTestRange ; i <= nTestRange ; i++ )
  {
    for( Int j = - nTestRange ; j <= nTestRange ; j++ )
    {
      for( Int k = - nTestRange ; k <= nTestRange ; k++ )
      {
        Int nTestP0 = Clip3( nMin , nMax , nInitP0 + i * nStepSize );
        Int nTestP1 = Clip3( nMin , nMax , nInitP1 + j * nStepSize );
        Int nTestP3 = Clip3( nMin , nMax , nInitP3 + k * nStepSize );
        Double a = 1.0 * nTestP0 / nOne;
        Double b = 1.0 * nTestP1 / nOne;
        Double c = 1.0 * nTestP3 / nOne;
        Double d = ( Ys - a * ys - b * us - c * vs ) / N;
        nInitP7 = ( ( Int )d ) >> nResQuantBit << nResQuantBit;
        for( Int m = 0 ; m < 2 ; m++ )
        {
          Int nTestP7 = Clip3( nMin , nMax , nInitP7 + m * nStepSize );
          Double dError = xCalEstDist( N , Ys , Yy , Yu , Yv , ys , us , vs , yy , yu , yv , uu , uv , vv , YY , a , b , c , nTestP7 );
          if( dError < dMinError )
          {
            dMinError = dError;
            rP0 = ( Pel )nTestP0;
            rP1 = ( Pel )nTestP1;
            rP3 = ( Pel )nTestP3;
            rP7 = ( Pel )nTestP7;
          }
        }
      }
    }
  }
  assert( !( rP0 & nMask ) && !( rP1 & nMask ) && !( rP3 & nMask ) && !( rP7 & nMask ) );

  return( dMinError );
}

Double TEnc3DAsymLUT::estimateDistWithCur3DAsymLUT( TComPic * pCurPic, UInt refLayerIdc )
{
  xCollectData( pCurPic , refLayerIdc );

  Double dErrorLuma = 0 , dErrorChroma = 0;
  Int nYSize = 1 << ( getCurOctantDepth() + getCurYPartNumLog2() );
  Int nUVSize = 1 << getCurOctantDepth();
  for( Int yIdx = 0 ; yIdx < nYSize ; yIdx++ )
  {
    for( Int uIdx = 0 ; uIdx < nUVSize ; uIdx++ )
    {
      for( Int vIdx = 0 ; vIdx < nUVSize ; vIdx++ )
      {
        SColorInfo & rCuboidColorInfo = m_pColorInfo[yIdx][uIdx][vIdx];
        SColorInfo & rCuboidColorInfoC = m_pColorInfoC[yIdx][uIdx][vIdx];
        SCuboid & rCuboid = xGetCuboid( yIdx , uIdx , vIdx );
        if( rCuboidColorInfo.N > 0 )
        {
#if SCALABLE_REXT
          dErrorLuma += xCalEstPelDist( rCuboidColorInfo.N , rCuboidColorInfo.Ys , rCuboidColorInfo.Yy , rCuboidColorInfo.Yu , rCuboidColorInfo.Yv , rCuboidColorInfo.ys , rCuboidColorInfo.us , rCuboidColorInfo.vs , rCuboidColorInfo.yy , rCuboidColorInfo.yu , rCuboidColorInfo.yv , rCuboidColorInfo.uu , rCuboidColorInfo.uv , rCuboidColorInfo.vv , rCuboidColorInfo.YY ,
            rCuboid.P[0].Y , rCuboid.P[1].Y , rCuboid.P[2].Y , rCuboid.P[3].Y );
#else
          dErrorLuma += xCalEstDist( rCuboidColorInfo.N , rCuboidColorInfo.Ys , rCuboidColorInfo.Yy , rCuboidColorInfo.Yu , rCuboidColorInfo.Yv , rCuboidColorInfo.ys , rCuboidColorInfo.us , rCuboidColorInfo.vs , rCuboidColorInfo.yy , rCuboidColorInfo.yu , rCuboidColorInfo.yv , rCuboidColorInfo.uu , rCuboidColorInfo.uv , rCuboidColorInfo.vv , rCuboidColorInfo.YY ,
            rCuboid.P[0].Y , rCuboid.P[1].Y , rCuboid.P[2].Y , rCuboid.P[3].Y );
#endif
        }
        if( rCuboidColorInfoC.N > 0 )
        {
#if SCALABLE_REXT
          dErrorChroma += xCalEstPelDist( rCuboidColorInfoC.N , rCuboidColorInfoC.Us , rCuboidColorInfoC.Uy , rCuboidColorInfoC.Uu , rCuboidColorInfoC.Uv , rCuboidColorInfoC.ys , rCuboidColorInfoC.us , rCuboidColorInfoC.vs , rCuboidColorInfoC.yy , rCuboidColorInfoC.yu , rCuboidColorInfoC.yv , rCuboidColorInfoC.uu , rCuboidColorInfoC.uv , rCuboidColorInfoC.vv , rCuboidColorInfoC.UU ,
            rCuboid.P[0].U , rCuboid.P[1].U , rCuboid.P[2].U , rCuboid.P[3].U );
          dErrorChroma += xCalEstPelDist( rCuboidColorInfoC.N , rCuboidColorInfoC.Vs , rCuboidColorInfoC.Vy , rCuboidColorInfoC.Vu , rCuboidColorInfoC.Vv , rCuboidColorInfoC.ys , rCuboidColorInfoC.us , rCuboidColorInfoC.vs , rCuboidColorInfoC.yy , rCuboidColorInfoC.yu , rCuboidColorInfoC.yv , rCuboidColorInfoC.uu , rCuboidColorInfoC.uv , rCuboidColorInfoC.vv , rCuboidColorInfoC.VV ,
            rCuboid.P[0].V , rCuboid.P[1].V , rCuboid.P[2].V , rCuboid.P[3].V );
#else
          dErrorChroma += xCalEstDist( rCuboidColorInfoC.N , rCuboidColorInfoC.Us , rCuboidColorInfoC.Uy , rCuboidColorInfoC.Uu , rCuboidColorInfoC.Uv , rCuboidColorInfoC.ys , rCuboidColorInfoC.us , rCuboidColorInfoC.vs , rCuboidColorInfoC.yy , rCuboidColorInfoC.yu , rCuboidColorInfoC.yv , rCuboidColorInfoC.uu , rCuboidColorInfoC.uv , rCuboidColorInfoC.vv , rCuboidColorInfoC.UU ,
            rCuboid.P[0].U , rCuboid.P[1].U , rCuboid.P[2].U , rCuboid.P[3].U );
          dErrorChroma += xCalEstDist( rCuboidColorInfoC.N , rCuboidColorInfoC.Vs , rCuboidColorInfoC.Vy , rCuboidColorInfoC.Vu , rCuboidColorInfoC.Vv , rCuboidColorInfoC.ys , rCuboidColorInfoC.us , rCuboidColorInfoC.vs , rCuboidColorInfoC.yy , rCuboidColorInfoC.yu , rCuboidColorInfoC.yv , rCuboidColorInfoC.uu , rCuboidColorInfoC.uv , rCuboidColorInfoC.vv , rCuboidColorInfoC.VV ,
            rCuboid.P[0].V , rCuboid.P[1].V , rCuboid.P[2].V , rCuboid.P[3].V );
#endif
        }
      }
    }
  }

  return( dErrorLuma + dErrorChroma);
}

#if R0179_ENC_OPT_3DLUT_SIZE
Double TEnc3DAsymLUT::derive3DAsymLUT( TComSlice * pSlice, TComPic * pCurPic, UInt refLayerIdc, TEncCfg * pCfg, Bool bSignalPPS, Bool bElRapSliceTypeB, Double dFrameLambda )
{
  m_nLUTBitDepth = pCfg->getCGSLUTBit();

  Int nBestAdaptCThresholdU = 1 << ( getInputBitDepthC() - 1 );
  Int nBestAdaptCThresholdV = 1 << ( getInputBitDepthC() - 1 );
  Int nAdaptCThresholdU, nAdaptCThresholdV;

  Int nTmpLutBits[MAX_Y_SIZE][MAX_C_SIZE] ;
  memset(nTmpLutBits, 0, sizeof(nTmpLutBits)); 

  SLUTSize sMaxLutSize;  

  // collect stats for the most partitions 
  Int nCurYPartNumLog2 = 0 , nCurOctantDepth = 0; 
  Int nMaxPartNumLog2 = xGetMaxPartNumLog2();

  xMapPartNum2DepthYPart( nMaxPartNumLog2 , nCurOctantDepth , nCurYPartNumLog2 ); 
  xUpdatePartitioning( nCurOctantDepth , nCurYPartNumLog2, nBestAdaptCThresholdU, nBestAdaptCThresholdV ); 
  xCollectData( pCurPic , refLayerIdc );
  xCopyColorInfo(m_pMaxColorInfo, m_pColorInfo, m_pMaxColorInfoC, m_pColorInfoC); 
 
  sMaxLutSize.iCPartNumLog2 = nCurOctantDepth; 
  sMaxLutSize.iYPartNumLog2 = nCurOctantDepth + nCurYPartNumLog2; 

  m_pBitstreamRedirect->clear();

  // find the best partition based on RD cost 
  Int i; 
  Double dMinCost, dCurCost;

  Int iBestLUTSizeIdx = 0;   
  Int nBestResQuanBit = 0;
  Double dCurError, dMinError; 
  Int iNumBitsCurSize; 
  Int iNumBitsCurSizeSave = m_pEncCavlc->getNumberOfWrittenBits(); 
  Double dDistFactor = getDistFactor(pSlice->getSliceType(), pSlice->getDepth());

  // check all LUT sizes 
  xGetAllLutSizes(pSlice);  
  if (m_nTotalLutSizes == 0) // return if no valid size is found, LUT will not be updated
  {
    nCurOctantDepth = sMaxLutSize.iCPartNumLog2;
    nCurYPartNumLog2 = sMaxLutSize.iYPartNumLog2-nCurOctantDepth; 
    xUpdatePartitioning( nCurOctantDepth , nCurYPartNumLog2, nBestAdaptCThresholdU, nBestAdaptCThresholdV ); 
    return MAX_DOUBLE; 
  }

  dMinCost = MAX_DOUBLE; dMinError = MAX_DOUBLE;
  for (i = 0; i < m_nTotalLutSizes; i++)
  {
    // add up the stats
    nCurOctantDepth = m_sLutSizes[i].iCPartNumLog2;
    nCurYPartNumLog2 = m_sLutSizes[i].iYPartNumLog2-nCurOctantDepth; 
    xUpdatePartitioning( nCurOctantDepth , nCurYPartNumLog2, nBestAdaptCThresholdU, nBestAdaptCThresholdV ); 
    xConsolidateData( &m_sLutSizes[i], &sMaxLutSize );
  
    dCurError = xDeriveVertexes(nBestResQuanBit, m_pEncCuboid);

    setResQuantBit( nBestResQuanBit );
    xSaveCuboids( m_pEncCuboid ); 
    m_pEncCavlc->xCode3DAsymLUT( this ); 
    iNumBitsCurSize = m_pEncCavlc->getNumberOfWrittenBits();
    dCurCost = dCurError/dDistFactor + dFrameLambda*(Double)(iNumBitsCurSize-iNumBitsCurSizeSave);  
    nTmpLutBits[m_sLutSizes[i].iYPartNumLog2][m_sLutSizes[i].iCPartNumLog2] = iNumBitsCurSize-iNumBitsCurSizeSave; // store LUT size 
    iNumBitsCurSizeSave = iNumBitsCurSize;
    if(dCurCost < dMinCost )
    {
      SCuboid *** tmp = m_pBestEncCuboid;
      m_pBestEncCuboid = m_pEncCuboid;
      m_pEncCuboid = tmp;
      dMinCost = dCurCost; 
      dMinError = dCurError;
      iBestLUTSizeIdx = i; 
    }
  }

  nCurOctantDepth = m_sLutSizes[iBestLUTSizeIdx].iCPartNumLog2;
  nCurYPartNumLog2 = m_sLutSizes[iBestLUTSizeIdx].iYPartNumLog2-nCurOctantDepth; 

  xUpdatePartitioning( nCurOctantDepth , nCurYPartNumLog2, nBestAdaptCThresholdU, nBestAdaptCThresholdV ); 

  Bool bUseNewColorInfo = false; 
  if( pCfg->getCGSAdaptChroma() && nCurOctantDepth <= 1 ) // if the best size found so far has depth = 0 or 1, then check AdaptC U/V thresholds
  {
    nAdaptCThresholdU = ( Int )( m_dSumU / m_nNChroma + 0.5 );
    nAdaptCThresholdV = ( Int )( m_dSumV / m_nNChroma + 0.5 );
    if( !(nAdaptCThresholdU == nBestAdaptCThresholdU && nAdaptCThresholdV == nBestAdaptCThresholdV ) ) 
    {
      nCurOctantDepth = 1;
      if( nCurOctantDepth + nCurYPartNumLog2 > getMaxYPartNumLog2()+getMaxOctantDepth() )
        nCurYPartNumLog2 = getMaxYPartNumLog2()+getMaxOctantDepth()-nCurOctantDepth; 
      xUpdatePartitioning( nCurOctantDepth , nCurYPartNumLog2 , nAdaptCThresholdU , nAdaptCThresholdV );
      xCollectData( pCurPic , refLayerIdc );

      dCurError = xDeriveVertexes( nBestResQuanBit , m_pEncCuboid ) ;
      setResQuantBit( nBestResQuanBit );
      xSaveCuboids( m_pEncCuboid ); 
      m_pEncCavlc->xCode3DAsymLUT( this ); 
      iNumBitsCurSize = m_pEncCavlc->getNumberOfWrittenBits();
      dCurCost = dCurError/dDistFactor + dFrameLambda*(Double)(iNumBitsCurSize-iNumBitsCurSizeSave);  
      iNumBitsCurSizeSave = iNumBitsCurSize;
      if(dCurCost < dMinCost )
      {
        SCuboid *** tmp = m_pBestEncCuboid;
        m_pBestEncCuboid = m_pEncCuboid;
        m_pEncCuboid = tmp;
        dMinCost = dCurCost; 
        dMinError = dCurError;
        nBestAdaptCThresholdU = nAdaptCThresholdU;
        nBestAdaptCThresholdV = nAdaptCThresholdV;
        bUseNewColorInfo = true; 
      }
    }
  }

  xUpdatePartitioning( nCurOctantDepth , nCurYPartNumLog2, nBestAdaptCThresholdU, nBestAdaptCThresholdV ); 

  // check res_quant_bits only for the best table size and best U/V threshold
  if( !bUseNewColorInfo )
  {
    xConsolidateData( &m_sLutSizes[iBestLUTSizeIdx], &sMaxLutSize );
  }

  //    xCollectData( pCurPic , refLayerIdc );
  for( Int nResQuanBit = 1 ; nResQuanBit < 4 ; nResQuanBit++ )
  {
    dCurError = xDeriveVertexes( nResQuanBit , m_pEncCuboid );

    setResQuantBit( nResQuanBit );
    xSaveCuboids( m_pEncCuboid ); 
    m_pEncCavlc->xCode3DAsymLUT( this ); 
    iNumBitsCurSize = m_pEncCavlc->getNumberOfWrittenBits();
    dCurCost = dCurError/dDistFactor + dFrameLambda*(Double)(iNumBitsCurSize-iNumBitsCurSizeSave);   

    iNumBitsCurSizeSave = iNumBitsCurSize;
    if(dCurCost < dMinCost)
    {
      nBestResQuanBit = nResQuanBit;
      SCuboid *** tmp = m_pBestEncCuboid;
      m_pBestEncCuboid = m_pEncCuboid;
      m_pEncCuboid = tmp;
      dMinCost = dCurCost; 
      dMinError = dCurError;
    }
    else
    {
      break;
    }
  }
    
  setResQuantBit( nBestResQuanBit );
  xSaveCuboids( m_pBestEncCuboid );

  // update LUT size stats 
  for(Int iLutSizeY = 0; iLutSizeY < MAX_Y_SIZE; iLutSizeY++)
  {
    for(Int iLutSizeC = 0; iLutSizeC < MAX_C_SIZE; iLutSizeC++) 
    {
      if(nTmpLutBits[iLutSizeY][iLutSizeC] != 0) 
        m_nNumLUTBits[iLutSizeY][iLutSizeC] =  (m_nNumLUTBits[iLutSizeY][iLutSizeC] + nTmpLutBits[iLutSizeY][iLutSizeC]*3+2)>>2; // update with new stats
    }
  }

  // return cost rather than error
  return( dMinCost );
}
#endif 

Double TEnc3DAsymLUT::derive3DAsymLUT( TComSlice * pSlice, TComPic * pCurPic, UInt refLayerIdc, TEncCfg * pCfg, Bool bSignalPPS, Bool bElRapSliceTypeB )
{
  m_nLUTBitDepth = pCfg->getCGSLUTBit();
  Int nCurYPartNumLog2 = 0 , nCurOctantDepth = 0; 
  xDerivePartNumLog2( pSlice , pCfg , nCurOctantDepth , nCurYPartNumLog2 , bSignalPPS , bElRapSliceTypeB );

  Int nBestResQuanBit = 0;
  Int nBestAdaptCThresholdU = 1 << ( getInputBitDepthC() - 1 );
  Int nBestAdaptCThresholdV = 1 << ( getInputBitDepthC() - 1 );
  Int nBestOctantDepth = nCurOctantDepth;
  Int nBestYPartNumLog2 = nCurYPartNumLog2;
  Int nTargetLoop = 1 + ( pCfg->getCGSAdaptChroma() && ( nCurOctantDepth == 1 || ( nCurOctantDepth * 3 + nCurYPartNumLog2 ) >= 5 ) );
  Double dMinError = MAX_DOUBLE;
  for( Int nLoop = 0 ; nLoop < nTargetLoop ; nLoop++ )
  {
    Int nAdaptCThresholdU = 1 << ( getInputBitDepthC() - 1 );
    Int nAdaptCThresholdV = 1 << ( getInputBitDepthC() - 1 );
    if( nLoop > 0 )
    {
      nAdaptCThresholdU = ( Int )( m_dSumU / m_nNChroma + 0.5 );
      nAdaptCThresholdV = ( Int )( m_dSumV / m_nNChroma + 0.5 );
      if( nCurOctantDepth > 1 )
      {
        nCurOctantDepth = 1;
        nCurYPartNumLog2 = 2;
      }
      if( nAdaptCThresholdU == nBestAdaptCThresholdU && nAdaptCThresholdV == nBestAdaptCThresholdV 
        && nCurOctantDepth == nBestOctantDepth && nCurYPartNumLog2 == nBestYPartNumLog2 )
        break;
    }

    xUpdatePartitioning( nCurOctantDepth , nCurYPartNumLog2 , nAdaptCThresholdU , nAdaptCThresholdV );
    xCollectData( pCurPic , refLayerIdc );
    for( Int nResQuanBit = 0 ; nResQuanBit < 4 ; nResQuanBit++ )
    {
      Double dError = xDeriveVertexes( nResQuanBit , m_pEncCuboid ) / ( 1 + ( nResQuanBit > 0 ) * 0.001 * ( pSlice->getDepth() + 1 ) );
      if( dError <= dMinError )
      {
        nBestResQuanBit = nResQuanBit;
        nBestAdaptCThresholdU = nAdaptCThresholdU;
        nBestAdaptCThresholdV = nAdaptCThresholdV;
        nBestOctantDepth = nCurOctantDepth;
        nBestYPartNumLog2 = nCurYPartNumLog2;
        SCuboid *** tmp = m_pBestEncCuboid;
        m_pBestEncCuboid = m_pEncCuboid;
        m_pEncCuboid = tmp;
        dMinError = dError;
      }
      else
      {
        break;
      }
    }
  }

  setResQuantBit( nBestResQuanBit );
  xUpdatePartitioning( nBestOctantDepth, nBestYPartNumLog2, nBestAdaptCThresholdU, nBestAdaptCThresholdV );

  xSaveCuboids( m_pBestEncCuboid );
  return( dMinError );
}

Double TEnc3DAsymLUT::xDeriveVertexes( Int nResQuanBit, SCuboid *** pCurCuboid )
{
  Double dErrorLuma = 0 , dErrorChroma = 0;
  Int nYSize = 1 << ( getCurOctantDepth() + getCurYPartNumLog2() );
  Int nUVSize = 1 << getCurOctantDepth();

  for( Int yIdx = 0 ; yIdx < nYSize ; yIdx++ )
  {
    for( Int uIdx = 0 ; uIdx < nUVSize ; uIdx++ )
    {
      for( Int vIdx = 0 ; vIdx < nUVSize ; vIdx++ )
      {
        SColorInfo & rCuboidColorInfo = m_pColorInfo[yIdx][uIdx][vIdx];
        SColorInfo & rCuboidColorInfoC = m_pColorInfoC[yIdx][uIdx][vIdx];
        SCuboid & rCuboid = pCurCuboid[yIdx][uIdx][vIdx];

        for( Int idxVertex = 0 ; idxVertex < 4 ; idxVertex++ )
        {
          rCuboid.P[idxVertex] = xGetCuboidVertexPredAll( yIdx , uIdx , vIdx , idxVertex , pCurCuboid );
        }

        if( rCuboidColorInfo.N > 0 )
        {
          dErrorLuma += xDeriveVertexPerColor( rCuboidColorInfo.N , rCuboidColorInfo.Ys , rCuboidColorInfo.Yy , rCuboidColorInfo.Yu , rCuboidColorInfo.Yv , rCuboidColorInfo.ys , rCuboidColorInfo.us , rCuboidColorInfo.vs , rCuboidColorInfo.yy , rCuboidColorInfo.yu , rCuboidColorInfo.yv , rCuboidColorInfo.uu , rCuboidColorInfo.uv , rCuboidColorInfo.vv , rCuboidColorInfo.YY ,
            rCuboid.P[0].Y , rCuboid.P[1].Y , rCuboid.P[2].Y , rCuboid.P[3].Y , nResQuanBit );
        }

        if( rCuboidColorInfoC.N > 0 )
        {
          dErrorChroma += xDeriveVertexPerColor( rCuboidColorInfoC.N , rCuboidColorInfoC.Us , rCuboidColorInfoC.Uy , rCuboidColorInfoC.Uu , rCuboidColorInfoC.Uv , rCuboidColorInfoC.ys , rCuboidColorInfoC.us , rCuboidColorInfoC.vs , rCuboidColorInfoC.yy , rCuboidColorInfoC.yu , rCuboidColorInfoC.yv , rCuboidColorInfoC.uu , rCuboidColorInfoC.uv , rCuboidColorInfoC.vv , rCuboidColorInfoC.UU ,
            rCuboid.P[0].U , rCuboid.P[1].U , rCuboid.P[2].U , rCuboid.P[3].U , nResQuanBit );

          dErrorChroma += xDeriveVertexPerColor( rCuboidColorInfoC.N , rCuboidColorInfoC.Vs , rCuboidColorInfoC.Vy , rCuboidColorInfoC.Vu , rCuboidColorInfoC.Vv , rCuboidColorInfoC.ys , rCuboidColorInfoC.us , rCuboidColorInfoC.vs , rCuboidColorInfoC.yy , rCuboidColorInfoC.yu , rCuboidColorInfoC.yv , rCuboidColorInfoC.uu , rCuboidColorInfoC.uv , rCuboidColorInfoC.vv , rCuboidColorInfoC.VV ,
            rCuboid.P[0].V , rCuboid.P[1].V , rCuboid.P[2].V , rCuboid.P[3].V , nResQuanBit );
        }

        if( nResQuanBit > 0 )
        {
          // check quantization
          for( Int idxVertex = 0 ; idxVertex < 4 ; idxVertex++ )
          {
            SYUVP sPred = xGetCuboidVertexPredAll( yIdx , uIdx , vIdx , idxVertex , pCurCuboid );
            assert( ( ( rCuboid.P[idxVertex].Y - sPred.Y ) >> nResQuanBit << nResQuanBit ) == rCuboid.P[idxVertex].Y - sPred.Y );
            assert( ( ( rCuboid.P[idxVertex].U - sPred.U ) >> nResQuanBit << nResQuanBit ) == rCuboid.P[idxVertex].U - sPred.U );
            assert( ( ( rCuboid.P[idxVertex].V - sPred.V ) >> nResQuanBit << nResQuanBit ) == rCuboid.P[idxVertex].V - sPred.V );
          }
        }
      }
    }
  }

  return( dErrorLuma + dErrorChroma );
}

Void TEnc3DAsymLUT::xCollectData( TComPic * pCurPic, UInt refLayerIdc )
{
  Pel * pSrcY = m_pDsOrigPic->getAddr(COMPONENT_Y);
  Pel * pSrcU = m_pDsOrigPic->getAddr(COMPONENT_Cb);
  Pel * pSrcV = m_pDsOrigPic->getAddr(COMPONENT_Cr);
  Int nStrideSrcY = m_pDsOrigPic->getStride(COMPONENT_Y);
  Int nStrideSrcC = m_pDsOrigPic->getStride(COMPONENT_Cb);
  TComPicYuv *pRecPic = pCurPic->getSlice(pCurPic->getCurrSliceIdx())->getBaseColPic(refLayerIdc)->getPicYuvRec();
  Pel * pIRLY = pRecPic->getAddr(COMPONENT_Y);
  Pel * pIRLU = pRecPic->getAddr(COMPONENT_Cb);
  Pel * pIRLV = pRecPic->getAddr(COMPONENT_Cr);
  Int nStrideILRY = pRecPic->getStride(COMPONENT_Y);
  Int nStrideILRC = pRecPic->getStride(COMPONENT_Cb);
#if R0179_ENC_OPT_3DLUT_SIZE
  xReset3DArray( m_pColorInfo  , getMaxYSize() , getMaxCSize() , getMaxCSize() );
  xReset3DArray( m_pColorInfoC , getMaxYSize() , getMaxCSize() , getMaxCSize() );
#else
  xReset3DArray( m_pColorInfo , xGetYSize() , xGetUSize() , xGetVSize() );
  xReset3DArray( m_pColorInfoC , xGetYSize() , xGetUSize() , xGetVSize() );
#endif

  //alignment padding
  pRecPic->setBorderExtension( false );
  pRecPic->extendPicBorder();

  TComSlice * pSlice = pCurPic->getSlice(pCurPic->getCurrSliceIdx());
  UInt refLayerId = pSlice->getVPS()->getRefLayerId(pSlice->getLayerId(), refLayerIdc);
  Window scalEL = pSlice->getPPS()->getScaledRefLayerWindowForLayer(refLayerId); 
  TComPicYuv *pcRecPicBL = pSlice->getBaseColPic(refLayerIdc)->getPicYuvRec();
  // borders of down-sampled picture
  Int leftDS =  (scalEL.getWindowLeftOffset() * pSlice->getPic()->getPosScalingFactor(refLayerIdc, 0)+(1<<15))>>16;
  Int rightDS = pcRecPicBL->getWidth(COMPONENT_Y) - 1 + (((scalEL.getWindowRightOffset()) * pSlice->getPic()->getPosScalingFactor(refLayerIdc, 0)+(1<<15))>>16);
  Int topDS = (((scalEL.getWindowTopOffset()) * pSlice->getPic()->getPosScalingFactor(refLayerIdc, 1)+(1<<15))>>16);
  Int bottomDS = pcRecPicBL->getHeight(COMPONENT_Y) - 1 + (((scalEL.getWindowBottomOffset()) * pSlice->getPic()->getPosScalingFactor(refLayerIdc, 1)+(1<<15))>>16);
  // overlapped region
  Int left = max( 0 , leftDS );
  Int right = min( pcRecPicBL->getWidth(COMPONENT_Y) - 1 , rightDS );
  Int top = max( 0 , topDS );
  Int bottom = min( pcRecPicBL->getHeight(COMPONENT_Y) - 1 , bottomDS );
  // since we do data collection only for overlapped region, the border extension is good enough

  m_dSumU = m_dSumV = 0;
  m_nNChroma = 0;

  for( Int i = top ; i <= bottom ; i++ )
  {
    Int iDS = i-topDS;
    Int jDS = left-leftDS;
    Int posSrcY = iDS * nStrideSrcY + jDS;
    Int posIRLY = i * nStrideILRY + left;
    Int posSrcUV = ( iDS >> 1 ) * nStrideSrcC + (jDS>>1);
    Int posIRLUV = ( i >> 1 ) * nStrideILRC + (left>>1);
    for( Int j = left ; j <= right ; j++ , posSrcY++ , posIRLY++ , posSrcUV += !( j & 0x01 ) , posIRLUV += !( j & 0x01 ) )
    {
      Int Y = pSrcY[posSrcY];
      Int y = pIRLY[posIRLY];
      Int U = pSrcU[posSrcUV];
      Int u = pIRLU[posIRLUV];
      Int V = pSrcV[posSrcUV];
      Int v = pIRLV[posIRLUV];

      // alignment
      //filtering u, v for luma;
      Int posIRLUVN =  posIRLUV + ((i&1)? nStrideILRC : -nStrideILRC);
      if((j&1))
      {
        u = (pIRLU[posIRLUVN] + pIRLU[posIRLUVN+1] +(u + pIRLU[posIRLUV+1])*3 +4)>>3;
        v = (pIRLV[posIRLUVN] + pIRLV[posIRLUVN+1] +(v + pIRLV[posIRLUV+1])*3 +4)>>3;
      }
      else
      { 
        u = (pIRLU[posIRLUVN] +u*3 +2)>>2;
        v = (pIRLV[posIRLUVN] +v*3 +2)>>2;
      }

      m_dSumU += u;
      m_dSumV += v;
      m_nNChroma++;

      SColorInfo sColorInfo;
      SColorInfo & rCuboidColorInfo = m_pColorInfo[xGetYIdx(y)][xGetUIdx(u)][xGetVIdx(v)];

      memset(&sColorInfo, 0, sizeof(SColorInfo));

      sColorInfo.Ys = Y;
      sColorInfo.ys = y;
      sColorInfo.us = u;
      sColorInfo.vs = v;
      sColorInfo.Yy = Y * y;
      sColorInfo.Yu = Y * u;
      sColorInfo.Yv = Y * v;
      sColorInfo.yy = y * y;
      sColorInfo.yu = y * u;
      sColorInfo.yv = y * v;
      sColorInfo.uu = u * u;
      sColorInfo.uv = u * v;
      sColorInfo.vv = v * v;
      sColorInfo.YY = Y * Y;
      sColorInfo.N  = 1;

      rCuboidColorInfo += sColorInfo;

      if(!((i&1) || (j&1)))
      {
        // alignment
        y =  (pIRLY[posIRLY] + pIRLY[posIRLY+nStrideILRY] + 1)>>1;

        u = pIRLU[posIRLUV];
        v = pIRLV[posIRLUV];

        SColorInfo & rCuboidColorInfoC = m_pColorInfoC[xGetYIdx(y)][xGetUIdx(u)][xGetVIdx(v)];

        sColorInfo.Us = U;
        sColorInfo.Vs = V;
        sColorInfo.ys = y;
        sColorInfo.us = u;
        sColorInfo.vs = v;

        sColorInfo.Uy = U * y;
        sColorInfo.Uu = U * u;
        sColorInfo.Uv = U * v;
        sColorInfo.Vy = V * y;
        sColorInfo.Vu = V * u;
        sColorInfo.Vv = V * v;
        sColorInfo.yy = y * y;
        sColorInfo.yu = y * u;
        sColorInfo.yv = y * v;
        sColorInfo.uu = u * u;
        sColorInfo.uv = u * v;
        sColorInfo.vv = v * v;
        sColorInfo.UU = U * U;
        sColorInfo.VV = V * V;
        sColorInfo.N  = 1;

        rCuboidColorInfoC += sColorInfo;
      }
    }
  }
}

Void TEnc3DAsymLUT::xDerivePartNumLog2( TComSlice * pSlice, TEncCfg * pcCfg, Int & rOctantDepth, Int & rYPartNumLog2, Bool bSignalPPS, Bool bElRapSliceTypeB )
{
  Int nPartNumLog2 = 4;
  if( pSlice->getBaseColPic( pSlice->getInterLayerPredLayerIdc( 0 ) )->getSlice( 0 )->isIntra() )
  {
    nPartNumLog2 = xGetMaxPartNumLog2();
  }

  if( m_nAccuFrameBit && pSlice->getPPS()->getCGSFlag() ) 
  {
    Double dBitCost = 1.0 * m_nAccuFrameCGSBit / m_nAccuFrameBit;
    nPartNumLog2 = m_nPrevFrameCGSPartNumLog2;

    Double dBitCostT = 0.03;
    if( dBitCost < dBitCostT / 6.0 )
    {
      nPartNumLog2++;
    }
    else if( dBitCost >= dBitCostT )
    {
      nPartNumLog2--;
    }
  }

  nPartNumLog2 = Clip3( 0 , xGetMaxPartNumLog2()  , nPartNumLog2 );
  xMapPartNum2DepthYPart( nPartNumLog2 , rOctantDepth , rYPartNumLog2 );
}

Void TEnc3DAsymLUT::xMapPartNum2DepthYPart( Int nPartNumLog2, Int & rOctantDepth, Int & rYPartNumLog2 )
{
  for( Int y = getMaxYPartNumLog2() ; y >= 0 ; y-- )
  {
    for( Int depth = ( nPartNumLog2 - y ) >> 1 ; depth >= 0 ; depth-- )
    {
      if( y + 3 * depth == nPartNumLog2 )
      {
        rOctantDepth = depth;
        rYPartNumLog2 = y;
        return;
      }
    }
  }
  rOctantDepth = min( getMaxOctantDepth() , nPartNumLog2 / 3 );
  rYPartNumLog2 = min( getMaxYPartNumLog2() , nPartNumLog2 - 3 * rOctantDepth );
}

Void TEnc3DAsymLUT::updatePicCGSBits( TComSlice * pcSlice, Int nPPSBit )
{
  for( Int i = 0; i < pcSlice->getActiveNumILRRefIdx(); i++ )
  {
    UInt refLayerIdc = pcSlice->getInterLayerPredLayerIdc(i);
    m_nAccuFrameBit += pcSlice->getPic()->getFrameBit() + pcSlice->getBaseColPic(refLayerIdc)->getFrameBit();
    m_dTotalFrameBit += pcSlice->getPic()->getFrameBit() + pcSlice->getBaseColPic(refLayerIdc)->getFrameBit();
  }

  m_nAccuFrameCGSBit += nPPSBit;
  m_nTotalCGSBit += nPPSBit;
  m_nPrevFrameCGSPartNumLog2 = getCurOctantDepth() * 3 + getCurYPartNumLog2();

#if R0179_ENC_OPT_3DLUT_SIZE
  Int nCurELFrameBit = pcSlice->getPic()->getFrameBit();
  const Int nSliceType = pcSlice->getSliceType();
  const Int nSliceTempLevel = pcSlice->getDepth();
  m_nPrevELFrameBit[nSliceType][nSliceTempLevel] = m_nPrevELFrameBit[nSliceType][nSliceTempLevel] == 0 ? nCurELFrameBit:((m_nPrevELFrameBit[nSliceType][nSliceTempLevel]+nCurELFrameBit)>>1);
#endif 
}

#if R0179_ENC_OPT_3DLUT_SIZE

Void TEnc3DAsymLUT::xGetAllLutSizes(TComSlice *pSlice)
{
  Int iMaxYPartNumLog2, iMaxCPartNumLog2; 
  Int iCurYPartNumLog2, iCurCPartNumLog2; 
  Int iMaxAddYPartNumLog2; 
  Int iNumELFrameBits = m_nPrevELFrameBit[pSlice->getSliceType()][pSlice->getDepth()];

  xMapPartNum2DepthYPart( xGetMaxPartNumLog2(), iMaxCPartNumLog2, iMaxYPartNumLog2 );
  iMaxAddYPartNumLog2 = iMaxYPartNumLog2; 
  iMaxYPartNumLog2 += iMaxCPartNumLog2; 

  //m_sLutSizes[0].iYPartNumLog2 = iMaxYPartNumLog2; 
  //m_sLutSizes[0].iCPartNumLog2 = iMaxCPartNumLog2; 
  m_nTotalLutSizes = 0; 


  for(iCurYPartNumLog2 = iMaxYPartNumLog2; iCurYPartNumLog2 >= 0; iCurYPartNumLog2--) 
  {
    for(iCurCPartNumLog2 = iMaxCPartNumLog2; iCurCPartNumLog2 >= 0; iCurCPartNumLog2--) 
    {
       // try more sizes
      if(iCurCPartNumLog2 <= iCurYPartNumLog2  && 
         (m_nNumLUTBits[iCurYPartNumLog2][iCurCPartNumLog2] < (iNumELFrameBits>>1)) && 
         m_nTotalLutSizes < MAX_NUM_LUT_SIZES)
      {
        m_sLutSizes[m_nTotalLutSizes].iYPartNumLog2 = iCurYPartNumLog2; 
        m_sLutSizes[m_nTotalLutSizes].iCPartNumLog2 = iCurCPartNumLog2; 
        m_nTotalLutSizes ++; 
      }
    }
  }

}

Void TEnc3DAsymLUT::xCopyColorInfo( SColorInfo *** dst, SColorInfo *** src,  SColorInfo *** dstC, SColorInfo *** srcC )
{
  Int yIdx, uIdx, vIdx; 

  // copy from pColorInfo to pMaxColorInfo
  for(yIdx = 0; yIdx < xGetYSize(); yIdx++)
  {
    for(uIdx = 0; uIdx < xGetUSize(); uIdx++)
    {
      for(vIdx = 0; vIdx < xGetVSize(); vIdx++)
      {
        dst [yIdx][uIdx][vIdx] = src [yIdx][uIdx][vIdx];
        dstC[yIdx][uIdx][vIdx] = srcC[yIdx][uIdx][vIdx];
      }
    }
  }
}

Void TEnc3DAsymLUT::xAddColorInfo( Int yIdx, Int uIdx, Int vIdx, Int iYDiffLog2, Int iCDiffLog2 )
{
  SColorInfo & rCuboidColorInfo  = m_pColorInfo [yIdx][uIdx][vIdx];
  SColorInfo & rCuboidColorInfoC = m_pColorInfoC[yIdx][uIdx][vIdx];
  
  for( Int i = 0; i < (1<<iYDiffLog2); i++)
  {
    for (Int j = 0; j < (1<<iCDiffLog2); j++)
    {
      for(Int k = 0; k < (1<<iCDiffLog2); k++)
      {
        rCuboidColorInfo  += m_pMaxColorInfo [(yIdx<<iYDiffLog2)+i][(uIdx<<iCDiffLog2)+j][(vIdx<<iCDiffLog2)+k];
        rCuboidColorInfoC += m_pMaxColorInfoC[(yIdx<<iYDiffLog2)+i][(uIdx<<iCDiffLog2)+j][(vIdx<<iCDiffLog2)+k];
      }
    }
  }
}

Void TEnc3DAsymLUT::xConsolidateData( SLUTSize *pCurLUTSize, SLUTSize *pMaxLUTSize )
{
  Int yIdx, uIdx, vIdx; 
  Int iYDiffLog2, iCDiffLog2;
  Int nYSize = 1<< pMaxLUTSize->iYPartNumLog2;
  Int nCSize = 1<< pMaxLUTSize->iCPartNumLog2;

  iYDiffLog2 = pMaxLUTSize->iYPartNumLog2-pCurLUTSize->iYPartNumLog2;
  iCDiffLog2 = pMaxLUTSize->iCPartNumLog2-pCurLUTSize->iCPartNumLog2;

  //assert(pMaxLUTSize->iCPartNumLog2 >= pCurLUTSize->iCPartNumLog2 && pMaxLUTSize->iYPartNumLog2 >= pCurLUTSize->iYPartNumLog2); 
  if (iYDiffLog2 == 0 && iCDiffLog2 == 0) // shouldn't have to do anything 
  {
    xCopyColorInfo(m_pColorInfo, m_pMaxColorInfo, m_pColorInfoC, m_pMaxColorInfoC);
    return; 
  }

  xReset3DArray( m_pColorInfo,   1<<pMaxLUTSize->iYPartNumLog2, 1<<pMaxLUTSize->iCPartNumLog2, 1<<pMaxLUTSize->iCPartNumLog2 );
  xReset3DArray( m_pColorInfoC,  1<<pMaxLUTSize->iYPartNumLog2, 1<<pMaxLUTSize->iCPartNumLog2, 1<<pMaxLUTSize->iCPartNumLog2 );

  for(yIdx = 0; yIdx < nYSize; yIdx++)
  {
    for(uIdx = 0; uIdx < nCSize; uIdx++)
    {
      for(vIdx = 0; vIdx < nCSize; vIdx++)
      {
        const SColorInfo & rCuboidSrc   = m_pMaxColorInfo [yIdx][uIdx][vIdx];
        const SColorInfo & rCuboidSrcC  = m_pMaxColorInfoC[yIdx][uIdx][vIdx];
        
        Int yIdx2, uIdx2, vIdx2; 
        yIdx2 = yIdx>>iYDiffLog2; 
        uIdx2 = uIdx>>iCDiffLog2;
        vIdx2 = vIdx>>iCDiffLog2; 

        m_pColorInfo [yIdx2][uIdx2][vIdx2] += rCuboidSrc;
        m_pColorInfoC[yIdx2][uIdx2][vIdx2] += rCuboidSrcC;
      }
    }
  }
}

Void TEnc3DAsymLUT::update3DAsymLUTParam( TEnc3DAsymLUT * pSrc )
{
  assert( pSrc->getMaxOctantDepth() == getMaxOctantDepth() && pSrc->getMaxYPartNumLog2() == getMaxYPartNumLog2() );
  xUpdatePartitioning( pSrc->getCurOctantDepth(), pSrc->getCurYPartNumLog2(), pSrc->getAdaptChromaThresholdU(), pSrc->getAdaptChromaThresholdV() );
  setResQuantBit( pSrc->getResQuantBit() );
}
#endif 
#endif
