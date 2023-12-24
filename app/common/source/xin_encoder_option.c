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
    { "screencontent",  required_argument, 0, 's' },
    { "transformSkip",  required_argument, 0, 'A' },
    { "preset",         required_argument, 0, 'p' },
    { "cclm",           required_argument, 0, 'C' },
    { "dmvr",           required_argument, 0, 'y' },
    { "mctf",           required_argument, 0, 'K' },
    { "wpp",            required_argument, 0, 'W' },
    { "fpp",            required_argument, 0, 'F' },
    { "bframes",        required_argument, 0, 'B' },
    { "intraperiod",    required_argument, 0, 'I' },
    { "intranxn",       required_argument, 0, 'N' },
    { "internxn",       required_argument, 0, 'X' },
    { "thread",         required_argument, 0, 'T' },
    { "sbh",            required_argument, 0, 'S' },
    { "sao",            required_argument, 0, 'O' },
    { "alf",            required_argument, 0, 'l' },
    { "deblock",        required_argument, 0, 'D' },
    { "unitTree",       required_argument, 0, 'u' },
    { "treeStrength",   required_argument, 0, 'e' },
    { "refreshtype",    required_argument, 0, 'U' },
    { "refframes",      required_argument, 0, 'M' },
    { "frameSkip",      required_argument, 0, '1' },
    { "ctuSize",        required_argument, 0, '2' },
    { "minQtSize",      required_argument, 0, '3' },
    { "maxBtSize",      required_argument, 0, '4' },
    { "maxTtsize",      required_argument, 0, '5' },
    { "maxMttDepth",    required_argument, 0, '6' },
    { "minCuSize",      required_argument, 0, '7' },
    { "lookAhead",      required_argument, 0, '8' },
    { "trSize64",       required_argument, 0, '9' },
    { "maxTrSkipSize",  required_argument, 0, 'G' },
    { "adaBFrame",      required_argument, 0, 'g' },
    { "rectparttype",   required_argument, 0, 'Q' },
    { "sbSize",         required_argument, 0, 'z' },
    { "rateControl",    required_argument, 0, 'r' },
    { "initqp",         required_argument, 0, 'q' },
    { "rdoq",           required_argument, 0, 'd' },
    { "statLevel",      required_argument, 0, 'E' },
    { "psnr",           required_argument, 0, 'P' },
    { "help",                 no_argument, 0, 'H' },
    { "version",              no_argument, 0, 'V' },
    { "sbTmvp",         required_argument, 0, 128 },
    { "affine",         required_argument, 0, 129 },
    { "mts",            required_argument, 0, 130 },
    { "gpb",            required_argument, 0, 131 },
    { "depQuant",       required_argument, 0, 132 },
    { "scenecut",       required_argument, 0, 133 },
    { "zerolatency",    required_argument, 0, 134 },
    { "amvr",           required_argument, 0, 135 },
    { "hidden",         required_argument, 0, 255 },
    { 0,                0,                 0, 0   }
};

static void SetPreset (
    encoder_option_struct *encoderOption)
{
    switch (encoderOption->xinConfig.encoderMode)
    {
    case 0:
        encoderOption->xinConfig.refFrameNum      = 1;
        encoderOption->xinConfig.motionSearchMode = 1;
        encoderOption->xinConfig.enableRdoq       = 0;
        encoderOption->xinConfig.enableMctf       = 0;

        // VVC
        encoderOption->xinConfig.ctuSize          = 64;
        encoderOption->xinConfig.maxMttDepth      = 0;
        encoderOption->xinConfig.lumaTrSize64     = 0;
        encoderOption->xinConfig.enableCclm       = 0;
        encoderOption->xinConfig.enableDmvr       = 0;
        encoderOption->xinConfig.enableAlf        = 0;
        encoderOption->xinConfig.enableAmvr       = FALSE;

        // HEVC
        encoderOption->xinConfig.enableSmp        = 0;

        break;

    case 1:
        encoderOption->xinConfig.refFrameNum      = 1;
        encoderOption->xinConfig.motionSearchMode = 1;
        encoderOption->xinConfig.enableRdoq       = 0;
        encoderOption->xinConfig.enableMctf       = 0;

        // VVC
        encoderOption->xinConfig.ctuSize          = 64;
        encoderOption->xinConfig.maxMttDepth      = 1;
        encoderOption->xinConfig.maxBtSize        = 64;
        encoderOption->xinConfig.maxTtSize        = 8;
        encoderOption->xinConfig.lumaTrSize64     = 0;
        encoderOption->xinConfig.enableCclm       = 0;
        encoderOption->xinConfig.enableDmvr       = 0;
        encoderOption->xinConfig.enableAlf        = 0;
        encoderOption->xinConfig.enableAmvr       = FALSE;

        // HEVC
        encoderOption->xinConfig.enableSmp        = 0;

        break;

    case 2:
        encoderOption->xinConfig.refFrameNum      = 2;
        encoderOption->xinConfig.enableRdoq       = 0;
        encoderOption->xinConfig.motionSearchMode = 1;
        encoderOption->xinConfig.enableMctf       = 1;

        // VVC
        encoderOption->xinConfig.ctuSize          = 64;
        encoderOption->xinConfig.maxMttDepth      = 1;
        encoderOption->xinConfig.maxBtSize        = 64;
        encoderOption->xinConfig.maxTtSize        = 8;
        encoderOption->xinConfig.lumaTrSize64     = 1;
        encoderOption->xinConfig.enableCclm       = 0;
        encoderOption->xinConfig.enableDmvr       = 1;
        encoderOption->xinConfig.enableAlf        = 1;
        encoderOption->xinConfig.enableAmvr       = FALSE;

        // HEVC
        encoderOption->xinConfig.enableSmp        = 0;

        break;

    case 3:
        encoderOption->xinConfig.refFrameNum      = 3;
        encoderOption->xinConfig.motionSearchMode = 1;
        encoderOption->xinConfig.enableRdoq       = 0;
        encoderOption->xinConfig.enableMctf       = 1;

        // VVC
        encoderOption->xinConfig.ctuSize          = 64;
        encoderOption->xinConfig.maxMttDepth      = 1;
        encoderOption->xinConfig.maxBtSize        = 64;
        encoderOption->xinConfig.maxTtSize        = 64;
        encoderOption->xinConfig.lumaTrSize64     = 1;
        encoderOption->xinConfig.enableCclm       = 0;
        encoderOption->xinConfig.enableDmvr       = 1;
        encoderOption->xinConfig.enableAlf        = 1;
        encoderOption->xinConfig.enableAmvr       = FALSE;

        // HEVC
        encoderOption->xinConfig.enableSmp        = 0;

        break;

    case 4:
        encoderOption->xinConfig.refFrameNum      = 2;
        encoderOption->xinConfig.enableRdoq       = 1;
        encoderOption->xinConfig.motionSearchMode = 2;
        encoderOption->xinConfig.enableMctf       = 1;

        // VVC
        encoderOption->xinConfig.ctuSize          = 64;
        encoderOption->xinConfig.maxMttDepth      = 1;
        encoderOption->xinConfig.maxBtSize        = 64;
        encoderOption->xinConfig.maxTtSize        = 64;
        encoderOption->xinConfig.lumaTrSize64     = 1;
        encoderOption->xinConfig.enableCclm       = 0;
        encoderOption->xinConfig.enableDmvr       = 1;
        encoderOption->xinConfig.enableAlf        = 1;
        encoderOption->xinConfig.enableSbTmvp     = FALSE;
        encoderOption->xinConfig.enableAffine     = FALSE;
        encoderOption->xinConfig.enableMts        = TRUE;
        encoderOption->xinConfig.enableDepQuant   = TRUE;
        encoderOption->xinConfig.enableAmvr       = FALSE;

        // HEVC
        encoderOption->xinConfig.enableSmp        = 1;

        break;

    case 5:
        encoderOption->xinConfig.refFrameNum      = 3;
        encoderOption->xinConfig.enableRdoq       = 1;
        encoderOption->xinConfig.motionSearchMode = 2;
        encoderOption->xinConfig.enableMctf       = 1;

        // VVC
        encoderOption->xinConfig.ctuSize          = 128;
        encoderOption->xinConfig.maxMttDepth      = 1;
        encoderOption->xinConfig.maxBtSize        = 64;
        encoderOption->xinConfig.maxTtSize        = 64;
        encoderOption->xinConfig.lumaTrSize64     = 1;
        encoderOption->xinConfig.enableCclm       = 0;
        encoderOption->xinConfig.enableDmvr       = 1;
        encoderOption->xinConfig.enableAlf        = TRUE;
        encoderOption->xinConfig.enableSbTmvp     = TRUE;
        encoderOption->xinConfig.enableAffine     = TRUE;
        encoderOption->xinConfig.enableMts        = TRUE;
        encoderOption->xinConfig.enableDepQuant   = TRUE;
        encoderOption->xinConfig.enableAmvr       = FALSE;

        // HEVC
        encoderOption->xinConfig.enableSmp        = 1;

        break;

    case 6:
        encoderOption->xinConfig.refFrameNum      = 4;
        encoderOption->xinConfig.enableRdoq       = 1;
        encoderOption->xinConfig.motionSearchMode = 2;
        encoderOption->xinConfig.enableMctf       = 1;

        // VVC
        encoderOption->xinConfig.ctuSize          = 128;
        encoderOption->xinConfig.maxMttDepth      = 1;
        encoderOption->xinConfig.maxBtSize        = 64;
        encoderOption->xinConfig.maxTtSize        = 64;
        encoderOption->xinConfig.lumaTrSize64     = 1;
        encoderOption->xinConfig.enableCclm       = 1;
        encoderOption->xinConfig.enableDmvr       = 1;
        encoderOption->xinConfig.enableAlf        = TRUE;
        encoderOption->xinConfig.enableSbTmvp     = TRUE;
        encoderOption->xinConfig.enableAffine     = TRUE;
        encoderOption->xinConfig.enableMts        = TRUE;
        encoderOption->xinConfig.enableDepQuant   = TRUE;
        encoderOption->xinConfig.enableAmvr       = TRUE;

        // HEVC
        encoderOption->xinConfig.enableSmp        = 1;

        break;

    default:
		encoderOption->xinConfig.refFrameNum      = 4;
        encoderOption->xinConfig.enableRdoq       = 1;
        encoderOption->xinConfig.motionSearchMode = 2;
        encoderOption->xinConfig.enableMctf       = 1;

        // VVC
        encoderOption->xinConfig.ctuSize          = 128;
        encoderOption->xinConfig.maxMttDepth      = 1;
        encoderOption->xinConfig.maxBtSize        = 64;
        encoderOption->xinConfig.maxTtSize        = 64;
        encoderOption->xinConfig.lumaTrSize64     = 1;
        encoderOption->xinConfig.enableCclm       = 1;
        encoderOption->xinConfig.enableDmvr       = 1;
        encoderOption->xinConfig.enableAlf        = TRUE;
        encoderOption->xinConfig.enableSbTmvp     = TRUE;
        encoderOption->xinConfig.enableAffine     = TRUE;
        encoderOption->xinConfig.enableMts        = TRUE;
        encoderOption->xinConfig.enableDepQuant   = TRUE;
        encoderOption->xinConfig.enableAmvr       = TRUE;

        // HEVC
        encoderOption->xinConfig.enableSmp        = 1;
        break;

    }

}

static void SetScreenContentMode (
    xin26x_params *xinConfig)
{
    if (xinConfig->screenContentMode)
    {
        // h266 & h265
        xinConfig->transformSkipFlag = TRUE;
        xinConfig->motionSearchMode  = 2;
        xinConfig->enableMctf        = FALSE;

        // h265
        xinConfig->enableIntraNxN    = TRUE;
        xinConfig->enableInterNxN    = TRUE;

        // h266
        xinConfig->maxTrSkipSize     = xinConfig->maxTrSkipSize > 8 ? xinConfig->maxTrSkipSize : 8;
        xinConfig->minCuSize         = 4;
        xinConfig->minQtSize         = 4;
        xinConfig->enableCclm        = TRUE;
    }
    
}

static void SetZeroLatencyMode (
    xin26x_params *xinConfig)
{
    if (xinConfig->zeroLatency)
    {
        xinConfig->bFrameNum      = 0;
        xinConfig->lookAhead      = 0;
        xinConfig->enableMctf     = FALSE;
        xinConfig->enableSceneCut = FALSE;
        xinConfig->unitTree       = FALSE;
        xinConfig->rcMode         = 3;
    }
    
}

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
                    else if (strcmp(configFile->key, "LoopFilter") == 0)
                    {
                        encoderOption->xinConfig.enableDeblock = atoi(configFile->value);
                    }
                    else if (strcmp(configFile->key, "SAO") == 0)
                    {
                        encoderOption->xinConfig.enableSao = atoi(configFile->value);
                    }
                    else if (strcmp(configFile->key, "ALF") == 0)
                    {
                        encoderOption->xinConfig.enableAlf = atoi(configFile->value);
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

                        SetScreenContentMode (
                            &encoderOption->xinConfig);
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
                    else if (strcmp(configFile->key, "WPP") == 0)
                    {
                        encoderOption->xinConfig.enableWpp = atoi(configFile->value);
                    }
                    else if (strcmp(configFile->key, "EncoderMode") == 0)
                    {
                        encoderOption->xinConfig.encoderMode = atoi(configFile->value);

                        SetPreset (encoderOption);
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
                    else if (strcmp(configFile->key, "MCTF") == 0)
                    {
                        encoderOption->xinConfig.enableMctf = atoi(configFile->value);
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

    if (encoderOption->reconFileHandle)
    {
        encoderOption->xinConfig.needRecon = true;
    }
    else
    {
        encoderOption->xinConfig.needRecon = false;
    }

    return ret;

}

encoder_option_struct* CreateEncoderOption(int argc, char**argv)
{
    const char            *configFileName;
    encoder_option_struct *encoderOption;
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

    encoderOption = (encoder_option_struct*)malloc(sizeof(encoder_option_struct));

    memset(encoderOption, 0, sizeof(encoder_option_struct));

    Xin26xSetDefaultParam (
        &encoderOption->xinConfig);

    if (configFileName)
    {
        //configEncoder with config file
        if (!parseConfigFile(encoderOption, configFileName))
        {
            printf("Config file is invalid\n");

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
            memcpy(encoderOption->inputFileName, optarg,
                   fileNameLen);
            break;

        case 'o':
            fileNameLen
                = (MAX_FILE_NAME_LEN < ((int)strlen(optarg) + 1)) ?
                  MAX_FILE_NAME_LEN : ((int)strlen(optarg) + 1);
            memcpy(encoderOption->outputFileName, optarg,
                   fileNameLen);
            break;

        case 'a':
            encoderOption->xinConfig.algorithmMode = atoi(optarg);
            break;

        case 'n':
            encoderOption->xinConfig.frameToBeEncoded = atoi(optarg);
            break;

        case 'R':
            fileNameLen
                = (MAX_FILE_NAME_LEN < ((int)strlen(optarg) + 1)) ?
                  MAX_FILE_NAME_LEN : ((int)strlen(optarg) + 1);
            memcpy(encoderOption->reconFileName, optarg,
                   fileNameLen);
            break;

        case 'w':
            encoderOption->xinConfig.inputWidth = atoi(optarg);
            break;

        case 'h':
            encoderOption->xinConfig.inputHeight = atoi(optarg);
            break;

        case 'f':
            encoderOption->xinConfig.frameRate = (float)atof(optarg);
            break;

        case 'b':
            encoderOption->xinConfig.bitRate = atoi(optarg);
            break;

        case 't':
            encoderOption->xinConfig.temporalLayerNum = atoi(optarg);
            break;

        case 's':
            encoderOption->xinConfig.screenContentMode = atoi(optarg);

            SetScreenContentMode (
                &encoderOption->xinConfig);

            break;

        case 'r':
            encoderOption->xinConfig.rcMode = atoi(optarg);
            break;

        case 'u':
            encoderOption->xinConfig.unitTree = atoi(optarg);
            break;

        case 'e':
            encoderOption->xinConfig.unitTreeStrength = (double)atoi(optarg);
            break;

        case 'O':
            encoderOption->xinConfig.enableSao = atoi(optarg);
            break;

        case 'l':
            encoderOption->xinConfig.enableAlf = atoi(optarg);
            break;

        case 'D':
            encoderOption->xinConfig.enableDeblock = atoi(optarg);
            break;

        case 'P':
            encoderOption->xinConfig.calcPsnr = atoi(optarg);
            break;

        case 'B':
            encoderOption->xinConfig.bFrameNum = atoi(optarg);
            break;

        case 'W':
            encoderOption->xinConfig.enableWpp = atoi(optarg);
            break;

        case 'F':
            encoderOption->xinConfig.enableFpp = atoi(optarg);
            break;

        case 'T':
            encoderOption->xinConfig.threadNum = atoi(optarg);
            break;

        case 'N':
            encoderOption->xinConfig.enableIntraNxN = atoi(optarg);
            break;

        case 'X':
            encoderOption->xinConfig.enableInterNxN = atoi(optarg);
            break;

        case 'p':
            encoderOption->xinConfig.encoderMode = atoi(optarg);

            SetPreset (encoderOption);

            break;

        case 'S':
            encoderOption->xinConfig.enableSignDataHiding = atoi(optarg);
            break;

        case 'U':
            encoderOption->xinConfig.refreshType = atoi(optarg);
            break;

        case 'M':
            encoderOption->xinConfig.refFrameNum = atoi(optarg);
            break;

        case 'g':
            encoderOption->xinConfig.adaptiveBFrame = atoi(optarg);
            break;

        case 'A':
            encoderOption->xinConfig.transformSkipFlag = atoi(optarg);
            break;

        case '1':
            encoderOption->xinConfig.frameSkip = atoi(optarg);
            break;

        case '2':
            encoderOption->xinConfig.ctuSize = atoi(optarg);
            break;

        case '3':
            encoderOption->xinConfig.minQtSize = atoi(optarg);
            break;

        case '4':
            encoderOption->xinConfig.maxBtSize = atoi(optarg);
            break;

        case '5':
            encoderOption->xinConfig.maxTtSize = atoi(optarg);
            break;

        case '6':
            encoderOption->xinConfig.maxMttDepth = atoi(optarg);
            break;

        case '7':
            encoderOption->xinConfig.minCuSize = atoi(optarg);
            break;

        case '8':
            encoderOption->xinConfig.lookAhead = atoi(optarg);
            break;

        case '9':
            encoderOption->xinConfig.lumaTrSize64 = atoi(optarg);
            break;

        case 'I':
            encoderOption->xinConfig.intraPeriod = atoi(optarg);
            break;

        case 'q':
            encoderOption->xinConfig.qp = atoi(optarg);
            break;

        case 'd':
            encoderOption->xinConfig.enableRdoq = atoi(optarg);
            break;

        case 'C':
            encoderOption->xinConfig.enableCclm = atoi(optarg);
            break;

        case 'K':
            encoderOption->xinConfig.enableMctf = atoi(optarg);
            break;

        case 'y':
            encoderOption->xinConfig.enableDmvr = atoi(optarg);
            break;

        case 'G':
            encoderOption->xinConfig.maxTrSkipSize = atoi(optarg);
            break;

        case 'Q':
            encoderOption->xinConfig.enableRectPartType = atoi(optarg);
            break;

        case 'z':
            encoderOption->xinConfig.sbSize = atoi(optarg);
            break;

        case 'E':
            encoderOption->xinConfig.statLevel = atoi(optarg);
            break;

        case 128:
            encoderOption->xinConfig.enableSbTmvp = atoi(optarg);
            break;

        case 129:
            encoderOption->xinConfig.enableAffine = atoi(optarg);
            break;

        case 130:
            encoderOption->xinConfig.enableMts = atoi(optarg);
            break;

        case 131:
            encoderOption->xinConfig.enableGpb = atoi(optarg);
            break;

        case 132:
            encoderOption->xinConfig.enableDepQuant = atoi(optarg);
            break;

        case 133:
            encoderOption->xinConfig.enableSceneCut = atoi(optarg);
            break;

        case 134:
            encoderOption->xinConfig.zeroLatency = atoi(optarg);

            SetZeroLatencyMode (
                &encoderOption->xinConfig);
            
            break;

        case 135:
            encoderOption->xinConfig.enableAmvr = atoi(optarg);            
            break;

        case 255:
            encoderOption->xinConfig.hiddenOption = atoi(optarg);
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

    if (verifyEncoderOption(encoderOption, bConsulting) == false)
    {
        if (!bConsulting)
        {
            printf("Option is wrong!\n");

            ShowHelp();
        }

        free (encoderOption);

        encoderOption = NULL;

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

void PrintEncoderOption(encoder_option_struct *encoderOption)
{
    (void)encoderOption;
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
    printf ("--minQtSize <integer>        Minimum allowed quaternary tree leaf node size.\n");
    printf ("--maxBtSize <integer>        Maximum allowed binary tree root node size.\n");
    printf ("--maxTtSize <integer>        Maximum allowed ternary tree root node size.\n");
    printf ("--maxMttDepth <integer>      Maximum allowed hierarchy depth of multi-type tree splitting from a quadtree leaf.\n");
    printf ("--minCuSize <integer>        Minimum coding unit size.\n");
    printf ("--trSize64 <integer>         Enable max transform size 64 for luma, 0: disable, 1: enable.\n");
    printf ("--maxTrSkipSize <integer>    Max transform skip size.\n");
    printf ("--sbTmvp <integer>           Enbale SbTMVP.\n");
    printf ("\n");

    printf ("H265 and H266 Coding Tools:\n");
    printf ("--sbh <integer>              Enable sign bit hidden, 0: disable, 1: enable\n");
    printf ("--transformSkip <integer>    Enable transform skip. 0: disable, 1: enable\n");
    printf ("--sao <integer>              Enable sample adaptive offset. 0: disable, 1: enable\n");
    printf ("\n");

    printf ("General Coding Tools:\n");
    printf ("-p/--preset <integer>        Encoder preset. 0: superfast, 1: veryfast, 2: fast, 3: medium, 4 slow, 5 veryslow.\n");
    printf ("-s/--screencontent <integer> Enable screen content, 0: disable, 1: enable\n");
    printf ("--lookAhead <integer>        How many frames are lookahead.\n");
    printf ("-d/--rdoq <integer>          Enable rate distortion optimization. 0: disable, 1: Enable.\n");
    printf ("-L/--lathread <integer>      How many thread used for lookahead.\n");
    printf ("-u/--unitTree <integer>      Enable unit tree, 0: disable, 1: enable.\n");
    printf ("-e/--treeStrength <double>   Unit tree strength.\n");
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
    printf ("--frameSkip <integer>        Enable frame skip when bit rate is insufficient. 0: disable, 1: enable\n");
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

