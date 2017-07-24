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

/** \file     TCom3DAsymLUT.h
    \brief    TCom3DAsymLUT class header
*/


#ifndef __TCOM3DASYMLUT__
#define __TCOM3DASYMLUT__

#include "CommonDef.h"

#if CGS_3D_ASYMLUT
#include <cassert>
#include <vector>

typedef struct _SYUVP
{
  Pel   Y , U , V;
}SYUVP;

typedef struct _SCuboid // vertexes for tetrahedral interpolation
{
  SYUVP P[4];     // YUV: P0(0,0,0), P1(0,1,0), P3(0,1,1), P7(1,1,1)
}SCuboid;

class TComPicYuv;

class TCom3DAsymLUT
{
public:
  TCom3DAsymLUT();
  virtual ~TCom3DAsymLUT();
    
  Void create( Int nMaxOctantDepth, Int nInputBitDepth, Int nInputBitDepthC, Int nOutputBitDepth, Int nOutputBitDepthC, Int nMaxYPartNumLog2, Int nAdaptCThresholdU, Int nAdaptCThresholdV );
  virtual Void destroy();

  Int   getMaxOctantDepth()   { return m_nMaxOctantDepth;  }
  Int   getCurOctantDepth()   { return m_nCurOctantDepth;  }
  Int   getInputBitDepthY()   { return m_nInputBitDepthY;  }
  Int   getOutputBitDepthY()  { return m_nOutputBitDepthY; }
  Int   getInputBitDepthC()   { return m_nInputBitDepthC;  }
  Int   getOutputBitDepthC()  { return m_nOutputBitDepthC; }
  Int   getResQuantBit()      { return m_nResQuanBit;      }
  Void  setResQuantBit(Int n) { m_nResQuanBit = n;         }

  Int   getMappingShift()     { return m_nMappingShift;    }
  Int   getDeltaBits()        { return m_nDeltaBits;       }
  Void  setDeltaBits(Int n)   { m_nDeltaBits = n;          }

  Int   getMaxYPartNumLog2()  { return m_nMaxYPartNumLog2; }
  Int   getCurYPartNumLog2()  { return m_nCurYPartNumLog2; }

  Void  addRefLayerId( UInt uiRefLayerId )  
  { 
    if( !isRefLayer( uiRefLayerId ) )
      m_vRefLayerId.push_back( uiRefLayerId ); 
  }
  size_t  getRefLayerNum()     { return m_vRefLayerId.size();  }
  UInt  getRefLayerId( UInt n )  { assert( n < m_vRefLayerId.size() ); return m_vRefLayerId[n];   }
  Bool  isRefLayer( UInt uiRefLayerId );

  Void  setAdaptChromaThresholdU( Int n ) { m_nAdaptCThresholdU = n; }
  Int   getAdaptChromaThresholdU()        { return m_nAdaptCThresholdU; }
  Void  setAdaptChromaThresholdV( Int n ) { m_nAdaptCThresholdV = n;  }
  Int   getAdaptChromaThresholdV()        { return m_nAdaptCThresholdV; }

#if R0179_ENC_OPT_3DLUT_SIZE
  Int   getMaxYSize() { return 1<<(m_nMaxOctantDepth+m_nMaxYPartNumLog2); }
  Int   getMaxCSize() { return 1<<m_nMaxOctantDepth; }
#endif 

  Void  colorMapping( TComPicYuv * pcPicSrc,  TComPicYuv * pcPicDst );
  Void  copy3DAsymLUT( TCom3DAsymLUT * pSrc );

  SYUVP xGetCuboidVertexPredAll( Int yIdx , Int uIdx , Int vIdx , Int nVertexIdx , SCuboid *** pCurCuboid=NULL );
  SYUVP getCuboidVertexResTree( Int yIdx , Int uIdx , Int vIdx , Int nVertexIdx );
  Void  setCuboidVertexResTree( Int yIdx , Int uIdx , Int vIdx , Int nVertexIdx , Int deltaY , Int deltaU , Int deltaV );
#if R0164_CGS_LUT_BUGFIX_CHECK
  Void  xInitCuboids( );
  Void  xCuboidsFilledCheck( Int yIdx , Int uIdx , Int vIdx );
  Void  xCuboidsFilledCheck( Bool bDecode );
  Void  display( Bool bFilled=true );
  Void  xSetExplicit( Int yIdx , Int uIdx , Int vIdx )  { m_pCuboidExplicit[yIdx][uIdx][vIdx] = true ; xSetFilled(yIdx,uIdx,vIdx); }
  Void  xSetFilled( Int yIdx , Int uIdx , Int vIdx )    { m_pCuboidFilled[yIdx][uIdx][vIdx] = true ; }
#endif

private:
  Int   m_nMaxOctantDepth;
  Int   m_nCurOctantDepth;
  Int   m_nInputBitDepthY;
  Int   m_nOutputBitDepthY;
  Int   m_nInputBitDepthC;
  Int   m_nOutputBitDepthC;
  Int   m_nDeltaBitDepthC;
  Int   m_nDeltaBitDepth;
  Int   m_nMaxYPartNumLog2;
  Int   m_nCurYPartNumLog2;
  Int   m_nMaxPartNumLog2;
  Int   m_nYSize;
  Int   m_nUSize;
  Int   m_nVSize;
  Int   m_nYShift2Idx;
  Int   m_nUShift2Idx;
  Int   m_nVShift2Idx;
  Int   m_nMappingShift;
  Int   m_nMappingOffset;
  Int   m_nResQuanBit;
  Int   m_nDeltaBits;

  SCuboid *** m_pCuboid;
  static const Int m_nVertexIdxOffset[4][3];
  std::vector<UInt> m_vRefLayerId;

  Int   m_nAdaptCThresholdU;
  Int   m_nAdaptCThresholdV;

#if R0164_CGS_LUT_BUGFIX_CHECK
  Bool  *** m_pCuboidExplicit;
  Bool  *** m_pCuboidFilled;
#endif

protected:
  template <class T> 
  Void xAllocate3DArray( T*** &p , Int xSize , Int ySize , Int zSize );
  template <class T> 
  Void xReset3DArray( T*** &p , Int xSize , Int ySize , Int zSize );
  template <class T>
  Void xFree3DArray( T *** &p );

  Void  xUpdatePartitioning( Int nCurOctantDepth, Int nCurYPartNumLog2, Int nAdaptCThresholdU, Int nAdaptCThreshodV );
  SYUVP xGetCuboidVertexPredA( Int yIdx , Int uIdx , Int vIdx , Int nVertexIdx );
  Pel   xMapY( Pel y , Pel u , Pel v );
  SYUVP xMapUV( Pel y , Pel u , Pel v );
  Int   xGetMaxPartNumLog2()  { return m_nMaxPartNumLog2; }
  Int   xGetYSize()  { return m_nYSize;  }
  Int   xGetUSize()  { return m_nUSize;  }
  Int   xGetVSize()  { return m_nVSize;  }
  Int   xGetYIdx(Pel y)  { return( y >> m_nYShift2Idx ); }
  Int   xGetUIdx(Pel u)  { return( m_nCurOctantDepth == 1 ? u >= m_nAdaptCThresholdU : u >> m_nUShift2Idx ); }
  Int   xGetVIdx(Pel v)  { return( m_nCurOctantDepth == 1 ? v >= m_nAdaptCThresholdV : v >> m_nVShift2Idx ); }
  Int   xGetNormCoeffOne()    { return( 1 << m_nMappingShift ); }
  SCuboid & xGetCuboid( Int yIdx , Int uIdx , Int vIdx ){ return m_pCuboid[yIdx][uIdx][vIdx];  }
  Void  xSaveCuboids( SCuboid *** pSrcCuboid );
};

template <class T> 
Void TCom3DAsymLUT::xAllocate3DArray( T *** &p , Int xSize , Int ySize , Int zSize )
{
  p = new T**[xSize];
  p[0] = new T*[xSize*ySize];
  for( Int x = 1 ; x < xSize ; x++ )
  {
    p[x] = p[x-1] + ySize;
  }
  p[0][0] = new T[xSize*ySize*zSize];
  for( Int x = 0 ; x < xSize ; x++ )
  {
    for( Int y = 0 ; y < ySize ; y++ )
    {
      p[x][y] = p[0][0] + x * ySize * zSize + y * zSize;
    }
  }
}

template <class T>
Void TCom3DAsymLUT::xFree3DArray( T *** &p )
{
  if( p != NULL )
  {
    if( p[0] != NULL )
    {
      if( p[0][0] != NULL )
      {
        delete [] p[0][0];
      }
      delete [] p[0];
    }
    delete [] p;
    p = NULL;
  }
}

template <class T>
Void TCom3DAsymLUT::xReset3DArray( T*** &p , Int xSize , Int ySize , Int zSize )
{
  memset( p[0][0] , 0 , sizeof( T ) * xSize * ySize * zSize );
}

#endif

#endif
