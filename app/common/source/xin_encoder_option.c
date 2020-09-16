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
    { "encodertype",    required_argument, 0, 'y' },
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
                    else if (strcmp(configFile->key, "AlgorithmMode") == 0)
                    {
                        encoderOption->xinConfig.algorithmMode = atoi(configFile->value);
                    }
                    else if (strcmp(configFile->key, "RefFrames") == 0)
                    {
                        encoderOption->xinConfig.refFrameNum = atoi(configFile->value);
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

encoder_option_struct* CreateEncoderOption(int argc, char**argv)
{
    const char *configFileName = 0;
    encoder_option_struct* encoderOption = 0;
    bool bConsulting = false;
    if (argc <= 1)
    {
        printf("Argument is invalid. Run xin26x_test --help for a list of options.\n");
        return encoderOption;
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

    memset (encoderOption, 0, sizeof(encoder_option_struct));

    if (encoderOption)
    {
        Xin26xSetDefaultParam (
            &encoderOption->xinConfig);

        if (configFileName)
        {
            //configEncoder with config file
            if (!parseConfigFile(encoderOption, configFileName))
            {
                printf("Config file is invalid\n");
                
                DeleteEncoderOption(encoderOption);
                encoderOption = 0;
                return encoderOption;
            }
        }

        //replace config file setting if there is custom setting
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

                case 'D':
                    encoderOption->xinConfig.disableDeblockFilter = atoi(optarg);
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

                case 'L':
                    encoderOption->xinConfig.laThreadNum = atoi(optarg);
                    break;

                case 'N':
                    encoderOption->xinConfig.enableIntraNxN = atoi(optarg);
                    break;

                case 'X':
                    encoderOption->xinConfig.enableInterNxN = atoi(optarg);
                    break;

                case 'p':
                    encoderOption->xinConfig.encoderMode = atoi(optarg);
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

                case 'E':
                    encoderOption->xinConfig.statLevel = atoi(optarg);
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
    }

    if (verifyEncoderOption(encoderOption, bConsulting) == false)
    {
        DeleteEncoderOption(encoderOption);
        
        encoderOption = 0;
        
        if (!bConsulting)
        {
            printf("Option is wrong!\n");
            
            ShowHelp();
        }
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


    if (encoderOption)
    {
        free(encoderOption);
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
    printf("\nSyntax: xin26x_test [options]\n");

    printf("\nEncoder config file option: if there is no config file, app will use custom options\n");
    printf("-c/--config <filename>       Config file\n");

    printf("\nEncoder custom options: if there are some custom options, it will replace the value in config file\n");
    printf("-i/--input <filename>        Input YUV420 file\n");
    printf("-o/--output <filename>       Output raw bitstream file\n");
    printf("-a/--algmode <integer>       Which algorithm mode is used. 0: H.265, 1: AV1 2: H.266.\n");
    printf("-n/--framenumber <integer>   Frames to be encoded\n");
    printf("-w/--width <integer>         Width of input YUV420 file\n");
    printf("-h/--height <integer>        Height of input YUV420 file\n");
    printf("-f/--framerate <float>       Encode frame rate\n");
    printf("-b/--bitrate <integer>       Encode bit rate\n");
    printf("-t/--temporallayer <integer> Temporal layer\n");
    printf("-s/--screencontent <integer> Enable screen content, 0: disable, 1: enable\n");
    printf("-r/--ratecontrol <integer>   Enable rate control, 0: disable, 1: cbr people 2: cbr content 3: vbr\n");
    printf("-q/--qp <integer>            Encode QP, work when rate control is disabled\n");
    printf("-p/--preset <integer>        Encoder preset. 0: veryfast, 1: fast, 2: medium, 3 slow\n");
    printf("-B/--bframes <integer>       B frame number in a prediciton gop\n");
    printf("-I/--intraperiod <integer>   Intra Period Length\n");
    printf("-W/--wpp <integer>           Enable WaveFront Parallel Processing. 0: disable, 1: Enable\n");
    printf("-F/--fpp <integer>           Enable Frame Parallel Processing. 0: disable, 1: Enable\n");
    printf("-T/--thread <integer>        Specify thread number in thread pool. It is decided by local system if thread number is 0.\n");
    printf("--intranxn <integer>         Enable intra 4x4, 0: disable, 1: enable\n");
    printf("--internxn <integer>         Enable inter 4x4, 0: disable, 1: enable\n");
    printf("--signbithide <integer>      Enable sign bit hidden, 0: disable, 1: enable\n");
    printf("--refreshtype <integer>      Refresh Type, Only works under b frame cases. 0:CRA, 1:IDR\n");
    printf("--refframes <integer>        Reference frame number, Only works under none b frame cases.\n");
    printf("--frameskip <integer>        Enable frame skip when bit rate is insufficient. 0: disable, 1: enable\n");
    printf("--transformskip <integer>    Enable transform skip. 0: disable, 1: enable\n");
    printf("--lookahead <integer>        How many frames are lookahead.\n");
    printf("--trsize64 <integer>         Enable max transform size 64 for luma, 0: disable, 1: enable.\n");
    printf("-d/--rdoq <integer>          Enable rate distortion optimization. 0: disable, 1: Enable.\n");
    printf("-L/--lathread <integer>      How many thread used for lookahead.\n");
    printf("-u/--unittree <integer>      Enable unit tree, 0: disable, 1: enable.\n");
    printf("-e/--treestrength <double>   Unit tree strength.\n");
    printf("-E/--statlevel <integer>     Statistics level. 0: no stat, 1: stat in sequence level, 2: stat in picture level.\n");

    printf("\nOther options:\n");
    printf("-H/--help                    Show this help text\n");
    printf("-V/--version                 Show version info\n");
    
}

void ShowVersion()
{
    printf("xin26x v1.0\n");
}

