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

/** \file     TComPic.cpp
    \brief    picture class
*/

#include "TComPic.h"
#include "SEI.h"

//! \ingroup TLibCommon
//! \{

// ====================================================================================================================
// Constructor / destructor / create / destroy
// ====================================================================================================================

TComPic::TComPic()
: m_uiTLayer                              (0)
, m_bUsedByCurr                           (false)
, m_bIsLongTerm                           (false)
, m_pcPicYuvPred                          (NULL)
, m_pcPicYuvResi                          (NULL)
, m_bReconstructed                        (false)
, m_bNeededForOutput                      (false)
, m_uiCurrSliceIdx                        (0)
, m_bCheckLTMSB                           (false)
#if SVC_EXTENSION
, m_layerId( 0 )
#endif
{
#if SVC_EXTENSION
  memset( m_pcFullPelBaseRec, 0, sizeof( m_pcFullPelBaseRec ) );
  memset( m_requireResampling, false, sizeof( m_requireResampling ) );
  memset( m_equalPictureSizeAndOffsetFlag, false, sizeof( m_equalPictureSizeAndOffsetFlag ) );
  memset( m_mvScalingFactor, 0, sizeof( m_mvScalingFactor ) );
  memset( m_posScalingFactor, 0, sizeof( m_posScalingFactor ) );
#endif
  for(UInt i=0; i<NUM_PIC_YUV; i++)
  {
    m_apcPicYuv[i]      = NULL;
  }
}

TComPic::~TComPic()
{
  destroy();
}
#if SVC_EXTENSION
#if REDUCED_ENCODER_MEMORY
Void TComPic::create( const TComSPS &sps, const TComPPS &pps, const Bool bCreateEncoderSourcePicYuv, const Bool bCreateForImmediateReconstruction, const UInt layerId )
#else
Void TComPic::create( const TComSPS &sps, const TComPPS &pps, const Bool bIsVirtual, const UInt layerId )
#endif
{
  destroy();

  const ChromaFormat chromaFormatIDC = sps.getChromaFormatIdc();
  const Int          iWidth          = sps.getPicWidthInLumaSamples();
  const Int          iHeight         = sps.getPicHeightInLumaSamples();
  const UInt         uiMaxCuWidth    = sps.getMaxCUWidth();
  const UInt         uiMaxCuHeight   = sps.getMaxCUHeight();
  const UInt         uiMaxDepth      = sps.getMaxTotalCUDepth();
  const Window& conformanceWindow    = sps.getConformanceWindow();

  m_layerId = layerId;

#if REDUCED_ENCODER_MEMORY
  m_picSym.create( sps, pps, uiMaxDepth, bCreateForImmediateReconstruction, layerId );
  if (bCreateEncoderSourcePicYuv)
#else
  m_picSym.create( sps, pps, uiMaxDepth, layerId );

  if (!bIsVirtual)
#endif
  {
    m_apcPicYuv[PIC_YUV_ORG     ]  = new TComPicYuv;  m_apcPicYuv[PIC_YUV_ORG     ]->create( iWidth, iHeight, chromaFormatIDC, uiMaxCuWidth, uiMaxCuHeight, uiMaxDepth, true, &conformanceWindow );
    m_apcPicYuv[PIC_YUV_TRUE_ORG]  = new TComPicYuv;  m_apcPicYuv[PIC_YUV_TRUE_ORG]->create( iWidth, iHeight, chromaFormatIDC, uiMaxCuWidth, uiMaxCuHeight, uiMaxDepth, true, &conformanceWindow );
  }
#if REDUCED_ENCODER_MEMORY
  if (bCreateForImmediateReconstruction)
  {
#endif
  m_apcPicYuv[PIC_YUV_REC]  = new TComPicYuv;  m_apcPicYuv[PIC_YUV_REC]->create( iWidth, iHeight, chromaFormatIDC, uiMaxCuWidth, uiMaxCuHeight, uiMaxDepth, true, &conformanceWindow );

  for( Int i = 0; i < MAX_LAYERS; i++ )
  {
    if( m_requireResampling[i] )
    {
      m_pcFullPelBaseRec[i] = new TComPicYuv;  m_pcFullPelBaseRec[i]->create( iWidth, iHeight, chromaFormatIDC, uiMaxCuWidth, uiMaxCuHeight, uiMaxDepth, true, &conformanceWindow );
    }
  }
#if REDUCED_ENCODER_MEMORY
  }
#endif

  // there are no SEI messages associated with this picture initially
  if (m_SEIs.size() > 0)
  {
    deleteSEIs (m_SEIs);
  }
  m_bUsedByCurr = false;
}
#else
#if REDUCED_ENCODER_MEMORY
Void TComPic::create( const TComSPS &sps, const TComPPS &pps, const Bool bCreateEncoderSourcePicYuv, const Bool bCreateForImmediateReconstruction )
#else
Void TComPic::create( const TComSPS &sps, const TComPPS &pps, const Bool bIsVirtual)
#endif
{
  destroy();

  const ChromaFormat chromaFormatIDC = sps.getChromaFormatIdc();
  const Int          iWidth          = sps.getPicWidthInLumaSamples();
  const Int          iHeight         = sps.getPicHeightInLumaSamples();
  const UInt         uiMaxCuWidth    = sps.getMaxCUWidth();
  const UInt         uiMaxCuHeight   = sps.getMaxCUHeight();
  const UInt         uiMaxDepth      = sps.getMaxTotalCUDepth();

#if REDUCED_ENCODER_MEMORY
  m_picSym.create( sps, pps, uiMaxDepth, bCreateForImmediateReconstruction );
  if (bCreateEncoderSourcePicYuv)
#else
  m_picSym.create( sps, pps, uiMaxDepth );
  if (!bIsVirtual)
#endif
  {
    m_apcPicYuv[PIC_YUV_ORG    ]   = new TComPicYuv;  m_apcPicYuv[PIC_YUV_ORG     ]->create( iWidth, iHeight, chromaFormatIDC, uiMaxCuWidth, uiMaxCuHeight, uiMaxDepth, true );
    m_apcPicYuv[PIC_YUV_TRUE_ORG]  = new TComPicYuv;  m_apcPicYuv[PIC_YUV_TRUE_ORG]->create( iWidth, iHeight, chromaFormatIDC, uiMaxCuWidth, uiMaxCuHeight, uiMaxDepth, true );
  }
#if REDUCED_ENCODER_MEMORY
  if (bCreateForImmediateReconstruction)
  {
#endif
    m_apcPicYuv[PIC_YUV_REC]  = new TComPicYuv;  m_apcPicYuv[PIC_YUV_REC]->create( iWidth, iHeight, chromaFormatIDC, uiMaxCuWidth, uiMaxCuHeight, uiMaxDepth, true );
#if REDUCED_ENCODER_MEMORY
  }
#endif

  // there are no SEI messages associated with this picture initially
  if (m_SEIs.size() > 0)
  {
    deleteSEIs (m_SEIs);
  }
  m_bUsedByCurr = false;
}
#endif

#if REDUCED_ENCODER_MEMORY
Void TComPic::prepareForEncoderSourcePicYuv()
{
  const TComSPS &sps=m_picSym.getSPS();

  const ChromaFormat chromaFormatIDC = sps.getChromaFormatIdc();
  const Int          iWidth          = sps.getPicWidthInLumaSamples();
  const Int          iHeight         = sps.getPicHeightInLumaSamples();
  const UInt         uiMaxCuWidth    = sps.getMaxCUWidth();
  const UInt         uiMaxCuHeight   = sps.getMaxCUHeight();
  const UInt         uiMaxDepth      = sps.getMaxTotalCUDepth();

#if SVC_EXTENSION
  const Window& conformanceWindow    = sps.getConformanceWindow();

  if (m_apcPicYuv[PIC_YUV_ORG    ]==NULL)
  {
    m_apcPicYuv[PIC_YUV_ORG    ]   = new TComPicYuv;  m_apcPicYuv[PIC_YUV_ORG     ]->create( iWidth, iHeight, chromaFormatIDC, uiMaxCuWidth, uiMaxCuHeight, uiMaxDepth, true, &conformanceWindow );
  }
  if (m_apcPicYuv[PIC_YUV_TRUE_ORG    ]==NULL)
  {
    m_apcPicYuv[PIC_YUV_TRUE_ORG]  = new TComPicYuv;  m_apcPicYuv[PIC_YUV_TRUE_ORG]->create( iWidth, iHeight, chromaFormatIDC, uiMaxCuWidth, uiMaxCuHeight, uiMaxDepth, true, &conformanceWindow );
  }
#else
  if (m_apcPicYuv[PIC_YUV_ORG    ]==NULL)
  {
    m_apcPicYuv[PIC_YUV_ORG    ]   = new TComPicYuv;  m_apcPicYuv[PIC_YUV_ORG     ]->create( iWidth, iHeight, chromaFormatIDC, uiMaxCuWidth, uiMaxCuHeight, uiMaxDepth, true );
  }
  if (m_apcPicYuv[PIC_YUV_TRUE_ORG    ]==NULL)
  {
    m_apcPicYuv[PIC_YUV_TRUE_ORG]  = new TComPicYuv;  m_apcPicYuv[PIC_YUV_TRUE_ORG]->create( iWidth, iHeight, chromaFormatIDC, uiMaxCuWidth, uiMaxCuHeight, uiMaxDepth, true );
  }
#endif
}

Void TComPic::prepareForReconstruction()
{
  if (m_apcPicYuv[PIC_YUV_REC] == NULL)
  {
    const TComSPS &sps=m_picSym.getSPS();
    const ChromaFormat chromaFormatIDC = sps.getChromaFormatIdc();
    const Int          iWidth          = sps.getPicWidthInLumaSamples();
    const Int          iHeight         = sps.getPicHeightInLumaSamples();
    const UInt         uiMaxCuWidth    = sps.getMaxCUWidth();
    const UInt         uiMaxCuHeight   = sps.getMaxCUHeight();
    const UInt         uiMaxDepth      = sps.getMaxTotalCUDepth();

#if SVC_EXTENSION
    const Window& conformanceWindow    = sps.getConformanceWindow();

    m_apcPicYuv[PIC_YUV_REC]  = new TComPicYuv;  m_apcPicYuv[PIC_YUV_REC]->create( iWidth, iHeight, chromaFormatIDC, uiMaxCuWidth, uiMaxCuHeight, uiMaxDepth, true, &conformanceWindow );

    for( Int i = 0; i < MAX_LAYERS; i++ )
    {
      if( m_requireResampling[i] )
      {
        m_pcFullPelBaseRec[i] = new TComPicYuv;  m_pcFullPelBaseRec[i]->create( iWidth, iHeight, chromaFormatIDC, uiMaxCuWidth, uiMaxCuHeight, uiMaxDepth, true, &conformanceWindow );
      }
    } 
#else
    m_apcPicYuv[PIC_YUV_REC]  = new TComPicYuv;  m_apcPicYuv[PIC_YUV_REC]->create( iWidth, iHeight, chromaFormatIDC, uiMaxCuWidth, uiMaxCuHeight, uiMaxDepth, true );
#endif
  }

  // mark it should be extended
  m_apcPicYuv[PIC_YUV_REC]->setBorderExtension(false);

  m_picSym.prepareForReconstruction();
}

Void TComPic::releaseReconstructionIntermediateData()
{
  m_picSym.releaseReconstructionIntermediateData();
}

Void TComPic::releaseEncoderSourceImageData()
{
  if (m_apcPicYuv[PIC_YUV_ORG    ])
  {
    m_apcPicYuv[PIC_YUV_ORG]->destroy();
    delete m_apcPicYuv[PIC_YUV_ORG];
    m_apcPicYuv[PIC_YUV_ORG] = NULL;
  }
  if (m_apcPicYuv[PIC_YUV_TRUE_ORG    ])
  {
    m_apcPicYuv[PIC_YUV_TRUE_ORG]->destroy();
    delete m_apcPicYuv[PIC_YUV_TRUE_ORG];
    m_apcPicYuv[PIC_YUV_TRUE_ORG] = NULL;
  }
}

Void TComPic::releaseAllReconstructionData()
{
  if (m_apcPicYuv[PIC_YUV_REC    ])
  {
    m_apcPicYuv[PIC_YUV_REC]->destroy();
    delete m_apcPicYuv[PIC_YUV_REC];
    m_apcPicYuv[PIC_YUV_REC] = NULL;
  }
  m_picSym.releaseAllReconstructionData();
}
#endif

Void TComPic::destroy()
{
  m_picSym.destroy();

  for(UInt i=0; i<NUM_PIC_YUV; i++)
  {
    if (m_apcPicYuv[i])
    {
      m_apcPicYuv[i]->destroy();
      delete m_apcPicYuv[i];
      m_apcPicYuv[i]  = NULL;
    }
  }

  deleteSEIs(m_SEIs);
#if SVC_EXTENSION
  for( Int i = 0; i < MAX_LAYERS; i++ )
  {
    if( m_requireResampling[i] && m_pcFullPelBaseRec[i] )
    {
      m_pcFullPelBaseRec[i]->destroy();
      delete m_pcFullPelBaseRec[i];
      m_pcFullPelBaseRec[i]  = NULL;
    }
  }

  for( Int comp = 0; comp < 2; comp++ )
  {
    if( m_mvScalingFactor[comp] )
    {
      delete [] m_mvScalingFactor[comp];
      m_mvScalingFactor[comp] = NULL;
    }

    if( m_posScalingFactor[comp] )
    {
      delete [] m_posScalingFactor[comp];
      m_posScalingFactor[comp] = NULL;
    }
  }
#endif 
}

Void TComPic::compressMotion()
{
  TComPicSym* pPicSym = getPicSym();
  for ( UInt uiCUAddr = 0; uiCUAddr < pPicSym->getNumberOfCtusInFrame(); uiCUAddr++ )
  {
    TComDataCU* pCtu = pPicSym->getCtu(uiCUAddr);
    pCtu->compressMV();
  }
}

Bool  TComPic::getSAOMergeAvailability(Int currAddr, Int mergeAddr)
{
  Bool mergeCtbInSliceSeg = (mergeAddr >= getPicSym()->getCtuTsToRsAddrMap(getCtu(currAddr)->getSlice()->getSliceCurStartCtuTsAddr()));
  Bool mergeCtbInTile     = (getPicSym()->getTileIdxMap(mergeAddr) == getPicSym()->getTileIdxMap(currAddr));
  return (mergeCtbInSliceSeg && mergeCtbInTile);
}

UInt TComPic::getSubstreamForCtuAddr(const UInt ctuAddr, const Bool bAddressInRaster, TComSlice *pcSlice)
{
  UInt subStrm;
  const bool bWPPEnabled=pcSlice->getPPS()->getEntropyCodingSyncEnabledFlag();
  const TComPicSym &picSym            = *(getPicSym());

  if ((bWPPEnabled && picSym.getFrameHeightInCtus()>1) || (picSym.getNumTiles()>1)) // wavefronts, and possibly tiles being used.
  {
    if (bWPPEnabled)
    {
      const UInt ctuRsAddr                = bAddressInRaster?ctuAddr : picSym.getCtuTsToRsAddrMap(ctuAddr);
      const UInt frameWidthInCtus         = picSym.getFrameWidthInCtus();
      const UInt tileIndex                = picSym.getTileIdxMap(ctuRsAddr);
      const UInt numTileColumns           = (picSym.getNumTileColumnsMinus1()+1);
      const TComTile *pTile               = picSym.getTComTile(tileIndex);
      const UInt firstCtuRsAddrOfTile     = pTile->getFirstCtuRsAddr();
      const UInt tileYInCtus              = firstCtuRsAddrOfTile / frameWidthInCtus;
      // independent tiles => substreams are "per tile"
      const UInt ctuLine                  = ctuRsAddr / frameWidthInCtus;
      const UInt startingSubstreamForTile =(tileYInCtus*numTileColumns) + (pTile->getTileHeightInCtus()*(tileIndex%numTileColumns));
      subStrm = startingSubstreamForTile + (ctuLine - tileYInCtus);
    }
    else
    {
      const UInt ctuRsAddr                = bAddressInRaster?ctuAddr : picSym.getCtuTsToRsAddrMap(ctuAddr);
      const UInt tileIndex                = picSym.getTileIdxMap(ctuRsAddr);
      subStrm=tileIndex;
    }
  }
  else
  {
    // dependent tiles => substreams are "per frame".
    subStrm = 0;
  }
  return subStrm;
}

#if SVC_EXTENSION
Void TComPic::createMvScalingFactor(UInt numOfILRPs)
{
  // picture object might be reused and hence m_mvScalingFactor[0] can be already allocated
  if(m_mvScalingFactor[0])
  {
    delete m_mvScalingFactor[0];
  }
  m_mvScalingFactor[0] = new Int[numOfILRPs];

  // picture object might be reused and hence m_mvScalingFactor[1] can be already allocated
  if(m_mvScalingFactor[1])
  {
    delete m_mvScalingFactor[1];
  }
  m_mvScalingFactor[1] = new Int[numOfILRPs];
}

Void TComPic::createPosScalingFactor(UInt numOfILRPs)
{
  // picture object might be reused and hence m_posScalingFactor[0] can be already allocated
  if(m_posScalingFactor[0])
  {
    delete m_posScalingFactor[0];
  }
  m_posScalingFactor[0] = new Int[numOfILRPs];

  // picture object might be reused and hence m_posScalingFactor[1] can be already allocated
  if(m_posScalingFactor[1])
  {
    delete m_posScalingFactor[1];
  }
  m_posScalingFactor[1] = new Int[numOfILRPs];
}

Void copyOnetoOnePicture(    // SVC_NONCOLL
                  Pel *in,        
                  Pel *out,      
                  Int nCols,
                  Int nRows, 
                  Int fullRowWidth)
{
  Int rX;

  for (rX = 0; rX < nRows; rX++)       
  {
    memcpy( out, in, sizeof(Pel) * nCols );
    in = in + fullRowWidth;
    out = out + fullRowWidth;
  }
}

Void TComPic::copyUpsampledPictureYuv(TComPicYuv*   pcPicYuvIn, TComPicYuv*   pcPicYuvOut)
{
#if SCALABLE_REXT
  Int upsampledRowWidthLuma = pcPicYuvOut->getStride(COMPONENT_Y); // 2 * pcPicYuvOut->getLumaMargin() + pcPicYuvOut->getWidth(); 
  copyOnetoOnePicture(
    pcPicYuvIn->getAddr(COMPONENT_Y),        
    pcPicYuvOut->getAddr(COMPONENT_Y),      
    pcPicYuvOut->getWidth(COMPONENT_Y), 
    pcPicYuvOut->getHeight(COMPONENT_Y),
    upsampledRowWidthLuma);

  if(pcPicYuvOut->getChromaFormat() != CHROMA_400)
  {
    Int upsampledRowWidthChroma = pcPicYuvOut->getStride(COMPONENT_Cb); //2 * pcPicYuvOut->getChromaMargin() + (pcPicYuvOut->getWidth()>>1);

    copyOnetoOnePicture(
      pcPicYuvIn->getAddr(COMPONENT_Cr),        
      pcPicYuvOut->getAddr(COMPONENT_Cr),      
      pcPicYuvOut->getWidth(COMPONENT_Cr), 
      pcPicYuvOut->getHeight(COMPONENT_Cr),
      upsampledRowWidthChroma);
    copyOnetoOnePicture(
      pcPicYuvIn->getAddr(COMPONENT_Cb),        
      pcPicYuvOut->getAddr(COMPONENT_Cb),      
      pcPicYuvOut->getWidth(COMPONENT_Cb), 
      pcPicYuvOut->getHeight(COMPONENT_Cb),
      upsampledRowWidthChroma);
  }
#else
  Int upsampledRowWidthLuma = pcPicYuvOut->getStride(COMPONENT_Y); // 2 * pcPicYuvOut->getLumaMargin() + pcPicYuvOut->getWidth(); 
  Int upsampledRowWidthCroma = pcPicYuvOut->getStride(COMPONENT_Cb); //2 * pcPicYuvOut->getChromaMargin() + (pcPicYuvOut->getWidth()>>1);

  copyOnetoOnePicture(
    pcPicYuvIn->getAddr(COMPONENT_Y),        
    pcPicYuvOut->getAddr(COMPONENT_Y),      
    pcPicYuvOut->getWidth(COMPONENT_Y), 
    pcPicYuvOut->getHeight(COMPONENT_Y),
    upsampledRowWidthLuma);
  copyOnetoOnePicture(
    pcPicYuvIn->getAddr(COMPONENT_Cr),        
    pcPicYuvOut->getAddr(COMPONENT_Cr),      
    pcPicYuvOut->getWidth(COMPONENT_Y)>>1, 
    pcPicYuvOut->getHeight(COMPONENT_Y)>>1,
    upsampledRowWidthCroma);
  copyOnetoOnePicture(
    pcPicYuvIn->getAddr(COMPONENT_Cb),        
    pcPicYuvOut->getAddr(COMPONENT_Cb),      
    pcPicYuvOut->getWidth(COMPONENT_Y)>>1, 
    pcPicYuvOut->getHeight(COMPONENT_Y)>>1,
    upsampledRowWidthCroma);
#endif
}

Void TComPic::copyUpsampledMvField(UInt refLayerIdc, Int** mvScalingFactor, Int** posScalingFactor)
{
  const TComSPS *sps       = getSlice(0)->getSPS();
  const UInt uiMaxDepth    = sps->getMaxTotalCUDepth();
  const UInt numPartitions = 1<<(uiMaxDepth<<1);
  const UInt widthMinPU    = sps->getMaxCUWidth()  / (1<<uiMaxDepth);
  const UInt heightMinPU   = sps->getMaxCUHeight() / (1<<uiMaxDepth);
  const Int  unitNum       = max( 1, (Int)((16/widthMinPU)*(16/heightMinPU)) ); 

  for( UInt ctuRsAddr = 0; ctuRsAddr < m_picSym.getNumberOfCtusInFrame(); ctuRsAddr++ )  //each CTU
  {
    TComDataCU* pcCUDes = getCtu(ctuRsAddr);

#if REDUCED_ENCODER_MEMORY
    TComPicSym::DPBPerCtuData &dpbForCtu = m_picSym.getDPBPerCtuData(ctuRsAddr);
#endif

    for(UInt absPartIdx = 0; absPartIdx < numPartitions; absPartIdx+=unitNum )  //each 16x16 unit
    {
      //pixel position of each unit in up-sampled layer
      UInt pelX = pcCUDes->getCUPelX() + g_auiRasterToPelX[ g_auiZscanToRaster[absPartIdx] ];
      UInt pelY = pcCUDes->getCUPelY() + g_auiRasterToPelY[ g_auiZscanToRaster[absPartIdx] ];
      UInt baseCUAddr, baseAbsPartIdx;

      TComDataCU *pcColCU = 0;
      pcColCU = pcCUDes->getBaseColCU(refLayerIdc, pelX, pelY, baseCUAddr, baseAbsPartIdx, posScalingFactor, true);

      if( pcColCU && pcColCU->getPredictionMode(baseAbsPartIdx) == MODE_INTER )  //base layer unit not skip and invalid mode
      {
        for(UInt refPicList = 0; refPicList < 2; refPicList++)  //for each reference list
        {
          TComMvField sMvFieldBase, sMvField;
          pcColCU->getMvField( pcColCU, baseAbsPartIdx, (RefPicList)refPicList, sMvFieldBase);
          pcCUDes->scaleBaseMV( refLayerIdc, sMvField, sMvFieldBase, mvScalingFactor );

          pcCUDes->getCUMvField((RefPicList)refPicList)->setMvField(sMvField, absPartIdx);
          pcCUDes->setPredictionMode(absPartIdx, MODE_INTER);
        }
      }
      else
      {
        TComMvField zeroMvField;  //zero MV and invalid reference index
        pcCUDes->getCUMvField(REF_PIC_LIST_0)->setMvField(zeroMvField, absPartIdx);
        pcCUDes->getCUMvField(REF_PIC_LIST_1)->setMvField(zeroMvField, absPartIdx);
        pcCUDes->setPredictionMode(absPartIdx, MODE_INTRA);
      }

#if REDUCED_ENCODER_MEMORY
      dpbForCtu.m_CUMvField[REF_PIC_LIST_0].setMvField(pcCUDes->getCUMvField(REF_PIC_LIST_0)->getMv(absPartIdx), pcCUDes->getCUMvField(REF_PIC_LIST_0)->getRefIdx(absPartIdx), absPartIdx);
      dpbForCtu.m_CUMvField[REF_PIC_LIST_1].setMvField(pcCUDes->getCUMvField(REF_PIC_LIST_1)->getMv(absPartIdx), pcCUDes->getCUMvField(REF_PIC_LIST_1)->getRefIdx(absPartIdx), absPartIdx);
#endif

      for(UInt i = 1; i < unitNum; i++ )  
      {
        pcCUDes->getCUMvField(REF_PIC_LIST_0)->setMvField(pcCUDes->getCUMvField(REF_PIC_LIST_0)->getMv(absPartIdx), pcCUDes->getCUMvField(REF_PIC_LIST_0)->getRefIdx(absPartIdx), absPartIdx + i);
        pcCUDes->getCUMvField(REF_PIC_LIST_1)->setMvField(pcCUDes->getCUMvField(REF_PIC_LIST_1)->getMv(absPartIdx), pcCUDes->getCUMvField(REF_PIC_LIST_1)->getRefIdx(absPartIdx), absPartIdx + i);

#if REDUCED_ENCODER_MEMORY
        dpbForCtu.m_CUMvField[REF_PIC_LIST_0].setMvField(pcCUDes->getCUMvField(REF_PIC_LIST_0)->getMv(absPartIdx), pcCUDes->getCUMvField(REF_PIC_LIST_0)->getRefIdx(absPartIdx), absPartIdx + i);
        dpbForCtu.m_CUMvField[REF_PIC_LIST_1].setMvField(pcCUDes->getCUMvField(REF_PIC_LIST_1)->getMv(absPartIdx), pcCUDes->getCUMvField(REF_PIC_LIST_1)->getRefIdx(absPartIdx), absPartIdx + i);
#endif
        pcCUDes->setPredictionMode(absPartIdx+i, pcCUDes->getPredictionMode(absPartIdx));
      }
    }
    memset( pcCUDes->getPartitionSize(), SIZE_2Nx2N, sizeof(*pcCUDes->getPartitionSize())*numPartitions );

#if REDUCED_ENCODER_MEMORY
    memcpy( dpbForCtu.m_pePredMode, pcCUDes->getPredictionMode(), sizeof(*pcCUDes->getPredictionMode()) * numPartitions );
    memcpy( dpbForCtu.m_pePartSize, pcCUDes->getPartitionSize(), sizeof(*pcCUDes->getPartitionSize()) * numPartitions );
    dpbForCtu.m_pSlice = pcCUDes->getSlice();
#endif
  }
}

Void TComPic::initUpsampledMvField()
{
  const TComSPS *sps         = getSlice(0)->getSPS();
  const UInt uiMaxDepth      = sps->getMaxTotalCUDepth();
  const UInt numPartitions = 1<<(uiMaxDepth<<1);

  for( UInt ctuRsAddr = 0; ctuRsAddr < m_picSym.getNumberOfCtusInFrame(); ctuRsAddr++ )  //each CTU
  {
    TComDataCU* pcCUDes = getCtu(ctuRsAddr);
    TComMvField zeroMvField;

#if REDUCED_ENCODER_MEMORY
    TComPicSym::DPBPerCtuData &dpbForCtu = m_picSym.getDPBPerCtuData(ctuRsAddr);
#endif

    for( UInt i = 0; i < numPartitions; i++ )  
    {
      pcCUDes->getCUMvField(REF_PIC_LIST_0)->setMvField(zeroMvField, i);
      pcCUDes->getCUMvField(REF_PIC_LIST_1)->setMvField(zeroMvField, i);
      pcCUDes->setPredictionMode(i, MODE_INTRA);
      pcCUDes->setPartitionSize(i, SIZE_2Nx2N);

#if REDUCED_ENCODER_MEMORY
      dpbForCtu.m_CUMvField[REF_PIC_LIST_0].setMvField(zeroMvField, i);
      dpbForCtu.m_CUMvField[REF_PIC_LIST_1].setMvField(zeroMvField, i);
#endif
    }

#if REDUCED_ENCODER_MEMORY
    memcpy( dpbForCtu.m_pePredMode, pcCUDes->getPredictionMode(), sizeof(*pcCUDes->getPredictionMode()) * numPartitions );
    memcpy( dpbForCtu.m_pePartSize, pcCUDes->getPartitionSize(), sizeof(*pcCUDes->getPartitionSize()) * numPartitions );
    dpbForCtu.m_pSlice = pcCUDes->getSlice();
#endif
  }
}

Bool TComPic::checkSameRefInfo()
{
  Bool bSameRefInfo = true;
  TComSlice * pSlice0 = getSlice( 0 );
  for( UInt uSliceID = getNumAllocatedSlice() - 1 ; bSameRefInfo && uSliceID > 0 ; uSliceID-- )
  {
    TComSlice * pSliceN = getSlice( uSliceID );
    if( pSlice0->getSliceType() != pSliceN->getSliceType() )
    {
      bSameRefInfo = false;
    }
    else if( pSlice0->getSliceType() != I_SLICE )
    {
      Int nListNum = pSlice0->getSliceType() == B_SLICE ? 2 : 1;
      for( Int nList = 0 ; nList < nListNum ; nList++ )
      {
        RefPicList eRefList = ( RefPicList )nList;
        if( pSlice0->getNumRefIdx( eRefList ) == pSliceN->getNumRefIdx( eRefList ) )
        {
          for( Int refIdx = pSlice0->getNumRefIdx( eRefList ) - 1 ; refIdx >= 0 ; refIdx-- )
          {
            if( pSlice0->getRefPic( eRefList , refIdx ) != pSliceN->getRefPic( eRefList , refIdx ) )
            {
              bSameRefInfo = false;
              break;
            }
          }
        }
        else
        {
          bSameRefInfo = false;
          break;
        }
      }
    }
  }

  return( bSameRefInfo );  
}
#endif //SVC_EXTENSION

//! \}
