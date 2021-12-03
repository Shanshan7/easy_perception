#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <vector>
#include <string.h>
#include <math.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/time.h>
#include <thread>
#include <mutex>
#include <future>


unsigned long get_current_time(void);

long get_time_stamp(void);

void ListPath(std::string const &path, std::vector<std::string> &paths);

void ListImages(std::string const &path, std::vector<std::string> &images);

#endif //__UTILS_H__