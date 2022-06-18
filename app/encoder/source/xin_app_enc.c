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
#if _WIN32
#include <sys/types.h>
#include <sys/timeb.h>
#include <io.h>
#include <fcntl.h>
#else
#include <sys/time.h>
#endif


#if defined(_MSC_VER)
#define fseeko  _fseeki64
#define ftello  _ftelli64
#endif



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
        return XIN_FAIL;
    }

    readCount = fread (
                    inputFrame->yuvBuf[1],
                    sizeof(PIXEL),
                    inputFrame->lumaHeight*inputFrame->lumaWidth/4,
                    inputFile);

    if (readCount != inputFrame->lumaHeight*inputFrame->lumaWidth/4)
    {   
        return XIN_FAIL;
    }

    readCount = fread (
                    inputFrame->yuvBuf[2],
                    sizeof(PIXEL),
                    inputFrame->lumaHeight*inputFrame->lumaWidth/4,
                    inputFile);

    if (readCount != inputFrame->lumaHeight*inputFrame->lumaWidth/4)
    {   
        return XIN_FAIL;
    }

    return XIN_SUCCESS;

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

int64_t Xin26xGetTime ( )
{
#if _WIN32
    struct timeb time;
    ftime(&time);
    return ((int64_t)time.time * 1000 + (int64_t)time.millitm) * 1000;
#else
    struct timeval time;
    gettimeofday(&time, NULL);
    return (int64_t)time.tv_sec * 1000000 + (int64_t)time.tv_usec;
#endif
}

int main(int argc, char **argv)
{
    encoder_option_struct *encoderOption;
    XIN_HANDLE            encoderHandle;
    XIN_HANDLE            fileReadHandle;
    UINT32                inputFrameIdx;
    UINT32                trailingFrame;
    UINT32                encodedFrame;
    xin26x_params         *config;
    xin_frame_desc        inputFrame;
    xin_frame_desc        reconFrame[4];
    UINT32                reconFrameNum;
    UINT32                reconFrameIdx;
    xin_out_buf_desc      outputBuffer;
    SINT64                startTime;
    SINT64                endTime;
    double                encoderTime;
    UINT64                totalBitSize;
    UINT32                frameToBeEncoded;
    SINT32                passIdx;

    totalBitSize  = 0;
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

        if (Xin26xReadFrameCreate (&fileReadHandle, ReadFrame, config, encoderOption->inputFileHandle))
        {
            printf("Fail to create file reader\n");

            return -1;
        }

        printf("Start coding...\n");
        
        encodedFrame  = 0;
        trailingFrame = 0;
        startTime     = Xin26xGetTime ();

        for (passIdx = 0; passIdx <= config->twoPassEncoder; passIdx++)
        {
            for (inputFrameIdx = 0; inputFrameIdx < config->frameToBeEncoded; inputFrameIdx++)
            {
                if (passIdx == 0)
                {
                    Xin26xReadFrame (
                        fileReadHandle, 
                        &inputFrame);
                }

                Xin26xEncodeFrame (
                    encoderHandle,
                    (passIdx == 0) ? &inputFrame : NULL,
                    &outputBuffer);

                totalBitSize += outputBuffer.bytesGenerate*8;

                if (config->statLevel >= XIN26X_STAT_PIC)
                {
                    printf ("frame %d, encoded %d bytes.\n", inputFrameIdx, outputBuffer.bytesGenerate);
                }

                if ((encoderOption->reconFileHandle) && (outputBuffer.bytesGenerate > 0) && (passIdx == config->twoPassEncoder))
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

                if ((outputBuffer.bytesGenerate > 0) && (passIdx == config->twoPassEncoder))
                {
                    encodedFrame++;

                    fwrite (
                        outputBuffer.bitsBuf,
                        1,
                        outputBuffer.bytesGenerate,
                        encoderOption->outputFileHandle);
                }

            }

            // Last pass
            if (config->twoPassEncoder == passIdx)
            {
                // Flush traling frames
                do
                {
                    Xin26xEncodeFrame (
                        encoderHandle,
                        NULL,
                        &outputBuffer);

                    totalBitSize += (outputBuffer.bytesGenerate > 0) ? outputBuffer.bytesGenerate*8 : 0;

                    if (config->statLevel >= XIN26X_STAT_PIC)
                    {
                        printf ("frame %d, encoded %d bytes.\n", trailingFrame+config->frameToBeEncoded, outputBuffer.bytesGenerate);
                    }

                    if ((encoderOption->reconFileHandle) && (outputBuffer.bytesGenerate > 0) && (passIdx == config->twoPassEncoder))
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

                    if ((outputBuffer.bytesGenerate > 0) && (passIdx == config->twoPassEncoder))
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

            }

        }

        endTime     = Xin26xGetTime ();
        encoderTime = (double)(endTime - startTime) / 1000000;

        if (config->statLevel >= XIN26X_STAT_SEQ)
        {
            printf ("%d frames encoded, coding speed fps:%3.4f bitrate: %4.2f kbps.\n", config->frameToBeEncoded, (double)(config->frameToBeEncoded) / encoderTime, ((double)totalBitSize)/((double)config->frameToBeEncoded/(double)config->frameRate)/1000.0);
        }

        printf("Complete coding.\n");

		Xin26xReadFrameDelete (
			fileReadHandle);

        Xin26xEncoderDelete (
            encoderHandle);

        DeleteEncoderOption (
            encoderOption);

        free (encoderOption);

    }

    return 0;

}

