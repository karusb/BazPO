#ifndef BAZ_PO_HPP
#define BAZ_PO_HPP

#include <string>
#include <map>
#include <vector>
#include <memory>
#include <queue>
#include <functional>

namespace BazPO
{
	class Option;

	class ICli
	{
	public:

		virtual void Add(std::string option, std::string secondOption = "", std::string description = "", bool mandatory = false, bool multipleOptions = false) = 0;
		virtual void Add(size_t valueCount = 1, std::string description = "", bool mandatory = false) = 0;
		virtual void Add(std::string option, std::function<void(const Option&)> action, std::string secondOption = "", std::string description = "", bool mandatory = false, bool multipleOptions = false) = 0;
		virtual void Add(Option& option) = 0;

		virtual void SetTaglessMode() { m_taglessMode = true; }
		virtual bool IsTagless() { return m_taglessMode; }

	private: 
		bool m_taglessMode = false;
	};

	enum OptionParseType
	{
		// Single Value
		Value,
		// Multiple Values Possible, via single tag "-o val1 val2" 
		MultiValue,
		// No tag is given
		Unidentified
	};

	class Option
	{
	protected:
		Option(std::string parameter, std::string secondParameter, std::string description, bool mandatory, OptionParseType parser)
			: Parameter(parameter)
			, SecondParameter(secondParameter)
			, Description(description)
			, Mandatory(mandatory)
			, ParseType(parser)
		{}
	public:
		Option() = delete;
		virtual ~Option() {};

		const char* value() const { return Value; }
		std::string str_value() const { return std::string(Value); }
		int int_value() const { return std::atoi(Value); }
		long long_value() const { return std::atol(Value); }
		long long long_long_value() const { return std::atoll(Value); }
		float float_value() const { return std::stof(std::string(Value)); }
		double double_value() const { return std::stod(std::string(Value)); }
		bool bool_value() const { return Value == "1" || Value == "True" || Value == "true" || Value == "t" || Value == "y"; }

		std::deque<const char*> values() const { return Values; }
		std::deque<std::string> str_values() const
		{
			std::deque<std::string> ret;
			for (auto it = Values.begin(); it != Values.end(); ++it)
				ret.push_back(std::string(*it));
			return ret;
		};

		std::deque<int> int_values() const
		{
			std::deque<int> ret;
			for (auto it = Values.begin(); it != Values.end(); ++it)
				ret.push_back(std::atoi(*it));
			return ret;
		};
		std::deque<long> long_values() const
		{
			std::deque<long> ret;
			for (auto it = Values.begin(); it != Values.end(); ++it)
				ret.push_back(std::atol(*it));
			return ret;
		};
		std::deque<long long> long_long_values() const
		{
			std::deque<long long> ret;
			for (auto it = Values.begin(); it != Values.end(); ++it)
				ret.push_back(std::atoll(*it));
			return ret;
		};

		std::deque<float> float_values() const
		{
			std::deque<float> ret;
			for (auto it = Values.begin(); it != Values.end(); ++it)
				ret.push_back(std::stof(*it));
			return ret;
		};

		std::deque<double> double_values() const
		{
			std::deque<double> ret;
			for (auto it = Values.begin(); it != Values.end(); ++it)
				ret.push_back(std::stod(*it));
			return ret;
		};

	public: 
		void setValue(const char* value) { Value = value; Values.push_back(value); };

		virtual void Execute(const Option&) const {};
		virtual size_t GetValueCount() const { return ParseType == OptionParseType::Value ? 1 : SIZE_MAX; }

		const char* Value = "";
		std::deque<const char*> Values;

		std::string Parameter;
		std::string SecondParameter;
		std::string Description;
		OptionParseType ParseType;
	public:
		bool Exists = false;
		bool Mandatory = false;

	};

	class ValueOption
		: public Option
	{
	public:
		ValueOption(std::string parameter, std::string secondParameter = "", std::string description = "", bool mandatory = false, ICli* po = nullptr)
			: Option(parameter, secondParameter, description, mandatory, OptionParseType::Value)
		{
			if (po != nullptr)
				po->Add(*this);
		};
	private:
		friend class Cli;
	};

	class MultiOption
		: public Option
	{
	public:
		MultiOption(std::string parameter, std::string secondParameter = "", std::string description = "", bool mandatory = false, ICli* po = nullptr)
			: Option(parameter, secondParameter, description, mandatory, OptionParseType::MultiValue)
		{
			if (po != nullptr)
				po->Add(*this);
		};
	private:
		friend class Cli;
	};
	namespace _detail
	{
		extern size_t TaglessOptionIDCounter;
	}
	class TaglessOption
		: public Option
	{
	public:
		TaglessOption(size_t valueCount = 1, std::string description = "", bool mandatory = false, ICli* po = nullptr)
			: Option(std::to_string(_detail::TaglessOptionIDCounter), "", description, mandatory, OptionParseType::Unidentified)
			, valueCount(valueCount)
		{
			++_detail::TaglessOptionIDCounter;
			if (po != nullptr)
			{
				po->Add(*this);
				po->SetTaglessMode();
			}

		};
	protected:
		virtual size_t GetValueCount() const override { return valueCount; }

	private:
		size_t valueCount;
		friend class Cli;
	};

	class FunctionOption
		: public Option
	{
	public:
		FunctionOption(std::string parameter, std::function<void(const Option&)> action, std::string secondParameter = "", std::string description = "", bool mandatory = false, ICli* po = nullptr)
			: Option(parameter, secondParameter, description, mandatory, OptionParseType::MultiValue)
			, f(action)
		{
			if (po != nullptr)
				po->Add(*this);
		}
		virtual void Execute(const Option& option) const override { f(option); }

	private:
		std::function<void(const Option&)> f;
		friend class Cli;
	};

	class Cli
		: public ICli
	{
	public:
		Cli(int argc,const char* argv[], const char* programDescription = "")
			: m_argc(argc)
			, m_argv(argv)
			, m_programDescription(programDescription)
		{
#ifndef BazPO_DISABLE_AUTO_HELP_MESSAGE
			Add("-h", [this](const Option&)
				{
					PrintOptions();
					exit(0);
				}
			, "--help", "Prints this help message");
#endif
		};
		~Cli() { BazPO::_detail::TaglessOptionIDCounter = 0; };
		// Implementation of ICli
		virtual void Add(std::string option, std::string secondOption = "", std::string description = "", bool mandatory = false, bool multipleOptions = false) override;
		virtual void Add(std::string option, std::function<void(const Option&)> action, std::string secondOption = "", std::string description = "", bool mandatory = false, bool multipleOptions = false) override;
		virtual void Add(size_t valueCount = 1, std::string description = "", bool mandatory = false) override;
		void Add(Option& option) override;
		//

		const Option& GetOption(std::string option);
		//template <typename T>
		//T GetValueAs(std::string option)
		//{
		//	if (!m_parsed)
		//		ParseArguments();
		//	auto key = GetKey(option);
		//	return m_refMap.at(key).value<T>();
		//}
		bool Exists(std::string option);
		void PrintOptions();
		void ParseArguments();
		void ChangeIO(std::ostream* ostream, std::istream* istream = &std::cin);
		void UserInputRequiredForAbsentMandatoryOptions();
		void NonsenseNotAccepted();

	private:
		void ParseNormal();
		void ParseTagless();
		void AskUserInputForMandatoryOptions();
		std::string CreateSpaces(size_t spaceCount);
		std::string GetKey(std::string option);
		void RegisterLargestInput(size_t optionSize, size_t secondOptionSize);
		void RegisterAlias(std::string option, std::string secondOption);
		Option& FindOption(std::string key);
		void PrintMultiArgumentParsingError(std::string key, std::string value);

		size_t m_maxOptionParameterSize = 0;
		size_t m_maxSecondOptionParameterSize = 0;
		const int m_argc;
		const char** m_argv;
		const char* m_programDescription;
		std::deque<ValueOption> m_values;
		std::deque<MultiOption> m_multiValues;
		std::deque<FunctionOption> m_functionalValues;
		std::deque<TaglessOption> m_taglessOptions;
		std::map<std::string, Option&> m_refMap;
		std::map<std::string, std::string> m_aliasMap;
		std::vector<std::string> m_inputStorage;
		bool m_parsed = false;
		bool m_askInputForMandatoryOptions = false;
		bool m_exitOnUnexpectedValue = false;

		std::istream* m_inputStream = &std::cin;
		std::ostream* m_outputStream = &std::cout;

		friend class Option;
	};
}
#endif
