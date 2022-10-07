/***************************************************************************//**
*
* @file          encoder_option.h
* @authors       shennung
* @copyright     (c) 2020, shennung <shennung@hotmail.com>  All rights reserved
*
*******************************************************************************/
#ifndef _xin_encoder_option_h_
#define _xin_encoder_option_h_

#include <stdio.h>
#include "xin26x_params.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_FILE_NAME_LEN   256
#define MAX_FRAME_WIDTH     1920
#define MAX_FRAME_HEIGHT    1088

typedef struct encoder_option_struct
{
    //xin encoder configure
    xin26x_params xinConfig;

    //app option
    char  inputFileName[MAX_FILE_NAME_LEN];
    char  outputFileName[MAX_FILE_NAME_LEN];
    char  reconFileName[MAX_FILE_NAME_LEN];
    FILE* inputFileHandle;
    FILE* outputFileHandle;
    FILE* reconFileHandle;
    
}encoder_option_struct;


encoder_option_struct* CreateEncoderOption(int argc, char**argv);
void DeleteEncoderOption(encoder_option_struct* encoder_option);
void PrintEncoderOption(encoder_option_struct* encoder_option);
void ShowHelp();
void ShowVersion();


#ifdef __cplusplus
}
#endif

#endif // _encoder_option_h_
