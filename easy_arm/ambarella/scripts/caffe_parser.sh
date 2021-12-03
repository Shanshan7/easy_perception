# Please read before you use
# necessary
# -p prototxt
# -m caffemodel
# -i RGB/YUV444/Y images list, --inputimages RGB/YUV444/Y images list
#                        Text file containing list of input images
#                        (RGB/YUV444/Y) to run DRA

# not necessary(preprocess)
# -is input image shape(NHWC)
# -it Axes information, --itranspose Axes information
#                       List of ints for permuting axes (same as numpy
#                       transpose)
# -c select the data format of outputs: 
#                       Best performance (act-force-fx8, coeff-force-fx8); 
#                       Best performance (act-force-fx16, coeff-force-fx16)
# -s Shape of the converted image, --shape Shape of the converted image
#                       Resize the image with the given shape:
#                       "plane,channel,height,width"(ex: -s 1,3,500,300)
# -m Mean Subtraction Value(s), --mean Mean Subtraction Value(s)
#                       Do mean subtraction for the input image after color
#                       format conversion. Can be a single value(all channels
#                       will use the same mean value) or 3 values(correspond
#                       to 3 channels).
# -iswap, --iswapchannels
#                       Flag to swap 0 and 2 channel order

# imgtobin.py
# -i Input, --input Input
#                       The input which can be a directory which contains the
#                       test images, a list(.txt) which lists the image paths,
#                       or a single image.
# -o Output directory of converted binary files, --outputdir Output directory of converted binary files
#                       The directory of the converted output binary. If it is
#                       not given, the output will be under the same folder as
#                       the input.
# -c Color format, --colorformat Color format
#                       Set the color format for the binary(0:BGR, 1:RGB,
#                       2:Reserved, 3:YUV444, 4:Grayscale, 5:YUV420p(YUV420
#                       planar), 6:YUV420sp(YUV420 semi-planar))
# -d Data format of the binary, --dataformat Data format of the binary
#                       Set the data format of the binary file(sign, bit, eo,
#                       eb). [sign]0:unsigned, 1:signed [bit]0:8bits, 1:16bits
#                       [eo]0-15 [eb]0:fixed, 4:float
# -cr Crop input image, --crop Crop input image
#                       Crop the image with the given crop window:
#                       "x(0~width-1), y(0~height-1), width, height"(ex: -cr
#                       100,0,200,150)
# -s Shape of the converted image, --shape Shape of the converted image
#                       Resize the image with the given shape:
#                       "plane,channel,height,width"(ex: -s 1,3,500,300)
# -m Mean Subtraction Value(s), --mean Mean Subtraction Value(s)
#                       Do mean subtraction for the input image after color
#                       format conversion. Can be a single value(all channels
#                       will use the same mean value) or 3 values(correspond
#                       to 3 channels).
# -sc Scale Value(s), --scale Scale Value(s)
#                       Divide input by scale value(s) after color format
#                       conversion(output=input/scale). Can be a single
#                       value(all channels will use the same mean value) or 3
#                       values(correspond to 3 channels).
# -p Pitch of the converted image, --pitch Pitch of the converted image
#                       Pad the image to the given pitch (ex: -p 352)
# -ca, --caffe          Use Caffe's imread & imresize functions(skimage) for
#                       preprocessing
set -v
modelDir="/home/lpj/Downloads/DVT2-sdk2.0/Ambarella_Toolchain_CNNGen_2.1.6_20190612/my_test/yolov3/model"
imageDir="/home/lpj/Downloads/DVT2-sdk2.0/Ambarella_Toolchain_CNNGen_2.1.6_20190612/my_test/yolov3/dra_img"
outDir="/home/lpj/Downloads/DVT2-sdk2.0/Ambarella_Toolchain_CNNGen_2.1.6_20190612/my_test/yolov3/out"
caffeNetName=yolov3
outNetName=yolov3

inputColorFormat=0
outputShape=1,3,352,640
outputLayerName="o:628|odf:fp32"
outputLayerName1="o:653|odf:fp32"
outputLayerName2="o:678|odf:fp32"
inputDataFormat=0,0,0,0

mean=0.0
scale=255.0

rm -r $outDir
mkdir $outDir
mkdir $outDir/dra_image_bin

#cuda-9.0
export PATH=/usr/local/cuda-9.0/bin${PATH:+:${PATH}}
export LD_LIBRARY_PATH=/usr/local/cuda-9.0/lib64${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}

#caffe
export PYTHONPATH=/home/lpj/github/caffe/python:$PYTHONPATH

ls $imageDir/*.jpg>$imageDir/img_list.txt

imgtobin.py -i $imageDir/img_list.txt \
            -o $outDir/dra_image_bin \
            -c $inputColorFormat \
            -d 0,0,0,0 \
            -s $outputShape

ls $outDir/dra_image_bin/*.bin > $outDir/dra_image_bin/dra_bin_list.txt

caffeparser.py -p $modelDir/$caffeNetName.prototxt \
               -m $modelDir/$caffeNetName.caffemodel \
               -i $outDir/dra_image_bin/dra_bin_list.txt \
               -o $outNetName \
               -of $outDir/out_parser \
               -im $mean -ic $scale  -it 0,1,2,3 \
               -iq -idf $inputDataFormat -odst $outputLayerName -odst $outputLayerName1 -odst $outputLayerName2 # -c act-force-fx16,coeff-force-fx16 

cd $outDir/out_parser;vas -auto -show-progress $outNetName.vas

rm -rf $outDir/cavalry
mkdir $outDir/cavalry
cavalry_gen -d $outDir/out_parser/vas_output/ \
            -f $outDir/cavalry/$outNetName.bin \
            -p $outDir/ \
            -v > $outDir/cavalry/cavalry_info.txt

