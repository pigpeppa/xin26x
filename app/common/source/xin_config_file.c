/***************************************************************************//**
*
* @file          xin_config_file.c
* @authors       shennung
* @brief         xin26x encoder configuration parse related subroutines.
* @copyright     (c) 2020, shennung <shennung@hotmail.com>  All rights reserved
*
*******************************************************************************/
#include <stdlib.h>
#include <memory.h>
#include "xin_typedef.h"
#include "xin_config_file.h"

static const int MAX_OPTION_LEN = 256;

config_file_struct* CreateConfigFile(const char* configFileName)
{
    config_file_struct* configFile = 0;
    if (configFileName == 0)
    {
        printf("Config file name is null\n");
        return configFile;
    }


    configFile = (config_file_struct*)malloc(sizeof(config_file_struct));
    if (configFile)
    {
        memset((void*)configFile, 0, sizeof(config_file_struct));
        configFile->fileHandle = fopen(configFileName, "r");
        if (configFile->fileHandle == 0)
        {
            free(configFile);
            configFile = 0;
        }
        else
        {
            configFile->key = (char*)malloc(MAX_OPTION_LEN);
            configFile->value = (char*)malloc(MAX_OPTION_LEN);
        }
    }

    return configFile;
}

void DeleteConfigFile(const config_file_struct* configFile)
{
    if (configFile)
    {
        fclose(configFile->fileHandle);
        free(configFile->key);
        free(configFile->value);
        free((void*)configFile);
    }
}

bool ReadOneLine(const config_file_struct* configFile)
{
    bool ret = false;
    bool commentFlag = false;
    int strLen = 0;
    char* str;

    if (configFile == 0)
    {
        return ret;
    }

    memset((void*)configFile->key, 0, MAX_OPTION_LEN);
    memset((void*)configFile->value, 0, MAX_OPTION_LEN);

    str = configFile->key;

    for (;;)
    {
        const char c = (char)fgetc(configFile->fileHandle);
        if (c == '\n' || feof(configFile->fileHandle))
        {
            break;
        }

        if (c == '#')
        {
            commentFlag = true;
        }

        if (!commentFlag)
        {
            if (c == '\t' || c == ' ')
            {
                if (*(configFile->key) != 0)
                {
                    str = configFile->value;
                    strLen = 0;
                }
                if (*(configFile->value) != 0)
                {
                    str = 0;
                    strLen = 0;
                }
            }
            else
            {
                if (str != 0)
                {
                    *str = c;
                    str++;
                    strLen++;
                }
            }
        }
    }

    if (*(configFile->value) != 0)
    {
        ret = true;
    }

    return ret;
}

bool EndOfFile(const config_file_struct* configFile)
{
    if(configFile)
    {
        if (feof(configFile->fileHandle))
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return true;
    }
}

