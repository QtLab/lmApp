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

/** \file     TCom3DAsymLUT.cpp
    \brief    TCom3DAsymLUT class
*/

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <algorithm>

#include "CommonDef.h"
#include "TCom3DAsymLUT.h"
#include "TComPicYuv.h"

#if CGS_3D_ASYMLUT
const Int TCom3DAsymLUT::m_nVertexIdxOffset[4][3] = { { 0 , 0 , 0 } , { 0 , 1 , 0 } , { 0 , 1 , 1 } , { 1 , 1 , 1 } };

TCom3DAsymLUT::TCom3DAsymLUT()
{
  m_pCuboid = NULL;
  m_nResQuanBit = 0;
#if R0164_CGS_LUT_BUGFIX_CHECK
  m_pCuboidExplicit = NULL;
  m_pCuboidFilled = NULL;
#endif
}

TCom3DAsymLUT::~TCom3DAsymLUT()
{
  destroy();
}

Void TCom3DAsymLUT::create( Int nMaxOctantDepth, Int nInputBitDepth, Int nInputBitDepthC, Int nOutputBitDepth, Int nOutputBitDepthC, Int nMaxYPartNumLog2, Int nAdaptCThresholdU, Int nAdaptCThresholdV )
{
  m_nMaxOctantDepth = nMaxOctantDepth;
  m_nInputBitDepthY = nInputBitDepth;
  m_nOutputBitDepthY = nOutputBitDepth;
  m_nInputBitDepthC = nInputBitDepthC;
  m_nOutputBitDepthC = nOutputBitDepthC;
  m_nDeltaBitDepthC = m_nOutputBitDepthC - m_nInputBitDepthC;
  m_nDeltaBitDepth = m_nOutputBitDepthY - m_nInputBitDepthY;
  m_nMaxYPartNumLog2 = nMaxYPartNumLog2;
  m_nMaxPartNumLog2 = 3 * m_nMaxOctantDepth + m_nMaxYPartNumLog2;

  xUpdatePartitioning( nMaxOctantDepth, nMaxYPartNumLog2, nAdaptCThresholdU, nAdaptCThresholdV );

  m_nYSize = 1 << ( m_nMaxOctantDepth + m_nMaxYPartNumLog2 );
  m_nUSize = 1 << m_nMaxOctantDepth;
  m_nVSize = 1 << m_nMaxOctantDepth;
  assert( m_nYSize > 0 && m_nUSize > 0 && m_nVSize > 0 );

  if( m_pCuboid != NULL )
  {
    destroy();
  }
  xAllocate3DArray( m_pCuboid , m_nYSize , m_nUSize , m_nVSize );

#if R0164_CGS_LUT_BUGFIX_CHECK
  xAllocate3DArray( m_pCuboidExplicit , m_nYSize , m_nUSize , m_nVSize );
  xAllocate3DArray( m_pCuboidFilled   , m_nYSize , m_nUSize , m_nVSize );
#endif
}

Void TCom3DAsymLUT::destroy()
{
  xFree3DArray( m_pCuboid );
#if R0164_CGS_LUT_BUGFIX_CHECK
  xFree3DArray( m_pCuboidExplicit );
  xFree3DArray( m_pCuboidFilled   );
#endif
}


Void TCom3DAsymLUT::xUpdatePartitioning( Int nCurOctantDepth, Int nCurYPartNumLog2, Int nAdaptCThresholdU, Int nAdaptCThresholdV  )
{
  assert( nCurOctantDepth <= m_nMaxOctantDepth );
  assert( nCurYPartNumLog2 + nCurOctantDepth <= m_nMaxYPartNumLog2 + m_nMaxOctantDepth );

  m_nCurOctantDepth = nCurOctantDepth;
  m_nCurYPartNumLog2 = nCurYPartNumLog2;
  m_nYShift2Idx = m_nInputBitDepthY - m_nCurOctantDepth - m_nCurYPartNumLog2;
  m_nUShift2Idx = m_nVShift2Idx = m_nInputBitDepthC - m_nCurOctantDepth;

  m_nMappingShift = 10 + m_nInputBitDepthY - m_nOutputBitDepthY; 
  m_nAdaptCThresholdU = nAdaptCThresholdU;
  m_nAdaptCThresholdV = nAdaptCThresholdV;

  m_nMappingOffset = 1 << ( m_nMappingShift - 1 );

#if R0179_ENC_OPT_3DLUT_SIZE
  m_nYSize = 1 << ( m_nCurOctantDepth + m_nCurYPartNumLog2 );
  m_nUSize = 1 << m_nCurOctantDepth;
  m_nVSize = 1 << m_nCurOctantDepth;
#endif
}

Void TCom3DAsymLUT::colorMapping( TComPicYuv * pcPic, TComPicYuv * pcPicDst )
{
  Int nWidth = pcPic->getWidth(COMPONENT_Y);
  Int nHeight = pcPic->getHeight(COMPONENT_Y);
  Int nStrideY = pcPic->getStride(COMPONENT_Y);
  Int nStrideC = pcPic->getStride(COMPONENT_Cb);
  Pel * pY = pcPic->getAddr(COMPONENT_Y);
  Pel * pU = pcPic->getAddr(COMPONENT_Cb);
  Pel * pV = pcPic->getAddr(COMPONENT_Cr);

  Int nDstStrideY = pcPicDst->getStride(COMPONENT_Y);
  Int nDstStrideC = pcPicDst->getStride(COMPONENT_Cb);
  Pel * pYDst = pcPicDst->getAddr(COMPONENT_Y);
  Pel * pUDst = pcPicDst->getAddr(COMPONENT_Cb);
  Pel * pVDst = pcPicDst->getAddr(COMPONENT_Cr);

  Pel *pUPrev = pU;
  Pel *pVPrev = pV;
  Pel *pUNext = pU+nStrideC;
  Pel *pVNext = pV+nStrideC;

  // alignment padding
  pcPic->setBorderExtension( false );
  pcPic->extendPicBorder();

  Pel iMaxValY = (1<<getOutputBitDepthY())-1;
  Pel iMaxValC = (1<<getOutputBitDepthC())-1;
  for( Int y = 0 ; y < nHeight ; y += 2 )
  {
    for( Int xY = 0 , xC = 0 ; xY < nWidth ; xY += 2 , xC++ )
    {
      Pel srcY00 = pY[xY];
      Pel srcY01 = pY[xY+1];
      Pel srcY10 = pY[xY+nStrideY];
      Pel srcY11 = pY[xY+nStrideY+1];
      Pel srcYaver;
      Pel srcU = pU[xC];
      Pel srcV = pV[xC];
      Pel dstY00, dstY01, dstY10, dstY11;

      // alignment
      srcYaver =  (srcY00 + srcY10 + 1 ) >> 1;
      Pel srcUP0 = pUPrev[xC];
      Pel srcVP0 = pVPrev[xC];        
      Pel tmpU =  (srcUP0 + srcU + (srcU<<1) + 2 ) >> 2;
      Pel tmpV =  (srcVP0 + srcV + (srcV<<1) + 2 ) >> 2;
      dstY00 = xMapY( srcY00 , tmpU , tmpV );
      Pel a = pU[xC+1] + srcU;
      tmpU =  ((a<<1) + a + srcUP0 + pUPrev[xC+1] + 4 ) >> 3;
      Pel b = pV[xC+1] + srcV;
      tmpV =  ((b<<1) + b + srcVP0 + pVPrev[xC+1] + 4 ) >> 3;
      dstY01 = xMapY( srcY01 , tmpU , tmpV );

      srcUP0 = pUNext[xC];
      srcVP0 = pVNext[xC];
      tmpU =  (srcUP0 + srcU + (srcU<<1) + 2 ) >> 2;
      tmpV =  (srcVP0 + srcV + (srcV<<1) + 2 ) >> 2;
      dstY10 = xMapY( srcY10 , tmpU , tmpV );
      tmpU =  ((a<<1) + a + srcUP0 + pUNext[xC+1] + 4 ) >> 3;
      tmpV =  ((b<<1) + b + srcVP0 + pVNext[xC+1] + 4 ) >> 3;
      dstY11 = xMapY( srcY11 , tmpU , tmpV );

      SYUVP dstUV = xMapUV( srcYaver , srcU , srcV );
      pYDst[xY] = Clip3((Pel)0, iMaxValY, dstY00 );
      pYDst[xY+1] = Clip3((Pel)0, iMaxValY, dstY01 );
      pYDst[xY+nDstStrideY] = Clip3((Pel)0, iMaxValY, dstY10 );
      pYDst[xY+nDstStrideY+1] = Clip3((Pel)0, iMaxValY, dstY11 );
      pUDst[xC] = Clip3((Pel)0, iMaxValC, dstUV.U );
      pVDst[xC] = Clip3((Pel)0, iMaxValC, dstUV.V );
    }
    pY += nStrideY + nStrideY;

    // alignment
    pUPrev = pU;
    pVPrev = pV;
    pU = pUNext;
    pV = pVNext;
    pUNext += nStrideC;
    pVNext += nStrideC;

    pYDst += nDstStrideY + nDstStrideY;
    pUDst += nDstStrideC;
    pVDst += nDstStrideC;
  }
}

SYUVP TCom3DAsymLUT::xGetCuboidVertexPredA( Int yIdx , Int uIdx , Int vIdx , Int nVertexIdx )
{
  assert( nVertexIdx < 4 );
  
  SYUVP sPred;

  sPred.Y = sPred.U = sPred.V = 0;

  if( nVertexIdx == 0 )
  {
    sPred.Y = xGetNormCoeffOne() << ( m_nOutputBitDepthY - m_nInputBitDepthY );
  }
  else if( nVertexIdx == 1 )
  {
    sPred.U = xGetNormCoeffOne() << ( m_nOutputBitDepthY - m_nInputBitDepthY );
  }
  else if( nVertexIdx == 2 )
  {
    sPred.V = xGetNormCoeffOne() << ( m_nOutputBitDepthY - m_nInputBitDepthY );
  }

  return sPred;
}

SYUVP  TCom3DAsymLUT::xGetCuboidVertexPredAll( Int yIdx , Int uIdx , Int vIdx , Int nVertexIdx , SCuboid *** pCurCuboid )
{
  SCuboid***  pCuboid = pCurCuboid ? pCurCuboid : m_pCuboid ;

  SYUVP sPred;
  if( yIdx == 0 )
  {
    sPred.Y = nVertexIdx == 0 ? 1024 : 0;
    sPred.U = nVertexIdx == 1 ? 1024 : 0;
    sPred.V = nVertexIdx == 2 ? 1024 : 0;
  }
  else
  {
    sPred = pCuboid[yIdx-1][uIdx][vIdx].P[nVertexIdx];
  }

  return sPred ;
}

SYUVP TCom3DAsymLUT::getCuboidVertexResTree( Int yIdx , Int uIdx , Int vIdx , Int nVertexIdx )
{
  const SYUVP & rYUVP = m_pCuboid[yIdx][uIdx][vIdx].P[nVertexIdx];
  SYUVP sPred = xGetCuboidVertexPredAll( yIdx , uIdx , vIdx , nVertexIdx );

  SYUVP sResidue;
  sResidue.Y = ( rYUVP.Y - sPred.Y ) >> m_nResQuanBit;
  sResidue.U = ( rYUVP.U - sPred.U ) >> m_nResQuanBit;
  sResidue.V = ( rYUVP.V - sPred.V ) >> m_nResQuanBit;
  return( sResidue );
}

Void TCom3DAsymLUT::setCuboidVertexResTree( Int yIdx , Int uIdx , Int vIdx , Int nVertexIdx , Int deltaY , Int deltaU , Int deltaV )
{
  SYUVP & rYUVP = m_pCuboid[yIdx][uIdx][vIdx].P[nVertexIdx];
  SYUVP sPred = xGetCuboidVertexPredAll( yIdx , uIdx , vIdx , nVertexIdx );

  rYUVP.Y = sPred.Y + ( deltaY << m_nResQuanBit );
  rYUVP.U = sPred.U + ( deltaU << m_nResQuanBit );
  rYUVP.V = sPred.V + ( deltaV << m_nResQuanBit );

  // LUT coefficients are less than 12-bit
  assert( -2048 <= rYUVP.Y && rYUVP.Y <= 2047 );
  assert( -2048 <= rYUVP.U && rYUVP.U <= 2047 );
  assert( -2048 <= rYUVP.V && rYUVP.V <= 2047 );
}

Pel TCom3DAsymLUT::xMapY( Pel y , Pel u , Pel v )
{
  const SCuboid & rCuboid = m_pCuboid[xGetYIdx(y)][xGetUIdx(u)][xGetVIdx(v)];
  Pel dstY = ( ( rCuboid.P[0].Y * y + rCuboid.P[1].Y * u + rCuboid.P[2].Y * v + m_nMappingOffset ) >> m_nMappingShift ) + rCuboid.P[3].Y;

  return( dstY );
}

SYUVP TCom3DAsymLUT::xMapUV( Pel y , Pel u , Pel v )
{
  const SCuboid & rCuboid = m_pCuboid[xGetYIdx(y)][xGetUIdx(u)][xGetVIdx(v)];
  SYUVP dst;
  dst.Y = 0;
  dst.U = ( ( rCuboid.P[0].U * y + rCuboid.P[1].U * u + rCuboid.P[2].U * v + m_nMappingOffset ) >> m_nMappingShift ) + rCuboid.P[3].U;
  dst.V = ( ( rCuboid.P[0].V * y + rCuboid.P[1].V * u + rCuboid.P[2].V * v + m_nMappingOffset ) >> m_nMappingShift ) + rCuboid.P[3].V;

  return( dst );
}

Void TCom3DAsymLUT::xSaveCuboids( SCuboid *** pSrcCuboid )
{
#if R0179_ENC_OPT_3DLUT_SIZE
  memcpy( m_pCuboid[0][0] , pSrcCuboid[0][0] , sizeof( SCuboid ) * getMaxYSize() * getMaxCSize() * getMaxCSize() );
#else
  memcpy( m_pCuboid[0][0] , pSrcCuboid[0][0] , sizeof( SCuboid ) * m_nYSize * m_nUSize * m_nVSize );
#endif 
}

Void TCom3DAsymLUT::copy3DAsymLUT( TCom3DAsymLUT * pSrc )
{
  assert( pSrc->getMaxOctantDepth() == getMaxOctantDepth() && pSrc->getMaxYPartNumLog2() == getMaxYPartNumLog2() );
  xUpdatePartitioning( pSrc->getCurOctantDepth(), pSrc->getCurYPartNumLog2(), pSrc->getAdaptChromaThresholdU(), pSrc->getAdaptChromaThresholdV() );
  setResQuantBit( pSrc->getResQuantBit() );
  xSaveCuboids( pSrc->m_pCuboid );
}

#if R0164_CGS_LUT_BUGFIX_CHECK
Void TCom3DAsymLUT::xInitCuboids( )
{
  // All vertices are initialized as non-exlicitly-encoded
  for( Int yIdx = 0 ; yIdx < m_nYSize ; yIdx++ )
  {
    for( Int uIdx = 0 ; uIdx < m_nUSize ; uIdx++ )
    {
      for( Int vIdx = 0 ; vIdx < m_nVSize ; vIdx++ )
      { 
        m_pCuboidExplicit[yIdx][uIdx][vIdx] = false;
        m_pCuboidFilled[yIdx][uIdx][vIdx]   = false;
      }
    }
  }
}

Void TCom3DAsymLUT::xCuboidsFilledCheck( Int yIdx , Int uIdx , Int vIdx )
{
  if ( m_pCuboidFilled[yIdx][uIdx][vIdx] == false )
  {
    if( yIdx > 0) 
      assert ( m_pCuboidFilled[yIdx-1][uIdx][vIdx] );

    for ( Int nVertexIdx=0 ; nVertexIdx<4 ; nVertexIdx++ )
      m_pCuboid[yIdx][uIdx][vIdx].P[nVertexIdx] = yIdx == 0 ? xGetCuboidVertexPredA( yIdx , uIdx , vIdx , nVertexIdx ): xGetCuboidVertexPredAll( yIdx , uIdx , vIdx , nVertexIdx );

    m_pCuboidFilled[yIdx][uIdx][vIdx] = true ;
  }
}


Void TCom3DAsymLUT::xCuboidsFilledCheck( Bool bDecode )
{
  Int ySize = 1 << ( getCurOctantDepth() + getCurYPartNumLog2() );
  Int uSize = 1 << getCurOctantDepth();
  Int vSize = 1 << getCurOctantDepth();
  for( Int yIdx = 0 ; yIdx < ySize ; yIdx++ )
  {
    for( Int uIdx = 0 ; uIdx < uSize ; uIdx++ )
    {
      for( Int vIdx = 0 ; vIdx < vSize ; vIdx++ )
      { 
        if ( bDecode )
          xCuboidsFilledCheck( yIdx , uIdx , vIdx );

        assert( m_pCuboidFilled[yIdx][uIdx][vIdx] );
      }
    }
  }

}
#endif

Bool TCom3DAsymLUT::isRefLayer( UInt uiRefLayerId )
{
  Bool bIsRefLayer = false;
  for( UInt i = 0 ; i < m_vRefLayerId.size() ; i++ )
  {
    if( m_vRefLayerId[i] == uiRefLayerId )
    {
      bIsRefLayer = true;
      break;
    }
  }

  return( bIsRefLayer );
}

#if R0164_CGS_LUT_BUGFIX_CHECK
Void  TCom3DAsymLUT::display( Bool bFilled )
{
  Int ySize = 1 << ( getCurOctantDepth() + getCurYPartNumLog2() );
  Int uSize = 1 << getCurOctantDepth();
  Int vSize = 1 << getCurOctantDepth();
  Int vIdx=0;

  printf("\n");
  printf("3DLut Explicit flag:\n");
  for( Int uIdx = 0 ; uIdx < uSize ; uIdx++ )
  {
    for( Int yIdx = 0 ; yIdx < ySize ; yIdx++ )
    {
      printf("%d\t", m_pCuboidExplicit[yIdx][uIdx][vIdx] );
    }
    printf("\n");
  }

  printf("3DLut values (explicit):\n");
  for( Int uIdx = 0 ; uIdx < uSize ; uIdx++ )
  {
    for( Int yIdx = 0 ; yIdx < ySize ; yIdx++ )
    {
      if ( m_pCuboidExplicit[yIdx][uIdx][vIdx] )  printf("%d\t", m_pCuboid[yIdx][uIdx][vIdx].P[0].Y );
      else                                        printf("?\t", m_pCuboid[yIdx][uIdx][vIdx].P[0].Y );
    }
    printf("\n");
  }

  printf("3DLut values (all):\n");
  for( Int uIdx = 0 ; uIdx < uSize ; uIdx++ )
  {
    for( Int yIdx = 0 ; yIdx < ySize ; yIdx++ )
    {
      if ( bFilled ) {
        if ( m_pCuboidFilled[yIdx][uIdx][vIdx] )  printf("%d\t"  , m_pCuboid[yIdx][uIdx][vIdx].P[0].Y );
        else                                      printf("unk\t" , m_pCuboid[yIdx][uIdx][vIdx].P[0].Y );
      }
      else
        printf("%d\t"  , m_pCuboid[yIdx][uIdx][vIdx].P[0].Y );
    }
    printf("\n");
  }

}
#endif

#endif

