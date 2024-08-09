#include "../includes/utils.hpp"

std::string toLower(const std::string &str)
{
	std::string result = str; // Make a copy of the input string
	for (std::string::size_type i = 0; i < result.size(); ++i)
	{
		result[i] = std::tolower(static_cast<unsigned char>(result[i]));
	}
	return result;
}

bool containsOnlyASCII(const std::string &str)
{
	for (size_t i = 0; i < str.size(); i++)
	{
		if (static_cast<unsigned char>(str[i]) > 127)
			return false;
	}
	return true;
}

std::string trim(const std::string &str)
{
	std::string whitespaces(" \t\f\v\n\r");
	size_t first = str.find_first_not_of(whitespaces);
	if (std::string::npos == first)
	{
		return str;
	}
	size_t last = str.find_last_not_of(whitespaces);
	return str.substr(first, (last - first + 1));
}