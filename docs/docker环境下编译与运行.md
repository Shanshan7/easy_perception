docker环境下编译与运行
================================

### easy_arm 编译
1. 进入在workspace镜像中
2. 下载 [amba sdk](http://118.31.19.101:8080/software/amba/cv22_linux_sdk.tar.gz), 并解压
2. 
   ```
   git clone https://github.com/MiniBullLab/easy_perception.git
   ```
3. 
   ```
   cd easy_perception/

   git checkout develop
   
   cd easy_arm/ambarella

   mkdir build

   cd build/

   cmake ..

   make

   ```
3. 在build目录下，将相应的应用程序拷贝到amba平台上运行