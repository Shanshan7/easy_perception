#include <iostream>

#include "h3cprocess.h"


int main()
{
    int rval = 0;

    H3CProcess h3cProcess;
    rval = h3cProcess.loginCamera();
    if(rval < 0)
    {
        std::cout << "连接摄像头失败" << std::endl;
    }
    else
    {
        std::cout << "连接摄像头成功" << std::endl;
    }

    h3cProcess.startEvent();
    while (1)
    {
        std::cout << "--------------" << std::endl;
    }
    
    h3cProcess.stopEvent();
    return rval;
}