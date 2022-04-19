#ifndef BAZ_PO_HPP
#define BAZ_PO_HPP

#include <string>
#include <map>
#include <memory>
#include <queue>
#include <functional>
#include <sstream>
#include <iostream>
#include <iomanip>

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
    namespace _detail
    {
        enum OptionParseType
        {
            // Single Value
            Value,
            // Multiple Values Possible, via single tag "-o val1 val2" 
            MultiValue,
            // Tagless
            Unidentified
        };
    }
	class Option
	{
	protected:
		Option(std::string parameter, std::string secondParameter, std::string description, bool mandatory, _detail::OptionParseType parser)
			: Parameter(parameter)
			, SecondParameter(secondParameter)
			, Description(description)
			, Mandatory(mandatory)
			, ParseType(parser)
		{}
	public:
		Option() = delete;
		virtual ~Option() {};

		bool exists() { return Exists; }
		const char* value() const { return Value; }
		template <typename T>
		T value_as()
		{
			std::stringstream ss;
			T v;
			ss << Value;
			ss >> v;
			return v;
		}
		std::string value_str() const { return std::string(Value); }
		bool value_bool() const { return std::string(Value) == "1" || std::string(Value) == "True" || std::string(Value) == "true" || std::string(Value) == "t" || std::string(Value) == "y"; }

		std::deque<const char*> values() const { return Values; }
		template <typename T>
		T values_as()
		{
			std::deque<T> ret;
			for (auto it = Values.begin(); it != Values.end(); ++it)
				ret.push_back(value_as<T>(*it));

			return ret;
		}
		std::deque<std::string> values_str() const
		{
			std::deque<std::string> ret;
			for (auto it = Values.begin(); it != Values.end(); ++it)
				ret.push_back(std::string(*it));
			return ret;
		};
		std::deque<bool> values_bool() const
		{
			std::deque<bool> ret;
			for (auto it = Values.begin(); it != Values.end(); ++it)
				ret.push_back(value_bool());
			return ret;
		};

	protected: 
		void setValue(const char* value) { Value = value; Values.push_back(value); };

		virtual void Execute(const Option&) const {};
		virtual size_t GetValueCount() const { return ParseType == _detail::OptionParseType::Value ? 1 : SIZE_MAX; }

		const char* Value = "";
		std::deque<const char*> Values;

		std::string Parameter;
		std::string SecondParameter;
		std::string Description;
		_detail::OptionParseType ParseType;
		bool Exists = false;
        // Add exists count
		bool Mandatory = false;
		friend class Cli;
	};

	class ValueOption
		: public Option
	{
	public:
		ValueOption(std::string parameter, std::string secondParameter = "", std::string description = "", bool mandatory = false, ICli* po = nullptr)
			: Option(parameter, secondParameter, description, mandatory, _detail::OptionParseType::Value)
		{
			if (po != nullptr)
				po->Add(*this);
		};
	};

	class MultiOption
		: public Option
	{
	public:
		MultiOption(std::string parameter, std::string secondParameter = "", std::string description = "", bool mandatory = false, ICli* po = nullptr)
			: Option(parameter, secondParameter, description, mandatory, _detail::OptionParseType::MultiValue)
		{
			if (po != nullptr)
				po->Add(*this);
		};
	};

	namespace _detail
	{
		size_t TaglessOptionIDCounter = 0;
	}

	class TaglessOption
		: public Option
	{
	public:
		TaglessOption(size_t valueCount = 1, std::string description = "", bool mandatory = false, ICli* po = nullptr)
			: Option(std::to_string(_detail::TaglessOptionIDCounter), "", description, mandatory, _detail::OptionParseType::Unidentified)
			, valueCount(valueCount)
		{
			++_detail::TaglessOptionIDCounter;
			if (po != nullptr)
			{
				po->Add(*this);
				// use this for the id counter ?
				po->SetTaglessMode();
			}

		};
	protected:
		virtual size_t GetValueCount() const override { return valueCount; }

	private:
		size_t valueCount;
	};

	class FunctionOption
		: public Option
	{
	public:
		FunctionOption(std::string parameter, std::function<void(const Option&)> action, std::string secondParameter = "", std::string description = "", bool mandatory = false, ICli* po = nullptr)
			: Option(parameter, secondParameter, description, mandatory, _detail::OptionParseType::MultiValue)
			, f(action)
		{
			if (po != nullptr)
				po->Add(*this);
		}
		virtual void Execute(const Option& option) const override { f(option); }

	private:
		std::function<void(const Option&)> f;
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

		const Option& GetOption(std::string option);
		template <typename T>
		T GetValueAs(std::string option)
		{
			if (!m_parsed)
				ParseArguments();
			auto key = GetKey(option);
			return m_refMap.at(key).value_as<T>();
		}
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
		void RegisterLargestInput(size_t optionSize, size_t secondOptionSize, size_t descriptionSize);
		void RegisterAlias(std::string option, std::string secondOption);
		Option& FindOption(std::string key);
		void PrintMultiArgumentParsingError(std::string key, std::string value);
		void PrintOptionUsage(const Option& option);
		void PrintOption(const Option& option);
		std::string GetParameterWithMandatorySyntax(std::string value, bool mandatory);


		size_t m_maxOptionParameterSize = 0;
		size_t m_maxSecondOptionParameterSize = 0;
		size_t m_maxDescriptionSize = 0;
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
	};
    void Cli::Add(std::string option, std::string secondOption, std::string description, bool mandatory, bool multipleOptions)
    {
        RegisterLargestInput(option.size(), secondOption.size(), description.size());
        Option* optionRef = nullptr;
        if (multipleOptions)
        {
            m_multiValues.push_back(MultiOption(option, secondOption, description, mandatory));
            optionRef = &m_multiValues.back();
        }
        else
        {
            m_values.push_back(ValueOption(option, secondOption, description, mandatory));
            optionRef = &m_values.back();
        }

        m_refMap.insert({ option, *optionRef });
        RegisterAlias(option, secondOption);
    }

    void Cli::Add(std::string option, std::function<void(const Option&)> action, std::string secondOption, std::string description, bool mandatory, bool multipleOptions)
    {
        RegisterLargestInput(option.size(), secondOption.size(), description.size());
        m_functionalValues.push_back(FunctionOption(option, action, secondOption, description, mandatory));

        m_refMap.insert({ option, *&m_functionalValues.back() });
        RegisterAlias(option, secondOption);
    }

    void Cli::Add(size_t valueCount, std::string description, bool mandatory)
    {
        RegisterLargestInput(_detail::TaglessOptionIDCounter % 10 + 1, 0, description.size());
        m_taglessOptions.push_back(TaglessOption(valueCount, description, mandatory));

        auto id = std::to_string(_detail::TaglessOptionIDCounter - 1);
        m_refMap.insert({ id , *&m_taglessOptions.back() });

        SetTaglessMode();
    }

    void Cli::Add(Option& option)
    {
        RegisterLargestInput(option.Parameter.size(), option.SecondParameter.size(), option.Description.size());
        m_refMap.insert({ option.Parameter, option });
        RegisterAlias(option.Parameter, option.SecondParameter);
    }

    const Option& Cli::GetOption(std::string option)
    {
        if (!m_parsed)
            ParseArguments();
        auto key = GetKey(option);
        return m_refMap.at(key);
    }

    bool Cli::Exists(std::string option)
    {
        if (!m_parsed)
            ParseArguments();
        auto key = GetKey(option);
        return m_refMap.at(key).Exists;
    }

    void Cli::ParseArguments()
    {
        if (!m_parsed)
        {
            if (IsTagless())
                ParseTagless();
            else
                ParseNormal();
            // At this moment we know if options are present or not
            AskUserInputForMandatoryOptions();
            m_parsed = true;
            // Execute Actions
            for (auto it = m_refMap.begin(); it != m_refMap.end(); ++it)
                if (it->second.Exists)
                    it->second.Execute(it->second);
        }
    }

    void Cli::ParseTagless()
    {
        int cursor = 1;
        if (m_argc == 2 && std::string(m_argv[1]) == "help" || std::string(m_argv[1]) == "-h" || std::string(m_argv[1]) == "--help" || std::string(m_argv[1]) == "h")
        {
            PrintOptions();
            exit(0);
        }
        for (auto it = m_refMap.begin(); it != m_refMap.end(); ++it)
        {
            if (it->second.ParseType == _detail::OptionParseType::Unidentified)
                for (int i = 0; cursor < m_argc && i < it->second.GetValueCount(); ++cursor, ++i)
                {
                    it->second.Exists = true;
                    it->second.setValue(m_argv[cursor]);
                }
        }
    }

    void Cli::ParseNormal()
    {
        Option* lastOption = nullptr;
        for (int i = 1; i < m_argc; ++i)
        {
            // Set the key, if secondOption i.e an alias is used set it to original parameter, else set it to argument
            auto key = GetKey(m_argv[i]);
            auto option = m_refMap.find(key);
            if (option != m_refMap.end())
            {
                option->second.Exists = true;
                if ((i + 1) < m_argc) // Value might exist if its within bounds and not in the options map otherwise it's empty
                    option->second.setValue(m_argv[i + 1]);
                lastOption = &option->second;
            }
            else if (lastOption != nullptr)
            {
                if ((i + 1) < m_argc && m_refMap.find(GetKey(m_argv[i + 1])) == m_refMap.end())
                {
                    if (lastOption->ParseType == _detail::OptionParseType::MultiValue)
                        lastOption->setValue(m_argv[i + 1]);
                    else if (lastOption->ParseType == _detail::OptionParseType::Value && m_exitOnUnexpectedValue)
                        PrintMultiArgumentParsingError(lastOption->Parameter, m_argv[i + 1]);
                }
            }
        }
    }

    void Cli::AskUserInputForMandatoryOptions()
    {
        for (auto& pair : m_refMap)
        {
            if (pair.second.Mandatory && !pair.second.Exists)
            {
                *m_outputStream << pair.second.Parameter << " is a required parameter" << std::endl;
                *m_outputStream << pair.second.Parameter << ": " << std::endl;
                if (m_askInputForMandatoryOptions)
                {
                    std::string temp;
                    *m_inputStream >> temp;

                    m_inputStorage.push_back(temp);
                    pair.second.Value = m_inputStorage.back().c_str();
                    pair.second.Exists = true;
                }
                else
                {
                    PrintOptions();
                    exit(-1);
                }
            }
        }
    }

    inline std::string Cli::CreateSpaces(size_t spaceCount)
    {
        return std::string(" ", spaceCount);
    }

    inline std::string Cli::GetKey(std::string option)
    {
        return (m_aliasMap.find(option) != m_aliasMap.end()) ? m_aliasMap.at(option) : option;
    }

    void Cli::PrintOptions()
    {
        std::string prgName(m_argv[0]);
        prgName = prgName.substr(prgName.find_last_of("\\") + 1);
        if (prgName.empty())
            prgName = m_argv[0];
        // Program Description
        *m_outputStream << std::endl << std::left << std::setw(m_maxSecondOptionParameterSize + prgName.size()) << prgName << m_programDescription << std::endl;
        // Program Usage
        *m_outputStream << "usage: " << prgName << " ";
        for (const auto& pair : m_refMap)
        {
            *m_outputStream << " ";
            PrintOptionUsage(pair.second);
        }
        *m_outputStream << std::endl;
        // Options
        *m_outputStream << "Program Options: " << std::endl;
        for (const auto& pair : m_refMap)
            PrintOption(pair.second);
    }

    void Cli::RegisterLargestInput(size_t optionSize, size_t secondOptionSize, size_t descriptionSize)
    {
        // Register largest parameter size to use for padding later on
        m_maxOptionParameterSize = optionSize > m_maxOptionParameterSize ? optionSize : m_maxOptionParameterSize;
        m_maxSecondOptionParameterSize = secondOptionSize > m_maxSecondOptionParameterSize ? secondOptionSize : m_maxSecondOptionParameterSize;
        m_maxDescriptionSize = descriptionSize > m_maxDescriptionSize ? descriptionSize : m_maxDescriptionSize;
    }

    void Cli::RegisterAlias(std::string option, std::string secondOption)
    {
        if (secondOption != "")
            m_aliasMap.insert(std::make_pair(secondOption, option));
    }

    void Cli::ChangeIO(std::ostream* ostream, std::istream* istream)
    {
        m_inputStream = istream;
        m_outputStream = ostream;
    }

    void Cli::PrintMultiArgumentParsingError(std::string key, std::string value)
    {
        *m_outputStream << "Expected only one value to argument '" << key << "'" << "where -> '" << value << "' is not expected";
        PrintOptions();
        exit(1);
    }

    void Cli::UserInputRequiredForAbsentMandatoryOptions()
    {
        m_askInputForMandatoryOptions = true;
    }

    void Cli::PrintOptionUsage(const Option& option)
    {
        if (option.ParseType == _detail::OptionParseType::Unidentified)
            if (option.Mandatory)
                *m_outputStream << option.Description << "(" << option.GetValueCount() << ") ";
            else
                *m_outputStream << "[" << option.Description << "(" << option.GetValueCount() << ")" << "] ";
        else
            *m_outputStream << GetParameterWithMandatorySyntax(option.Parameter, option.Mandatory);
    }

    void Cli::PrintOption(const Option& option)
    {
        if (option.ParseType != _detail::OptionParseType::Unidentified)
            *m_outputStream << std::left << std::setw(m_maxOptionParameterSize + 9) << GetParameterWithMandatorySyntax(option.Parameter, option.Mandatory) << std::left << std::setw(m_maxSecondOptionParameterSize + 10) << option.SecondParameter << option.Description << std::endl;
    }

    void Cli::NonsenseNotAccepted()
    {
        m_exitOnUnexpectedValue = true;
    }

    std::string Cli::GetParameterWithMandatorySyntax(std::string value, bool mandatory)
    {
        std::string str;
        if (mandatory)
            str.append("<").append(value).append(">");
        else
            str.append("[").append(value).append("]");
        return str;
    }
}
#endif
