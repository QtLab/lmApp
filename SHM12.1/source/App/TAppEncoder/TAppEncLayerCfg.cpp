/** \file     TAppEncLayerCfg.cpp
\brief    Handle encoder configuration parameters
*/

#include <stdlib.h>
#include <cassert>
#include <cstring>
#include <string>
#include "TLibCommon/TComRom.h"
#include "TAppEncCfg.h"
#include "TAppEncLayerCfg.h"
#include "TAppCommon/program_options_lite.h"

#ifdef WIN32
#define strdup _strdup
#endif

using namespace std;
namespace po = df::program_options_lite;

//! \ingroup TAppEncoder
//! \{

// ====================================================================================================================
// Constructor / destructor / initialization / destroy
// ====================================================================================================================
#if SVC_EXTENSION
TAppEncLayerCfg::TAppEncLayerCfg()
: m_conformanceWindowMode(0)
, m_aidQP(NULL)
, m_repFormatIdx(-1)
{
  m_confWinLeft = m_confWinRight = m_confWinTop = m_confWinBottom = 0;
  m_aiPad[1] = m_aiPad[0] = 0;
  m_numRefLayerLocationOffsets = 0;
  ::memset(m_refLocationOffsetLayerId,   0, sizeof(m_refLocationOffsetLayerId));
  ::memset(m_scaledRefLayerLeftOffset,   0, sizeof(m_scaledRefLayerLeftOffset));
  ::memset(m_scaledRefLayerTopOffset,    0, sizeof(m_scaledRefLayerTopOffset));
  ::memset(m_scaledRefLayerRightOffset,  0, sizeof(m_scaledRefLayerRightOffset));
  ::memset(m_scaledRefLayerBottomOffset, 0, sizeof(m_scaledRefLayerBottomOffset));
  ::memset(m_scaledRefLayerOffsetPresentFlag, 0, sizeof(m_scaledRefLayerOffsetPresentFlag));
  ::memset(m_refRegionOffsetPresentFlag, 0, sizeof(m_refRegionOffsetPresentFlag));
  ::memset(m_refRegionLeftOffset,   0, sizeof(m_refRegionLeftOffset));
  ::memset(m_refRegionTopOffset,    0, sizeof(m_refRegionTopOffset));
  ::memset(m_refRegionRightOffset,  0, sizeof(m_refRegionRightOffset));
  ::memset(m_refRegionBottomOffset, 0, sizeof(m_refRegionBottomOffset));
  ::memset(m_resamplePhaseSetPresentFlag, 0, sizeof(m_resamplePhaseSetPresentFlag));
  ::memset(m_phaseHorLuma,   0, sizeof(m_phaseHorLuma));
  ::memset(m_phaseVerLuma,   0, sizeof(m_phaseVerLuma));
  ::memset(m_phaseHorChroma, 0, sizeof(m_phaseHorChroma));
  ::memset(m_phaseVerChroma, 0, sizeof(m_phaseVerChroma));
#if SCALABLE_REXT
  // variables uninitialized otherwise
  m_intraConstraintFlag = false;
  m_lowerBitRateConstraintFlag = false;
  m_onePictureOnlyConstraintFlag = false; 
#endif
}

TAppEncLayerCfg::~TAppEncLayerCfg()
{
  if( m_numSamplePredRefLayers > 0 )
  {
    delete [] m_samplePredRefLayerIds;
    m_samplePredRefLayerIds = NULL;
  }

  if( m_numMotionPredRefLayers > 0 )
  {
    delete [] m_motionPredRefLayerIds;
    m_motionPredRefLayerIds = NULL;
  }

  if( m_numActiveRefLayers > 0 )
  {
    delete [] m_predLayerIds;
    m_predLayerIds = NULL;
  }

  if( m_aidQP )
  {
    delete [] m_aidQP;
  }
}

#endif //SVC_EXTENSION


//! \}
