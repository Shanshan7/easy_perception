#include "utils.h"

unsigned long get_current_time(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec*1000000 + tv.tv_usec);
}

long get_time_stamp(void)
{
	struct timeval tv;
	long time_stamp;

	gettimeofday(&tv, NULL);

	time_stamp = tv.tv_sec * 1000 + tv.tv_usec / 1000;	//Accurate to millisecond

	return time_stamp;
}

void ListPath(std::string const &path, std::vector<std::string> &paths) {
    paths.clear();
    struct dirent *entry;

    /*Check if path is a valid directory path. */
    struct stat s;
    lstat(path.c_str(), &s);
    if (!S_ISDIR(s.st_mode)) {
        fprintf(stderr, "Error: %s is not a valid directory!\n", path.c_str());
        exit(1);
    }

    DIR *dir = opendir(path.c_str());
    if (dir == nullptr) {
        fprintf(stderr, "Error: Open %s path failed.\n", path.c_str());
        exit(1);
    }

    while ((entry = readdir(dir)) != nullptr) {
 /*   
        if (entry->d_type == DT_REG || entry->d_type == DT_UNKNOWN) {
            string name = entry->d_name;
            cout << "name: "<<name<<endl;
            paths.push_back(name);
        }*/
        std::string name = entry->d_name;
        int type = (int)(entry->d_type);
       
       if(type != 8)
       {
         if((strcmp(name.c_str(), ".")!=0) && (strcmp(name.c_str(), "..")!=0) && (strcmp(name.c_str(), "results")!=0)) 
          {            
            #if(YUV420OPEN)
            if((strcmp(name.c_str(), "yuv")==0))
            {
                std::cout << "Dir name: "<<name<<std::endl;
                paths.push_back(name);  
            }
            #else
            if(strcmp(name.c_str(), "yuv")!=0)
            {
                std::cout << "Dir name: "<<name<<std::endl;
                paths.push_back(name);  
            }
            #endif
          }
        }
        
    }

    closedir(dir);
}
/**
 * @brief put image names to a vector
 *
 * @param path - path of the image direcotry
 * @param images - the vector of image name
 *
 * @return none
 */
void ListImages(std::string const &path, std::vector<std::string> &images) {
    images.clear();
    struct dirent *entry;

    /*Check if path is a valid directory path. */
    struct stat s;
    lstat(path.c_str(), &s);
    if (!S_ISDIR(s.st_mode)) {
        fprintf(stderr, "Error: %s is not a valid directory!\n", path.c_str());
        exit(1);
    }

    DIR *dir = opendir(path.c_str());
    if (dir == nullptr) {
        fprintf(stderr, "Error: Open %s path failed.\n", path.c_str());
        exit(1);
    }

    while ((entry = readdir(dir)) != nullptr) {
        if (entry->d_type == DT_REG || entry->d_type == DT_UNKNOWN) {
            std::string name = entry->d_name;
            std::string ext = name.substr(name.find_last_of(".") + 1);
            if ((ext == "JPEG") || (ext == "jpeg") || (ext == "JPG") ||
                (ext == "jpg") || (ext == "PNG") || (ext == "png") || (ext == "420") || (ext == "bin")) {
                images.push_back(name);
            }
        }
    }

    closedir(dir);
}

