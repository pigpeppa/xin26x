/***************************************************************************//**
*
* @file          xin_app_enc.c
* @authors       shennung
* @brief         xin26x encoder test client.
* @copyright     (c) 2020, shennung <shennung@hotmail.com>  All rights reserved
*
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include "xin_typedef.h"
#include "xin26x_params.h"
#include "xin_encoder_option.h"
#include "xin26x_encoder_api.h"
#if defined(_MSC_VER)
#include <windows.h>
#elif defined(__linux__) || defined(__APPLE__)
#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#endif

#if defined(_MSC_VER)
#define fseeko  _fseeki64
#define ftello  _ftelli64
#endif

static int ReadFrame (
    FILE           *inputFile,
    xin_frame_desc *inputFrame)
{
    size_t readCount;

    readCount = fread (
                    inputFrame->yuvBuf[0],
                    sizeof(PIXEL),
                    inputFrame->lumaHeight*inputFrame->lumaWidth,
                    inputFile);

    if (readCount != inputFrame->lumaHeight*inputFrame->lumaWidth)
    {
        printf("File read error\n");

        return XIN_FAIL;
    }

    readCount = fread (
                    inputFrame->yuvBuf[1],
                    sizeof(PIXEL),
                    inputFrame->lumaHeight*inputFrame->lumaWidth/4,
                    inputFile);

    if (readCount != inputFrame->lumaHeight*inputFrame->lumaWidth/4)
    {
        printf("File read error\n");

        return XIN_FAIL;
    }

    readCount = fread (
                    inputFrame->yuvBuf[2],
                    sizeof(PIXEL),
                    inputFrame->lumaHeight*inputFrame->lumaWidth/4,
                    inputFile);

    if (readCount != inputFrame->lumaHeight*inputFrame->lumaWidth/4)
    {
        printf("File read error\n");

        return XIN_FAIL;
    }

    return XIN_SUCCESS;

}

static void WriteFrame (
    FILE           *reconFile,
    xin_frame_desc *reconFrame)
{
    UINT32  rowIdx;
    PIXEL   *yBuf;
    PIXEL   *uBuf;
    PIXEL   *vBuf;

    yBuf = reconFrame->yuvBuf[0];
    uBuf = reconFrame->yuvBuf[1];
    vBuf = reconFrame->yuvBuf[2];

    for (rowIdx = 0; rowIdx < reconFrame->lumaHeight; rowIdx++)
    {
        fwrite (
            yBuf,
            1,
            reconFrame->lumaWidth,
            reconFile);

        yBuf += reconFrame->lumaStride;
    }

    for (rowIdx = 0; rowIdx < reconFrame->lumaHeight/2; rowIdx++)
    {
        fwrite (
            uBuf,
            1,
            reconFrame->lumaWidth/2,
            reconFile);

        uBuf += reconFrame->chromaStride;
    }

    for (rowIdx = 0; rowIdx < reconFrame->lumaHeight/2; rowIdx++)
    {
        fwrite (
            vBuf,
            1,
            reconFrame->lumaWidth/2,
            reconFile);

        vBuf += reconFrame->chromaStride;
    }

}

static void GetFrameCount (
    FILE   *inputFile,
    UINT32 width,
    UINT32 height,
    UINT32 *frameCount)
{
    UINT64  fileSize;
    UINT32  frameSize;

    frameSize = 3*width*height/2;

    fseeko (
        inputFile,
        0,
        SEEK_END);

    fileSize = ftello (inputFile);

    fseeko (
        inputFile,
        0,
        SEEK_SET);

    *frameCount = (UINT32)(fileSize/frameSize);

}


int main(int argc, char **argv)
{
    encoder_option_struct *encoderOption;
    XIN_HANDLE            encoderHandle;
    UINT32                inputFrameIdx;
    UINT32                trailingFrame;
    UINT32                encodedFrame;
    xin26x_params         *config;
    xin_frame_desc        inputFrame;
    xin_frame_desc        reconFrame[4];
    UINT32                reconFrameNum;
    UINT32                reconFrameIdx;
    xin_out_buf_desc      outputBuffer;
#if defined(_MSC_VER)
    LARGE_INTEGER         ticksBefore, ticksAfter;
    LARGE_INTEGER         cpuFrequency;
#elif defined(__linux__) || defined(__APPLE__)
    struct timeval        ticksBefore, ticksAfter;
#endif
    UINT64                oneFrameTicks;
    UINT64                totalFrameTicks;
    xin26x_dynamic_params dynParam;
    UINT64                totalBitSize;
    UINT32                frameToBeEncoded;

    totalFrameTicks = 0;
    totalBitSize    = 0;

    encoderOption = CreateEncoderOption (argc, argv);

    if (encoderOption)
    {
        PrintEncoderOption (encoderOption);

        config = &encoderOption->xinConfig;

        GetFrameCount (
            encoderOption->inputFileHandle,
            config->inputWidth,
            config->inputHeight,
            &frameToBeEncoded);

        if ((frameToBeEncoded < config->frameToBeEncoded) || (config->frameToBeEncoded == 0))
        {
            config->frameToBeEncoded = frameToBeEncoded;
        }

        if (Xin26xEncoderCreate (&encoderHandle, config))
        {
            printf("Fail to create encoder\n");

            return -1;
        }

        inputFrame.yuvBuf[0] = malloc (config->inputWidth*config->inputHeight);
        inputFrame.yuvBuf[1] = malloc (config->inputWidth*config->inputHeight/4);
        inputFrame.yuvBuf[2] = malloc (config->inputWidth*config->inputHeight/4);

        inputFrame.lumaHeight   = config->inputHeight;
        inputFrame.lumaWidth    = config->inputWidth;
        inputFrame.lumaStride   = config->inputWidth;
        inputFrame.chromaStride = config->inputWidth/2;

        encodedFrame  = 0;
        trailingFrame = 0;

        printf("Start coding...\n");

        for (inputFrameIdx = 0; inputFrameIdx < config->frameToBeEncoded; inputFrameIdx++)
        {
            if (ReadFrame (
                        encoderOption->inputFileHandle,
                        &inputFrame) != XIN_SUCCESS)
            {
                break;
            }

#if defined(_MSC_VER)
            QueryPerformanceCounter (&ticksBefore);
#elif defined(__linux__) || defined(__APPLE__)
            gettimeofday (&ticksBefore, NULL);
#endif

            Xin26xEncodeFrame (
                encoderHandle,
                &inputFrame,
                &outputBuffer);

#if defined(_MSC_VER)
            QueryPerformanceFrequency(&cpuFrequency);
            QueryPerformanceCounter (&ticksAfter);
            oneFrameTicks = (((__int64)(ticksAfter.QuadPart - ticksBefore.QuadPart))*1000)/(cpuFrequency.QuadPart);
#elif defined(__linux__) || defined(__APPLE__)
            gettimeofday (&ticksAfter, NULL);
            oneFrameTicks = ((ticksAfter.tv_sec  - ticksBefore.tv_sec) * 1000000 + (ticksAfter.tv_usec - ticksBefore.tv_usec))/1000;
#endif
            totalFrameTicks += oneFrameTicks;
            totalBitSize    += outputBuffer.bytesGenerate*8;

            if (config->statLevel >= XIN26X_STAT_PIC)
            {
                printf ("frame %d, encoded %d bytes, time %d.\n", inputFrameIdx, outputBuffer.bytesGenerate, (UINT32)oneFrameTicks);
            }

            if ((encoderOption->reconFileHandle) && (outputBuffer.bytesGenerate > 0))
            {
                Xin26xGetReconFrame (
                    encoderHandle,
                    reconFrame,
                    &reconFrameNum);

                for (reconFrameIdx = 0; reconFrameIdx < reconFrameNum; reconFrameIdx++)
                {
                    WriteFrame (
                        encoderOption->reconFileHandle,
                        reconFrame + reconFrameIdx);
                }
            }

            if (outputBuffer.bytesGenerate > 0)
            {
                encodedFrame++;

                fwrite (
                    outputBuffer.bitsBuf,
                    1,
                    outputBuffer.bytesGenerate,
                    encoderOption->outputFileHandle);
            }

        }

        // Flush trailing frames
        do
        {

#if defined(_MSC_VER)
            QueryPerformanceCounter (&ticksBefore);
#elif defined(__linux__) || defined(__APPLE__)
            gettimeofday (&ticksBefore, NULL);
#endif

            Xin26xEncodeFrame (
                encoderHandle,
                NULL,
                &outputBuffer);

#if defined(_MSC_VER)
            QueryPerformanceFrequency(&cpuFrequency);
            QueryPerformanceCounter (&ticksAfter);
            oneFrameTicks = (((__int64)(ticksAfter.QuadPart - ticksBefore.QuadPart))*1000)/(cpuFrequency.QuadPart);
#elif defined(__linux__) || defined(__APPLE__)
            gettimeofday (&ticksAfter, NULL);
            oneFrameTicks = ((ticksAfter.tv_sec  - ticksBefore.tv_sec) * 1000000 + (ticksAfter.tv_usec - ticksBefore.tv_usec))/1000;
#endif
            totalFrameTicks += oneFrameTicks;
            totalBitSize    += (outputBuffer.bytesGenerate > 0) ? outputBuffer.bytesGenerate*8 : 0;

            if (config->statLevel >= XIN26X_STAT_PIC)
            {
                printf ("frame %d, encoded %d bytes, time %I64d.\n", trailingFrame+config->frameToBeEncoded, outputBuffer.bytesGenerate, oneFrameTicks);
            }

            if ((encoderOption->reconFileHandle) && (outputBuffer.bytesGenerate > 0))
            {
                Xin26xGetReconFrame (
                    encoderHandle,
                    reconFrame,
                    &reconFrameNum);

                for (reconFrameIdx = 0; reconFrameIdx < reconFrameNum; reconFrameIdx++)
                {
                    WriteFrame (
                        encoderOption->reconFileHandle,
                        reconFrame + reconFrameIdx);
                }

            }

            if (outputBuffer.bytesGenerate > 0)
            {
                encodedFrame++;

                fwrite (
                    outputBuffer.bitsBuf,
                    1,
                    outputBuffer.bytesGenerate,
                    encoderOption->outputFileHandle);
            }

            trailingFrame++;

        }
        while (outputBuffer.bytesGenerate >= 0);

        //printf ("Encoded frame %d, avg time per frame %I64d.\n", config->frameToBeEncoded, totalFrameTicks/config->frameToBeEncoded);

        if (config->calcPsnr)
        {
            dynParam.optionId = XIN26X_OPTION_GET_PSNR;

            Xin26xControlOption (
                encoderHandle,
                &dynParam);

            printf ("PSNR Y:%2.4f, U:%2.4f, V:%2.4f PSNR YUV:%2.4f.\n", dynParam.psnrYuv[0], dynParam.psnrYuv[1], dynParam.psnrYuv[2],
                    (dynParam.psnrYuv[0]*4 + dynParam.psnrYuv[1] + dynParam.psnrYuv[2])/6);
        }

        printf("Complete coding.\n");

        if (config->statLevel >= XIN26X_STAT_SEQ)
        {
            printf("%d frames encoded, coding speed fps:%3.4f bitrate: %4.2f kbps.\n", config->frameToBeEncoded, 1000.0/((double)totalFrameTicks/(double)config->frameToBeEncoded), ((double)totalBitSize)/((double)config->frameToBeEncoded/(double)config->frameRate)/1000.0);
        }

        Xin26xEncoderDelete (
            encoderHandle);

        free (inputFrame.yuvBuf[0]);
        free (inputFrame.yuvBuf[1]);
        free (inputFrame.yuvBuf[2]);

        DeleteEncoderOption (
            encoderOption);

    }

    return 0;

}

