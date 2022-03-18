#pragma once
#include <string>
#include <map>
#include <vector>
#include <memory>
class BazPO
{

public:
	struct Option
	{
		std::string Parameter;
		std::string SecondParameter;
		const char* Value;
		std::string Description;
		bool Exists;
		bool Mandatory ;
		Option(std::string parameter, const char* value, std::string secondParameter = "" ,std::string description = "", bool mandatory = false ,bool exists = false) :
			Parameter(parameter), Value(value), Description(description), SecondParameter(secondParameter), Mandatory(mandatory), Exists(exists) { };
	};
	BazPO(int argc, char* argv[], bool askInputForMandatoryOptions = false ) 
		: m_argc(argc), m_argv(argv), m_askInputForMandatoryOptions(askInputForMandatoryOptions) {};
	// Add program option. You can use custom separators.
	void Add(std::string option,
		std::string secondOption = "", std::string description = "" ,  bool mandatory = false);
	Option GetOption(std::string option);
	std::string GetValue(std::string option);
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
	std::map<std::string,std::shared_ptr<Option>> m_map;
	std::map<std::string, std::string> m_aliasMap;
	std::vector<std::shared_ptr<std::string>> m_optionStorage;
	bool m_parsed = false;
	bool m_askInputForMandatoryOptions = false;
};

