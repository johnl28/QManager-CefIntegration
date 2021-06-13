#ifndef _CONF_HEADER_
#define _CONF_HEADER_
#include "json/json.h"
#include <mutex>

class Conf
{
public:
	Conf();
	~Conf();
	
	void SaveData();

	void SetValue(const char* index, const char* value);
	void SetValue(const char* index, int value);
	std::string GetString(const char* index) const;
	int GetInt(const char* index) const;

	Json::Value GetTable() const { return _root;  }

private:
	Json::Value _root;

};

#endif