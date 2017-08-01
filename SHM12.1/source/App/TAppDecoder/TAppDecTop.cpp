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

/** \file     TAppDecTop.cpp
    \brief    Decoder application class
*/

#include <list>
#include <vector>
#include <stdio.h>
#include <fcntl.h>
#include <assert.h>

#include "TAppDecTop.h"
#include "TLibDecoder/AnnexBread.h"
#include "TLibDecoder/NALread.h"
#if RExt__DECODER_DEBUG_BIT_STATISTICS
#include "TLibCommon/TComCodingStatistics.h"
#endif
#if CONFORMANCE_BITSTREAM_MODE
#include "TLibCommon/TComPicYuv.h"
#include "libmd5/MD5.h"
#endif

//! \ingroup TAppDecoder
//! \{

// ====================================================================================================================
// Constructor / destructor / initialization / destroy
// ====================================================================================================================

#if SVC_EXTENSION
TAppDecTop::TAppDecTop()
: m_pcSeiColourRemappingInfoPrevious(NULL)
{
  memset( m_apcTDecTop, 0, sizeof(m_apcTDecTop) );
  memset( m_apcTVideoIOYuvReconFile, 0, sizeof(m_apcTVideoIOYuvReconFile) );

  for(UInt layer=0; layer < MAX_LAYERS; layer++)
  {
    m_aiPOCLastDisplay[layer]  = -MAX_INT;
  }
}
#else
TAppDecTop::TAppDecTop()
: m_iPOCLastDisplay(-MAX_INT)
 ,m_pcSeiColourRemappingInfoPrevious(NULL)
{
}
#endif

Void TAppDecTop::create()
{
}

Void TAppDecTop::destroy()
{
  m_bitstreamFileName.clear();
#if SVC_EXTENSION
#if CONFORMANCE_BITSTREAM_MODE
  for(Int i = 0; i < MAX_VPS_LAYER_IDX_PLUS1; i++ )
#else
  for( Int i = 0; i <= m_commonDecoderParams.getTargetLayerId(); i++ )
#endif
  {
    m_reconFileName[i].clear();

    if( m_apcTDecTop[i] )
    {
      delete m_apcTDecTop[i];
      m_apcTDecTop[i] = NULL;
    }

    if( m_apcTVideoIOYuvReconFile[i] )
    {
      delete m_apcTVideoIOYuvReconFile[i];
      m_apcTVideoIOYuvReconFile[i] = NULL;
    }
  }
#if AVC_BASE
  m_reconFileNameBL.clear();
#endif
#else
  m_reconFileName.clear();
#endif
}

// ====================================================================================================================
// Public member functions
// ====================================================================================================================

/**
 - create internal class
 - initialize internal class
 - until the end of the bitstream, call decoding function in TDecTop class
 - delete allocated buffers
 - destroy internal class
 .
 */
Void TAppDecTop::decode()
{
  Int                 poc;
  TComList<TComPic*>* pcListPic = NULL;

  ifstream bitstreamFile(m_bitstreamFileName.c_str(), ifstream::in | ifstream::binary);
  if (!bitstreamFile)
  {
    fprintf(stderr, "\nfailed to open bitstream file `%s' for reading\n", m_bitstreamFileName.c_str());
    exit(EXIT_FAILURE);
  }

  InputByteStream bytestream(bitstreamFile);

  if (!m_outputDecodedSEIMessagesFilename.empty() && m_outputDecodedSEIMessagesFilename!="-")
  {
    m_seiMessageFileStream.open(m_outputDecodedSEIMessagesFilename.c_str(), std::ios::out);
    if (!m_seiMessageFileStream.is_open() || !m_seiMessageFileStream.good())
    {
      fprintf(stderr, "\nUnable to open file `%s' for writing decoded SEI messages\n", m_outputDecodedSEIMessagesFilename.c_str());
      exit(EXIT_FAILURE);
    }
  }

  // create & initialize internal classes
  xCreateDecLib();
  xInitDecLib  ();
#if !SVC_EXTENSION
  m_iPOCLastDisplay += m_iSkipFrame;      // set the last displayed POC correctly for skip forward.
#endif

  // clear contents of colour-remap-information-SEI output file
  if (!m_colourRemapSEIFileName.empty())
  {
    std::ofstream ofile(m_colourRemapSEIFileName.c_str());
    if (!ofile.good() || !ofile.is_open())
    {
      fprintf(stderr, "\nUnable to open file '%s' for writing colour-remap-information-SEI video\n", m_colourRemapSEIFileName.c_str());
      exit(EXIT_FAILURE);
    }
  }

  // main decoder loop
#if SVC_EXTENSION
  Bool openedReconFile[MAX_LAYERS]; // reconstruction file not yet opened. (must be performed after SPS is seen)
  Bool loopFiltered[MAX_LAYERS];
  memset( loopFiltered, false, sizeof( loopFiltered ) );

#if CONFORMANCE_BITSTREAM_MODE
  for(UInt layer = 0; layer < MAX_VPS_LAYER_IDX_PLUS1; layer++)
#else
  for(UInt layer=0; layer <= m_commonDecoderParams.getTargetLayerId(); layer++)
#endif
  {
    openedReconFile[layer] = false;
    m_aiPOCLastDisplay[layer] += m_iSkipFrame;      // set the last displayed POC correctly for skip forward.
  }

  UInt curLayerId = 0;     // current layer to be reconstructed

#if AVC_BASE
  TComPic pcBLPic;
  fstream streamYUV;
  if( !m_reconFileNameBL.empty() )
  {
    streamYUV.open( m_reconFileNameBL.c_str(), fstream::in | fstream::binary );
  }
  m_apcTDecTop[0]->setBLReconFile( &streamYUV );
  pcBLPic.setLayerId( 0 );
  m_apcTDecTop[0]->setBlPic(&pcBLPic);
#endif

  while (!!bitstreamFile)
  {
#if 1
	  if (isPreDecode)
	  {
		  lmAllDecInfo *lminfo = lmAllDecInfo::getInstance();
		  ParameterSetManager *allps = m_apcTDecTop[0]->getParameterSetManager();
		  int i = 0;
		  TComVPS*      fvps = allps->getVPS(i);
		  while (fvps != nullptr)
		  {
			  lmPSData tVPS(lmPStype[paraTYPE::vps]);
			  tVPS << sParam(tVPS.getParamName(0), int(fvps->getMaxLayers()))
				  << sParam(tVPS.getParamName(1), int(fvps->getVPSId()));
			  (*lminfo) << tVPS;
			  i++;
			  fvps = allps->getVPS(i);
		  }
		  if (lminfo->preDecReady())
		  {
			  lminfo->outputPreDec();
			  exit(0);
		  }
	  }
#endif
    /* location serves to work around a design fault in the decoder, whereby
     * the process of reading a new slice that is the first slice of a new frame
     * requires the TDecTop::decode() method to be called again with the same
     * nal unit. */
#if RExt__DECODER_DEBUG_BIT_STATISTICS
    TComCodingStatistics::TComCodingStatisticsData backupStats(TComCodingStatistics::GetStatistics());
    streampos location = bitstreamFile.tellg() - streampos(bytestream.GetNumBufferedBytes());
#else
    streampos location = bitstreamFile.tellg();
#endif
    AnnexBStats stats = AnnexBStats();

    InputNALUnit nalu;
    byteStreamNALUnit(bytestream, nalu.getBitstream().getFifo(), stats);

    // call actual decoding function
    Bool bNewPicture = false;
    Bool bNewPOC = false;

    if (nalu.getBitstream().getFifo().empty())
    {
      /* this can happen if the following occur:
       *  - empty input file
       *  - two back-to-back start_code_prefixes
       *  - start_code_prefix immediately followed by EOF
       */
      fprintf(stderr, "Warning: Attempt to decode an empty NAL unit\n");
    }
    else
    {
      read(nalu);

      // ignore any NAL units with nuh_layer_id == 63
      if( nalu.m_nuhLayerId == 63 )
      {
        printf("Ignore NAL unit with m_nuhLayerId equal to 63\n");
        continue;
      }

      if( (m_iMaxTemporalLayer >= 0 && nalu.m_temporalId > m_iMaxTemporalLayer) || !isNaluWithinTargetDecLayerIdSet(&nalu) || nalu.m_nuhLayerId > m_commonDecoderParams.getTargetLayerId() )
      {
        bNewPicture = false;
      }
      else
      {
        bNewPicture = m_apcTDecTop[nalu.m_nuhLayerId]->decode(nalu, m_iSkipFrame, m_aiPOCLastDisplay[nalu.m_nuhLayerId], curLayerId, bNewPOC);

#if SVC_POC
        if( (bNewPicture && m_apcTDecTop[nalu.m_nuhLayerId]->getParseIdc() == 3) || (m_apcTDecTop[nalu.m_nuhLayerId]->getParseIdc() == 0) )
#else
        if (bNewPicture)
#endif
        {
          bitstreamFile.clear();
          /* location points to the current nalunit payload[1] due to the
           * need for the annexB parser to read three extra bytes.
           * [1] except for the first NAL unit in the file
           *     (but bNewPicture doesn't happen then) */
#if RExt__DECODER_DEBUG_BIT_STATISTICS
          bitstreamFile.seekg(location);
          bytestream.reset();
          TComCodingStatistics::SetStatistics(backupStats);
#else
          bitstreamFile.seekg(location-streamoff(3));
          bytestream.reset();
#endif
        }
#if SVC_POC
        else if(m_apcTDecTop[nalu.m_nuhLayerId]->getParseIdc() == 1) 
        {
          bitstreamFile.clear();
          // This is before third parse of the NAL unit, and 
          // location points to correct beginning of the NALU
          bitstreamFile.seekg(location);
          bytestream.reset();
#if RExt__DECODER_DEBUG_BIT_STATISTICS
          TComCodingStatistics::SetStatistics(backupStats);
#endif
        }
#endif
      }
    }

#if SVC_POC
    if( ( (bNewPicture && m_apcTDecTop[nalu.m_nuhLayerId]->getParseIdc() == 3) || m_apcTDecTop[nalu.m_nuhLayerId]->getParseIdc() == 0 || !bitstreamFile || nalu.m_nalUnitType == NAL_UNIT_EOS ) && 
        !m_apcTDecTop[nalu.m_nuhLayerId]->getFirstSliceInSequence() )
#else
    if ( (bNewPicture || !bitstreamFile || nalu.m_nalUnitType == NAL_UNIT_EOS) &&
        !m_apcTDecTop[nalu.m_nuhLayerId]->getFirstSliceInSequence() )
#endif
    {
      if (!loopFiltered[curLayerId] || bitstreamFile)
      {
        m_apcTDecTop[curLayerId]->executeLoopFilters(poc, pcListPic);
      }
      loopFiltered[curLayerId] = (nalu.m_nalUnitType == NAL_UNIT_EOS);

      if (nalu.m_nalUnitType == NAL_UNIT_EOS)
      {
        m_apcTDecTop[nalu.m_nuhLayerId]->setFirstSliceInSequence(true);
      }
    }
    else if ( (bNewPicture || !bitstreamFile || nalu.m_nalUnitType == NAL_UNIT_EOS ) &&
              m_apcTDecTop[nalu.m_nuhLayerId]->getFirstSliceInSequence () ) 
    {
      m_apcTDecTop[nalu.m_nuhLayerId]->setFirstSliceInPicture (true);
    }

#if SVC_POC
    if( bNewPicture && m_apcTDecTop[nalu.m_nuhLayerId]->getParseIdc() == 0 )
    {
      outputAllPictures( nalu.m_nuhLayerId, true );
    }
#endif

    if( pcListPic )
    {
      if ( !m_reconFileName[curLayerId].empty() && !openedReconFile[curLayerId] && m_apcTDecTop[curLayerId]->getParameterSetManager()->getActiveSPS())
      {
        const BitDepths& bitDepths = m_apcTDecTop[curLayerId]->getParameterSetManager()->getActiveSPS()->getBitDepths();

        for( UInt channelType = 0; channelType < MAX_NUM_CHANNEL_TYPE; channelType++)
        {
          if( m_outputBitDepth[curLayerId][channelType] == 0 )
          {
            m_outputBitDepth[curLayerId][channelType] = bitDepths.recon[channelType];
          }
        }
        m_apcTVideoIOYuvReconFile[curLayerId]->open( m_reconFileName[curLayerId], true, m_outputBitDepth[curLayerId], m_outputBitDepth[curLayerId], bitDepths.recon ); // write mode

        openedReconFile[curLayerId] = true;
      }
#if ALIGNED_BUMPING
      Bool outputPicturesFlag = true;  

      if( m_apcTDecTop[nalu.m_nuhLayerId]->getNoOutputPriorPicsFlag() )
      {
        outputPicturesFlag = false;
      }

      if (nalu.m_nalUnitType == NAL_UNIT_EOS) // End of sequence
      {
        flushAllPictures( nalu.m_nuhLayerId, outputPicturesFlag );       
      }

#if SVC_POC
      if( bNewPicture && m_apcTDecTop[nalu.m_nuhLayerId]->getParseIdc() != 0 )
      // New picture, slice header parsed but picture not decoded
#else
      if( bNewPicture ) // New picture, slice header parsed but picture not decoded
#endif
      {
         if(   nalu.m_nalUnitType == NAL_UNIT_CODED_SLICE_IDR_W_RADL
            || nalu.m_nalUnitType == NAL_UNIT_CODED_SLICE_IDR_N_LP
            || nalu.m_nalUnitType == NAL_UNIT_CODED_SLICE_BLA_N_LP
            || nalu.m_nalUnitType == NAL_UNIT_CODED_SLICE_BLA_W_RADL
            || nalu.m_nalUnitType == NAL_UNIT_CODED_SLICE_BLA_W_LP   )
        {
          flushAllPictures( nalu.m_nuhLayerId, outputPicturesFlag );
        }
        else
        {
          this->checkOutputBeforeDecoding( nalu.m_nuhLayerId );
        }
      }

      /* The following code has to be executed when the last DU of the picture is decoded
         TODO: Need code to identify end of decoding a picture
      {
        this->checkOutputAfterDecoding( );
      } */
#else
      if ( bNewPicture && bNewPOC &&
           (   nalu.m_nalUnitType == NAL_UNIT_CODED_SLICE_IDR_W_RADL
            || nalu.m_nalUnitType == NAL_UNIT_CODED_SLICE_IDR_N_LP
            || nalu.m_nalUnitType == NAL_UNIT_CODED_SLICE_BLA_N_LP
            || nalu.m_nalUnitType == NAL_UNIT_CODED_SLICE_BLA_W_RADL
            || nalu.m_nalUnitType == NAL_UNIT_CODED_SLICE_BLA_W_LP ) )
      {
        xFlushOutput( pcListPic, curLayerId );
      }
      if (nalu.m_nalUnitType == NAL_UNIT_EOS)
      {
        xFlushOutput( pcListPic, curLayerId );        
      }
      // write reconstruction to file
      if(bNewPicture)
      {
        xWriteOutput( pcListPic, curLayerId, nalu.m_temporalId );
      }
#endif
    }
  }
#if ALIGNED_BUMPING
   flushAllPictures( true );   
#else
  for(UInt layer = 0; layer <= m_tgtLayerId; layer++)
  {
    xFlushOutput( m_apcTDecTop[layer]->getListPic(), layer );
  }
#endif
  // delete buffers
#if AVC_BASE
  UInt layerIdxmin = m_apcTDecTop[0]->getBLReconFile()->is_open() ? 1 : 0;

  if( streamYUV.is_open() )
  {
    streamYUV.close();
  }

#if CONFORMANCE_BITSTREAM_MODE
  for(UInt layer = layerIdxmin; layer < MAX_VPS_LAYER_IDX_PLUS1; layer++)
#else
  for(UInt layer = layerIdxmin; layer <= m_commonDecoderParams.getTargetLayerId(); layer++)
#endif
#else
  for(UInt layer = 0; layer <= m_commonDecoderParams.getTargetLayerId(); layer++)
#endif
  {
    m_apcTDecTop[layer]->deletePicBuffer();
  }

  // destroy internal classes
  xDestroyDecLib();
#else
  Bool openedReconFile = false; // reconstruction file not yet opened. (must be performed after SPS is seen)
  Bool loopFiltered = false;

  while (!!bitstreamFile)
  {
    /* location serves to work around a design fault in the decoder, whereby
     * the process of reading a new slice that is the first slice of a new frame
     * requires the TDecTop::decode() method to be called again with the same
     * nal unit. */
#if RExt__DECODER_DEBUG_BIT_STATISTICS
    TComCodingStatistics::TComCodingStatisticsData backupStats(TComCodingStatistics::GetStatistics());
    streampos location = bitstreamFile.tellg() - streampos(bytestream.GetNumBufferedBytes());
#else
    streampos location = bitstreamFile.tellg();
#endif
    AnnexBStats stats = AnnexBStats();

    InputNALUnit nalu;
    byteStreamNALUnit(bytestream, nalu.getBitstream().getFifo(), stats);

    // call actual decoding function
    Bool bNewPicture = false;
    if (nalu.getBitstream().getFifo().empty())
    {
      /* this can happen if the following occur:
       *  - empty input file
       *  - two back-to-back start_code_prefixes
       *  - start_code_prefix immediately followed by EOF
       */
      fprintf(stderr, "Warning: Attempt to decode an empty NAL unit\n");
    }
    else
    {
      read(nalu);
      if( (m_iMaxTemporalLayer >= 0 && nalu.m_temporalId > m_iMaxTemporalLayer) || !isNaluWithinTargetDecLayerIdSet(&nalu)  )
      {
        bNewPicture = false;
      }
      else
      {
        bNewPicture = m_cTDecTop.decode(nalu, m_iSkipFrame, m_iPOCLastDisplay);
        if (bNewPicture)
        {
          bitstreamFile.clear();
          /* location points to the current nalunit payload[1] due to the
           * need for the annexB parser to read three extra bytes.
           * [1] except for the first NAL unit in the file
           *     (but bNewPicture doesn't happen then) */
#if RExt__DECODER_DEBUG_BIT_STATISTICS
          bitstreamFile.seekg(location);
          bytestream.reset();
          TComCodingStatistics::SetStatistics(backupStats);
#else
          bitstreamFile.seekg(location-streamoff(3));
          bytestream.reset();
#endif
        }
      }
    }

    if ( (bNewPicture || !bitstreamFile || nalu.m_nalUnitType == NAL_UNIT_EOS) &&
        !m_cTDecTop.getFirstSliceInSequence () )
    {
      if (!loopFiltered || bitstreamFile)
      {
        m_cTDecTop.executeLoopFilters(poc, pcListPic);
      }
      loopFiltered = (nalu.m_nalUnitType == NAL_UNIT_EOS);
      if (nalu.m_nalUnitType == NAL_UNIT_EOS)
      {
        m_cTDecTop.setFirstSliceInSequence(true);
      }
    }
    else if ( (bNewPicture || !bitstreamFile || nalu.m_nalUnitType == NAL_UNIT_EOS ) &&
              m_cTDecTop.getFirstSliceInSequence () ) 
    {
      m_cTDecTop.setFirstSliceInPicture (true);
    }

    if( pcListPic )
    {
      if ( (!m_reconFileName.empty()) && (!openedReconFile) )
      {
        const BitDepths &bitDepths=pcListPic->front()->getPicSym()->getSPS().getBitDepths(); // use bit depths of first reconstructed picture.
        for (UInt channelType = 0; channelType < MAX_NUM_CHANNEL_TYPE; channelType++)
        {
          if (m_outputBitDepth[channelType] == 0)
          {
            m_outputBitDepth[channelType] = bitDepths.recon[channelType];
          }
        }

        m_cTVideoIOYuvReconFile.open( m_reconFileName, true, m_outputBitDepth, m_outputBitDepth, bitDepths.recon ); // write mode
        openedReconFile = true;
      }
      // write reconstruction to file
      if( bNewPicture )
      {
        xWriteOutput( pcListPic, nalu.m_temporalId );
      }
      if ( (bNewPicture || nalu.m_nalUnitType == NAL_UNIT_CODED_SLICE_CRA) && m_cTDecTop.getNoOutputPriorPicsFlag() )
      {
        m_cTDecTop.checkNoOutputPriorPics( pcListPic );
        m_cTDecTop.setNoOutputPriorPicsFlag (false);
      }
      if ( bNewPicture &&
           (   nalu.m_nalUnitType == NAL_UNIT_CODED_SLICE_IDR_W_RADL
            || nalu.m_nalUnitType == NAL_UNIT_CODED_SLICE_IDR_N_LP
            || nalu.m_nalUnitType == NAL_UNIT_CODED_SLICE_BLA_N_LP
            || nalu.m_nalUnitType == NAL_UNIT_CODED_SLICE_BLA_W_RADL
            || nalu.m_nalUnitType == NAL_UNIT_CODED_SLICE_BLA_W_LP ) )
      {
        xFlushOutput( pcListPic );
      }
      if (nalu.m_nalUnitType == NAL_UNIT_EOS)
      {
        xWriteOutput( pcListPic, nalu.m_temporalId );
        m_cTDecTop.setFirstSliceInPicture (false);
      }
      // write reconstruction to file -- for additional bumping as defined in C.5.2.3
      if(!bNewPicture && nalu.m_nalUnitType >= NAL_UNIT_CODED_SLICE_TRAIL_N && nalu.m_nalUnitType <= NAL_UNIT_RESERVED_VCL31)
      {
        xWriteOutput( pcListPic, nalu.m_temporalId );
      }
    }
  }

  xFlushOutput( pcListPic );
  // delete buffers
  m_cTDecTop.deletePicBuffer();

  // destroy internal classes
  xDestroyDecLib();
#endif
}

// ====================================================================================================================
// Protected member functions
// ====================================================================================================================

Void TAppDecTop::xCreateDecLib()
{
#if SVC_EXTENSION
  // initialize global variables
  initROM();

#if CONFORMANCE_BITSTREAM_MODE
  for(UInt layer = 0; layer < MAX_VPS_LAYER_IDX_PLUS1; layer++)
#else
  for(UInt layer = 0; layer <= m_commonDecoderParams.getTargetLayerId(); layer++)
#endif
  {
    m_apcTDecTop[layer] = new TDecTop;
    m_apcTVideoIOYuvReconFile[layer] = new TVideoIOYuv;

    // set layer ID
    m_apcTDecTop[layer]->setLayerId                      ( layer );

    // create decoder class
    m_apcTDecTop[layer]->create();

    m_apcTDecTop[layer]->setLayerDec(m_apcTDecTop);
  }
#else
  // create decoder class
  m_cTDecTop.create();
#endif
}

Void TAppDecTop::xDestroyDecLib()
{
#if SVC_EXTENSION
  // destroy ROM
  destroyROM();

#if CONFORMANCE_BITSTREAM_MODE
  for(UInt layer = 0; layer < MAX_VPS_LAYER_IDX_PLUS1; layer++)
#else
  for(UInt layer = 0; layer <= m_commonDecoderParams.getTargetLayerId(); layer++)
#endif
  {
    if ( !m_reconFileName[layer].empty() )
    {
      m_apcTVideoIOYuvReconFile[layer]->close();
    }

    // destroy decoder class
    m_apcTDecTop[layer]->destroy();
  }
#else
  if ( !m_reconFileName.empty() )
  {
    m_cTVideoIOYuvReconFile.close();
  }

  // destroy decoder class
  m_cTDecTop.destroy();
#endif
  if (m_pcSeiColourRemappingInfoPrevious != NULL)
  {
    delete m_pcSeiColourRemappingInfoPrevious;
    m_pcSeiColourRemappingInfoPrevious = NULL;
  }
}

Void TAppDecTop::xInitDecLib()
{
  // initialize decoder class
#if SVC_EXTENSION
#if CONFORMANCE_BITSTREAM_MODE
  for(UInt layer = 0; layer < MAX_VPS_LAYER_IDX_PLUS1; layer++)
#else
  for(UInt layer = 0; layer <= m_commonDecoderParams.getTargetLayerId(); layer++)
#endif
  {
    TDecTop& m_cTDecTop = *m_apcTDecTop[layer];
#endif

  m_cTDecTop.init();
  m_cTDecTop.setDecodedPictureHashSEIEnabled(m_decodedPictureHashSEIEnabled);
#if O0043_BEST_EFFORT_DECODING
  m_cTDecTop.setForceDecodeBitDepth(m_forceDecodeBitDepth);
#endif
  if (!m_outputDecodedSEIMessagesFilename.empty())
  {
    std::ostream &os=m_seiMessageFileStream.is_open() ? m_seiMessageFileStream : std::cout;
    m_cTDecTop.setDecodedSEIMessageOutputStream(&os);
  }

#if SVC_EXTENSION  
#if CONFORMANCE_BITSTREAM_MODE
    m_cTDecTop.setConfModeFlag( m_confModeFlag );
#endif
    m_cTDecTop.setCommonDecoderParams( &m_commonDecoderParams );
  }
#endif

  if (m_pcSeiColourRemappingInfoPrevious != NULL)
  {
    delete m_pcSeiColourRemappingInfoPrevious;
    m_pcSeiColourRemappingInfoPrevious = NULL;
  }
}

/** \param pcListPic list of pictures to be written to file
    \param tId       temporal sub-layer ID
 */
#if SVC_EXTENSION
Void TAppDecTop::xWriteOutput( TComList<TComPic*>* pcListPic, UInt layerId, UInt tId )
#else
Void TAppDecTop::xWriteOutput( TComList<TComPic*>* pcListPic, UInt tId )
#endif
{
  if (pcListPic->empty())
  {
    return;
  }

  TComList<TComPic*>::iterator iterPic   = pcListPic->begin();
  Int numPicsNotYetDisplayed = 0;
  Int dpbFullness = 0;
  const TComSPS* activeSPS = &(pcListPic->front()->getPicSym()->getSPS());
  UInt numReorderPicsHighestTid;
  UInt maxDecPicBufferingHighestTid;
  UInt maxNrSublayers = activeSPS->getMaxTLayers();

  if(m_iMaxTemporalLayer == -1 || m_iMaxTemporalLayer >= maxNrSublayers)
  {
    numReorderPicsHighestTid = activeSPS->getNumReorderPics(maxNrSublayers-1);
    maxDecPicBufferingHighestTid =  activeSPS->getMaxDecPicBuffering(maxNrSublayers-1); 
  }
  else
  {
    numReorderPicsHighestTid = activeSPS->getNumReorderPics(m_iMaxTemporalLayer);
    maxDecPicBufferingHighestTid = activeSPS->getMaxDecPicBuffering(m_iMaxTemporalLayer); 
  }

  while (iterPic != pcListPic->end())
  {
    TComPic* pcPic = *(iterPic);
#if SVC_EXTENSION
    if(pcPic->getOutputMark() && pcPic->getPOC() > m_aiPOCLastDisplay[layerId])
#else
    if(pcPic->getOutputMark() && pcPic->getPOC() > m_iPOCLastDisplay)
#endif
    {
       numPicsNotYetDisplayed++;
      dpbFullness++;
    }
    else if(pcPic->getSlice( 0 )->isReferenced())
    {
      dpbFullness++;
    }
    iterPic++;
  }

  iterPic = pcListPic->begin();

  if (numPicsNotYetDisplayed>2)
  {
    iterPic++;
  }

  TComPic* pcPic = *(iterPic);
  if (numPicsNotYetDisplayed>2 && pcPic->isField()) //Field Decoding
  {
    TComList<TComPic*>::iterator endPic   = pcListPic->end();
    endPic--;
    iterPic   = pcListPic->begin();
    while (iterPic != endPic)
    {
      TComPic* pcPicTop = *(iterPic);
      iterPic++;
      TComPic* pcPicBottom = *(iterPic);

#if SVC_EXTENSION
      if( pcPicTop->getOutputMark() && pcPicBottom->getOutputMark() &&
        (numPicsNotYetDisplayed >  numReorderPicsHighestTid || dpbFullness > maxDecPicBufferingHighestTid) &&
        (!(pcPicTop->getPOC()%2) && pcPicBottom->getPOC() == pcPicTop->getPOC()+1) &&        
        (pcPicTop->getPOC() == m_aiPOCLastDisplay[layerId]+1 || m_aiPOCLastDisplay[layerId]<0) )
#else
      if ( pcPicTop->getOutputMark() && pcPicBottom->getOutputMark() &&
          (numPicsNotYetDisplayed >  numReorderPicsHighestTid || dpbFullness > maxDecPicBufferingHighestTid) &&
          (!(pcPicTop->getPOC()%2) && pcPicBottom->getPOC() == pcPicTop->getPOC()+1) &&
          (pcPicTop->getPOC() == m_iPOCLastDisplay+1 || m_iPOCLastDisplay < 0))
#endif
      {
        // write to file
        numPicsNotYetDisplayed = numPicsNotYetDisplayed-2;
#if SVC_EXTENSION
        if ( !m_reconFileName[layerId].empty() )
        {
          const Window &conf = pcPicTop->getConformanceWindow();
          const Window  defDisp = m_respectDefDispWindow ? pcPicTop->getDefDisplayWindow() : Window();
          const Bool isTff = pcPicTop->isTopField();

          Bool display = true;
          if( m_decodedNoDisplaySEIEnabled )
          {
            SEIMessages noDisplay = getSeisByType(pcPic->getSEIs(), SEI::NO_DISPLAY );
            const SEINoDisplay *nd = ( noDisplay.size() > 0 ) ? (SEINoDisplay*) *(noDisplay.begin()) : NULL;
            if( (nd != NULL) && nd->m_noDisplay )
            {
              display = false;
            }
          }

          if (display)
          {            
            UInt chromaFormatIdc = pcPic->getSlice(0)->getSPS()->getChromaFormatIdc();
            Int xScal =  TComSPS::getWinUnitX( chromaFormatIdc ), yScal = TComSPS::getWinUnitY( chromaFormatIdc );

            m_apcTVideoIOYuvReconFile[layerId]->write( pcPicTop->getPicYuvRec(), pcPicBottom->getPicYuvRec(),
              m_outputColourSpaceConvert,
              conf.getWindowLeftOffset()  * xScal + defDisp.getWindowLeftOffset(),
              conf.getWindowRightOffset() * xScal + defDisp.getWindowRightOffset(),
              conf.getWindowTopOffset()   * yScal + defDisp.getWindowTopOffset(),
              conf.getWindowBottomOffset()* yScal + defDisp.getWindowBottomOffset(), NUM_CHROMA_FORMAT, isTff );
          }
        }

        // update POC of display order
        m_aiPOCLastDisplay[layerId] = pcPicBottom->getPOC();
#else
        if ( !m_reconFileName.empty() )
        {
          const Window &conf = pcPicTop->getConformanceWindow();
          const Window  defDisp = m_respectDefDispWindow ? pcPicTop->getDefDisplayWindow() : Window();
          const Bool isTff = pcPicTop->isTopField();

          Bool display = true;
          if( m_decodedNoDisplaySEIEnabled )
          {
            SEIMessages noDisplay = getSeisByType(pcPic->getSEIs(), SEI::NO_DISPLAY );
            const SEINoDisplay *nd = ( noDisplay.size() > 0 ) ? (SEINoDisplay*) *(noDisplay.begin()) : NULL;
            if( (nd != NULL) && nd->m_noDisplay )
            {
              display = false;
            }
          }

          if (display)
          {
            m_cTVideoIOYuvReconFile.write( pcPicTop->getPicYuvRec(), pcPicBottom->getPicYuvRec(),
                                           m_outputColourSpaceConvert,
                                           conf.getWindowLeftOffset() + defDisp.getWindowLeftOffset(),
                                           conf.getWindowRightOffset() + defDisp.getWindowRightOffset(),
                                           conf.getWindowTopOffset() + defDisp.getWindowTopOffset(),
                                           conf.getWindowBottomOffset() + defDisp.getWindowBottomOffset(), NUM_CHROMA_FORMAT, isTff );
          }
        }

        // update POC of display order
        m_iPOCLastDisplay = pcPicBottom->getPOC();
#endif

        // erase non-referenced picture in the reference picture list after display
        if ( !pcPicTop->getSlice(0)->isReferenced() && pcPicTop->getReconMark() == true )
        {
          pcPicTop->setReconMark(false);

          // mark it should be extended later
          pcPicTop->getPicYuvRec()->setBorderExtension( false );
        }
        if ( !pcPicBottom->getSlice(0)->isReferenced() && pcPicBottom->getReconMark() == true )
        {
          pcPicBottom->setReconMark(false);

          // mark it should be extended later
          pcPicBottom->getPicYuvRec()->setBorderExtension( false );
        }
        pcPicTop->setOutputMark(false);
        pcPicBottom->setOutputMark(false);
      }
    }
  }
  else if (!pcPic->isField()) //Frame Decoding
  {
    iterPic = pcListPic->begin();

    while (iterPic != pcListPic->end())
    {
      pcPic = *(iterPic);

#if SVC_EXTENSION
      if( pcPic->getOutputMark() && pcPic->getPOC() > m_aiPOCLastDisplay[layerId] &&
        (numPicsNotYetDisplayed >  numReorderPicsHighestTid || dpbFullness > maxDecPicBufferingHighestTid) )
#else
      if(pcPic->getOutputMark() && pcPic->getPOC() > m_iPOCLastDisplay &&
        (numPicsNotYetDisplayed >  numReorderPicsHighestTid || dpbFullness > maxDecPicBufferingHighestTid))
#endif
      {
        // write to file
         numPicsNotYetDisplayed--;
        if(pcPic->getSlice(0)->isReferenced() == false)
        {
          dpbFullness--;
        }
#if SVC_EXTENSION
        if ( !m_reconFileName[layerId].empty() )
        {
          const Window &conf = pcPic->getConformanceWindow();
          const Window  defDisp = m_respectDefDispWindow ? pcPic->getDefDisplayWindow() : Window();          

          UInt chromaFormatIdc = pcPic->getSlice(0)->getSPS()->getChromaFormatIdc();
          Int xScal =  TComSPS::getWinUnitX( chromaFormatIdc ), yScal = TComSPS::getWinUnitY( chromaFormatIdc );

          m_apcTVideoIOYuvReconFile[layerId]->write( pcPic->getPicYuvRec(), m_outputColourSpaceConvert,
            conf.getWindowLeftOffset()  * xScal + defDisp.getWindowLeftOffset(),
            conf.getWindowRightOffset() * xScal + defDisp.getWindowRightOffset(),
            conf.getWindowTopOffset()   * yScal + defDisp.getWindowTopOffset(),
            conf.getWindowBottomOffset()* yScal + defDisp.getWindowBottomOffset(),
            NUM_CHROMA_FORMAT, m_bClipOutputVideoToRec709Range  );
        }

        // update POC of display order
        m_aiPOCLastDisplay[layerId] = pcPic->getPOC();
#else
        if ( !m_reconFileName.empty() )
        {
          const Window &conf    = pcPic->getConformanceWindow();
          const Window  defDisp = m_respectDefDispWindow ? pcPic->getDefDisplayWindow() : Window();

          m_cTVideoIOYuvReconFile.write( pcPic->getPicYuvRec(),
                                         m_outputColourSpaceConvert,
                                         conf.getWindowLeftOffset() + defDisp.getWindowLeftOffset(),
                                         conf.getWindowRightOffset() + defDisp.getWindowRightOffset(),
                                         conf.getWindowTopOffset() + defDisp.getWindowTopOffset(),
                                         conf.getWindowBottomOffset() + defDisp.getWindowBottomOffset(),
                                         NUM_CHROMA_FORMAT, m_bClipOutputVideoToRec709Range  );
        }

        if (!m_colourRemapSEIFileName.empty())
        {
          xOutputColourRemapPic(pcPic);
        }

        // update POC of display order
        m_iPOCLastDisplay = pcPic->getPOC();
#endif

        // erase non-referenced picture in the reference picture list after display
        if ( !pcPic->getSlice(0)->isReferenced() && pcPic->getReconMark() == true )
        {
          pcPic->setReconMark(false);

          // mark it should be extended later
          pcPic->getPicYuvRec()->setBorderExtension( false );
        }
        pcPic->setOutputMark(false);
      }

      iterPic++;
    }
  }
}

/** \param pcListPic list of pictures to be written to file
 */
#if SVC_EXTENSION
Void TAppDecTop::xFlushOutput( TComList<TComPic*>* pcListPic, UInt layerId )
#else
Void TAppDecTop::xFlushOutput( TComList<TComPic*>* pcListPic )
#endif
{
  if(!pcListPic || pcListPic->empty())
  {
    return;
  }
  TComList<TComPic*>::iterator iterPic   = pcListPic->begin();

  iterPic   = pcListPic->begin();
  TComPic* pcPic = *(iterPic);

  if (pcPic->isField()) //Field Decoding
  {
    TComList<TComPic*>::iterator endPic   = pcListPic->end();
    endPic--;
    TComPic *pcPicTop, *pcPicBottom = NULL;
    while (iterPic != endPic)
    {
      pcPicTop = *(iterPic);
      iterPic++;
      pcPicBottom = *(iterPic);

      if ( pcPicTop->getOutputMark() && pcPicBottom->getOutputMark() && !(pcPicTop->getPOC()%2) && (pcPicBottom->getPOC() == pcPicTop->getPOC()+1) )
      {
        // write to file
#if SVC_EXTENSION
        if ( !m_reconFileName[layerId].empty() )
        {
          const Window &conf = pcPicTop->getConformanceWindow();
          const Window  defDisp = m_respectDefDispWindow ? pcPicTop->getDefDisplayWindow() : Window();
          const Bool isTff = pcPicTop->isTopField();          

          UInt chromaFormatIdc = pcPic->getSlice(0)->getSPS()->getChromaFormatIdc();
          Int xScal =  TComSPS::getWinUnitX( chromaFormatIdc ), yScal = TComSPS::getWinUnitY( chromaFormatIdc );

          m_apcTVideoIOYuvReconFile[layerId]->write( pcPicTop->getPicYuvRec(), pcPicBottom->getPicYuvRec(), m_outputColourSpaceConvert,
            conf.getWindowLeftOffset()  *xScal + defDisp.getWindowLeftOffset(),
            conf.getWindowRightOffset() *xScal + defDisp.getWindowRightOffset(),
            conf.getWindowTopOffset()   *yScal + defDisp.getWindowTopOffset(),
            conf.getWindowBottomOffset()*yScal + defDisp.getWindowBottomOffset(), NUM_CHROMA_FORMAT, isTff );
        }

        // update POC of display order
        m_aiPOCLastDisplay[layerId] = pcPicBottom->getPOC();
#else
        if ( !m_reconFileName.empty() )
        {
          const Window &conf = pcPicTop->getConformanceWindow();
          const Window  defDisp = m_respectDefDispWindow ? pcPicTop->getDefDisplayWindow() : Window();
          const Bool isTff = pcPicTop->isTopField();
          m_cTVideoIOYuvReconFile.write( pcPicTop->getPicYuvRec(), pcPicBottom->getPicYuvRec(),
                                         m_outputColourSpaceConvert,
                                         conf.getWindowLeftOffset() + defDisp.getWindowLeftOffset(),
                                         conf.getWindowRightOffset() + defDisp.getWindowRightOffset(),
                                         conf.getWindowTopOffset() + defDisp.getWindowTopOffset(),
                                         conf.getWindowBottomOffset() + defDisp.getWindowBottomOffset(), NUM_CHROMA_FORMAT, isTff );
        }

        // update POC of display order
        m_iPOCLastDisplay = pcPicBottom->getPOC();
#endif

        // erase non-referenced picture in the reference picture list after display
        if ( !pcPicTop->getSlice(0)->isReferenced() && pcPicTop->getReconMark() == true )
        {
          pcPicTop->setReconMark(false);

          // mark it should be extended later
          pcPicTop->getPicYuvRec()->setBorderExtension( false );
        }
        if ( !pcPicBottom->getSlice(0)->isReferenced() && pcPicBottom->getReconMark() == true )
        {
          pcPicBottom->setReconMark(false);

          // mark it should be extended later
          pcPicBottom->getPicYuvRec()->setBorderExtension( false );
        }
        pcPicTop->setOutputMark(false);
        pcPicBottom->setOutputMark(false);

        if(pcPicTop)
        {
          pcPicTop->destroy();
          delete pcPicTop;
          pcPicTop = NULL;
        }
      }
    }
    if(pcPicBottom)
    {
      pcPicBottom->destroy();
      delete pcPicBottom;
      pcPicBottom = NULL;
    }
  }
  else //Frame decoding
  {
    while (iterPic != pcListPic->end())
    {
      pcPic = *(iterPic);

      if ( pcPic->getOutputMark() )
      {
        // write to file
#if SVC_EXTENSION
        if ( !m_reconFileName[layerId].empty() )
        {
          const Window &conf = pcPic->getConformanceWindow();
          const Window  defDisp = m_respectDefDispWindow ? pcPic->getDefDisplayWindow() : Window();          

          UInt chromaFormatIdc = pcPic->getSlice(0)->getSPS()->getChromaFormatIdc();
          Int xScal =  TComSPS::getWinUnitX( chromaFormatIdc ), yScal = TComSPS::getWinUnitY( chromaFormatIdc );

          m_apcTVideoIOYuvReconFile[layerId]->write( pcPic->getPicYuvRec(),
                                                   m_outputColourSpaceConvert,
                                                   conf.getWindowLeftOffset()  *xScal + defDisp.getWindowLeftOffset(),
                                                   conf.getWindowRightOffset() *xScal + defDisp.getWindowRightOffset(),
                                                   conf.getWindowTopOffset()   *yScal + defDisp.getWindowTopOffset(),
                                                   conf.getWindowBottomOffset()*yScal + defDisp.getWindowBottomOffset(),            
                                                   NUM_CHROMA_FORMAT, m_bClipOutputVideoToRec709Range );
        }

        // update POC of display order
        m_aiPOCLastDisplay[layerId] = pcPic->getPOC();
#else
        if ( !m_reconFileName.empty() )
        {
          const Window &conf    = pcPic->getConformanceWindow();
          const Window  defDisp = m_respectDefDispWindow ? pcPic->getDefDisplayWindow() : Window();

          m_cTVideoIOYuvReconFile.write( pcPic->getPicYuvRec(),
                                         m_outputColourSpaceConvert,
                                         conf.getWindowLeftOffset() + defDisp.getWindowLeftOffset(),
                                         conf.getWindowRightOffset() + defDisp.getWindowRightOffset(),
                                         conf.getWindowTopOffset() + defDisp.getWindowTopOffset(),
                                         conf.getWindowBottomOffset() + defDisp.getWindowBottomOffset(),
                                         NUM_CHROMA_FORMAT, m_bClipOutputVideoToRec709Range );
        }

        if (!m_colourRemapSEIFileName.empty())
        {
          xOutputColourRemapPic(pcPic);
        }

        // update POC of display order
        m_iPOCLastDisplay = pcPic->getPOC();
#endif

        // erase non-referenced picture in the reference picture list after display
        if ( !pcPic->getSlice(0)->isReferenced() && pcPic->getReconMark() == true )
        {
          pcPic->setReconMark(false);

          // mark it should be extended later
          pcPic->getPicYuvRec()->setBorderExtension( false );
        }
        pcPic->setOutputMark(false);
      }
#if !SVC_EXTENSION
      if(pcPic != NULL)
      {
        pcPic->destroy();
        delete pcPic;
        pcPic = NULL;
      }
#endif
      iterPic++;
    }
  }
#if SVC_EXTENSION
  m_aiPOCLastDisplay[layerId] = -MAX_INT;
#else
  pcListPic->clear();
  m_iPOCLastDisplay = -MAX_INT;
#endif
}

/** \param nalu Input nalu to check whether its LayerId is within targetDecLayerIdSet
 */
Bool TAppDecTop::isNaluWithinTargetDecLayerIdSet( InputNALUnit* nalu )
{
  if ( m_targetDecLayerIdSet.size() == 0 ) // By default, the set is empty, meaning all LayerIds are allowed
  {
    return true;
  }
#if SVC_EXTENSION
  if (nalu->m_nuhLayerId == 0 && (nalu->m_nalUnitType == NAL_UNIT_VPS || nalu->m_nalUnitType == NAL_UNIT_SPS || nalu->m_nalUnitType == NAL_UNIT_PPS || nalu->m_nalUnitType == NAL_UNIT_EOS))
  {
    return true;
  }
#endif
  for (std::vector<Int>::iterator it = m_targetDecLayerIdSet.begin(); it != m_targetDecLayerIdSet.end(); it++)
  {
    if ( nalu->m_nuhLayerId == (*it) )
    {
      return true;
    }
  }
  return false;
}

Void TAppDecTop::xOutputColourRemapPic(TComPic* pcPic)
{
  const TComSPS &sps=pcPic->getPicSym()->getSPS();
  SEIMessages colourRemappingInfo = getSeisByType(pcPic->getSEIs(), SEI::COLOUR_REMAPPING_INFO );
  SEIColourRemappingInfo *seiColourRemappingInfo = ( colourRemappingInfo.size() > 0 ) ? (SEIColourRemappingInfo*) *(colourRemappingInfo.begin()) : NULL;

  if (colourRemappingInfo.size() > 1)
  {
    printf ("Warning: Got multiple Colour Remapping Information SEI messages. Using first.");
  }
  if (seiColourRemappingInfo)
  {
    applyColourRemapping(*pcPic->getPicYuvRec(), *seiColourRemappingInfo, sps);

    // save the last CRI SEI received
    if (m_pcSeiColourRemappingInfoPrevious == NULL)
    {
      m_pcSeiColourRemappingInfoPrevious = new SEIColourRemappingInfo();
    }
    m_pcSeiColourRemappingInfoPrevious->copyFrom(*seiColourRemappingInfo);
  }
  else  // using the last CRI SEI received
  {
    // TODO: prevent persistence of CRI SEI across C(L)VS.
    if (m_pcSeiColourRemappingInfoPrevious != NULL)
    {
      if (m_pcSeiColourRemappingInfoPrevious->m_colourRemapPersistenceFlag == false)
      {
        printf("Warning No SEI-CRI message is present for the current picture, persistence of the CRI is not managed\n");
      }
      applyColourRemapping(*pcPic->getPicYuvRec(), *m_pcSeiColourRemappingInfoPrevious, sps);
    }
  }
}

// compute lut from SEI
// use at lutPoints points aligned on a power of 2 value
// SEI Lut must be in ascending values of coded Values
static std::vector<Int>
initColourRemappingInfoLut(const Int                                          bitDepth_in,     // bit-depth of the input values of the LUT
                           const Int                                          nbDecimalValues, // Position of the fixed point
                           const std::vector<SEIColourRemappingInfo::CRIlut> &lut,
                           const Int                                          maxValue, // maximum output value
                           const Int                                          lutOffset)
{
  const Int lutPoints = (1 << bitDepth_in) + 1 ;
  std::vector<Int> retLut(lutPoints);

  // missing values: need to define default values before first definition (check codedValue[0] == 0)
  Int iTargetPrev = (lut.size() && lut[0].codedValue == 0) ? lut[0].targetValue: 0;
  Int startPivot = (lut.size())? ((lut[0].codedValue == 0)? 1: 0): 1;
  Int iCodedPrev  = 0;
  // set max value with the coded bit-depth
  // + ((1 << nbDecimalValues) - 1) is for the added bits
  const Int maxValueFixedPoint = (maxValue << nbDecimalValues) + ((1 << nbDecimalValues) - 1);

  Int iValue = 0;

  for ( Int iPivot=startPivot ; iPivot < (Int)lut.size(); iPivot++ )
  {
    Int iCodedNext  = lut[iPivot].codedValue;
    Int iTargetNext = lut[iPivot].targetValue;

    // ensure correct bit depth and avoid overflow in lut address
    Int iCodedNext_bitDepth = std::min(iCodedNext, (1 << bitDepth_in));

    const Int divValue =  (iCodedNext - iCodedPrev > 0)? (iCodedNext - iCodedPrev): 1;
    const Int lutValInit = (lutOffset + iTargetPrev) << nbDecimalValues;
    const Int roundValue = divValue / 2;
    for ( ; iValue<iCodedNext_bitDepth; iValue++ )
    {
      Int value = iValue;
      Int interpol = ((((value-iCodedPrev) * (iTargetNext - iTargetPrev)) << nbDecimalValues) + roundValue) / divValue;               
      retLut[iValue]  = std::min(lutValInit + interpol , maxValueFixedPoint);
    }
    iCodedPrev  = iCodedNext;
    iTargetPrev = iTargetNext;
  }
  // fill missing values if necessary
  if(iCodedPrev < (1 << bitDepth_in)+1)
  {
    Int iCodedNext  = (1 << bitDepth_in);
    Int iTargetNext = (1 << bitDepth_in) - 1;

    const Int divValue =  (iCodedNext - iCodedPrev > 0)? (iCodedNext - iCodedPrev): 1;
    const Int lutValInit = (lutOffset + iTargetPrev) << nbDecimalValues;
    const Int roundValue = divValue / 2;

    for ( ; iValue<=iCodedNext; iValue++ )
    {
      Int value = iValue;
      Int interpol = ((((value-iCodedPrev) * (iTargetNext - iTargetPrev)) << nbDecimalValues) + roundValue) / divValue; 
      retLut[iValue]  = std::min(lutValInit + interpol , maxValueFixedPoint);
    }
  }
  return retLut;
}

static Void
initColourRemappingInfoLuts(std::vector<Int>      (&preLut)[3],
                            std::vector<Int>      (&postLut)[3],
                            SEIColourRemappingInfo &pCriSEI,
                            const Int               maxBitDepth)
{
  Int internalBitDepth = pCriSEI.m_colourRemapBitDepth;
  for ( Int c=0 ; c<3 ; c++ )
  {
    std::sort(pCriSEI.m_preLut[c].begin(), pCriSEI.m_preLut[c].end()); // ensure preLut is ordered in ascending values of codedValues   
    preLut[c] = initColourRemappingInfoLut(pCriSEI.m_colourRemapInputBitDepth, maxBitDepth - pCriSEI.m_colourRemapInputBitDepth, pCriSEI.m_preLut[c], ((1 << internalBitDepth) - 1), 0); //Fill preLut

    std::sort(pCriSEI.m_postLut[c].begin(), pCriSEI.m_postLut[c].end()); // ensure postLut is ordered in ascending values of codedValues       
    postLut[c] = initColourRemappingInfoLut(pCriSEI.m_colourRemapBitDepth, maxBitDepth - pCriSEI.m_colourRemapBitDepth, pCriSEI.m_postLut[c], (1 << internalBitDepth) - 1, 0); //Fill postLut
  }
}

// apply lut.
// Input lut values are aligned on power of 2 boundaries
static Int
applyColourRemappingInfoLut1D(Int inVal, const std::vector<Int> &lut, const Int inValPrecisionBits)
{
  const Int roundValue = (inValPrecisionBits)? 1 << (inValPrecisionBits - 1): 0;
  inVal = std::min(std::max(0, inVal), (Int)(((lut.size()-1) << inValPrecisionBits)));
  Int index  = (Int) std::min((inVal >> inValPrecisionBits), (Int)(lut.size()-2));
  Int outVal = (( inVal - (index<<inValPrecisionBits) ) * (lut[index+1] - lut[index]) + roundValue) >> inValPrecisionBits;
  outVal +=  lut[index] ;

  return outVal;
}  

static Int
applyColourRemappingInfoMatrix(const Int (&colourRemapCoeffs)[3], const Int postOffsetShift, const Int p0, const Int p1, const Int p2, const Int offset)
{
  Int YUVMat = (colourRemapCoeffs[0]* p0 + colourRemapCoeffs[1]* p1 + colourRemapCoeffs[2]* p2  + offset) >> postOffsetShift;
  return YUVMat;
}

static Void
setColourRemappingInfoMatrixOffset(Int (&matrixOffset)[3], Int offset0, Int offset1, Int offset2)
{
  matrixOffset[0] = offset0;
  matrixOffset[1] = offset1;
  matrixOffset[2] = offset2;
}

static Void
setColourRemappingInfoMatrixOffsets(      Int  (&matrixInputOffset)[3],
                                          Int  (&matrixOutputOffset)[3],
                                    const Int  bitDepth,
                                    const Bool crInputFullRangeFlag,
                                    const Int  crInputMatrixCoefficients,
                                    const Bool crFullRangeFlag,
                                    const Int  crMatrixCoefficients)
{
  // set static matrix offsets
  Int crInputOffsetLuma = (crInputFullRangeFlag)? 0:-(16 << (bitDepth-8));
  Int crOffsetLuma = (crFullRangeFlag)? 0:(16 << (bitDepth-8));
  Int crInputOffsetChroma = 0;
  Int crOffsetChroma = 0;

  switch(crInputMatrixCoefficients)
  {
    case MATRIX_COEFFICIENTS_RGB:
      crInputOffsetChroma = 0;
      if(!crInputFullRangeFlag)
      {
        fprintf(stderr, "WARNING: crInputMatrixCoefficients set to MATRIX_COEFFICIENTS_RGB and crInputFullRangeFlag not set\n");
        crInputOffsetLuma = 0;
      }
      break;
    case MATRIX_COEFFICIENTS_UNSPECIFIED:
    case MATRIX_COEFFICIENTS_BT709:
    case MATRIX_COEFFICIENTS_BT2020_NON_CONSTANT_LUMINANCE:
      crInputOffsetChroma = -(1 << (bitDepth-1));
      break;
    default:
      fprintf(stderr, "WARNING: crInputMatrixCoefficients set to undefined value: %d\n", crInputMatrixCoefficients);
  }

  switch(crMatrixCoefficients)
  {
    case MATRIX_COEFFICIENTS_RGB:
      crOffsetChroma = 0;
      if(!crFullRangeFlag)
      {
        fprintf(stderr, "WARNING: crMatrixCoefficients set to MATRIX_COEFFICIENTS_RGB and crInputFullRangeFlag not set\n");
        crOffsetLuma = 0;
      }
      break;
    case MATRIX_COEFFICIENTS_UNSPECIFIED:
    case MATRIX_COEFFICIENTS_BT709:
    case MATRIX_COEFFICIENTS_BT2020_NON_CONSTANT_LUMINANCE:
      crOffsetChroma = (1 << (bitDepth-1));
      break;
    default:
      fprintf(stderr, "WARNING: crMatrixCoefficients set to undefined value: %d\n", crMatrixCoefficients);
  }

  setColourRemappingInfoMatrixOffset(matrixInputOffset, crInputOffsetLuma, crInputOffsetChroma, crInputOffsetChroma);
  setColourRemappingInfoMatrixOffset(matrixOutputOffset, crOffsetLuma, crOffsetChroma, crOffsetChroma);
}

Void TAppDecTop::applyColourRemapping(const TComPicYuv& pic, SEIColourRemappingInfo& criSEI, const TComSPS &activeSPS)
{  
  const Int maxBitDepth = 16;

  // create colour remapped picture
  if( !criSEI.m_colourRemapCancelFlag && pic.getChromaFormat()!=CHROMA_400) // 4:0:0 not supported.
  {
    const Int          iHeight         = pic.getHeight(COMPONENT_Y);
    const Int          iWidth          = pic.getWidth(COMPONENT_Y);
    const ChromaFormat chromaFormatIDC = pic.getChromaFormat();

    TComPicYuv picYuvColourRemapped;
    picYuvColourRemapped.createWithoutCUInfo( iWidth, iHeight, chromaFormatIDC );

    const Int  iStrideIn   = pic.getStride(COMPONENT_Y);
    const Int  iCStrideIn  = pic.getStride(COMPONENT_Cb);
    const Int  iStrideOut  = picYuvColourRemapped.getStride(COMPONENT_Y);
    const Int  iCStrideOut = picYuvColourRemapped.getStride(COMPONENT_Cb);
    const Bool b444        = ( pic.getChromaFormat() == CHROMA_444 );
    const Bool b422        = ( pic.getChromaFormat() == CHROMA_422 );
    const Bool b420        = ( pic.getChromaFormat() == CHROMA_420 );

    std::vector<Int> preLut[3];
    std::vector<Int> postLut[3];
    Int matrixInputOffset[3];
    Int matrixOutputOffset[3];
    const Pel *YUVIn[MAX_NUM_COMPONENT];
    Pel *YUVOut[MAX_NUM_COMPONENT];
    YUVIn[COMPONENT_Y]  = pic.getAddr(COMPONENT_Y);
    YUVIn[COMPONENT_Cb] = pic.getAddr(COMPONENT_Cb);
    YUVIn[COMPONENT_Cr] = pic.getAddr(COMPONENT_Cr);
    YUVOut[COMPONENT_Y]  = picYuvColourRemapped.getAddr(COMPONENT_Y);
    YUVOut[COMPONENT_Cb] = picYuvColourRemapped.getAddr(COMPONENT_Cb);
    YUVOut[COMPONENT_Cr] = picYuvColourRemapped.getAddr(COMPONENT_Cr);

    const Int bitDepth = criSEI.m_colourRemapBitDepth;
    BitDepths        bitDepthsCriFile;
    bitDepthsCriFile.recon[CHANNEL_TYPE_LUMA]   = bitDepth;
    bitDepthsCriFile.recon[CHANNEL_TYPE_CHROMA] = bitDepth; // Different bitdepth is not implemented

    const Int postOffsetShift = criSEI.m_log2MatrixDenom;
    const Int matrixRound = 1 << (postOffsetShift - 1);
    const Int postLutInputPrecision = (maxBitDepth - criSEI.m_colourRemapBitDepth);

    if ( ! criSEI.m_colourRemapVideoSignalInfoPresentFlag ) // setting default
    {
      setColourRemappingInfoMatrixOffsets(matrixInputOffset, matrixOutputOffset, maxBitDepth,
          activeSPS.getVuiParameters()->getVideoFullRangeFlag(), activeSPS.getVuiParameters()->getMatrixCoefficients(),
          activeSPS.getVuiParameters()->getVideoFullRangeFlag(), activeSPS.getVuiParameters()->getMatrixCoefficients());
    }
    else
    {
      setColourRemappingInfoMatrixOffsets(matrixInputOffset, matrixOutputOffset, maxBitDepth,
          activeSPS.getVuiParameters()->getVideoFullRangeFlag(), activeSPS.getVuiParameters()->getMatrixCoefficients(),
          criSEI.m_colourRemapFullRangeFlag, criSEI.m_colourRemapMatrixCoefficients);
    }

    // add matrix rounding to output matrix offsets
    matrixOutputOffset[0] = (matrixOutputOffset[0] << postOffsetShift) + matrixRound;
    matrixOutputOffset[1] = (matrixOutputOffset[1] << postOffsetShift) + matrixRound;
    matrixOutputOffset[2] = (matrixOutputOffset[2] << postOffsetShift) + matrixRound;

    // Merge   matrixInputOffset and matrixOutputOffset to matrixOutputOffset
    matrixOutputOffset[0] += applyColourRemappingInfoMatrix(criSEI.m_colourRemapCoeffs[0], 0, matrixInputOffset[0], matrixInputOffset[1], matrixInputOffset[2], 0);
    matrixOutputOffset[1] += applyColourRemappingInfoMatrix(criSEI.m_colourRemapCoeffs[1], 0, matrixInputOffset[0], matrixInputOffset[1], matrixInputOffset[2], 0);
    matrixOutputOffset[2] += applyColourRemappingInfoMatrix(criSEI.m_colourRemapCoeffs[2], 0, matrixInputOffset[0], matrixInputOffset[1], matrixInputOffset[2], 0);

    // rescaling output: include CRI/output frame difference
    const Int scaleShiftOut_neg = abs(bitDepth - maxBitDepth);
    const Int scaleOut_round = 1 << (scaleShiftOut_neg-1);

    initColourRemappingInfoLuts(preLut, postLut, criSEI, maxBitDepth);

    assert(pic.getChromaFormat() != CHROMA_400);
    const Int hs = pic.getComponentScaleX(ComponentID(COMPONENT_Cb));
    const Int maxOutputValue = (1 << bitDepth) - 1;

    for( Int y = 0; y < iHeight; y++ )
    {
      for( Int x = 0; x < iWidth; x++ )
      {
        const Int xc = (x>>hs);
        Bool computeChroma = b444 || ((b422 || !(y&1)) && !(x&1));

        Int YUVPre_0 = applyColourRemappingInfoLut1D(YUVIn[COMPONENT_Y][x], preLut[0], 0);
        Int YUVPre_1 = applyColourRemappingInfoLut1D(YUVIn[COMPONENT_Cb][xc], preLut[1], 0);
        Int YUVPre_2 = applyColourRemappingInfoLut1D(YUVIn[COMPONENT_Cr][xc], preLut[2], 0);

        Int YUVMat_0 = applyColourRemappingInfoMatrix(criSEI.m_colourRemapCoeffs[0], postOffsetShift, YUVPre_0, YUVPre_1, YUVPre_2, matrixOutputOffset[0]);
        Int YUVLutB_0 = applyColourRemappingInfoLut1D(YUVMat_0, postLut[0], postLutInputPrecision);
        YUVOut[COMPONENT_Y][x] = std::min(maxOutputValue, (YUVLutB_0 + scaleOut_round) >> scaleShiftOut_neg);

        if( computeChroma )
        {
          Int YUVMat_1 = applyColourRemappingInfoMatrix(criSEI.m_colourRemapCoeffs[1], postOffsetShift, YUVPre_0, YUVPre_1, YUVPre_2, matrixOutputOffset[1]);
          Int YUVLutB_1 = applyColourRemappingInfoLut1D(YUVMat_1, postLut[1], postLutInputPrecision);
          YUVOut[COMPONENT_Cb][xc] = std::min(maxOutputValue, (YUVLutB_1 + scaleOut_round) >> scaleShiftOut_neg);

          Int YUVMat_2 = applyColourRemappingInfoMatrix(criSEI.m_colourRemapCoeffs[2], postOffsetShift, YUVPre_0, YUVPre_1, YUVPre_2, matrixOutputOffset[2]);
          Int YUVLutB_2 = applyColourRemappingInfoLut1D(YUVMat_2, postLut[2], postLutInputPrecision);
          YUVOut[COMPONENT_Cr][xc] = std::min(maxOutputValue, (YUVLutB_2 + scaleOut_round) >> scaleShiftOut_neg);
        }
      }

      YUVIn[COMPONENT_Y]  += iStrideIn;
      YUVOut[COMPONENT_Y] += iStrideOut;
      if( !(b420 && !(y&1)) )
      {
         YUVIn[COMPONENT_Cb]  += iCStrideIn;
         YUVIn[COMPONENT_Cr]  += iCStrideIn;
         YUVOut[COMPONENT_Cb] += iCStrideOut;
         YUVOut[COMPONENT_Cr] += iCStrideOut;
      }
    }
    //Write remapped picture in display order
    picYuvColourRemapped.dump( m_colourRemapSEIFileName, bitDepthsCriFile, true );
    picYuvColourRemapped.destroy();
  }
}

#if ALIGNED_BUMPING
// Function outputs a picture, and marks it as not needed for output.
Void TAppDecTop::xOutputAndMarkPic( TComPic *pic, std::string& reconFileName, const Int layerId, Int &pocLastDisplay, DpbStatus &dpbStatus )
{
  if( !reconFileName.empty() )
  {
    const Window &conf = pic->getConformanceWindow();
    const Window &defDisp = m_respectDefDispWindow ? pic->getDefDisplayWindow() : Window();
    Int xScal =  1, yScal = 1;

    UInt chromaFormatIdc = pic->getSlice(0)->getSPS()->getChromaFormatIdc();
    xScal = TComSPS::getWinUnitX( chromaFormatIdc );
    yScal = TComSPS::getWinUnitY( chromaFormatIdc );

    TComPicYuv* pPicCYuvRec = pic->getPicYuvRec();
    m_apcTVideoIOYuvReconFile[layerId]->write( pPicCYuvRec, m_outputColourSpaceConvert,
      conf.getWindowLeftOffset()  * xScal + defDisp.getWindowLeftOffset(),
      conf.getWindowRightOffset() * xScal + defDisp.getWindowRightOffset(),
      conf.getWindowTopOffset()   * yScal + defDisp.getWindowTopOffset(),
      conf.getWindowBottomOffset()* yScal + defDisp.getWindowBottomOffset() );

    if( !m_colourRemapSEIFileName.empty() )
    {
      xOutputColourRemapPic(pic);
    }
  }
  // update POC of display order
  pocLastDisplay = pic->getPOC();

  // Mark as not needed for output
  pic->setOutputMark(false);

  // "erase" non-referenced picture in the reference picture list after display
  if ( !pic->getSlice(0)->isReferenced() && pic->getReconMark() == true )
  {
    pic->setReconMark(false);

    // mark it should be extended later
    pic->getPicYuvRec()->setBorderExtension( false );

    dpbStatus.m_numPicsInSubDpb[dpbStatus.m_layerIdToSubDpbIdMap[layerId]]--;
  }
}

Void TAppDecTop::flushAllPictures(Int layerId, Bool outputPictures)
{
  // First "empty" all pictures that are not used for reference and not needed for output
  emptyUnusedPicturesNotNeededForOutput();

  if( outputPictures )  // All pictures in the DPB in that layer are to be output; this means other pictures would also be output
  {
    std::vector<Int>  listOfPocs;
    std::vector<Int>  listOfPocsInEachLayer[MAX_VPS_LAYER_IDX_PLUS1];
    std::vector<Int>  listOfPocsPositionInEachLayer[MAX_VPS_LAYER_IDX_PLUS1];

    DpbStatus dpbStatus;

    // Find the status of the DPB
    xFindDPBStatus(listOfPocs, listOfPocsInEachLayer, listOfPocsPositionInEachLayer, dpbStatus);

    if( listOfPocs.size() )
    {
      while( listOfPocsInEachLayer[layerId].size() )    // As long as there picture in the layer to be output
      {
        bumpingProcess( listOfPocs, listOfPocsInEachLayer, listOfPocsPositionInEachLayer, dpbStatus );
      }
    }
  }

  // Now remove all pictures from the layer DPB?
  markAllPicturesAsErased(layerId);
}

Void TAppDecTop::flushAllPictures(Bool outputPictures)
{
  // First "empty" all pictures that are not used for reference and not needed for output
  emptyUnusedPicturesNotNeededForOutput();

  if( outputPictures )  // All pictures in the DPB are to be output
  {
    std::vector<Int>  listOfPocs;
    std::vector<Int>  listOfPocsInEachLayer[MAX_VPS_LAYER_IDX_PLUS1];
    std::vector<Int>  listOfPocsPositionInEachLayer[MAX_VPS_LAYER_IDX_PLUS1];

    DpbStatus dpbStatus;

    // Find the status of the DPB
    xFindDPBStatus(listOfPocs, listOfPocsInEachLayer, listOfPocsPositionInEachLayer, dpbStatus, false);

    while( dpbStatus.m_numAUsNotDisplayed )
    {
      bumpingProcess( listOfPocs, listOfPocsInEachLayer, listOfPocsPositionInEachLayer, dpbStatus );
    }
  }

  // Now remove all pictures from the DPB?
  markAllPicturesAsErased();
}

Void TAppDecTop::markAllPicturesAsErased()
{
  for(Int i = 0; i < MAX_VPS_LAYER_IDX_PLUS1; i++)
  {
    markAllPicturesAsErased(i);
  }
}

Void TAppDecTop::markAllPicturesAsErased(Int layerIdx)
{
  TComList<TComPic*>::iterator  iterPic = m_apcTDecTop[layerIdx]->getListPic()->begin();
  Int iSize = Int( m_apcTDecTop[layerIdx]->getListPic()->size() );
  
  for (Int i = 0; i < iSize; i++ )
  {
    TComPic* pcPic = *(iterPic++);

    if( pcPic )
    {
      pcPic->destroy();

      // pcPic is statically created for the external (AVC) base layer, no need to delete it
      if( !m_apcTDecTop[layerIdx]->getParameterSetManager()->getActiveVPS()->getNonHEVCBaseLayerFlag() || layerIdx )
      {
        delete pcPic;
        pcPic = NULL;
      }
    }
  }

  m_apcTDecTop[layerIdx]->getListPic()->clear();
}

Void TAppDecTop::checkOutputBeforeDecoding(Int layerIdx)
{    
	//输出到yuv文件;
  std::vector<Int>  listOfPocs;
  std::vector<Int>  listOfPocsInEachLayer[MAX_VPS_LAYER_IDX_PLUS1];
  std::vector<Int>  listOfPocsPositionInEachLayer[MAX_VPS_LAYER_IDX_PLUS1];

  DpbStatus dpbStatus;

  // First "empty" all pictures that are not used for reference and not needed for output
  emptyUnusedPicturesNotNeededForOutput();

  // Find the status of the DPB
  xFindDPBStatus(listOfPocs, listOfPocsInEachLayer, listOfPocsPositionInEachLayer, dpbStatus);

  // If not picture to be output, return
  if( listOfPocs.size() == 0 )
  {
    return;
  }

  // Find DPB-information from the VPS
  DpbStatus maxDpbLimit;

  Int subDpbIdx = getCommonDecoderParams()->getTargetOutputLayerSetIdx() == 0 ? dpbStatus.m_layerIdToSubDpbIdMap[0] : dpbStatus.m_layerIdToSubDpbIdMap[layerIdx];

  findDpbParametersFromVps(listOfPocs, listOfPocsInEachLayer, listOfPocsPositionInEachLayer, maxDpbLimit);

  // Assume that listOfPocs is sorted in increasing order - if not have to sort it.
  while( ifInvokeBumpingBeforeDecoding(dpbStatus, maxDpbLimit, layerIdx, subDpbIdx) )
  {
    bumpingProcess( listOfPocs, listOfPocsInEachLayer, listOfPocsPositionInEachLayer, dpbStatus );
  }  
}

Void TAppDecTop::checkOutputAfterDecoding()
{    
  std::vector<Int>  listOfPocs;
  std::vector<Int>  listOfPocsInEachLayer[MAX_VPS_LAYER_IDX_PLUS1];
  std::vector<Int>  listOfPocsPositionInEachLayer[MAX_VPS_LAYER_IDX_PLUS1];

  DpbStatus dpbStatus;

  // First "empty" all pictures that are not used for reference and not needed for output
  emptyUnusedPicturesNotNeededForOutput();

  // Find the status of the DPB
  xFindDPBStatus(listOfPocs, listOfPocsInEachLayer, listOfPocsPositionInEachLayer, dpbStatus);

  // If not picture to be output, return
  if( listOfPocs.size() == 0 )
  {
    return;
  }

  // Find DPB-information from the VPS
  DpbStatus maxDpbLimit;
  findDpbParametersFromVps(listOfPocs, listOfPocsInEachLayer, listOfPocsPositionInEachLayer, maxDpbLimit);

  // Assume that listOfPocs is sorted in increasing order - if not have to sort it.
  while( ifInvokeBumpingAfterDecoding(dpbStatus, maxDpbLimit) )
  {
    bumpingProcess( listOfPocs, listOfPocsInEachLayer, listOfPocsPositionInEachLayer, dpbStatus );
  }  
}

Void TAppDecTop::bumpingProcess(std::vector<Int> &listOfPocs, std::vector<Int> *listOfPocsInEachLayer, std::vector<Int> *listOfPocsPositionInEachLayer, DpbStatus &dpbStatus)
{
  // Choose the smallest POC value
  Int pocValue = *(listOfPocs.begin());
  std::vector<int>::iterator it;
  TComList<TComPic*>::iterator iterPic;

  for( Int dpbLayerCtr = 0; dpbLayerCtr < dpbStatus.m_numLayers; dpbLayerCtr++)
  {
    Int layerId  = dpbStatus.m_targetDecLayerIdList[dpbLayerCtr];

    // Check if picture with pocValue is present.
    it = find( listOfPocsInEachLayer[layerId].begin(), listOfPocsInEachLayer[layerId].end(), pocValue );
    if( it != listOfPocsInEachLayer[layerId].end() )  // picture found.
    {
      Int picPosition = (Int)std::distance( listOfPocsInEachLayer[layerId].begin(), it );
      Int j;
      for(j = 0, iterPic = m_apcTDecTop[layerId]->getListPic()->begin(); j < listOfPocsPositionInEachLayer[layerId][picPosition]; j++) // Picture to be output
      {
        iterPic++;
      }
      TComPic *pic = *iterPic;

      xOutputAndMarkPic( pic, m_reconFileName[layerId], layerId, m_aiPOCLastDisplay[layerId], dpbStatus );

#if CONFORMANCE_BITSTREAM_MODE
      FILE *fptr;
      if( m_confModeFlag )
      {
        if( m_metadataFileRefresh )
        {
          fptr = fopen( this->getMetadataFileName().c_str(), "w" );
          fprintf(fptr, " LayerId      POC    MD5\n");
          fprintf(fptr, "------------------------\n");
        }
        else
        {
          fptr = fopen( this->getMetadataFileName().c_str(), "a+" );
        }
        this->setMetadataFileRefresh(false);

        TComPictureHash recon_digest;
        Int numChar = calcMD5(*pic->getPicYuvRec(), recon_digest, pic->getSlice(0)->getSPS()->getBitDepths());
        fprintf(fptr, "%8d%9d    MD5:%s\n", pic->getLayerId(), pic->getSlice(0)->getPOC(), hashToString(recon_digest, numChar).c_str());
        fclose(fptr);
      }
#endif

      listOfPocsInEachLayer[layerId].erase( it );
      listOfPocsPositionInEachLayer[layerId].erase( listOfPocsPositionInEachLayer[layerId].begin() + picPosition );
      dpbStatus.m_numPicsInSubDpb[dpbStatus.m_layerIdToSubDpbIdMap[layerId]]--;
    }
  }

  dpbStatus.m_numAUsNotDisplayed--;

#if CONFORMANCE_BITSTREAM_MODE
  if( m_confModeFlag )
  {
    for( Int dpbLayerCtr = 0; dpbLayerCtr < dpbStatus.m_numLayers; dpbLayerCtr++)
    {
      Int layerId = dpbStatus.m_targetDecLayerIdList[dpbLayerCtr];
      // Output all picutres "decoded" in that layer that have POC less than the current picture
      std::vector<TComPic> *layerBuffer = m_apcTDecTop[layerId]->getConfListPic();
      // Write all pictures to the file.
      if( this->getDecodedYuvLayerRefresh(layerId) && m_apcTDecTop[layerId]->getParameterSetManager()->getActiveSPS())
      {
        char tempFileName[256];
        strcpy(tempFileName, this->getDecodedYuvLayerFileName( layerId ).c_str());

        const TComSPS *sps = m_apcTDecTop[layerId]->getParameterSetManager()->getActiveSPS();
        const BitDepths &bitDpeths = sps->getBitDepths();
        Int bitDepth[] = {bitDpeths.recon[CHANNEL_TYPE_LUMA], bitDpeths.recon[CHANNEL_TYPE_CHROMA]};

        m_confReconFile[layerId].open(tempFileName, true, bitDepth, bitDepth, bitDepth ); // write mode
        this->setDecodedYuvLayerRefresh( layerId, false );
      }

      std::vector<TComPic>::iterator itPic;
      for(itPic = layerBuffer->begin(); itPic != layerBuffer->end(); itPic++)
      {
        TComPic checkPic = *itPic;
        const Window &conf = checkPic.getConformanceWindow();
        const Window &defDisp = m_respectDefDispWindow ? checkPic.getDefDisplayWindow() : Window();
        Int xScal = 1, yScal = 1;
  
        UInt chromaFormatIdc = checkPic.getSlice(0)->getSPS()->getChromaFormatIdc();
        xScal = TComSPS::getWinUnitX( chromaFormatIdc );
        yScal = TComSPS::getWinUnitY( chromaFormatIdc );
  
        if( checkPic.getPOC() <= pocValue )
        {
          TComPicYuv* pPicCYuvRec = checkPic.getPicYuvRec();
          m_confReconFile[layerId].write( pPicCYuvRec, m_outputColourSpaceConvert,
            conf.getWindowLeftOffset()  * xScal + defDisp.getWindowLeftOffset(),
            conf.getWindowRightOffset() * xScal + defDisp.getWindowRightOffset(),
            conf.getWindowTopOffset()   * yScal + defDisp.getWindowTopOffset(),
            conf.getWindowBottomOffset()* yScal + defDisp.getWindowBottomOffset(),
            NUM_CHROMA_FORMAT, m_bClipOutputVideoToRec709Range );

          layerBuffer->erase(itPic);
          itPic = layerBuffer->begin();  // Ensure doesn't go to infinite loop
          if(layerBuffer->size() == 0)
          {
            break;
          }
        }
      }
    }
  }
#endif

  // Remove the picture from the listOfPocs
  listOfPocs.erase( listOfPocs.begin() );
}

const TComVPS *TAppDecTop::findDpbParametersFromVps(std::vector<Int> const &listOfPocs, std::vector<Int> const *listOfPocsInEachLayer, std::vector<Int> const *listOfPocsPositionInEachLayer, DpbStatus &maxDpbLimit)
{
  Int targetOutputLsIdx = getCommonDecoderParams()->getTargetOutputLayerSetIdx();
  const TComVPS *vps = NULL;

  if( targetOutputLsIdx == 0 )   // Only base layer is output
  {
    const TComSPS *sps = NULL;
    assert( listOfPocsInEachLayer[0].size() != 0 );
    TComList<TComPic*>::iterator iterPic;
    Int j;
    for(j = 0, iterPic = m_apcTDecTop[0]->getListPic()->begin(); j < listOfPocsPositionInEachLayer[0][0]; j++) // Picture to be output
    {
      iterPic++;
    }
    TComPic *pic = *iterPic;
    sps = pic->getSlice(0)->getSPS();   assert( sps->getLayerId() == 0 );
    vps = pic->getSlice(0)->getVPS();
    Int highestTId = sps->getMaxTLayers() - 1;

    maxDpbLimit.m_numAUsNotDisplayed = sps->getNumReorderPics( highestTId ); // m_numAUsNotDisplayed is only variable name - stores reorderpics
    maxDpbLimit.m_maxLatencyIncrease = sps->getMaxLatencyIncreasePlus1( highestTId ) > 0;
    if( maxDpbLimit.m_maxLatencyIncrease )
    {
      maxDpbLimit.m_maxLatencyPictures = sps->getMaxLatencyIncreasePlus1( highestTId ) + sps->getNumReorderPics( highestTId ) - 1;
    }
    maxDpbLimit.m_numPicsInSubDpb[0] = sps->getMaxDecPicBuffering( highestTId );
  }
  else
  {
    // -------------------------------------
    // Find the VPS used for the pictures
    // -------------------------------------
    for( Int i = 0; i < MAX_VPS_LAYER_IDX_PLUS1; i++ )
    {
      if( m_apcTDecTop[i]->getListPic()->empty() )
      {
        assert( listOfPocsInEachLayer[i].size() == 0 );
        continue;
      }

      std::vector<Int>::const_iterator it;
      it = find( listOfPocsInEachLayer[i].begin(), listOfPocsInEachLayer[i].end(), listOfPocs[0] );
      TComList<TComPic*>::iterator iterPic;

      if( it != listOfPocsInEachLayer[i].end() )
      {
        Int picPosition = (Int)std::distance( listOfPocsInEachLayer[i].begin(), it );
        Int j;

        // Picture to be output
        for( j = 0, iterPic = m_apcTDecTop[i]->getListPic()->begin(); j < listOfPocsPositionInEachLayer[i][picPosition]; j++ )
        {
          iterPic++;
        }

        TComPic *pic = *iterPic;
        vps = pic->getSlice(0)->getVPS();
        break;
      }
    }

    Int targetLsIdx       = vps->getOutputLayerSetIdx( getCommonDecoderParams()->getTargetOutputLayerSetIdx() );
    Int highestTId = vps->getMaxTLayers() - 1;

    maxDpbLimit.m_numAUsNotDisplayed = vps->getMaxVpsNumReorderPics( targetOutputLsIdx, highestTId ); // m_numAUsNotDisplayed is only variable name - stores reorderpics
    maxDpbLimit.m_maxLatencyIncrease  = vps->getMaxVpsLatencyIncreasePlus1(targetOutputLsIdx, highestTId ) > 0;

    if( maxDpbLimit.m_maxLatencyIncrease )
    {
      maxDpbLimit.m_maxLatencyPictures = vps->getMaxVpsNumReorderPics( targetOutputLsIdx, highestTId ) + vps->getMaxVpsLatencyIncreasePlus1(targetOutputLsIdx, highestTId ) - 1;
    }

    for(Int i = 0; i < vps->getNumLayersInIdList( targetLsIdx ); i++)
    {
      maxDpbLimit.m_numPicsInSubDpb[i] = vps->getMaxVpsDecPicBufferingMinus1( targetOutputLsIdx, i, highestTId) + 1;
    }
    // -------------------------------------
  }
  return vps;
}

Void TAppDecTop::emptyUnusedPicturesNotNeededForOutput()
{
  for( Int layerIdx = 0; layerIdx < MAX_VPS_LAYER_IDX_PLUS1; layerIdx++ )
  {
    TComList <TComPic*> *pcListPic = m_apcTDecTop[layerIdx]->getListPic();
    TComList<TComPic*>::iterator iterPic = pcListPic->begin();
    while ( iterPic != pcListPic->end() )
    {
      TComPic *pic = *iterPic;

      assert( pic->getPicSym() );

      if( !pic->getSlice(0)->isReferenced() && !pic->getOutputMark() )
      {
        // Emtpy the picture buffer
        pic->setReconMark( false );
      }
      iterPic++;
    }
  }
}

Bool TAppDecTop::ifInvokeBumpingBeforeDecoding( const DpbStatus &dpbStatus, const DpbStatus &dpbLimit, const Int layerIdx, const Int subDpbIdx )
{
  Bool retVal = false;
  // Number of reorder picutres
  retVal |= ( dpbStatus.m_numAUsNotDisplayed > dpbLimit.m_numAUsNotDisplayed );

  // Number of pictures in each sub-DPB
  retVal |= ( dpbStatus.m_numPicsInSubDpb[subDpbIdx] >= dpbLimit.m_numPicsInSubDpb[subDpbIdx] );

  return retVal;
}

Bool TAppDecTop::ifInvokeBumpingAfterDecoding( const DpbStatus &dpbStatus, const DpbStatus &dpbLimit )
{
  Bool retVal = false;

  // Number of reorder picutres
  retVal |= ( dpbStatus.m_numAUsNotDisplayed > dpbLimit.m_numAUsNotDisplayed );

  return retVal;
}

Void TAppDecTop::xFindDPBStatus( std::vector<Int> &listOfPocs
                            , std::vector<Int> *listOfPocsInEachLayer
                            , std::vector<Int> *listOfPocsPositionInEachLayer
                            , DpbStatus &dpbStatus
                            , Bool notOutputCurrAu
                            )
{
  const TComVPS *vps = NULL;
  dpbStatus.init();

  for( Int i = 0; i < MAX_VPS_LAYER_IDX_PLUS1; i++ )
  {
    if( m_apcTDecTop[i]->getListPic()->empty() )
    {
      continue;
    }
    
    // To check # AUs that have at least one picture not output,
    // For each layer, populate listOfPOcs if not already present
    TComList<TComPic*>::iterator iterPic = m_apcTDecTop[i]->getListPic()->begin();
    Int picPositionInList = 0;
    while (iterPic != m_apcTDecTop[i]->getListPic()->end())
    {
      TComPic* pic = *(iterPic);
      if( pic->getReconMark() )
      {
        if( vps == NULL )
        {
          vps = m_apcTDecTop[i]->getParameterSetManager()->getActiveVPS();
        }

        if( !(pic->isCurrAu() && notOutputCurrAu ) )
        {
          std::vector<Int>::iterator it;
          if( pic->getOutputMark() ) // && pic->getPOC() > m_aiPOCLastDisplay[i])
          {
            it = find( listOfPocs.begin(), listOfPocs.end(), pic->getPOC() ); // Check if already included

            if( it == listOfPocs.end() )  // New POC value - i.e. new AU - add to the list
            {
              listOfPocs.push_back( pic->getPOC() );
            }
            listOfPocsInEachLayer         [i].push_back( pic->getPOC()    );    // POC to be output in each layer
            listOfPocsPositionInEachLayer [i].push_back( picPositionInList  );  // For ease of access
          }

          if( pic->getSlice(0)->isReferenced() || pic->getOutputMark() )
          {
            dpbStatus.m_numPicsInSubDpb[i]++;  // Count pictures that are "used for reference" or "needed for output"
          }
        }
      }

      iterPic++;
      picPositionInList++;
    }
  }

#if CONFORMANCE_BITSTREAM_FIX
  if (!vps) return;
#else 
  assert( vps != NULL );    // No picture in any DPB?
#endif
  std::sort( listOfPocs.begin(), listOfPocs.end() );    // Sort in increasing order of POC
  Int targetLsIdx = vps->getOutputLayerSetIdx( getCommonDecoderParams()->getTargetOutputLayerSetIdx() );

  // Update status
  dpbStatus.m_numAUsNotDisplayed = (Int)listOfPocs.size();   // Number of AUs not displayed
  dpbStatus.m_numLayers = vps->getNumLayersInIdList( targetLsIdx );

  for( Int i = 0; i < dpbStatus.m_numLayers; i++ )
  {
    dpbStatus.m_layerIdToSubDpbIdMap[vps->getLayerSetLayerIdList(targetLsIdx, i)] = i;
    dpbStatus.m_targetDecLayerIdList[i] = vps->getLayerSetLayerIdList(targetLsIdx, i);  // Layer Id stored in a particular sub-DPB
  }
  dpbStatus.m_numSubDpbs = vps->getNumSubDpbs( targetLsIdx ); 

  for( Int i = 0; i < MAX_VPS_LAYER_IDX_PLUS1; i++ )
  {
    dpbStatus.m_numPicsNotDisplayedInLayer[i] = (Int)listOfPocsInEachLayer[i].size();
  }
  assert( dpbStatus.m_numAUsNotDisplayed != -1 );
}  

Void TAppDecTop::outputAllPictures(Int layerId, Bool notOutputCurrPic)
{
  { // All pictures in the DPB in that layer are to be output; this means other pictures would also be output
    std::vector<Int>  listOfPocs;
    std::vector<Int>  listOfPocsInEachLayer[MAX_VPS_LAYER_IDX_PLUS1];
    std::vector<Int>  listOfPocsPositionInEachLayer[MAX_VPS_LAYER_IDX_PLUS1];

    DpbStatus dpbStatus;

    // Find the status of the DPB
    xFindDPBStatus(listOfPocs, listOfPocsInEachLayer, listOfPocsPositionInEachLayer, dpbStatus, notOutputCurrPic);

    if( listOfPocs.size() )
    {
      while( listOfPocsInEachLayer[layerId].size() )    // As long as there picture in the layer to be output
      {
        bumpingProcess( listOfPocs, listOfPocsInEachLayer, listOfPocsPositionInEachLayer, dpbStatus );
      }
    }
  }
}
#endif //ALIGNED_BUMPING

//! \}
