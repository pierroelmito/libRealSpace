
#pragma once

#include <string>
#include <map>
#include <sstream>

class StringReader
{
public:
	virtual ~StringReader() {}
	virtual void ReadFromString(const std::string& s, const std::string& v) = 0;
};

template <class T>
class PropContainer : public StringReader
{
public:
	void Set(const std::string& ID, T value)
	{
		values[ID] = value;
	}
	T Get(const std::string& ID, const T& notFound) const
	{
		auto it = values.find(ID);
		if (it == values.end())
			return notFound;
		return it->second;
	}
	virtual void ReadFromString(const std::string& s, const std::string& v) override
	{
		std::stringstream ss(v);
		T& nv = values[s];
		ss >> nv;
	}
protected:
	std::map<std::string, T> values;
};

class UserProperties
{
public:
	PropContainer<bool> Bools;
	PropContainer<int> Ints;
	PropContainer<float> Floats;
	PropContainer<std::string> Strings;

	static UserProperties& Get();

	bool ReadFromFile(const char* fname);
	void Reload();

protected:
	std::string FileName;
};
