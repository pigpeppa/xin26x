/***************************************************************************//**
*
* @file          xin_encoder_option.c
* @authors       shennung
* @brief         xin26x encoder configuration parse related subroutines.
* @copyright     (c) 2020, shennung <shennung@hotmail.com>  All rights reserved
*
*******************************************************************************/
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include "getopt.h"
#include "xin_typedef.h"
#include "xin_encoder_option.h"
#include "xin_config_file.h"
#include "xin26x_encoder_api.h"

static const char encoder_short_options[] = "c:i:o:n:R:B:I:w:h:f:d:y:b:a:t:u:e:E:m:s:r:q:O:p:D:P:L:T:W:F:HV";
static const struct option encoder_long_options[] =
{
    { "config",         required_argument, 0, 'c' },
    { "input",          required_argument, 0, 'i' },
    { "output",         required_argument, 0, 'o' },
    { "algmode",        required_argument, 0, 'a' },
    { "framenumber",    required_argument, 0, 'n' },
    { "recon",          required_argument, 0, 'R' },
    { "width",          required_argument, 0, 'w' },
    { "height",         required_argument, 0, 'h' },
    { "framerate",      required_argument, 0, 'f' },
    { "bitrate",        required_argument, 0, 'b' },
    { "temporallayer",  required_argument, 0, 't' },
    { "multithread",    required_argument, 0, 'm' },
    { "screencontent",  required_argument, 0, 's' },
    { "transformskip",  required_argument, 0, 'A' },
    { "preset",         required_argument, 0, 'p' },
    { "cclm",           required_argument, 0, 'C' },
    { "dmvr",           required_argument, 0, 'y' },
    { "wpp",            required_argument, 0, 'W' },
    { "fpp",            required_argument, 0, 'F' },
    { "bframes",        required_argument, 0, 'B' },
    { "intraperiod",    required_argument, 0, 'I' },
    { "intranxn",       required_argument, 0, 'N' },
    { "internxn",       required_argument, 0, 'X' },
    { "thread",         required_argument, 0, 'T' },
    { "lathread",       required_argument, 0, 'L' },
    { "signbithide",    required_argument, 0, 'S' },
    { "sao",            required_argument, 0, 'O' },
    { "disabledeblock", required_argument, 0, 'D' },
    { "unittree",       required_argument, 0, 'u' },
    { "treestrength",   required_argument, 0, 'e' },
    { "refreshtype",    required_argument, 0, 'U' },
    { "refframes",      required_argument, 0, 'M' },
    { "frameskip",      required_argument, 0, '1' },
    { "ctusize",        required_argument, 0, '2' },
    { "minqtsize",      required_argument, 0, '3' },
    { "maxbtsize",      required_argument, 0, '4' },
    { "maxttsize",      required_argument, 0, '5' },
    { "maxmttdepth",    required_argument, 0, '6' },
    { "mincusize",      required_argument, 0, '7' },
    { "lookahead",      required_argument, 0, '8' },
    { "trsize64",       required_argument, 0, '9' },
    { "maxtrskipsize",  required_argument, 0, 'G' },
    { "adabframe",      required_argument, 0, 'g' },
    { "rectparttype",   required_argument, 0, 'Q' },
    { "sbsize",         required_argument, 0, 'z' },
    { "ratecontrol",    required_argument, 0, 'r' },
    { "initqp",         required_argument, 0, 'q' },
    { "rdoq",           required_argument, 0, 'd' },
    { "statlevel",      required_argument, 0, 'E' },
    { "psnr",           required_argument, 0, 'P' },
    { "help",                 no_argument, 0, 'H' },
    { "version",              no_argument, 0, 'V' },
    { 0,                0,                 0, 0   }
};

static bool parseConfigFile(encoder_option_struct* encoderOption, const char *configFileName)
{
    bool ret = false;
    if (encoderOption && configFileName)
    {
        const config_file_struct* configFile = CreateConfigFile(configFileName);
        if (configFile)
        {
            while (!EndOfFile(configFile))
            {
                if (ReadOneLine(configFile))
                {
                    //add all config options here
                    if (strcmp(configFile->key, "InputWidth") == 0)
                    {
                        encoderOption->xinConfig.inputWidth = atoi(configFile->value);
                    }
                    else if (strcmp(configFile->key, "InputHeight") == 0)
                    {
                        encoderOption->xinConfig.inputHeight = atoi(configFile->value);
                    }
                    else if (strcmp(configFile->key, "MinCbSize") == 0)
                    {
                        encoderOption->xinConfig.minCbSize = atoi(configFile->value);
                    }
                    else if (strcmp(configFile->key, "MaxCbSize") == 0)
                    {
                        encoderOption->xinConfig.maxCbSize = atoi(configFile->value);
                    }
                    else if (strcmp(configFile->key, "MinTbSize") == 0)
                    {
                        encoderOption->xinConfig.minTbSize = atoi(configFile->value);
                    }
                    else if (strcmp(configFile->key, "MaxTbSize") == 0)
                    {
                        encoderOption->xinConfig.maxTbSize = atoi(configFile->value);
                    }
                    else if (strcmp(configFile->key, "SbSize") == 0)
                    {
                        encoderOption->xinConfig.sbSize = atoi(configFile->value);
                    }
                    else if (strcmp(configFile->key, "RectPartType") == 0)
                    {
                        encoderOption->xinConfig.enableRectPartType = atoi(configFile->value);
                    }
                    else if (strcmp(configFile->key, "InputImageFile") == 0)
                    {
                        int fileNameLen
                            = (MAX_FILE_NAME_LEN < ((int)strlen(configFile->value) + 1))?
                              MAX_FILE_NAME_LEN : ((int)strlen(configFile->value) + 1);
                        memcpy(encoderOption->inputFileName, configFile->value,
                               fileNameLen);
                    }
                    else if (strcmp(configFile->key, "OutputStreamFile") == 0)
                    {
                        int fileNameLen
                            = (MAX_FILE_NAME_LEN < ((int)strlen(configFile->value) + 1)) ?
                              MAX_FILE_NAME_LEN : ((int)strlen(configFile->value) + 1);
                        memcpy(encoderOption->outputFileName, configFile->value, fileNameLen);
                    }
                    else if (strcmp(configFile->key, "ReconImageFile") == 0)
                    {
                        int fileNameLen
                            = (MAX_FILE_NAME_LEN < ((int)strlen(configFile->value) + 1)) ?
                              MAX_FILE_NAME_LEN : ((int)strlen(configFile->value) + 1);
                        memcpy(encoderOption->reconFileName, configFile->value, fileNameLen);
                    }
                    else if (strcmp(configFile->key, "TemporalLayers") == 0)
                    {
                        encoderOption->xinConfig.temporalLayerNum = atoi(configFile->value);
                    }
                    else if (strcmp(configFile->key, "InitialQP") == 0)
                    {
                        encoderOption->xinConfig.qp = atoi(configFile->value);
                    }
                    else if (strcmp(configFile->key, "FrameToBeEncoded") == 0)
                    {
                        encoderOption->xinConfig.frameToBeEncoded = atoi(configFile->value);
                    }
                    else if (strcmp(configFile->key, "IntraNxN") == 0)
                    {
                        encoderOption->xinConfig.enableIntraNxN = atoi(configFile->value);
                    }
                    else if (strcmp(configFile->key, "InterNxN") == 0)
                    {
                        encoderOption->xinConfig.enableInterNxN = atoi(configFile->value);
                    }
                    else if (strcmp(configFile->key, "SearchRange") == 0)
                    {
                        encoderOption->xinConfig.searchRange = atoi(configFile->value);
                    }
                    else if (strcmp(configFile->key, "UseDeltaQp") == 0)
                    {
                        encoderOption->xinConfig.enableCuQpDelta = atoi(configFile->value);
                    }
                    else if (strcmp(configFile->key, "TransformSkip") == 0)
                    {
                        encoderOption->xinConfig.transformSkipFlag = atoi(configFile->value);
                    }
                    else if (strcmp(configFile->key, "MaxTrSkipSize") == 0)
                    {
                        encoderOption->xinConfig.maxTrSkipSize = atoi(configFile->value);
                    }
                    else if (strcmp(configFile->key, "LoopFilterDisable") == 0)
                    {
                        encoderOption->xinConfig.disableDeblockFilter = atoi(configFile->value);
                    }
                    else if (strcmp(configFile->key, "SAO") == 0)
                    {
                        encoderOption->xinConfig.enableSao = atoi(configFile->value);
                    }
                    else if (strcmp(configFile->key, "RateControl") == 0)
                    {
                        encoderOption->xinConfig.rcMode = atoi(configFile->value);
                    }
                    else if (strcmp(configFile->key, "TargetBitrate") == 0)
                    {
                        encoderOption->xinConfig.bitRate = atoi(configFile->value);
                    }
                    else if (strcmp(configFile->key, "FrameRate") == 0)
                    {
                        encoderOption->xinConfig.frameRate = (float)atof(configFile->value);
                    }
                    else if (strcmp(configFile->key, "LookAhead") == 0)
                    {
                        encoderOption->xinConfig.lookAhead = (UINT32)atof(configFile->value);
                    }
                    else if (strcmp(configFile->key, "UnitTree") == 0)
                    {
                        encoderOption->xinConfig.unitTree = (UINT32)atof(configFile->value);
                    }
                    else if (strcmp(configFile->key, "TreeStrength") == 0)
                    {
                        encoderOption->xinConfig.unitTreeStrength = (double)atof(configFile->value);
                    }
                    else if (strcmp(configFile->key, "IntraPeriod") == 0)
                    {
                        encoderOption->xinConfig.intraPeriod = atoi(configFile->value);
                    }
                    else if (strcmp(configFile->key, "ScreenContent") == 0)
                    {
                        encoderOption->xinConfig.screenContentMode = atoi(configFile->value);
                    }
                    else if (strcmp(configFile->key, "TMVPMode") == 0)
                    {
                        encoderOption->xinConfig.enableTMvp = atoi(configFile->value);
                    }
                    else if (strcmp(configFile->key, "NumTileColumns") == 0)
                    {
                        encoderOption->xinConfig.numTileCols = atoi(configFile->value);
                    }
                    else if (strcmp(configFile->key, "NumTileRows") == 0)
                    {
                        encoderOption->xinConfig.numTileRows = atoi(configFile->value);
                    }
                    else if (strcmp(configFile->key, "MultiThread") == 0)
                    {
                        encoderOption->xinConfig.enableMultiThread = atoi(configFile->value);
                    }
                    else if (strcmp(configFile->key, "WPP") == 0)
                    {
                        encoderOption->xinConfig.enableWpp = atoi(configFile->value);
                    }
                    else if (strcmp(configFile->key, "EncoderMode") == 0)
                    {
                        encoderOption->xinConfig.encoderMode = atoi(configFile->value);
                    }
                    else if (strcmp(configFile->key, "CtuSize") == 0)
                    {
                        encoderOption->xinConfig.ctuSize = atoi(configFile->value);
                    }
                    else if (strcmp(configFile->key, "MinQtSize") == 0)
                    {
                        encoderOption->xinConfig.minQtSize = atoi(configFile->value);
                    }
                    else if (strcmp(configFile->key, "MaxBtSize") == 0)
                    {
                        encoderOption->xinConfig.maxBtSize = atoi(configFile->value);
                    }
                    else if (strcmp(configFile->key, "MaxTtSize") == 0)
                    {
                        encoderOption->xinConfig.maxTtSize = atoi(configFile->value);
                    }
                    else if (strcmp(configFile->key, "MaxMttDepth") == 0)
                    {
                        encoderOption->xinConfig.maxMttDepth = atoi(configFile->value);
                    }
                    else if (strcmp(configFile->key, "MinCuSize") == 0)
                    {
                        encoderOption->xinConfig.minCuSize = atoi(configFile->value);
                    }
                    else if (strcmp(configFile->key, "TrSize64") == 0)
                    {
                        encoderOption->xinConfig.lumaTrSize64 = atoi(configFile->value);
                    }
                    else if (strcmp(configFile->key, "Cclm") == 0)
                    {
                        encoderOption->xinConfig.enableCclm = atoi(configFile->value);
                    }
                    else if (strcmp(configFile->key, "Dmvr") == 0)
                    {
                        encoderOption->xinConfig.enableDmvr = atoi(configFile->value);
                    }
                    else if (strcmp(configFile->key, "AlgorithmMode") == 0)
                    {
                        encoderOption->xinConfig.algorithmMode = atoi(configFile->value);
                    }
                    else if (strcmp(configFile->key, "RefFrames") == 0)
                    {
                        encoderOption->xinConfig.refFrameNum = atoi(configFile->value);
                    }
                    else if (strcmp(configFile->key, "AdaptiveBFrame") == 0)
                    {
                        encoderOption->xinConfig.adaptiveBFrame = atoi(configFile->value);
                    }
                    else if (strcmp(configFile->key, "SignBitHidden") == 0)
                    {
                        encoderOption->xinConfig.enableSignDataHiding = atoi(configFile->value);
                    }
                    else if (strcmp(configFile->key, "RDOQ") == 0)
                    {
                        encoderOption->xinConfig.enableRdoq = atoi(configFile->value);
                    }
                    else if (strcmp(configFile->key, "BFrames") == 0)
                    {
                        encoderOption->xinConfig.bFrameNum = atoi(configFile->value);
                    }
                    else if (strcmp(configFile->key, "CalcPsnr") == 0)
                    {
                        encoderOption->xinConfig.calcPsnr = atoi(configFile->value);
                    }
                    else if (strcmp(configFile->key, "FrameSkip") == 0)
                    {
                        encoderOption->xinConfig.frameSkip = atoi(configFile->value);
                    }
                    else if (strcmp(configFile->key, "ThreadNum") == 0)
                    {
                        encoderOption->xinConfig.threadNum = atoi(configFile->value);
                    }
                    else if (strcmp(configFile->key, "LaThreadNum") == 0)
                    {
                        encoderOption->xinConfig.laThreadNum = atoi(configFile->value);
                    }
                    else if (strcmp(configFile->key, "FPP") == 0)
                    {
                        encoderOption->xinConfig.enableFpp = atoi(configFile->value);
                    }
                    else if (strcmp(configFile->key, "RefreshType") == 0)
                    {
                        encoderOption->xinConfig.refreshType = atoi(configFile->value);
                    }
                    else if (strcmp(configFile->key, "StatLevel") == 0)
                    {
                        encoderOption->xinConfig.statLevel = atoi(configFile->value);
                    }

                }

            }

            ret = true;
        }
        else
        {
            printf("CreateConfigFile %s failed\n", configFileName);
        }

        DeleteConfigFile(configFile);
        
    }

    return ret;
}

static bool verifyEncoderOption(
    encoder_option_struct *encoderOption,
    bool                  bConsulting)
{
    bool ret = true;

    if (strlen(encoderOption->inputFileName) > 0)
    {
        encoderOption->inputFileHandle = fopen(encoderOption->inputFileName, "rb");
        if (encoderOption->inputFileHandle == 0)
        {
            printf("Open %s failed\n", encoderOption->inputFileName);
            ret = false;
        }
    }
    else
    {
        if (!bConsulting)
        {
            printf("No input file\n");
        }
        ret = false;
    }

    if (strlen(encoderOption->outputFileName) > 0)
    {
        encoderOption->outputFileHandle = fopen(encoderOption->outputFileName, "wb");
        if (encoderOption->outputFileHandle == 0)
        {
            printf("Open %s failed\n", encoderOption->outputFileName);
            ret = false;
        }
    }

    if (strlen(encoderOption->reconFileName) > 0)
    {
        encoderOption->reconFileHandle = fopen(encoderOption->reconFileName, "wb");
        if (encoderOption->reconFileHandle == 0)
        {
            printf("Open %s failed\n", encoderOption->reconFileName);
            ret = false;
        }
    }

    return ret;

}

static void CopyEncoderOption (
    encoder_option_struct *srcOption,
    encoder_option_struct *dstOption)
{
    memset (dstOption, 0, sizeof(encoder_option_struct));

    Xin26xSetDefaultParam (
        &dstOption->xinConfig);

    dstOption->inputFileHandle  = srcOption->inputFileHandle;
    dstOption->outputFileHandle = srcOption->outputFileHandle;
    dstOption->reconFileHandle  = srcOption->reconFileHandle;

    switch (srcOption->xinConfig.encoderMode)
    {
    case 0:
        dstOption->xinConfig.refFrameNum      = 1;
        dstOption->xinConfig.motionSearchMode = 1;
        dstOption->xinConfig.enableRdoq       = 0;

        // VVC
        dstOption->xinConfig.ctuSize          = 64;
        dstOption->xinConfig.maxMttDepth      = 0;
        dstOption->xinConfig.lumaTrSize64     = 0;
        dstOption->xinConfig.enableCclm       = 0;
        dstOption->xinConfig.enableDmvr       = 0;

        // HEVC
        dstOption->xinConfig.enableSmp        = 0;
        
        break;

    case 1:
        dstOption->xinConfig.refFrameNum      = 1;
        dstOption->xinConfig.motionSearchMode = 1;
        dstOption->xinConfig.enableRdoq       = 0;

        // VVC
        dstOption->xinConfig.ctuSize          = 64;
        dstOption->xinConfig.maxMttDepth      = 1;
        dstOption->xinConfig.maxBtSize        = 64;
        dstOption->xinConfig.maxTtSize        = 8;
        dstOption->xinConfig.lumaTrSize64     = 1;
        dstOption->xinConfig.enableCclm       = 0;
        dstOption->xinConfig.enableDmvr       = 0;

        // HEVC
        dstOption->xinConfig.enableSmp        = 0;
        
        break;

    case 2:
        dstOption->xinConfig.refFrameNum      = 2;
        dstOption->xinConfig.enableRdoq       = 0;
        dstOption->xinConfig.motionSearchMode = 1;

        // VVC
        dstOption->xinConfig.ctuSize          = 64;
        dstOption->xinConfig.maxMttDepth      = 1;
        dstOption->xinConfig.maxBtSize        = 64;
        dstOption->xinConfig.maxTtSize        = 8;
        dstOption->xinConfig.lumaTrSize64     = 1;
        dstOption->xinConfig.enableCclm       = 1;
        dstOption->xinConfig.enableDmvr       = 1;

        // HEVC
        dstOption->xinConfig.enableSmp        = 0;
        
        break;

    case 3:
        dstOption->xinConfig.refFrameNum      = 3;
        dstOption->xinConfig.motionSearchMode = 1;
        dstOption->xinConfig.enableRdoq       = 0;
        
        // VVC
        dstOption->xinConfig.ctuSize          = 64;
        dstOption->xinConfig.maxMttDepth      = 1;
        dstOption->xinConfig.maxBtSize        = 64;
        dstOption->xinConfig.maxTtSize        = 64;
        dstOption->xinConfig.lumaTrSize64     = 1;
        dstOption->xinConfig.enableCclm       = 1;
        dstOption->xinConfig.enableDmvr       = 1;

        // HEVC
        dstOption->xinConfig.enableSmp        = 0;
        
        break;

    case 4:
        dstOption->xinConfig.refFrameNum      = 3;
        dstOption->xinConfig.motionSearchMode = 2;
        dstOption->xinConfig.enableRdoq       = 1;

        // VVC
        dstOption->xinConfig.ctuSize          = 128;
        dstOption->xinConfig.maxMttDepth      = 1;
        dstOption->xinConfig.maxBtSize        = 64;
        dstOption->xinConfig.maxTtSize        = 64;
        dstOption->xinConfig.lumaTrSize64     = 1;
        dstOption->xinConfig.enableCclm       = 1;
        dstOption->xinConfig.enableDmvr       = 1;

        // HEVC
        dstOption->xinConfig.enableSmp        = 1;
        
        break;

    case 5:
        dstOption->xinConfig.refFrameNum      = 5;
        dstOption->xinConfig.enableRdoq       = 1;
        dstOption->xinConfig.motionSearchMode = 2;

        // VVC
        dstOption->xinConfig.ctuSize          = 128;
        dstOption->xinConfig.maxMttDepth      = 1;
        dstOption->xinConfig.maxBtSize        = 128;
        dstOption->xinConfig.maxTtSize        = 64;
        dstOption->xinConfig.lumaTrSize64     = 1;
        dstOption->xinConfig.enableCclm       = 1;
        dstOption->xinConfig.enableDmvr       = 1;

        // HEVC
        dstOption->xinConfig.enableSmp        = 1;
        
        break;

    case 6:
        dstOption->xinConfig.refFrameNum      = 6;
        dstOption->xinConfig.enableRdoq       = 1;
        dstOption->xinConfig.motionSearchMode = 2;

        // VVC
        dstOption->xinConfig.ctuSize          = 128;
        dstOption->xinConfig.maxMttDepth      = 1;
        dstOption->xinConfig.maxBtSize        = 64;
        dstOption->xinConfig.maxTtSize        = 64;
        dstOption->xinConfig.lumaTrSize64     = 1;
        dstOption->xinConfig.enableCclm       = 1;
        dstOption->xinConfig.enableDmvr       = 1;

        // HEVC
        dstOption->xinConfig.enableSmp        = 1;
        
        break;

    default:
        break;

    }

    if (srcOption->xinConfig.maxMttDepth != 0xFF)
    {
        dstOption->xinConfig.maxMttDepth = srcOption->xinConfig.maxMttDepth;
    }

    if (srcOption->xinConfig.enableRdoq != 0xFF)
    {
        dstOption->xinConfig.enableRdoq = srcOption->xinConfig.enableRdoq;
    }

    if (srcOption->xinConfig.motionSearchMode != 0xFF)
    {
        dstOption->xinConfig.motionSearchMode = srcOption->xinConfig.motionSearchMode;
    }

    if (srcOption->xinConfig.enableSmp != 0xFF)
    {
        dstOption->xinConfig.enableSmp = srcOption->xinConfig.enableSmp;
    }

    if (srcOption->xinConfig.rcMode != 0xFF)
    {
        dstOption->xinConfig.rcMode = srcOption->xinConfig.rcMode;
    }
	
	if (srcOption->xinConfig.enableSignDataHiding != 0xFF)
    {
        dstOption->xinConfig.enableSignDataHiding = srcOption->xinConfig.enableSignDataHiding;
    }

    if (srcOption->xinConfig.bFrameNum != 0xFF)
    {
        dstOption->xinConfig.bFrameNum = srcOption->xinConfig.bFrameNum;
    }

    if (srcOption->xinConfig.enableCclm != 0xFF)
    {
        dstOption->xinConfig.enableCclm = srcOption->xinConfig.enableCclm;
    }

    if (srcOption->xinConfig.enableDmvr != 0xFF)
    {
        dstOption->xinConfig.enableDmvr = srcOption->xinConfig.enableDmvr;
    }

	if (srcOption->xinConfig.enableSao != 0xFF)
	{
		dstOption->xinConfig.enableSao = srcOption->xinConfig.enableSao;
	}

    if (srcOption->xinConfig.ctuSize)
    {
        dstOption->xinConfig.ctuSize = srcOption->xinConfig.ctuSize;
    }

    if (srcOption->xinConfig.maxBtSize)
    {
        dstOption->xinConfig.maxBtSize = srcOption->xinConfig.maxBtSize;
    }

    if (srcOption->xinConfig.maxTtSize)
    {
        dstOption->xinConfig.maxTtSize = srcOption->xinConfig.maxTtSize;
    }

    if (srcOption->xinConfig.refFrameNum)
    {
        dstOption->xinConfig.refFrameNum = srcOption->xinConfig.refFrameNum;
    }

    if (srcOption->xinConfig.inputWidth)
    {
        dstOption->xinConfig.inputWidth = srcOption->xinConfig.inputWidth;
    }

    if (srcOption->xinConfig.inputHeight)
    {
        dstOption->xinConfig.inputHeight = srcOption->xinConfig.inputHeight;
    }

    if (srcOption->xinConfig.frameRate != 0.0)
    {
        dstOption->xinConfig.frameRate = srcOption->xinConfig.frameRate;
    }

    if (srcOption->xinConfig.bitRate)
    {
        dstOption->xinConfig.bitRate = srcOption->xinConfig.bitRate;
    }

    if (srcOption->xinConfig.minQp)
    {
        dstOption->xinConfig.minQp = srcOption->xinConfig.minQp;
    }

    if (srcOption->xinConfig.maxQp)
    {
        dstOption->xinConfig.maxQp = srcOption->xinConfig.maxQp;
    }

    if (srcOption->xinConfig.frameSkip)
    {
        dstOption->xinConfig.frameSkip = srcOption->xinConfig.frameSkip;
    }

    if (srcOption->xinConfig.encoderMode)
    {
        dstOption->xinConfig.encoderMode = srcOption->xinConfig.encoderMode;
    }

    if (srcOption->xinConfig.algorithmMode)
    {
        dstOption->xinConfig.algorithmMode = srcOption->xinConfig.algorithmMode;
    }

    if (srcOption->xinConfig.frameToBeEncoded)
    {
        dstOption->xinConfig.frameToBeEncoded = srcOption->xinConfig.frameToBeEncoded;
    }

    if (srcOption->xinConfig.refFrameNum)
    {
        dstOption->xinConfig.refFrameNum = srcOption->xinConfig.refFrameNum;
    }

    if (srcOption->xinConfig.refreshType)
    {
        dstOption->xinConfig.refreshType = srcOption->xinConfig.refreshType;
    }

    if (srcOption->xinConfig.temporalLayerNum)
    {
        dstOption->xinConfig.temporalLayerNum = srcOption->xinConfig.temporalLayerNum;
    }

    if (srcOption->xinConfig.intraPeriod)
    {
        dstOption->xinConfig.intraPeriod = srcOption->xinConfig.intraPeriod;
    }

    if (srcOption->xinConfig.screenContentMode)
    {
        dstOption->xinConfig.screenContentMode = srcOption->xinConfig.screenContentMode;
    }

    if (srcOption->xinConfig.qp)
    {
        dstOption->xinConfig.qp = srcOption->xinConfig.qp;
    }

    if (srcOption->xinConfig.calcPsnr)
    {
        dstOption->xinConfig.calcPsnr = srcOption->xinConfig.calcPsnr;
    }

    if (srcOption->xinConfig.enableStrongIntraSmoothing)
    {
        dstOption->xinConfig.enableStrongIntraSmoothing = srcOption->xinConfig.enableStrongIntraSmoothing;
    }

    if (srcOption->xinConfig.enableTMvp)
    {
        dstOption->xinConfig.enableTMvp = srcOption->xinConfig.enableTMvp;
    }

    if (srcOption->xinConfig.enableIntraNxN)
    {
        dstOption->xinConfig.enableIntraNxN = srcOption->xinConfig.enableIntraNxN;
    }

    if (srcOption->xinConfig.enableInterNxN)
    {
        dstOption->xinConfig.enableInterNxN = srcOption->xinConfig.enableInterNxN;
    }

    if (srcOption->xinConfig.constrainedIntraPredFlag)
    {
        dstOption->xinConfig.constrainedIntraPredFlag = srcOption->xinConfig.constrainedIntraPredFlag;
    }

    if (srcOption->xinConfig.transformSkipFlag)
    {
        dstOption->xinConfig.transformSkipFlag = srcOption->xinConfig.transformSkipFlag;
    }

    if (srcOption->xinConfig.adaptiveBFrame)
    {
        dstOption->xinConfig.adaptiveBFrame = srcOption->xinConfig.adaptiveBFrame;
    }

    if (srcOption->xinConfig.enableCuQpDelta)
    {
        dstOption->xinConfig.enableCuQpDelta = srcOption->xinConfig.enableCuQpDelta;
    }

    if (srcOption->xinConfig.diffCuQpDeltaDepth)
    {
        dstOption->xinConfig.diffCuQpDeltaDepth = srcOption->xinConfig.diffCuQpDeltaDepth;
    }

    if (srcOption->xinConfig.searchRange)
    {
        dstOption->xinConfig.searchRange = srcOption->xinConfig.searchRange;
    }

    if (srcOption->xinConfig.enableAmp)
    {
        dstOption->xinConfig.enableAmp = srcOption->xinConfig.enableAmp;
    }

    if (srcOption->xinConfig.ctuSize)
    {
        dstOption->xinConfig.ctuSize = srcOption->xinConfig.ctuSize;
    }

    if (srcOption->xinConfig.minQtSize)
    {
        dstOption->xinConfig.minQtSize = srcOption->xinConfig.minQtSize;
    }

    if (srcOption->xinConfig.minCuSize)
    {
        dstOption->xinConfig.minCuSize = srcOption->xinConfig.minCuSize;
    }

    if (srcOption->xinConfig.maxTrSkipSize)
    {
        dstOption->xinConfig.maxTrSkipSize = srcOption->xinConfig.maxTrSkipSize;
    }

    if (srcOption->xinConfig.lumaTrSize64)
    {
        dstOption->xinConfig.lumaTrSize64 = srcOption->xinConfig.lumaTrSize64;
    }

    if (srcOption->xinConfig.sbSize)
    {
        dstOption->xinConfig.sbSize = srcOption->xinConfig.sbSize;
    }

    if (srcOption->xinConfig.enableMultiThread)
    {
        dstOption->xinConfig.enableMultiThread = srcOption->xinConfig.enableMultiThread;
    }

    if (srcOption->xinConfig.threadNum)
    {
        dstOption->xinConfig.threadNum = srcOption->xinConfig.threadNum;
    }

    if (srcOption->xinConfig.enableWpp)
    {
        dstOption->xinConfig.enableWpp = srcOption->xinConfig.enableWpp;
    }

    if (srcOption->xinConfig.enableFpp)
    {
        dstOption->xinConfig.enableFpp = srcOption->xinConfig.enableFpp;
    }

    if (srcOption->xinConfig.numTileCols)
    {
        dstOption->xinConfig.numTileCols = srcOption->xinConfig.numTileCols;
    }

    if (srcOption->xinConfig.numTileRows)
    {
        dstOption->xinConfig.numTileRows = srcOption->xinConfig.numTileRows;
    }

    if (srcOption->xinConfig.unitTree)
    {
        dstOption->xinConfig.unitTree = srcOption->xinConfig.unitTree;
    }

    if (srcOption->xinConfig.lookAhead)
    {
        dstOption->xinConfig.lookAhead = srcOption->xinConfig.lookAhead;
    }

    if (srcOption->xinConfig.laThreadNum)
    {
        dstOption->xinConfig.laThreadNum = srcOption->xinConfig.laThreadNum;
    }

    if (srcOption->xinConfig.unitTreeStrength != 0.0)
    {
        dstOption->xinConfig.unitTreeStrength = srcOption->xinConfig.unitTreeStrength;
    }

    if (srcOption->xinConfig.disableDeblockFilter)
    {
        dstOption->xinConfig.disableDeblockFilter = srcOption->xinConfig.disableDeblockFilter;
    }

    if (srcOption->xinConfig.statLevel)
    {
        dstOption->xinConfig.statLevel = srcOption->xinConfig.statLevel;
    }

}

encoder_option_struct* CreateEncoderOption(int argc, char**argv)
{
    const char            *configFileName;
    encoder_option_struct *encoderOption;
    encoder_option_struct localOption;
    bool                  bConsulting;

    configFileName = NULL;
    encoderOption  = NULL;
    bConsulting    = false;

    if (argc <= 1)
    {
        printf("Argument is invalid. Run xin26x_test --help for a list of options.\n");

        return NULL;
    }

    //get config file
    for (optind = 0; ; )
    {
        int opt = getopt_long(argc, argv, encoder_short_options, encoder_long_options, (int *)0);
        switch (opt)
        {
        case 'c':
            configFileName = optarg;
            break;

        default:
            break;
        }
        if (opt == -1)
        {
            break;
        }
    }

    memset(&localOption, 0, sizeof(encoder_option_struct));

    localOption.xinConfig.encoderMode          = 1;
    localOption.xinConfig.enableRdoq           = 0xFF;
    localOption.xinConfig.maxMttDepth          = 0xFF;
    localOption.xinConfig.motionSearchMode     = 0xFF;
    localOption.xinConfig.enableSmp            = 0xFF;
    localOption.xinConfig.rcMode               = 0xFF;
	localOption.xinConfig.enableSignDataHiding = 0xFF;
    localOption.xinConfig.bFrameNum            = 0xFF;
    localOption.xinConfig.enableCclm           = 0xFF;
    localOption.xinConfig.enableDmvr           = 0xFF;
	localOption.xinConfig.enableSao            = 0xFF;

    if (configFileName)
    {
        //configEncoder with config file
        if (!parseConfigFile(&localOption, configFileName))
        {
            printf("Config file is invalid\n");

            DeleteEncoderOption (&localOption);

            return NULL;
        }
    }

    // replace config file setting if there is custom setting
    for (optind = 0; ; )
    {
        int fileNameLen = 0;
        int opt = getopt_long(argc, argv, encoder_short_options, encoder_long_options, (int *)0);

        switch (opt)
        {
        case 'i':
            fileNameLen
                = (MAX_FILE_NAME_LEN < ((int)strlen(optarg) + 1)) ?
                  MAX_FILE_NAME_LEN : ((int)strlen(optarg) + 1);
            memcpy(localOption.inputFileName, optarg,
                   fileNameLen);
            break;

        case 'o':
            fileNameLen
                = (MAX_FILE_NAME_LEN < ((int)strlen(optarg) + 1)) ?
                  MAX_FILE_NAME_LEN : ((int)strlen(optarg) + 1);
            memcpy(localOption.outputFileName, optarg,
                   fileNameLen);
            break;

        case 'a':
            localOption.xinConfig.algorithmMode = atoi(optarg);
            break;

        case 'n':
            localOption.xinConfig.frameToBeEncoded = atoi(optarg);
            break;

        case 'R':
            fileNameLen
                = (MAX_FILE_NAME_LEN < ((int)strlen(optarg) + 1)) ?
                  MAX_FILE_NAME_LEN : ((int)strlen(optarg) + 1);
            memcpy(localOption.reconFileName, optarg,
                   fileNameLen);
            break;

        case 'w':
            localOption.xinConfig.inputWidth = atoi(optarg);
            break;

        case 'h':
            localOption.xinConfig.inputHeight = atoi(optarg);
            break;

        case 'f':
            localOption.xinConfig.frameRate = (float)atof(optarg);
            break;

        case 'b':
            localOption.xinConfig.bitRate = atoi(optarg);
            break;

        case 't':
            localOption.xinConfig.temporalLayerNum = atoi(optarg);
            break;

        case 's':
            localOption.xinConfig.screenContentMode = atoi(optarg);
            break;

        case 'r':
            localOption.xinConfig.rcMode = atoi(optarg);
            break;

        case 'u':
            localOption.xinConfig.unitTree = atoi(optarg);
            break;

        case 'e':
            localOption.xinConfig.unitTreeStrength = (double)atoi(optarg);
            break;

        case 'O':
            localOption.xinConfig.enableSao = atoi(optarg);
            break;

        case 'D':
            localOption.xinConfig.disableDeblockFilter = atoi(optarg);
            break;

        case 'P':
            localOption.xinConfig.calcPsnr = atoi(optarg);
            break;

        case 'B':
            localOption.xinConfig.bFrameNum = atoi(optarg);
            break;

        case 'W':
            localOption.xinConfig.enableWpp = atoi(optarg);
            break;

        case 'F':
            localOption.xinConfig.enableFpp = atoi(optarg);
            break;

        case 'T':
            localOption.xinConfig.threadNum = atoi(optarg);
            break;

        case 'L':
            localOption.xinConfig.laThreadNum = atoi(optarg);
            break;

        case 'N':
            localOption.xinConfig.enableIntraNxN = atoi(optarg);
            break;

        case 'X':
            localOption.xinConfig.enableInterNxN = atoi(optarg);
            break;

        case 'p':
            localOption.xinConfig.encoderMode = atoi(optarg);
            break;

        case 'S':
            localOption.xinConfig.enableSignDataHiding = atoi(optarg);
            break;

        case 'U':
            localOption.xinConfig.refreshType = atoi(optarg);
            break;

        case 'M':
            localOption.xinConfig.refFrameNum = atoi(optarg);
            break;

        case 'g':
            localOption.xinConfig.adaptiveBFrame = atoi(optarg);
            break;

        case 'A':
            localOption.xinConfig.transformSkipFlag = atoi(optarg);
            break;

        case '1':
            localOption.xinConfig.frameSkip = atoi(optarg);
            break;

        case '2':
            localOption.xinConfig.ctuSize = atoi(optarg);
            break;

        case '3':
            localOption.xinConfig.minQtSize = atoi(optarg);
            break;

        case '4':
            localOption.xinConfig.maxBtSize = atoi(optarg);
            break;

        case '5':
            localOption.xinConfig.maxTtSize = atoi(optarg);
            break;

        case '6':
            localOption.xinConfig.maxMttDepth = atoi(optarg);
            break;

        case '7':
            localOption.xinConfig.minCuSize = atoi(optarg);
            break;

        case '8':
            localOption.xinConfig.lookAhead = atoi(optarg);
            break;

        case '9':
            localOption.xinConfig.lumaTrSize64 = atoi(optarg);
            break;

        case 'I':
            localOption.xinConfig.intraPeriod = atoi(optarg);
            break;

        case 'q':
            localOption.xinConfig.qp = atoi(optarg);
            break;

        case 'd':
            localOption.xinConfig.enableRdoq = atoi(optarg);
            break;

        case 'C':
            localOption.xinConfig.enableCclm = atoi(optarg);
            break;

        case 'y':
            localOption.xinConfig.enableDmvr = atoi(optarg);
            break;

        case 'G':
            localOption.xinConfig.maxTrSkipSize = atoi(optarg);
            break;

        case 'Q':
            localOption.xinConfig.enableRectPartType = atoi(optarg);
            break;

        case 'z':
            localOption.xinConfig.sbSize = atoi(optarg);
            break;

        case 'E':
            localOption.xinConfig.statLevel = atoi(optarg);
            break;

        case 'H':
            ShowHelp();
            bConsulting = true;
            break;

        case 'V':
            ShowVersion();
            bConsulting = true;
            break;

        default:
            break;

        }

        if (opt == -1)
        {
            break;
        }

    }

    if (verifyEncoderOption(&localOption, bConsulting) == false)
    {
        DeleteEncoderOption(&localOption);

        encoderOption = NULL;

        if (!bConsulting)
        {
            printf("Option is wrong!\n");

            ShowHelp();
        }

    }
	else
	{

		encoderOption = (encoder_option_struct*)malloc(sizeof(encoder_option_struct));

		CopyEncoderOption (
			&localOption,
			encoderOption);
	}

    return encoderOption;

}

void DeleteEncoderOption(encoder_option_struct* encoderOption)
{
    if (encoderOption->inputFileHandle)
    {
        fclose(encoderOption->inputFileHandle);
        encoderOption->inputFileHandle = 0;
    }

    if (encoderOption->outputFileHandle)
    {
        fclose(encoderOption->outputFileHandle);
        encoderOption->outputFileHandle = 0;
    }

    if (encoderOption->reconFileHandle)
    {
        fclose(encoderOption->reconFileHandle);
        encoderOption->reconFileHandle = 0;
    }

}

void PrintEncoderOption(encoder_option_struct* encoderOption)
{
    if (encoderOption)
    {
        printf("xinConfig:\n");
        printf("InputFileName = ");
        printf("%s\n",  encoderOption->inputFileName);
        printf("OutputFileName = ");
        printf("%s\n",  encoderOption->outputFileName);
        printf("ReconFileName = ");
        printf("%s\n",  encoderOption->reconFileName);
        printf("inputWidth = ");
        printf("%d\n", encoderOption->xinConfig.inputWidth);
        printf("inputHeight = ");
        printf("%d\n", encoderOption->xinConfig.inputHeight);
    }
}

void ShowHelp()
{
    printf ("\nSyntax: xin26x_test [options]\n");

    printf ("\nEncoder config file option: if there is no config file, app will use custom options\n");
    printf ("-c/--config <filename>       Config file\n");
    printf ("\nEncoder custom options: if there are some custom options, it will replace the value in config file\n");

    printf ("Input and Ouput Options:\n");
    printf ("-i/--input <filename>        Input YUV420 file\n");
    printf ("-o/--output <filename>       Output raw bitstream file\n");
    printf ("-n/--framenumber <integer>   Frames to be encoded\n");
    printf ("-w/--width <integer>         Width of input YUV420 file\n");
    printf ("-h/--height <integer>        Height of input YUV420 file\n");
    printf ("\n");

    printf ("Algorithm Mode:\n");
    printf ("-a/--algmode <integer>       Which algorithm mode is used. 0: H.265, 1: AV1 2: H.266.\n");
    printf ("\n");

    printf ("H.265 Coding Tools:\n");
    printf ("--intranxn <integer>         Enable intra 4x4, 0: disable, 1: enable\n");
    printf ("--internxn <integer>         Enable inter 4x4, 0: disable, 1: enable\n");
    printf ("\n");

    printf ("AV1 Coding Toools:\n");
    printf ("--sbsize <integer>           Super Block size 64 or 128.\n");
    printf ("--rectparttype <integer>     Enable rectangle partition type.\n");
    printf ("\n");

    printf ("H266 Coding Tools:\n");
    printf ("--minqtsize <integer>        Minimum allowed quaternary tree leaf node size.\n");
    printf ("--maxbtsize <integer>        Maximum allowed binary tree root node size.\n");
    printf ("--maxttsize <integer>        Maximum allowed ternary tree root node size.\n");
    printf ("--maxmttdepth <integer>      Maximum allowed hierarchy depth of multi-type tree splitting from a quadtree leaf.\n");
    printf ("--mincusize <integer>        Minimum coding unit size.\n");
    printf ("--trsize64 <integer>         Enable max transform size 64 for luma, 0: disable, 1: enable.\n");
    printf ("--maxtrskipsize <integer>    Max transform skip size.\n");
    printf ("\n");

    printf ("H265 and H266 Coding Tools:\n");
    printf ("--signbithide <integer>      Enable sign bit hidden, 0: disable, 1: enable\n");
    printf ("--transformskip <integer>    Enable transform skip. 0: disable, 1: enable\n");
    printf ("--sao <integer>              Enable sample adaptive offset. 0: disable, 1: enable\n");
    printf ("\n");

    printf ("General Coding Tools:\n");
    printf ("-p/--preset <integer>        Encoder preset. 0: superfast, 1: veryfast, 2: fast, 3: medium, 4 slow, 5 veryslow.\n");
    printf ("-s/--screencontent <integer> Enable screen content, 0: disable, 1: enable\n");
    printf ("--lookahead <integer>        How many frames are lookahead.\n");
    printf ("-d/--rdoq <integer>          Enable rate distortion optimization. 0: disable, 1: Enable.\n");
    printf ("-L/--lathread <integer>      How many thread used for lookahead.\n");
    printf ("-u/--unittree <integer>      Enable unit tree, 0: disable, 1: enable.\n");
    printf ("-e/--treestrength <double>   Unit tree strength.\n");
    printf ("\n");

    printf ("Coding Structure:\n");
    printf ("-t/--temporallayer <integer> Temporal layer\n");
    printf ("-I/--intraperiod <integer>   Intra Period Length\n");
    printf ("--refreshtype <integer>      Refresh Type, Only works under b frame cases. 0:CRA, 1:IDR\n");
    printf ("--refframes <integer>        Reference frame number, Only works under none b frame cases.\n");
    printf ("-B/--bframes <integer>       B frame number in a prediciton gop.\n");
    printf ("--adabframe <integer>        Enable adapitve B frame number in a gop. 0: disable, 1: enable.\n");
    printf ("\n");

    printf ("Multiple Thread:\n");
    printf ("-W/--wpp <integer>           Enable WaveFront Parallel Processing. 0: disable, 1: Enable.\n");
    printf ("-F/--fpp <integer>           Enable Frame Parallel Processing. 0: disable, 1: Enable.\n");
    printf ("-T/--thread <integer>        Specify thread number in thread pool. It is decided by local system if thread number is 0.\n");
    printf ("\n");

    printf ("Rate Control:\n");
    printf ("-r/--ratecontrol <integer>   Enable rate control, 0: disable, 1: cbr people 2: cbr content 3: vbr.\n");
    printf ("-q/--qp <integer>            Encode QP, work when rate control is disabled\n");
    printf ("-f/--framerate <float>       Encode frame rate\n");
    printf ("-b/--bitrate <integer>       Encode bit rate\n");
    printf ("--frameskip <integer>        Enable frame skip when bit rate is insufficient. 0: disable, 1: enable\n");
    printf ("\n");

    printf ("Other Options:\n");
    printf ("-H/--help                    Show this help text\n");
    printf ("-V/--version                 Show version info\n");
    printf ("-E/--statlevel <integer>     Statistics level. 0: no stat, 1: stat in sequence level, 2: stat in picture level.\n");

}

void ShowVersion()
{
    printf ("xin26x v1.0\n");
}

