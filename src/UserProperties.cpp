
#include "UserProperties.h"

UserProperties& UserProperties::Get()
{
	static UserProperties properties;
	return properties;
}

bool UserProperties::ReadFromFile(const char* fname)
{
	FileName = fname;

	FILE* f = fopen(fname, "r");
	if (f == nullptr)
		return false;

	const std::map<std::string, StringReader*> readers = {
		{ "bool", &Bools },
		{ "int", &Ints },
		{ "float", &Floats },
		{ "string", &Strings },
	};

	char buffer[512];
	while (fgets(buffer, sizeof(buffer), f) != nullptr) {
		std::stringstream ss(buffer);
		std::string name, s0, type, val;
		ss >> type >> name >> val;
		auto it = readers.find(type);
		if (it == readers.end())
			continue;
		printf("set '%s' to '%s'\n", name.c_str(), val.c_str());
		it->second->ReadFromString(name, val);
	}

	return true;
}

void UserProperties::Reload()
{
	ReadFromFile(FileName.c_str());
}
