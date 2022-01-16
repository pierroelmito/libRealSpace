
#include "UserProperties.h"

std::istream& operator >> (std::istream& in, hmm_vec3& v)
{
	in >> v.X >> v.Y >> v.Z;
	return in;
}

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
		{ "float3", &Vectors3 },
		{ "string", &Strings },
	};

	char buffer[512];
	while (fgets(buffer, sizeof(buffer), f) != nullptr) {
		std::stringstream ss(buffer);
		std::string name, s0, type;
		ss >> type >> name;
		auto it = readers.find(type);
		if (it == readers.end())
			continue;
		const std::string val = ss.str().substr(ss.tellg());
		printf("set '%s' to: %s", name.c_str(), val.c_str());
		it->second->ReadFromString(name, val);
	}

	return true;
}

void UserProperties::Reload()
{
	ReadFromFile(FileName.c_str());
}
