#include <map>
#include <fstream>
#include "Config.h"
#include "json/json.h"

static std::mutex mtx;

Conf::Conf()
{
	std::ifstream file("config.json");
	file >> _root;

	if (!_root.isObject())
	{
		printf("ERROR TO READ config.json, type: %d\n", _root.type());
		return;
	}

	file.close();
}



Conf::~Conf()
{
	SaveData();
}


void Conf::SetValue(const char* index, const char* value)
{
	mtx.lock();
	_root[index] = value;
	mtx.unlock();
};

void Conf::SetValue(const char* index, int value)
{
	mtx.lock();
	_root[index] = value;
	mtx.unlock();
};

std::string Conf::GetString(const char* index) const
{
	if (!_root.isMember(index) || !_root[index].isString())
		return "NONE";

	return _root[index].asString();
}

int Conf::GetInt(const char* index) const
{
	if (!_root.isMember(index) || !_root[index].isInt())
		return 0;
	return _root[index].asInt();
}

void Conf::SaveData()
{
	std::ofstream file;
	file.open("config.json");

	Json::StreamWriterBuilder styledWriter;
	styledWriter.newStreamWriter()->write(_root, &file);

	file.close();
}
