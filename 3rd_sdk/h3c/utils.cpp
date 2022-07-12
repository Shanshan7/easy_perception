#include "utils.h"

int	SaveOnePicture(const IDM_DEV_ALARM_BUFFER_S & ast, std::string & filename)
{
	std::ofstream pic_file;
	// std::string filename = std::to_string(m_alarmcount) + std::string("__") + std::to_string(innerindex) + name + m_newalarmtime + ".jpg";
	// std::string filename = std::to_string(innerindex) + name + ".jpg";
	// filename.insert(0, "D:\\");
	pic_file.open(filename, std::ios::binary);
	if (!pic_file.is_open())
	{
		printf("Error When Open file %s for event", filename.c_str());
		return -1;

	}
	// dohaveimage[m_alarmcount] += (filename + ";");

	pic_file.write(ast.pBuffer, ast.ulBufferSize);

	pic_file.close();
	// printf("Saved AlarmPic file %s for event %s Now saved %s ", filename.c_str(), name.c_str(), dohaveimage[m_alarmcount].c_str());
	return 0;
}

void http_request_message(char *buffer, std::map<std::string, bool> &task_response_id)
{
	const char *d = "\r\n\r\n";
	char *p;
	p = strtok(buffer, d);
	while(p)
	{
		if(strstr(p, "{") != NULL)
		{
			Json::Reader reader;
			Json::Value root;

			if (reader.parse(p, root))
			{
				// Json::Value::Members members = root.getMemberNames();
				// for (Json::Value::Members::iterator iter = members.begin(); iter != members.end(); ++iter)
				// {
				// 	printf("member: %s\n", (*iter));
				// }
				if(root.isMember("person_attribute"))
				{
					task_response_id["person_attribute"] = root["person_attribute"].asBool();
				}
			    if (root.isMember("face_attribute")) {
					task_response_id["face_attribute"] = root["face_attribute"].asBool();
				}
				if (root.isMember("motor_vehicle_attribute")) {
					task_response_id["motor_vehicle_attribute"] = root["motor_vehicle_attribute"].asBool();
				}
				if (root.isMember("motor_vehicle_attribute")) {
					task_response_id["nomotor_vehicle_attribute"] = root["nomotor_vehicle_attribute"].asBool();
				}
				std::cout << "Reading Complete!" << std::endl;
			}
			else
			{
				std::cout << "parse error\n" << std::endl;
			}
		}
		p=strtok(NULL, d); // break
	}
}

void http_method_get(char *buffer, std::string &http_method)
{
	// get http method
	if (strstr(buffer, "GET") != NULL)
	{
		http_method = "GET";
	}
	else if(strstr(buffer, "POST") != NULL)
	{
		http_method = "POST";
	}
	else if(strstr(buffer, "OPTIONS") != NULL)
	{
		http_method = "OPTIONS";
	}
	else if(strstr(buffer, "PUT") != NULL)
	{
		http_method = "PUT";
	}
	else
	{
		printf("Unknown request!\n");
	}
}
