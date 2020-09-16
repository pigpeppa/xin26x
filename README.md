xin26x
=======
- Xin26x is an univeral video encoder framework, which can accommodate all video coding standard on this planet so far.
- It is optimized for real time video communication, live streaming application and offline video encoding.
- It currently supports HEVC, AV1(I frame), VVC video coding standards.
- It is a very high performance encoder.
- It is well-structured for understandability, maintainability and scalability.
- It is written from scratch and has a plain C API to enable a simple integration into other software.
- It is very small footprint encoder, it takes very small memory requirement under restricted mode.

Encoder Usage
----------
xin26x_test.exe -i input.yuv -w 1280 -h 720 -f 30 -a 2 -p 1 -r 1 -b 2000000 -o test.bin   
xin26x_test.exe -i input.yuv -w 1280 -h 720 -f 30 -a 2 -p 1 -r 0 -q 32 -o test.bin

Building Project
----------
- Our Windows build use CMake and Visual Studio. The workable version is CMake3.5 or above and Visual Studio 2013.
- Run make-solutions.bat under build folder, building projects will be generated under this folder. 
- In addition, we put a pre-built win64 exe under folder testbin, you can run it on win64 without build.

Basic Parameters
----------
-i/--input <filename>    
which specifies input YUV file name. Currently we accept YUV 420 video format.

-a/--algmode <integer>    
which specifies algorithm mode. 0: H265 1: AV1 2: H266

-w/--width <integer>    
which specifies input YUV luma width.

-h/--height <integer>    
which specifies input YUV luma height.

-f/--framerate <float>    
which specifies the frame rate of the input video.

-t/--temporallayer <integer>    
which specifies temporal layer number. It works under all P frame sequence.

-p/--preset <integer>    
which specifies the encoding preset, trading off compression efficiency against encoding speed. 0: superfast, 1 veryfast, 2: fast, 3: medium, 4: slow 5: veryslow.

-o/--output <filename>    
which specifies output bitstream file name.

-R/--recon <filename>    
which specifies reconstruction YUV file name. It is not a must input parameter.

-r/--ratecontrol <integer>    
which specifies rate control mode. 0: rate control is off (fixed qp), 1: rate control is on.

-b/--bitrate <integer>    
which specifies target bitrate. It works under rate control is on.

-q/--qp <integer>    
which specifies quantization paramter. It works under rate control is off.

-n/--framenumber <integer>    
which specifies how many frames to be encoded.

-B/--bframes <integer>    
which specifies how many B frames in a gop. Currently, we support 1, 3 and 7.

-W/--wpp <integer>    
which specifies whether wavefront parallel processing is enabled. 0: disable 1: enable.

-F/--fpp <integer>    
which specifies whether frame parallel processing is enabled. 0: disable 1: enable.

-T/--thread <integer>    
which specifies thread number in thread pool. It is decided by local system if this number is 0.

--refframes <integer>    
which specifies how many frames is used for reference.

--signbithide <integer>    
which specifies whether sign bit hidden coding tool is used in encoder. 0: off 1: on.

-d/--rdoq <integer>    
which specifies whether rate distortion optimization quantization is enabled. 0: disable 1: enable.

Api Usage
-----------------
Please refer to xin_app_enc.c.

Processor Support
-----------------
- Intel x86 with AVX2 support.

Known Issues
-----------------
- Max resolution we support is 3860x2160.
- Max temporal layer we support is 3.
- For AV1, the outputted frame are all IDR. Please ignore encoder speed, currently AV1 encoder is pure C code, without SIMD or multi-thread optimization.
- Under evaluation mode, only ABR is supported.
- For VVC, please use vtm 10 to decode the bitstream. You can download vtm 10 from https://vcgit.hhi.fraunhofer.de/jvet/VVCSoftware_VTM/-/releases/VTM-10.0 

Todo Lists
-----------------

HEVC
-----------------
- Minor structure adjustment.
- Local algorithm refinement.

AV1
-----------------
- P and B frame support.
- General video coding tools support.
- SIMD optimization.

VVC
-----------------
- Transform skip and MTS support.
- Screen content coding tools support.
- Ternary cu partition support.
- Adaptive loop filter support (I will evaluate the pros and cons to accommodate ALF in Xin26x).
- SIMD optimization.

Any question, please contact pig.peppa@qq.com
