#pragma once

#include <iostream>
#include "idm_netsdk.h"
#include <string>
#include <fstream>
#include <sstream>
#include <json/json.h>
#include <WinSock2.h>

#define SAVESNAPPATH "./snap_pic"


int	SaveOnePicture(const IDM_DEV_ALARM_BUFFER_S & ast, std::string & filename);
void http_request_message(char *buffer, std::map<std::string, bool> &task_response_id);
void http_method_get(char *buffer, std::string &http_method);