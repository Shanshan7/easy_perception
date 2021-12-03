set -v
modelDir="/home/lpj/Downloads/DVT2-sdk2.0/my_test/fgsegv2/model"
imageDir="/home/lpj/Downloads/DVT2-sdk2.0/my_test/fgsegv2/image"
outDir="/home/lpj/Downloads/DVT2-sdk2.0/my_test/fgsegv2/out"
tensorflowName=led1
outNetName=led1

inputColorFormat=1
outputShape=1,3,512,440
inputLayerName="i:net_input|is:1,512,440,3"
compare_inputLayerName="i:net_input=./out/dra_img/dra_list.txt|iq|idf:0,0,0,0|is:1,512,440,3"
temp_outputLayerName="conv2d_10/Sigmoid"
outputLayerName="o:conv2d_10/Sigmoid|ot:0,3,1,2|odf:fp32"
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

ls $imageDir/*.jpeg>$imageDir/img_list.txt

imgtobin.py -i $imageDir/img_list.txt \
            -o $outDir/dra_image_bin \
            -c $inputColorFormat \
            -d 0,0,0,0 \
            -s $outputShape

ls $outDir/dra_image_bin/*.bin > $outDir/dra_image_bin/dra_bin_list.txt

graph_surgery.py tf -p $modelDir/$tensorflowName.pb \
                    -o $modelDir/${tensorflowName}_temp.pb \
                    -isrc $inputLayerName \
                    -on $temp_outputLayerName \
                    -t ConstantifyShapes

CUDA_VISIBLE_DEVICES=-1
tfparser.py -p $modelDir/${tensorflowName}_temp.pb \
               -i $outDir/dra_image_bin/dra_bin_list.txt \
               -o $outNetName \
               -of $outDir/out_parser \
               -is $outputShape \
               -im $mean -ic $scale \
               -iq -idf $inputDataFormat \
               -odst $outputLayerName \
               -c act-allow-fp16,coeff-force-fx16

cd $outDir/out_parser;vas -auto -show-progress $outNetName.vas

#layer_compare.py tf -p $modelDir/${tensorflowName}_temp.pb -isrc "i:net_input=${outDir}/dra_image_bin/dra_bin_list.txt|iq|idf:0,0,0,0|is:1,3,512,440" -c act-allow-fp16,coeff-force-fx16 -v $outDir/out_parser -n $outNetName -o out

rm -rf $outDir/cavalry
mkdir $outDir/cavalry
cavalry_gen -d $outDir/out_parser/vas_output/ \
            -f $outDir/cavalry/$outNetName.bin \
            -p $outDir/ \
            -v > $outDir/cavalry/cavalry_info.txt

