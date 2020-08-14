/***************************************************************************//**
*
* @file          xin_config_file.h
* @authors       shennung
* @copyright     (c) 2020, shennung <shennung@hotmail.com>  All rights reserved
*
*******************************************************************************/
#ifndef _xin_config_file_h_
#define _xin_config_file_h_

#include <stdio.h>
#include <stdbool.h>
#include "xin26x_params.h"

#ifdef __cplusplus
extern "C" {
#endif
typedef struct config_file_struct
{
    FILE* fileHandle;
    char* key;
    char* value;
}config_file_struct;


config_file_struct* CreateConfigFile(const char* configFileName);
void DeleteConfigFile(const config_file_struct* configFile);
bool ReadOneLine(const config_file_struct* configFile);
bool EndOfFile(const config_file_struct* configFile);


#ifdef __cplusplus
}
#endif

#endif // _config_file_h_
