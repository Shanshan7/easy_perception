# Introduction

Cyber RT is an open source, high performance runtime framework designed specifically for autonomous driving scenarios. Based on a centralized computing model, it is greatly optimized for high concurrency, low latency, and high throughput in autonomous driving.

During the last few years of the development of autonomous driving technologies, we have learned a lot from our previous experience with Apollo. The industry is evolving and so is Apollo. Going forward, Apollo has already moved from development to productization, with volume deployments in the real world, we see the demands for the highest level of robustness and performance. That’s why we spent years building and perfecting Apollo Cyber RT, which addresses that requirements of autonomous driving solutions.

Key benefits of using Cyber RT:

- Accelerate development
  + Well defined task interface with data fusion
  + Array of development tools
  + Large set of sensor drivers
- Simplify deployment
  + Efficient and adaptive message communication
  + Configurable user level scheduler with resource awareness
  + Portable with fewer dependencies
- Empower your own autonomous vehicles
  + The default open source runtime framework
  + Building blocks specifically designed for autonomous driving
  + Plug and play your own AD system

# Ubuntu Install
- cmake version >= 3.12
- sudo apt install libasio-dev libtinyxml2-dev
- sudo apt-get install libncurses5-dev
- sudo apt-get install autoconf automake libtool

- 安装protobuffer: 
  + https://github.com/protocolbuffers/protobuf/releases 下载 3.3.0版本

- 安装python3.5-dev:
  + sudo apt-get install python3.5-dev

- 安装googletest: 
  + https://github.com/google/googletest 下载1.10.0版本

- 下载安装gflags, 
  + https://github.com/gflags/gflags 下载2.2.0版本（安装动态库）

- 下载glog源码，编译安装：
  + https://github.com/google/glog.git 下载0.3.5版本（安装动态库）

- 下载poco源码，编译安装：
  + https://github.com/pocoproject/poco 下载1.8.0版本

- 下载Foonathan memory源码，编译安装：
  + https://github.com/eProsima/foonathan_memory_vendor

- 下载Fast-CDR源码，编译安装：
  + https://github.com/ApolloAuto/Fast-CDR 下载apollo分支代码

- 下载Fast-RTPS源码，编译安装：
  + https://github.com/ApolloAuto/Fast-RTPS 下载apollo分支代码

备注：编译可能遇到报错，std::function相关，打开对应文件#include <functional>

# Documents

* [Apollo Cyber RT Quick Start](https://github.com/ApolloAuto/apollo/tree/master/docs/cyber/CyberRT_Quick_Start.md): Everything you need to know about how to start developing your first application module on top of Apollo Cyber RT.

* [Apollo Cyber RT Developer Tools](https://github.com/ApolloAuto/apollo/tree/master/docs/cyber/CyberRT_Developer_Tools.md): Detailed guidance on how to use the developer tools from Apollo Cyber RT.

* [Apollo Cyber RT API for Developers](https://github.com/ApolloAuto/apollo/tree/master/docs/cyber/CyberRT_API_for_Developers.md): A comprehensive guide to explore all the APIs of Apollo Cyber RT, with many concrete examples in source code.

* [Apollo Cyber RT FAQs](https://github.com/ApolloAuto/apollo/tree/master/docs/FAQs/CyberRT_FAQs.md): Answers to the most frequently asked questions about Apollo Cyber RT.

More documents to come soon!
