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
which specifies the encoding preset, trading off compression efficiency against encoding speed. [0, 6] Big number means high quality but low encoder speed.

-o/--output <filename>    
which specifies output bitstream file name. For HEVC and VVC, the output file accord with Annex B specification. For AV1, the output file accord with OBU format.

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

Performance Comparison
-----------------

Offline Comparison(Xin26x vs X265)
-----------------

| INPUT YUV  BD-PSNR (dB)            | x265 veryslow | xin265 p1 | xin265 p4 | xin265 p6 | xin266 p1 | xin266 p4 | xin266 p6 |
| -----------------------------------| --------------| ----------| ----------| ----------| ----------| ----------| ----------|
| pedestrian_area                    | 0             | -0.37     | 0.34      | 0.52      | -0.1      | 0.67      | 0.85      |
| B_Kimono1_1920x1080_24             | 0             | -0.50     | 0.25      | 0.39      | -0.16     | 0.67      | 0.86      |
| B_ParkScene_1920x1080_24           | 0             | -0.47     | 0.39      | 0.59      | -0.13     | 0.84      | 1.07      |
| B_BasketballDrive_1920x1080_50     | 0             | -0.61     | 0.10      | 0.24      | -0.30     | 0.50      | 0.67      |
| B_BQTerrace_1920x1080_60           | 0             | -0.17     | 0.57      | 0.71      | 0.32      | 0.73      | 0.91      |
| B_Cactus_1920x1080_50              | 0             | -0.38     | 0.49      | 0.67      | 0.08      | 0.78      | 0.98      |
| Overall                            | 0             | -0.42     | 0.36      | 0.52      | -0.05     | 0.70      | 0.89      |

-----------------

| INPUT YUV  BD-RATE (%)             | x265 veryslow | xin265 p1 | xin265 p4 | xin265 p6 | xin266 p1 | xin266 p4 | xin266 p6 |
| -----------------------------------| --------------| ----------| ----------| ----------| ----------| ----------| ----------|
| pedestrian_area                    | 0             | 18.9      | -16       | -23.64    | 4.59      | -30.66    | -37.70    |
| B_Kimono1_1920x1080_24             | 0             | 22.3      | -10.08    | -15.29    | 6.73      | -26.11    | -32.06    |
| B_ParkScene_1920x1080_24           | 0             | 17.45     | -12.76    | -18.80    | 4.21      | -25.96    | -31.71    |
| B_BasketballDrive_1920x1080_50     | 0             | 33.35     | -4.83     | -11.48    | 15.27     | -23.00    | -30.22    |
| B_BQTerrace_1920x1080_60           | 0             | 20.4      | -48.67    | -57.33    | -29.89    | -58.49    | -66.82    |
| B_Cactus_1920x1080_50              | 0             | 24.56     | -25.44    | -34.66    | -4.48     | -40.14    | -48.87    |
| Overall                            | 0             | 22.83     | -19.63    | -26.87    | -0.6      | -34.06    | -41.23    |

Offline encoder parameters for xin26x and x265 are as follows: 
   
x265 veryslow  
x265.exe -o test.bin --input-res 1920x1080 --fps 30 --frames frames --bitrate kbitrate --tune psnr -p veryslow input.yuv

xin265 p1   
xin26x_test.exe -o test.bin -i input.yuv -w 1920 -h 1080 -f 30 -n frames -r 6 -b bitrate -p 1 -a 0

xin265 p4  
xin26x_test.exe -o test.bin -i input.yuv -w 1920 -h 1080 -f 30 -n frames -r 6 -b bitrate -p 4 -a 0

xin265 p6   
xin26x_test.exe -o test.bin -i input.yuv -w 1920 -h 1080 -f 30 -n frames -r 6 -b bitrate -p 6 -a 0

xin266 p1    
xin26x_test.exe -o test.bin -i input.yuv -w 1920 -h 1080 -f 30 -n frames -r 6 -b bitrate -p 1 -a 2

xin266 p4    
xin26x_test.exe -o test.bin -i input.yuv -w 1920 -h 1080 -f 30 -n frames -r 6 -b bitrate -p 4 -a 2

xin266 p6    
xin26x_test.exe -o test.bin -i input.yuv -w 1920 -h 1080 -f 30 -n frames -r 6 -b bitrate -p 6 -a 2

In above table, we set x265(veryslow) as a reference, others are compared with x265(veryslow). x265 is 2020/12/25 version.

Lowdelay Comparison(Xin265)
-----------------

Single thread for IPPPPPPPPP under suitbale bitrate

| B_BasketballDrive_1920x1080_50.yuv | FPS           | PSNR         | Encoder parameters|
| -----------------------------------| --------------| -------------| ------------------|
| xin26x                             | 20.68         | 36.929       | xin26x_test.exe -i B_BasketballDrive_1920x1080_50.yuv -a 0 -o test.265 -w 1920 -h 1080 -b 3000000 -f 30 -I 0 --wpp 0 --thread 1 --preset 3 --bframes 0 -n 500 |
| x265                               |  7.96         | 36.913       | x265.exe --preset fast -I -1 --input-res 1920x1080 --input B_BasketballDrive_1920x1080_50.yuv test.265 --bitrate 3000 --fps 30 --pools 1 --bframes 0 --frame-threads 0 --no-wpp --tune psnr -f 500 |

| pedestrian_area.yuv                | FPS           | PSNR         | Encoder parameters|
| -----------------------------------| --------------| -------------| ------------------|
| xin26x                             | 39.203        | 39.719       | xin26x_test.exe -i pedestrian_area.yuv -a 0 -o test.265 -w 1920 -h 1080 -b 2000000 -f 30 -I 0 --wpp 0 --thread 1 --preset 0 --bframes 0 |
| x265                               |  8.35         | 39.707       | x265.exe --preset fast -I -1 --input-res 1920x1080 --input pedestrian_area.yuv test.265 --bitrate 2000 --fps 30 --pools 1 --bframes 0 --frame-threads 0 --no-wpp --tune psnr |

| E_KristenAndSara_1280x720_60.yuv   | FPS           | PSNR         | Encoder parameters|
| -----------------------------------| --------------| -------------| ------------------|
| xin26x                             | 103.68        | 43.218       | xin26x_test.exe -i E_KristenAndSara_1280x720_60.yuv -a 0 -o test.265 -w 1280 -h 720 -b 1000000 -f 30 -I 0 --wpp 0 --thread 1 --preset 0 --bframes 0 |
| x265                               |  23.56        | 43.183       | x265.exe --preset fast -I -1 --input-res 1280x720 --input E_KristenAndSara_1280x720_60.yuv test.265 --bitrate 1000 --fps 30 --pools 1 --bframes 0 --frame-threads 0 --no-wpp --tune psnr |

| sc_desktop_1920x1080_60_8bit_420.yuv | FPS           | PSNR         | Encoder parameters|
| -------------------------------------| --------------| -------------| ------------------|
| xin26x                               | 39.48         | 40.082       | xin26x_test.exe -i sc_desktop_1920x1080_60_8bit_420.yuv -a 0 -o test.265 -w 1920 -h 1080 -b 2000000 -f 30 -I 0 --wpp 0 --thread 1 --preset 0 --bframes 0 -s 1 |
| x265                                 | 16.83         | 29.817       | x265.exe --preset fast -I -1 --input-res 1920x1080 --input sc_desktop_1920x1080_60_8bit_420.yuv test.265 --bitrate 2000 --fps 30 --pools 1 --bframes 0 --frame-threads 0 --no-wpp --tune psnr |

For a encoder, we normally take a tradeoff between picture quality and encoder sppeed. For a RTC encoder, performance on unit core or whole system capacity is more important. The performance of xin26x(HEVC) is x4 compared to x265 under single unit core. One important thing is that, the screen content coding performance of xin26x(HEVC) outperform x265 greatly both for speed and quality.	

I would like to note that, to make fair comparison to x265, the lookahead and cu-tree are both enabled. In addition, ABR is adopted in both xin26x and x265. When it comes to real RTC world, none of these coding tools can be applied, so there will be a inevitable loss of PSNR for RTC encoder. But the speed for a real RTC encoder should be faster than the data in the table.

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

Xin26x
-----------------
- 2 pass coding tool support.
- 16 and 32 picture gop size support.
- Preprocessing algorithm refactor.
- Rate contronl algorithm refine.
- Encoder structre refactor for offline coding consideration.
- 10 bit encoder support.
- Fast algorithm optimization.

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
- Screen content coding tools support.
- SIMD optimization.
- More VVC coding tools support.
- Memory usage reduction.
- Affine motion estimation.

About Xin26x
-----------------
The main purpose of Xin26x is unification. Xin26x is designed to unify different video standards and different implementations for same video standard.

In past 30 years, there are a lot of video standards released by different groups. Fortunately, these standards are all block-based motion compensation and transform hybrid scheme. Actually, it never changed since h.261 had been released. Based on that, Xin26x is designed to unify mainstream video standards into ONE encoder. Xin26x is a flexible video encoder framework to accommodate all the video standard so far on this planet. Currently, Xin26x have implemented h.265 and h.266. For AV1, Xin26x supports static picture encoding. Later, Xin26x is planning to accommodate EVC.

For same video standard, there are different implememations target to different application scenario. In my opinion, there are mainly 3 categories of encoder according to application latency.
| Latency                            | Scenario                           |
| -----------------------------------| ---------------------------------- |
| 200ms or blow (real time)          | real time video communication      |
| 1s-3s (low latency)                | live video boardcast               |
| 10s or above (offline)             | offline video application          |

Different encoding tools are adopted for different encoder implementions. For example, B frame is normally not adopted for RTC(Real-Time Communication) encoder. Frame parallelism is not used for RTC encoder. Rate control for RTC encoder is very strict. Besides, lookahead and mb-tree are normally not applied on first 2 encoders. Screen content coding tools are normally considered for a RTC encoder. In current industry, Openh264 is for real time video communication application. X264 is mainly for offline and low latency. Xin26x begins at a RTC video encoder. To accommodate offline video coding tools and video standard evolution, the encoder architecture has been greatly refactored. Also to make encoder work under different application scenarios, multiple different rate control modes are designed in Xin26x. 

I wish video coding standardization would take a revolution, break though current hybrid scheme, stop piling up computation for better compression rate in the future, then Xin26x reaches its end-of-life. Otherwise, Xin26x will keep catching up with the latest video coding standard, till I reach my end-of-life.

Basicly, Xin26x is purely a result of coding happiness pursuit. It is named after my 8 year old beautiful child. I would like her know how great her father is. After she grows up, I wish she would know how father loved her.

Any question, please contact pig.peppa@qq.com

