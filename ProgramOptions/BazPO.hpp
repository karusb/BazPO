#ifndef BAZ_PO_HPP
#define BAZ_PO_HPP

#include <string>
#include <map>
#include <vector>
#include <memory>

class BazPO
{
public:
	struct Option
	{
		Option(std::string parameter, const char* value, std::string secondParameter = "" ,std::string description = "", bool mandatory = false ,bool exists = false) 
			: Parameter(parameter)
			, Value(value)
			, Description(description)
			, SecondParameter(secondParameter)
			, Mandatory(mandatory)
			, Exists(exists)
		{};

		std::string Parameter;
		std::string SecondParameter;
		const char* Value;
		std::string Description;
		bool Exists;
		bool Mandatory;
	};

	BazPO(int argc, char* argv[], bool askInputForMandatoryOptions = false ) 
		: m_argc(argc)
		, m_argv(argv)
		, m_askInputForMandatoryOptions(askInputForMandatoryOptions) 
	{};
	// Add program option. You can use custom separators.
	void Add(std::string option, std::string secondOption = "", std::string description = "",  bool mandatory = false);
	Option GetOption(std::string option);
	std::string GetValue(std::string option);
	template <typename T>
	T GetValueAs(std::string option);
	bool Exists(std::string option);
	void PrintOptions();
	void ParseArguments();

private:
	void AskUserInputForMandatoryOptions();
	std::string CreateSpaces(size_t spaceCount);
	std::string GetKey(std::string option);
	size_t m_maxOptionParameterSize = 0;
	size_t m_maxSecondOptionParameterSize = 0;
	int m_argc;
	char** m_argv;
	std::map<std::string, Option> m_map;
	std::map<std::string, std::string> m_aliasMap;
	std::vector<std::string> m_optionStorage;
	bool m_parsed = false;
	bool m_askInputForMandatoryOptions = false;
};

template<typename T>
inline T BazPO::GetValueAs(std::string option)
{
	return T();
}

template<>
inline int BazPO::GetValueAs<int>(std::string option)
{
	if (!m_parsed)
		ParseArguments();
	auto key = GetKey(option);
	if (m_map.at(key).Exists)
		return std::atoi(m_map.at(key).Value);
	else
		return 0;
}

template<>
inline long BazPO::GetValueAs<long>(std::string option)
{
	if (!m_parsed)
		ParseArguments();
	auto key = GetKey(option);
	if (m_map.at(key).Exists)
		return std::atol(m_map.at(key).Value);
	else
		return 0;
}

template<>
inline long long BazPO::GetValueAs<long long>(std::string option)
{
	if (!m_parsed)
		ParseArguments();
	auto key = GetKey(option);
	if (m_map.at(key).Exists)
		return std::atoll(m_map.at(key).Value);
	else
		return 0;
}

template<>
inline float BazPO::GetValueAs<float>(std::string option)
{
	if (!m_parsed)
		ParseArguments();
	auto key = GetKey(option);
	if (m_map.at(key).Exists)
		return std::stof(m_map.at(key).Value);
	else
		return 0.0f;
}

template<>
inline double BazPO::GetValueAs<double>(std::string option)
{
	if (!m_parsed)
		ParseArguments();
	auto key = GetKey(option);
	if (m_map.at(key).Exists)
		return std::stod(m_map.at(key).Value);
	else
		return 0.0;
}

template<>
inline bool BazPO::GetValueAs<bool>(std::string option)
{
	if (!m_parsed)
		ParseArguments();
	auto key = GetKey(option);
	if (m_map.at(key).Exists)
		return true;
	else
		return false;
}

#endif
