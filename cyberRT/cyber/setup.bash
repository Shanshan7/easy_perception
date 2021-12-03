export CYBER_PATH=$(cd "$(dirname "${BASH_SOURCE[0]}")/../" && pwd)
binary_path="${CYBER_PATH}/bin"
library_path="${CYBER_PATH}/lib"
third_path="${CYBER_PATH}/third_party"
cyber_tool_path="${binary_path}/cyber/tools"
recorder_path="${cyber_tool_path}/cyber_recorder"
monitor_path="${cyber_tool_path}/cyber_monitor"
launch_path="${cyber_tool_path}/cyber_launch"
channel_path="${cyber_tool_path}/cyber_channel"
node_path="${cyber_tool_path}/cyber_node"
service_path="${CYBER_PATH}/tools/cyber_service"
PYTHON_LD_PATH="${library_path}/cyber/python/internal"
#qt_path=/usr/local/Qt5.5.1/5.5/gcc_64

#protobuf
export LD_LIBRARY_PATH=${third_path}/protobuf/lib:$LD_LIBRARY_PATH
export PATH=${third_path}/protobuf/bin:$PATH
chmod +x ${third_path}/protobuf/bin/*

#fastcdr
export LD_LIBRARY_PATH=${third_path}/fastcdr/lib:$LD_LIBRARY_PATH

#foonathan_memory
export LD_LIBRARY_PATH=${third_path}/foonathan_memory/lib:$LD_LIBRARY_PATH

#fastrtps
export LD_LIBRARY_PATH=${third_path}/fastrtps/lib:$LD_LIBRARY_PATH

#Poco
export LD_LIBRARY_PATH=${third_path}/Poco/lib:$LD_LIBRARY_PATH

#gflags
export LD_LIBRARY_PATH=${third_path}/gflags/lib:$LD_LIBRARY_PATH

#glog
export LD_LIBRARY_PATH=${third_path}/glog/lib:$LD_LIBRARY_PATH

#gtest
export LD_LIBRARY_PATH=${third_path}/gtest/lib:$LD_LIBRARY_PATH

export LD_LIBRARY_PATH=${library_path}:$LD_LIBRARY_PATH
export PATH=${binary_path}:${recorder_path}:${monitor_path}:${launch_path}:${channel_path}:${node_path}:${service_path}:$PATH
export PYTHONPATH=${PYTHON_LD_PATH}:${library_path}/cyber/python:$PYTHONPATH

export CYBER_DOMAIN_ID=80
export CYBER_IP=127.0.0.1

export GLOG_log_dir=/home/$USER/cyber_data/log
export GLOG_alsologtostderr=0
export GLOG_colorlogtostderr=1
export GLOG_minloglevel=0

export sysmo_start=0

# for DEBUG log
#export GLOG_minloglevel=-1
#export GLOG_v=4

source ${CYBER_PATH}/bin/cyber/tools/cyber_tools_auto_complete.bash
