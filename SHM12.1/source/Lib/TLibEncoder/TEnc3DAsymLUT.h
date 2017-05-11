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
    \brief    TEnc3DAsymLUT encoder class header
*/

#ifndef __TENC3DASYMLUT__
#define __TENC3DASYMLUT__

#include "../TLibCommon/TCom3DAsymLUT.h"
#include "../TLibCommon/TComSlice.h"
#include "../TLibCommon/CommonDef.h"
#include "../TLibCommon/TComPic.h"
#include "TEncCfg.h"
#if R0179_ENC_OPT_3DLUT_SIZE
#include "TEncCavlc.h"
#define MAX_NUM_LUT_SIZES               10   // 4+3+2+1
#define MAX_Y_SIZE                       4
#define MAX_C_SIZE                       4
#endif

#if CGS_3D_ASYMLUT

typedef struct _ColorInfo
{
  Double YY , UU , VV;
  Double Ys , Us , Vs;  // sum of enhancement
  Double ys , us , vs;  // sum of base
  Double Yy , Yu , Yv;  // product of enhancement and base
  Double Uy , Uu , Uv;
  Double Vy , Vu , Vv;
  Double yy , yu , yv , uu , uv , vv; // product of base
  Double N; // number of pixel

public:
  _ColorInfo & operator += ( const _ColorInfo & rColorInfo )
  {
    YY += rColorInfo.YY;
    UU += rColorInfo.UU;
    VV += rColorInfo.VV;
    Ys += rColorInfo.Ys;
    Us += rColorInfo.Us;
    Vs += rColorInfo.Vs;
    ys += rColorInfo.ys;
    us += rColorInfo.us;
    vs += rColorInfo.vs;
    Yy += rColorInfo.Yy;
    Yu += rColorInfo.Yu;
    Yv += rColorInfo.Yv;
    Uy += rColorInfo.Uy;
    Uu += rColorInfo.Uu;
    Uv += rColorInfo.Uv;
    Vy += rColorInfo.Vy;
    Vu += rColorInfo.Vu;
    Vv += rColorInfo.Vv;
    yy += rColorInfo.yy;
    yu += rColorInfo.yu;
    yv += rColorInfo.yv;
    uu += rColorInfo.uu;
    uv += rColorInfo.uv;
    vv += rColorInfo.vv;
    N  += rColorInfo.N; 
    return *this;
  }

}SColorInfo;

#if R0179_ENC_OPT_3DLUT_SIZE
typedef struct _LUTSize 
{
  Int iYPartNumLog2; 
  Int iCPartNumLog2; 
} SLUTSize; 
#endif 

class TEnc3DAsymLUT : public TCom3DAsymLUT
{
public:
  TEnc3DAsymLUT();
  virtual ~TEnc3DAsymLUT();

  Void create( Int nMaxOctantDepth, Int nInputBitDepth, Int nInputBitDepthC, Int nOutputBitDepth, Int nOutputBitDepthC, Int nMaxYPartNumLog2 );
  virtual Void destroy();
  Double derive3DAsymLUT( TComSlice * pSlice, TComPic * pCurPic, UInt refLayerIdc, TEncCfg * pCfg, Bool bSignalPPS, Bool bElRapSliceTypeB );
  Double estimateDistWithCur3DAsymLUT( TComPic * pCurPic , UInt refLayerIdc );
#if R0179_ENC_OPT_3DLUT_SIZE
  Double getDistFactor( Int iSliceType, Int iLayer) { return m_dDistFactor[iSliceType][iLayer];}
  Double derive3DAsymLUT( TComSlice * pSlice , TComPic * pCurPic , UInt refLayerIdc , TEncCfg * pCfg , Bool bSignalPPS , Bool bElRapSliceTypeB, Double dFrameLambda );
  Void   update3DAsymLUTParam( TEnc3DAsymLUT * pSrc );
#endif

  Void  updatePicCGSBits( TComSlice * pcSlice , Int nPPSBit );
  Void  setPPSBit(Int n)  { m_nPPSBit = n;  }
  Int   getPPSBit()       { return m_nPPSBit;}
  Void  setDsOrigPic(TComPicYuv *pPicYuv) { m_pDsOrigPic = pPicYuv; };

protected:
  SColorInfo *** m_pColorInfo;
  SColorInfo *** m_pColorInfoC;
#if R0179_ENC_OPT_3DLUT_SIZE
  SColorInfo *** m_pMaxColorInfo;
  SColorInfo *** m_pMaxColorInfoC;
#endif 
  TComPicYuv* m_pDsOrigPic;
  SCuboid *** m_pEncCuboid;
  SCuboid *** m_pBestEncCuboid;
  Int         m_nAccuFrameBit;                  // base + enhancement layer
  Int         m_nAccuFrameCGSBit;
  Int         m_nPrevFrameCGSPartNumLog2;
  Double      m_dTotalFrameBit;
  Int         m_nTotalCGSBit;
  Int         m_nPPSBit;
  Int         m_nLUTBitDepth;
#if R0179_ENC_OPT_3DLUT_SIZE
  Double      m_dDistFactor[3][MAX_TLAYER];         
  Int         m_nNumLUTBits[MAX_Y_SIZE][MAX_C_SIZE]; 
  Int         m_nPrevELFrameBit[3][MAX_TLAYER];   


  Int         m_nTotalLutSizes;
  SLUTSize    m_sLutSizes[MAX_NUM_LUT_SIZES];
#endif 
  Double      m_dSumU;
  Double      m_dSumV;
  Int         m_nNChroma;
#if R0179_ENC_OPT_3DLUT_SIZE
  TComOutputBitstream  *m_pBitstreamRedirect;
  TEncCavlc  *m_pEncCavlc;
#endif 

private:
  Double  xDeriveVertexPerColor( Double N, Double Ys, Double Yy, Double Yu, Double Yv, Double ys, Double us, Double vs, Double yy, Double yu, Double yv, Double uu, Double uv, Double vv, Double YY, 
                                 Pel & rP0, Pel & rP1, Pel & rP3, Pel & rP7, Int nResQuantBit );

  Void    xDerivePartNumLog2( TComSlice * pSlice , TEncCfg * pcCfg , Int & rOctantDepth , Int & rYPartNumLog2 , Bool bSignalPPS , Bool bElRapSliceTypeB );
  Void    xMapPartNum2DepthYPart( Int nPartNumLog2 , Int & rOctantDepth , Int & rYPartNumLog2 );
  Int     xCoeff2Vertex( Double a , Double b , Double c , Double d , Int y , Int u , Int v ) { return ( ( Int )( a * y + b * u + c * v + d + 0.5 ) ); }
  Void    xCollectData( TComPic * pCurPic , UInt refLayerIdc );

  Double  xDeriveVertexes( Int nResQuantBit , SCuboid *** pCurCuboid );

  inline Double xCalEstDist( Double N, Double Ys, Double Yy, Double Yu, Double Yv, Double ys, Double us, Double vs, Double yy, Double yu, Double yv, Double uu, Double uv, Double vv, Double YY,
                             Int y0, Int u0, Int v0, Int nLengthY, Int nLengthUV, Pel nP0, Pel nP1, Pel nP3, Pel nP7 );

  inline Double xCalEstDist( Double N, Double Ys, Double Yy, Double Yu, Double Yv, Double ys, Double us, Double vs, Double yy, Double yu, Double yv, Double uu, Double uv, Double vv, Double YY, Double a, Double b, Double c, Double d );
#if SCALABLE_REXT
/* 
former xCalEstDist(Double, Double, Double, Double, Double, Double, Double, Double, Double, Double, Double, Double, Double, Double, Double, Pel, Pel, Pel, Pel)
replaced by this fucntion because behaving differently than other xCalEstDist and was conflicting with xCalEstDist when RExt__HIGH_BIT_DEPTH_SUPPORT = 1
*/
  inline Double xCalEstPelDist( Double N, Double Ys, Double Yy, Double Yu, Double Yv, Double ys, Double us, Double vs, Double yy, Double yu, Double yv, Double uu, Double uv, Double vv, Double YY, Pel nP0, Pel nP1, Pel nP3, Pel nP7 );
#else
  inline Double xCalEstDist( Double N, Double Ys, Double Yy, Double Yu, Double Yv, Double ys, Double us, Double vs, Double yy, Double yu, Double yv, Double uu, Double uv, Double vv, Double YY, Pel nP0, Pel nP1, Pel nP3, Pel nP7 );
#endif
#if R0179_ENC_OPT_3DLUT_SIZE
  Void    xConsolidateData( SLUTSize *pCurLUTSize, SLUTSize *pMaxLUTSize );
  Void    xGetAllLutSizes(TComSlice *pSlice);
  Void    xCopyColorInfo( SColorInfo *** dst, SColorInfo *** src ,  SColorInfo *** dstC, SColorInfo *** srcC ); 
  Void    xAddColorInfo( Int yIdx, Int uIdx, Int vIdx, Int iYDiffLog2, Int iCDiffLog2 );
#endif 
};

Double TEnc3DAsymLUT::xCalEstDist( Double N, Double Ys, Double Yy, Double Yu, Double Yv, Double ys, Double us, Double vs, Double yy, Double yu, Double yv, Double uu, Double uv, Double vv, Double YY,
                                   Int y0, Int u0, Int v0, Int nLengthY, Int nLengthUV, Pel nP0, Pel nP1, Pel nP3, Pel nP7 )
{
  Double a = 1.0 * ( nP7 - nP3 ) / nLengthY;
  Double b = 1.0 * ( nP1 - nP0 ) / nLengthUV;
  Double c = 1.0 * ( nP3 - nP1 ) / nLengthUV;
  Double d = ( ( nP0 * nLengthUV + u0 * nP0 + ( v0 - u0 ) * nP1 - v0 * nP3 ) * nLengthY + y0 * nLengthUV * ( nP3 - nP7 ) ) / nLengthUV / nLengthY;
  return( xCalEstDist( N , Ys , Yy , Yu , Yv , ys , us , vs , yy , yu , yv , uu , uv , vv , YY , a , b , c , d ) );
}

Double TEnc3DAsymLUT::xCalEstDist( Double N, Double Ys, Double Yy, Double Yu, Double Yv, Double ys, Double us, Double vs, Double yy, Double yu, Double yv, Double uu, Double uv, Double vv, Double YY, Double a, Double b, Double c, Double d )  
{
  Double dError = N * d * d + 2 * b * c * uv + 2 * a * c * yv + 2 * a * b * yu - 2 * c * Yv - 2 * b * Yu - 2 * a * Yy + 2 * c * d * vs + 2 * b * d * us + 2 * a * d * ys + a * a * yy + c * c * vv + b * b * uu - 2 * d * Ys + YY;
  return( dError );
};


#if SCALABLE_REXT
/* 
former xCalEstDist(Double, Double, Double, Double, Double, Double, Double, Double, Double, Double, Double, Double, Double, Double, Double, Pel, Pel, Pel, Pel)
replaced by this fucntion because behaving differently than other xCalEstDist and was conflicting with xCalEstDist when RExt__HIGH_BIT_DEPTH_SUPPORT = 1
*/
Double TEnc3DAsymLUT::xCalEstPelDist( Double N, Double Ys, Double Yy, Double Yu, Double Yv, Double ys, Double us, Double vs, Double yy, Double yu, Double yv, Double uu, Double uv, Double vv, Double YY, Pel nP0, Pel nP1, Pel nP3, Pel nP7 )  
{
  const Int nOne = xGetNormCoeffOne();
  Double a = 1.0 * nP0 / nOne;
  Double b = 1.0 * nP1 / nOne;
  Double c = 1.0 * nP3 / nOne;
  Double d = nP7;
  Double dError = N * d * d + 2 * b * c * uv + 2 * a * c * yv + 2 * a * b * yu - 2 * c * Yv - 2 * b * Yu - 2 * a * Yy + 2 * c * d * vs + 2 * b * d * us + 2 * a * d * ys + a * a * yy + c * c * vv + b * b * uu - 2 * d * Ys + YY;
  return( dError );
};
#else
Double TEnc3DAsymLUT::xCalEstDist( Double N, Double Ys, Double Yy, Double Yu, Double Yv, Double ys, Double us, Double vs, Double yy, Double yu, Double yv, Double uu, Double uv, Double vv, Double YY, Pel nP0, Pel nP1, Pel nP3, Pel nP7 )  
{
  const Int nOne = xGetNormCoeffOne();
  Double a = 1.0 * nP0 / nOne;
  Double b = 1.0 * nP1 / nOne;
  Double c = 1.0 * nP3 / nOne;
  Double d = nP7;
  Double dError = N * d * d + 2 * b * c * uv + 2 * a * c * yv + 2 * a * b * yu - 2 * c * Yv - 2 * b * Yu - 2 * a * Yy + 2 * c * d * vs + 2 * b * d * us + 2 * a * d * ys + a * a * yy + c * c * vv + b * b * uu - 2 * d * Ys + YY;
  return( dError );
};
#endif

#endif

#endif
