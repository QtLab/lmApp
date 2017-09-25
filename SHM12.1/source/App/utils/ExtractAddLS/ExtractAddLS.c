/* The copyright in this software is being made available under the BSD
* License, included below. This software may be subject to other third party
* and contributor rights, including patent rights, and no such rights are
* granted under this license.
*
* Copyright (c) 2010-2015, ITU/ISO/IEC
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


enum NalUnitType
{
  NAL_UNIT_CODED_SLICE_TRAIL_N = 0,
  NAL_UNIT_CODED_SLICE_TRAIL_R,

  NAL_UNIT_CODED_SLICE_TSA_N,
  NAL_UNIT_CODED_SLICE_TSA_R,

  NAL_UNIT_CODED_SLICE_STSA_N,
  NAL_UNIT_CODED_SLICE_STSA_R,

  NAL_UNIT_CODED_SLICE_RADL_N,
  NAL_UNIT_CODED_SLICE_RADL_R,

  NAL_UNIT_CODED_SLICE_RASL_N,
  NAL_UNIT_CODED_SLICE_RASL_R,

  NAL_UNIT_RESERVED_VCL_N10,
  NAL_UNIT_RESERVED_VCL_R11,
  NAL_UNIT_RESERVED_VCL_N12,
  NAL_UNIT_RESERVED_VCL_R13,
  NAL_UNIT_RESERVED_VCL_N14,
  NAL_UNIT_RESERVED_VCL_R15,

  NAL_UNIT_CODED_SLICE_BLA_W_LP,
  NAL_UNIT_CODED_SLICE_BLA_W_RADL,
  NAL_UNIT_CODED_SLICE_BLA_N_LP,
  NAL_UNIT_CODED_SLICE_IDR_W_RADL,
  NAL_UNIT_CODED_SLICE_IDR_N_LP,
  NAL_UNIT_CODED_SLICE_CRA,
  NAL_UNIT_RESERVED_IRAP_VCL22,
  NAL_UNIT_RESERVED_IRAP_VCL23,

  NAL_UNIT_RESERVED_VCL24,
  NAL_UNIT_RESERVED_VCL25,
  NAL_UNIT_RESERVED_VCL26,
  NAL_UNIT_RESERVED_VCL27,
  NAL_UNIT_RESERVED_VCL28,
  NAL_UNIT_RESERVED_VCL29,
  NAL_UNIT_RESERVED_VCL30,
  NAL_UNIT_RESERVED_VCL31,

  NAL_UNIT_VPS,
  NAL_UNIT_SPS,
  NAL_UNIT_PPS,
  NAL_UNIT_ACCESS_UNIT_DELIMITER,
  NAL_UNIT_EOS,
  NAL_UNIT_EOB,
  NAL_UNIT_FILLER_DATA,
  NAL_UNIT_PREFIX_SEI,
  NAL_UNIT_SUFFIX_SEI,
  NAL_UNIT_RESERVED_NVCL41,
  NAL_UNIT_RESERVED_NVCL42,
  NAL_UNIT_RESERVED_NVCL43,
  NAL_UNIT_RESERVED_NVCL44,
  NAL_UNIT_RESERVED_NVCL45,
  NAL_UNIT_RESERVED_NVCL46,
  NAL_UNIT_RESERVED_NVCL47,
  NAL_UNIT_UNSPECIFIED_48,
  NAL_UNIT_UNSPECIFIED_49,
  NAL_UNIT_UNSPECIFIED_50,
  NAL_UNIT_UNSPECIFIED_51,
  NAL_UNIT_UNSPECIFIED_52,
  NAL_UNIT_UNSPECIFIED_53,
  NAL_UNIT_UNSPECIFIED_54,
  NAL_UNIT_UNSPECIFIED_55,
  NAL_UNIT_UNSPECIFIED_56,
  NAL_UNIT_UNSPECIFIED_57,
  NAL_UNIT_UNSPECIFIED_58,
  NAL_UNIT_UNSPECIFIED_59,
  NAL_UNIT_UNSPECIFIED_60,
  NAL_UNIT_UNSPECIFIED_61,
  NAL_UNIT_UNSPECIFIED_62,
  NAL_UNIT_UNSPECIFIED_63,
  NAL_UNIT_INVALID,
};

char *nalUnitNames[] =
{
  "NAL_UNIT_CODED_SLICE_TRAIL_N",
  "NAL_UNIT_CODED_SLICE_TRAIL_R",

  "NAL_UNIT_CODED_SLICE_TSA_N",
  "NAL_UNIT_CODED_SLICE_TSA_R",

  "NAL_UNIT_CODED_SLICE_STSA_N",
  "NAL_UNIT_CODED_SLICE_STSA_R",

  "NAL_UNIT_CODED_SLICE_RADL_N",
  "NAL_UNIT_CODED_SLICE_RADL_R",

  "NAL_UNIT_CODED_SLICE_RASL_N",
  "NAL_UNIT_CODED_SLICE_RASL_R",

  "NAL_UNIT_RESERVED_VCL_N10",
  "NAL_UNIT_RESERVED_VCL_R11",
  "NAL_UNIT_RESERVED_VCL_N12",
  "NAL_UNIT_RESERVED_VCL_R13",
  "NAL_UNIT_RESERVED_VCL_N14",
  "NAL_UNIT_RESERVED_VCL_R15",

  "NAL_UNIT_CODED_SLICE_BLA_W_LP",
  "NAL_UNIT_CODED_SLICE_BLA_W_RADL",
  "NAL_UNIT_CODED_SLICE_BLA_N_LP",
  "NAL_UNIT_CODED_SLICE_IDR_W_RADL",
  "NAL_UNIT_CODED_SLICE_IDR_N_LP",
  "NAL_UNIT_CODED_SLICE_CRA",
  "NAL_UNIT_RESERVED_IRAP_VCL22",
  "NAL_UNIT_RESERVED_IRAP_VCL23",

  "NAL_UNIT_RESERVED_VCL24",
  "NAL_UNIT_RESERVED_VCL25",
  "NAL_UNIT_RESERVED_VCL26",
  "NAL_UNIT_RESERVED_VCL27",
  "NAL_UNIT_RESERVED_VCL28",
  "NAL_UNIT_RESERVED_VCL29",
  "NAL_UNIT_RESERVED_VCL30",
  "NAL_UNIT_RESERVED_VCL31",

  "NAL_UNIT_VPS",
  "NAL_UNIT_SPS",
  "NAL_UNIT_PPS",
  "NAL_UNIT_ACCESS_UNIT_DELIMITER",
  "NAL_UNIT_EOS",
  "NAL_UNIT_EOB",
  "NAL_UNIT_FILLER_DATA",
  "NAL_UNIT_PREFIX_SEI",
  "NAL_UNIT_SUFFIX_SEI",
  "NAL_UNIT_RESERVED_NVCL41",
  "NAL_UNIT_RESERVED_NVCL42",
  "NAL_UNIT_RESERVED_NVCL43",
  "NAL_UNIT_RESERVED_NVCL44",
  "NAL_UNIT_RESERVED_NVCL45",
  "NAL_UNIT_RESERVED_NVCL46",
  "NAL_UNIT_RESERVED_NVCL47",
  "NAL_UNIT_UNSPECIFIED_48",
  "NAL_UNIT_UNSPECIFIED_49",
  "NAL_UNIT_UNSPECIFIED_50",
  "NAL_UNIT_UNSPECIFIED_51",
  "NAL_UNIT_UNSPECIFIED_52",
  "NAL_UNIT_UNSPECIFIED_53",
  "NAL_UNIT_UNSPECIFIED_54",
  "NAL_UNIT_UNSPECIFIED_55",
  "NAL_UNIT_UNSPECIFIED_56",
  "NAL_UNIT_UNSPECIFIED_57",
  "NAL_UNIT_UNSPECIFIED_58",
  "NAL_UNIT_UNSPECIFIED_59",
  "NAL_UNIT_UNSPECIFIED_60",
  "NAL_UNIT_UNSPECIFIED_61",
  "NAL_UNIT_UNSPECIFIED_62",
  "NAL_UNIT_UNSPECIFIED_63"
};

typedef struct NalUnitHeader_s
{
  int nalUnitType;
  int nuhLayerId;
  int nuhTemporalIdPlus1;
} NalUnitHeader;


int findStartCodePrefix(FILE *inFile, int *numStartCodeZeros)
{
  int numZeros = 0;
  int currByte;
  
  for (;;)
  {
    currByte = fgetc(inFile);
    if (currByte == EOF)
    {
      return 0;
    }

    if (currByte == 0x01 && numZeros > 1)
    {
      *numStartCodeZeros = numZeros;
      return 1;
    }
    else if (currByte == 0)
    {
      numZeros++;
    }
    else
    {
      numZeros = 0;
    }
  }
}

int parseNalUnitHeader(FILE *inFile, NalUnitHeader *nalu)
{
  int byte0, byte1;

  byte0 = fgetc(inFile);
  byte1 = fgetc(inFile);

  if (byte0 == EOF || byte1 == EOF)
  {
    return 0;
  }

  nalu->nalUnitType = (byte0 >> 1) & 0x3f;
  nalu->nuhLayerId  = (((byte0 << 8) | byte1) >> 3) & 0x3f;
  nalu->nuhTemporalIdPlus1 = byte1 & 0x07;

  return 1;
}

void writeStartCodePrefixAndNUH(FILE *outFile, int numStartCodeZeros, NalUnitHeader *nalu)
{
  int byte0, byte1;
  int i;

  /* Start code prefix */
  if (numStartCodeZeros > 3)
  {
    numStartCodeZeros = 3;
  }
  for (i = 0; i < numStartCodeZeros; i++)
  {
    fputc(0, outFile);
  }
  fputc(0x01, outFile);

  /* NAL unit header */
  byte0 = ((nalu->nalUnitType << 6) | nalu->nuhLayerId) >> 5;
  byte1 = ((nalu->nuhLayerId << 3) | nalu->nuhTemporalIdPlus1) & 0xff;
  fputc(byte0, outFile);
  fputc(byte1, outFile);
}

int main(int argc, char **argv)
{
  FILE *inFile;
  FILE *outFile;
  int assignedBaseLayerId;
  int tIdTarget = 6;
  NalUnitHeader nalu;
  int numStartCodeZeros;
  int nalIsVpsSpsPpsEob;
  int nalIsVpsSpsPpsEos;
  int removeNal;
  int layerIdListTarget[8];
  int i;
  int layerIdx;
  int numLayerIds;
  char nalName[32];

  if (argc < 5 || argc > 10)
  {
    fprintf(stderr, "\n  Usage: ExtractAddLS <infile> <outfile> <max temporal ID> <list of layer IDs> \n\n");
    fprintf(stderr, "  If only one layer ID 0 is given, base layer extration process is performed\n");
    fprintf(stderr, "  If only one non-zero layer ID is given, independent non-base layer rewriting\n");
    fprintf(stderr, "  process is performed\n");
    fprintf(stderr, "  If more than one layer ID is given, sub-bitstream extraction process for\n");
    fprintf(stderr, "  additional layer sets is performed (Layer ID list should exactly match\n");
    fprintf(stderr, "  an additional layer set defined in VPS)\n");
    exit(1);
  }

  inFile = fopen(argv[1], "rb");
  if (inFile == NULL)
  {
    fprintf(stderr, "Cannot open input file %s\n", argv[1]);
    exit(1);
  }

  outFile = fopen(argv[2], "wb");
  if (outFile == NULL)
  {
    fprintf(stderr, "Cannot open output file %s\n", argv[2]);
    exit(1);
  }

  tIdTarget = atoi(argv[3]);
  if (tIdTarget < 0 || tIdTarget > 6)
  {
    fprintf(stderr, "Invalid maximum temporal ID (must be in range 0-6)\n");
    exit(1);
  }

  for (i = 4, layerIdx = 0; i < argc; i++, layerIdx++)
  {
    layerIdListTarget[layerIdx] = atoi(argv[i]);
    if (layerIdListTarget[layerIdx] < 0 || layerIdListTarget[layerIdx] > 7)
    {
      fprintf(stderr, "Invalid layer ID (must be in range 1-7)\n");
      exit(1);
    }
  }

  numLayerIds = layerIdx;
  assignedBaseLayerId = layerIdListTarget[0];

  /* Iterate through all NAL units */
  for (;;)
  {
    if (!findStartCodePrefix(inFile, &numStartCodeZeros))
    {
      break;
    }
    if (!parseNalUnitHeader(inFile, &nalu))
    {
      break;
    }

    if (numLayerIds == 1)
    {
      /* base layer or independent non-base layer bitstream extraction */

      /* temporary keep VPS so the extracted/rewriting bitstream can be decoded by HM */
      nalIsVpsSpsPpsEob = (nalu.nalUnitType == NAL_UNIT_VPS || nalu.nalUnitType == NAL_UNIT_SPS || nalu.nalUnitType == NAL_UNIT_PPS || nalu.nalUnitType == NAL_UNIT_EOB);
      removeNal = (!nalIsVpsSpsPpsEob && (nalu.nuhLayerId != assignedBaseLayerId))
                  || (nalIsVpsSpsPpsEob && (nalu.nuhLayerId != 0))
                  || ((nalu.nuhTemporalIdPlus1 - 1) > tIdTarget);
      nalu.nuhLayerId = 0;
    }
    else /* numLayerIds > 1 */
    {
      /* sub-bitstream extraction process for additional layer sets */

      int isTargetLayer = 0;

      for (i = 0; i < numLayerIds; i++)
      {
        if (layerIdListTarget[i] == nalu.nuhLayerId)
        {
          isTargetLayer = 1;
        }
      }
    
      nalIsVpsSpsPpsEos = (nalu.nalUnitType == NAL_UNIT_VPS || nalu.nalUnitType == NAL_UNIT_SPS || nalu.nalUnitType == NAL_UNIT_PPS || nalu.nalUnitType == NAL_UNIT_EOS);

      removeNal = (!(nalIsVpsSpsPpsEos || nalu.nalUnitType == NAL_UNIT_EOB) && !isTargetLayer)
                || (nalIsVpsSpsPpsEos && (nalu.nuhLayerId != 0) && !isTargetLayer)
                || ((nalu.nuhTemporalIdPlus1 - 1) > tIdTarget);
    }

    if (!removeNal)
    {
      /* Write current NAL unit to output bitstream */

      long naluBytesStartPos;
      long numNaluBytes;
      long i;

      printf("Keep  ");

      writeStartCodePrefixAndNUH(outFile, numStartCodeZeros, &nalu);

      naluBytesStartPos = ftell(inFile);
      /* Find beginning of the next NAL unit to calculate length of the current unit */
      if (findStartCodePrefix(inFile, &numStartCodeZeros))
      {
        numNaluBytes = ftell(inFile) - naluBytesStartPos - numStartCodeZeros - 1;
      }
      else
      {
        numNaluBytes = ftell(inFile) - naluBytesStartPos;
      }
      fseek(inFile, naluBytesStartPos, SEEK_SET);

      i = 0;

      if (numLayerIds == 1 && nalu.nalUnitType == NAL_UNIT_VPS)
      {
        char nalByte = fgetc(inFile);
        nalByte = nalByte | (0x0C);  // set vps_max_layers_minus1 bits(5-4) to 0 for HM decoder
        nalByte = nalByte & ~(0x03); // set vps_max_layers_minus1 bits(3-0) to 0 for HM decoder
        fputc(nalByte, outFile); 
        i++;
        nalByte = fgetc(inFile);
        nalByte = nalByte & ~(0xF0);
        fputc(nalByte, outFile); 
        i++;
      }
      else if (numLayerIds > 1 && nalu.nalUnitType == NAL_UNIT_VPS)
      {
        /* sub-bitstream extraction process for additional layer sets */
        int nalByte = fgetc(inFile);
        nalByte = nalByte & ~(0x04);  /* vps_base_layer_available_flag in each VPS is set equal to 0 */
        fputc(nalByte, outFile);
        i++;
      }

      for (; i < numNaluBytes; i++)
      {
        fputc(fgetc(inFile), outFile);
      }
    }
    else
    {
      printf("-     ");
    }

    strcpy(nalName, nalUnitNames[nalu.nalUnitType]);
    for (i = strlen(nalName); i < 31; i++)
    {
      nalName[i] = ' ';
    }
    nalName[31] = '\0';

    printf("%s  layer ID: %i  temporal ID: %i\n", nalName, nalu.nuhLayerId, nalu.nuhTemporalIdPlus1 - 1);
  }

  fclose(inFile);
  fclose(outFile);


  return 0;
}
