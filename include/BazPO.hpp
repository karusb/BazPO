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

		virtual void add(std::string option, std::string secondOption = "", std::string description = "", bool mandatory = false, bool multipleOptions = false, size_t maxValueCount = 1) = 0;
		virtual void add(size_t valueCount = 1, std::string description = "", bool mandatory = false) = 0;
		virtual void add(std::function<void(const Option&)> onExists, size_t valueCount = 1, std::string description = "", bool mandatory = false) = 0;
		virtual void add(std::string option, std::function<void(const Option&)> onExists, std::string secondOption = "", std::string description = "", bool mandatory = false, bool multipleOptions = false, size_t maxValueCount = 1) = 0;
        virtual void add(Option& option) = 0;

    protected:
		virtual void setTagless() { m_taglessMode = true; }
		virtual void setNormal() { m_normalMode = true; }
		void setUndefined() { m_normalMode = false; m_taglessMode = false; }
		bool tagless() const { return m_taglessMode; }
		bool normal() const { return m_normalMode; }

	private: 
		bool m_taglessMode = false;
		bool m_normalMode = false;
        friend class Option;
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
		Option(std::string parameter, std::string secondParameter, std::string description, bool mandatory, _detail::OptionParseType parser, ICli* po = nullptr)
			: Parameter(parameter)
			, SecondParameter(secondParameter)
			, Description(description)
			, Mandatory(mandatory)
			, ParseType(parser)
		{
            if (po != nullptr)
            {
                if (ParseType == _detail::OptionParseType::Unidentified)
                    po->setTagless();
                else
                    po->setNormal();
                po->add(*this);
            }
        }
	public:
		Option() = delete;
		virtual ~Option() {};

		bool exists() const { return Exists; }
		int existsCount() const { return ExistsCount; }
		const char* value() const { return Value; }
		template <typename T>
		T valueAs() const
		{
			std::stringstream ss;
			T v;
			ss << Value;
			ss >> v;
			return v;
		}
		std::string valueStr() const { return std::string(Value); }
		bool valueBool() const { return std::string(Value) == "1" || std::string(Value) == "True" || std::string(Value) == "true" || std::string(Value) == "t" || std::string(Value) == "y"; }

		const std::deque<const char*>& values() const { return Values; }
		template <typename T>
		T valuesAs() const
		{
			std::deque<T> ret;
			for (auto it = Values.begin(); it != Values.end(); ++it)
				ret.push_back(valueAs<T>(*it));

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
				ret.push_back(valueBool());
			return ret;
		};

	protected: 
		void setValue(const char* value) { Value = value; Values.push_back(value); };

		virtual void execute(const Option&) const {};
		virtual size_t maxValueCount() const { return ParseType == _detail::OptionParseType::Value ? 1 : SIZE_MAX; }

		const char* Value = "";
		std::deque<const char*> Values;

		std::string Parameter;
		std::string SecondParameter;
		std::string Description;
		_detail::OptionParseType ParseType;
		bool Exists = false;
        int ExistsCount = 0;
		bool Mandatory = false;
		friend class Cli;
	};

	class ValueOption
		: public Option
	{
	public:
		ValueOption(ICli* po, std::string parameter, std::string secondParameter = "", std::string description = "", bool mandatory = false)
			: Option(parameter, secondParameter, description, mandatory, _detail::OptionParseType::Value, po)
		{};
	};

	class MultiOption
		: public Option
	{
	public:
        MultiOption(ICli* po, std::string parameter, std::string secondParameter = "", std::string description = "", bool mandatory = false, size_t maxValueCount = SIZE_MAX)
            : Option(parameter, secondParameter, description, mandatory, _detail::OptionParseType::MultiValue, po)
            , valueCount(maxValueCount)
        {};

    protected:
        virtual size_t maxValueCount() const override { return valueCount; }

    private:
        size_t valueCount;
	};

	namespace _detail
	{
		size_t TaglessOptionIDCounter = 0;
	}

	class TaglessOption
		: public Option
	{
	public:
		TaglessOption(ICli* po, size_t valueCount = 1, std::string description = "", bool mandatory = false)
			: Option(std::to_string(_detail::TaglessOptionIDCounter), "", description, mandatory, _detail::OptionParseType::Unidentified, po)
			, valueCount(valueCount)
		{
			++_detail::TaglessOptionIDCounter;
		};
	protected:
		virtual size_t maxValueCount() const override { return valueCount; }

	private:
		size_t valueCount;
	};

	class FunctionOption
		: public ValueOption
	{
	public:
        FunctionOption(ICli* po ,std::string parameter, std::function<void(const Option&)> onExists, std::string secondParameter = "", std::string description = "", bool mandatory = false)
			: ValueOption(po, parameter, secondParameter, description, mandatory)
			, f(onExists)
		{}
        virtual void execute(const Option& option) const override { f(option); }

    private:
        std::function<void(const Option&)> f;
	};

    class FunctionMultiOption
        : public MultiOption
    {
    public:
        FunctionMultiOption(ICli* po, std::string parameter, std::function<void(const Option&)> onExists, std::string secondParameter = "", std::string description = "", bool mandatory = false, size_t maxValueCount = SIZE_MAX)
            : MultiOption(po, parameter, secondParameter, description, mandatory, maxValueCount)
            , f(onExists)
        {}
        virtual void execute(const Option& option) const override { f(option); }

    private:
        std::function<void(const Option&)> f;
    };

    class FunctionTaglessOption
        : public TaglessOption
    {
    public:
        FunctionTaglessOption(ICli* po, std::function<void(const Option&)> onExists, size_t valueCount = 1, std::string description = "", bool mandatory = false)
            : TaglessOption(po, valueCount, description, mandatory)
            , f(onExists)
        {}
        virtual void execute(const Option& option) const override { f(option); }

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
			add("-h", [this](const Option&)
				{
					printOptions();
					exit(0);
				}
			, "--help", "Prints this help message");
            setUndefined();
#endif
		};
		~Cli() { BazPO::_detail::TaglessOptionIDCounter = 0; };
		// Implementation of ICli
		virtual void add(std::string option, std::string secondOption = "", std::string description = "", bool mandatory = false, bool multipleOptions = false, size_t maxValueCount = 1) override;
		virtual void add(std::string option, std::function<void(const Option&)> onExists, std::string secondOption = "", std::string description = "", bool mandatory = false, bool multipleOptions = false, size_t maxValueCount = 1) override;
		virtual void add(size_t valueCount = 1, std::string description = "", bool mandatory = false) override;
        virtual void add(std::function<void(const Option&)> onExists, size_t valueCount = 1, std::string description = "", bool mandatory = false) override;
		virtual void add(Option& option) override;

        const Option& option(std::string option) const { return m_refMap.at(getKey(option)); }
		template <typename T>
		T valueAs(std::string option)
		{
			if (!m_parsed)
				parse();
			auto key = getKey(option);
			return m_refMap.at(key).valueAs<T>();
		}
        bool exists(std::string option) const { return m_refMap.at(getKey(option)).Exists; }
        bool existsCount(std::string option) const { return m_refMap.at(getKey(option)).ExistsCount; }
		void printOptions();
		void parse();
		void changeIO(std::ostream* ostream, std::istream* istream = &std::cin);
		void userInputRequired();
		void unexpectedArgumentsAcceptable();

	private:
		void parseNormal();
		void parseTagless();
		void askUserInput();
		std::string getKey(std::string option) const { return (m_aliasMap.find(option) != m_aliasMap.end()) ? m_aliasMap.at(option) : option; }
		void registerOptionSizes(size_t optionSize, size_t secondOptionSize, size_t descriptionSize);
		void registerAlias(std::string option, std::string secondOption);
		void multiArgParseError(std::string key, std::string value);
        void unknownArgParsingError(std::string value);
		void printOptionUsage(const Option& option);
		void printOption(const Option& option);
		std::string parameterSyntax(std::string value, bool mandatory);
        void throwOnMismatch(bool tagless);

		size_t m_maxOptionParameterSize = 0;
		size_t m_maxSecondOptionParameterSize = 0;
		size_t m_maxDescriptionSize = 0;
		const int m_argc;
		const char** m_argv;
		const char* m_programDescription;
		std::deque<ValueOption> m_values;
		std::deque<MultiOption> m_multiValues;
		std::deque<FunctionOption> m_functionalValues;
		std::deque<FunctionMultiOption> m_functionalMultiValues;
		std::deque<FunctionTaglessOption> m_functionalTaglessValues;
		std::deque<TaglessOption> m_taglessOptions;
		std::map<std::string, Option&> m_refMap;
		std::map<std::string, std::string> m_aliasMap;
		std::vector<std::string> m_inputStorage;
		bool m_parsed = false;
		bool m_askInputForMandatoryOptions = false;
		bool m_exitOnUnexpectedValue = true;

		std::istream* m_inputStream = &std::cin;
		std::ostream* m_outputStream = &std::cout;
	};
    void Cli::add(std::string option, std::string secondOption, std::string description, bool mandatory, bool multipleOptions, size_t maxValueCount)
    {
        setNormal();
        throwOnMismatch(false);
        registerOptionSizes(option.size(), secondOption.size(), description.size());
        Option* optionRef = nullptr;
        if (multipleOptions)
        {
            if (maxValueCount == 1)
                maxValueCount = SIZE_MAX;
            m_multiValues.push_back(MultiOption(nullptr, option, secondOption, description, mandatory, maxValueCount));
            optionRef = &m_multiValues.back();
        }
        else
        {
            m_values.push_back(ValueOption(nullptr, option, secondOption, description, mandatory));
            optionRef = &m_values.back();
        }

        m_refMap.insert({ option, *optionRef });
        registerAlias(option, secondOption);
    }

    void Cli::add(std::string option, std::function<void(const Option&)> onExists, std::string secondOption, std::string description, bool mandatory, bool multipleOptions, size_t maxValueCount)
    {
        setNormal();
        throwOnMismatch(false);
        registerOptionSizes(option.size(), secondOption.size(), description.size());
        Option* optionRef = nullptr;
        if (multipleOptions)
        {
            m_functionalMultiValues.push_back(FunctionMultiOption(nullptr, option, onExists, secondOption, description, mandatory, maxValueCount));
            optionRef = &m_functionalMultiValues.back();
        }
        else
        {
            m_functionalValues.push_back(FunctionOption(nullptr, option, onExists, secondOption, description, mandatory));
            optionRef = &m_functionalValues.back();
        }

        m_refMap.insert({ option, *optionRef });
        registerAlias(option, secondOption);
    }

    void Cli::add(size_t valueCount, std::string description, bool mandatory)
    {
        setTagless();
        throwOnMismatch(true);
        registerOptionSizes(_detail::TaglessOptionIDCounter % 10 + 1, 0, description.size());
        m_taglessOptions.push_back(TaglessOption(nullptr, valueCount, description, mandatory));

        auto id = std::to_string(_detail::TaglessOptionIDCounter - 1);
        m_refMap.insert({ id , *&m_taglessOptions.back() });
    }

    void Cli::add(std::function<void(const Option&)> onExists, size_t valueCount, std::string description, bool mandatory)
    {
        setTagless();
        throwOnMismatch(true);
        registerOptionSizes(_detail::TaglessOptionIDCounter % 10 + 1, 0, description.size());
        m_functionalTaglessValues.push_back(FunctionTaglessOption(nullptr, onExists, valueCount, description, mandatory));

        auto id = std::to_string(_detail::TaglessOptionIDCounter - 1);
        m_refMap.insert({ id , *&m_functionalTaglessValues.back() });
    }

    void Cli::add(Option& option)
    {
        throwOnMismatch(option.ParseType == _detail::OptionParseType::Unidentified);
        registerOptionSizes(option.Parameter.size(), option.SecondParameter.size(), option.Description.size());
        m_refMap.insert({ option.Parameter, option });
        registerAlias(option.Parameter, option.SecondParameter);
    }

    void Cli::parse()
    {
        if (!m_parsed)
        {
            if (tagless())
                parseTagless();
            else
                parseNormal();
            // At this moment we know if options are present or not
            askUserInput();
            m_parsed = true;
            // execute actions
            for (auto it = m_refMap.begin(); it != m_refMap.end(); ++it)
                if (it->second.Exists)
                    it->second.execute(it->second);
        }
    }

    void Cli::parseTagless()
    {
        int cursor = 1;
#ifndef BazPO_DISABLE_AUTO_HELP_MESSAGE
        if (m_argc == 2 && std::string(m_argv[1]) == "help" || std::string(m_argv[1]) == "-h" || std::string(m_argv[1]) == "--help" || std::string(m_argv[1]) == "h")
        {
            printOptions();
            exit(0);
        }
#endif      
        for (auto it = m_refMap.begin(); it != m_refMap.end(); ++it)
        {
            if (it->second.ParseType == _detail::OptionParseType::Unidentified)
                for (size_t i = 0; cursor < m_argc && i < it->second.maxValueCount(); ++cursor, ++i)
                {
                    it->second.Exists = true;
                    ++it->second.ExistsCount;
                    it->second.setValue(m_argv[cursor]);
                }
        }
        if(cursor < m_argc && m_exitOnUnexpectedValue)
            unknownArgParsingError(m_argv[cursor]);
    }

    void Cli::parseNormal()
    {
        Option* lastOption = nullptr;
        for (int i = 1; i < m_argc; ++i)
        {
            auto key = getKey(m_argv[i]);
            auto option = m_refMap.find(key);

            if (option != m_refMap.end())
            {
                option->second.Exists = true;
                ++option->second.ExistsCount;

                if ((i + 1) < m_argc && m_refMap.find(getKey(m_argv[i + 1])) == m_refMap.end())
                    option->second.setValue(m_argv[i + 1]);

                lastOption = &option->second;
            }
            else if (lastOption != nullptr)
            {
                if ((i + 1) < m_argc && m_refMap.find(getKey(m_argv[i + 1])) == m_refMap.end())
                {
                    if (lastOption->ParseType == _detail::OptionParseType::MultiValue && lastOption->maxValueCount() > lastOption->values().size())
                        lastOption->setValue(m_argv[i + 1]);
                    else if (m_exitOnUnexpectedValue)
                        multiArgParseError(lastOption->Parameter, m_argv[i + 1]);
                }
            }
            else if(m_exitOnUnexpectedValue)
                unknownArgParsingError(m_argv[i]);
        }
    }

    void Cli::askUserInput()
    {
        for (auto& pair : m_refMap)
        {
            if (pair.second.Mandatory && !pair.second.Exists)
            {
                printOptionUsage(pair.second);
                *m_outputStream << " is a required parameter" << std::endl;
                if (m_askInputForMandatoryOptions)
                {
                    printOptionUsage(pair.second);
                    *m_outputStream << ": ";
                    std::string temp;
                    std::getline(*m_inputStream, temp);

                    m_inputStorage.push_back(temp);
                    pair.second.Value = m_inputStorage.back().c_str();
                    pair.second.Exists = true;
                }
                else
                {
                    printOptions();
                    exit(-1);
                }
            }
        }
    }

    void Cli::printOptions()
    {
        std::string prgName(m_argv[0]);
        size_t prgNameStart = prgName.find_last_of("\\");
        if (prgNameStart == std::string::npos)
            prgNameStart = prgName.find_last_of("/");

        if (prgNameStart != std::string::npos)
            prgName = prgName.substr(prgNameStart + 1);
        // Program Description
        *m_outputStream << std::endl << std::left << std::setw(m_maxSecondOptionParameterSize + prgName.size()) << prgName << m_programDescription << std::endl;
        // Program Usage
        *m_outputStream << "usage: " << prgName << " ";
        for (const auto& pair : m_refMap)
        {
            *m_outputStream << " ";
            printOptionUsage(pair.second);
        }
        *m_outputStream << std::endl;
        // Options
        *m_outputStream << "Program Options: " << std::endl;
        for (const auto& pair : m_refMap)
            printOption(pair.second);
    }

    void Cli::registerOptionSizes(size_t optionSize, size_t secondOptionSize, size_t descriptionSize)
    {
        // Register largest parameter size to use for padding later on
        m_maxOptionParameterSize = optionSize > m_maxOptionParameterSize ? optionSize : m_maxOptionParameterSize;
        m_maxSecondOptionParameterSize = secondOptionSize > m_maxSecondOptionParameterSize ? secondOptionSize : m_maxSecondOptionParameterSize;
        m_maxDescriptionSize = descriptionSize > m_maxDescriptionSize ? descriptionSize : m_maxDescriptionSize;
    }

    void Cli::registerAlias(std::string option, std::string secondOption)
    {
        if (secondOption != "")
            m_aliasMap.insert(std::make_pair(secondOption, option));
    }

    void Cli::changeIO(std::ostream* ostream, std::istream* istream)
    {
        m_inputStream = istream;
        m_outputStream = ostream;
    }

    void Cli::multiArgParseError(std::string key, std::string value)
    {
        *m_outputStream << "Expected only one value to argument '" << key << "'" << "where -> '" << value << "' is not expected";
        printOptions();
        exit(1);
    }

    void Cli::unknownArgParsingError(std::string value)
    {
        *m_outputStream << "Given value -> '" << value << "' is not expected";
        printOptions();
        exit(1);
    }

    void Cli::userInputRequired()
    {
        m_askInputForMandatoryOptions = true;
    }

    void Cli::printOptionUsage(const Option& option)
    {
        if (option.ParseType == _detail::OptionParseType::Unidentified)
            if (option.Mandatory)
                *m_outputStream << option.Description << "(" << option.maxValueCount() << ") ";
            else
                *m_outputStream << "[" << option.Description << "(" << option.maxValueCount() << ")" << "] ";
        else
            *m_outputStream << parameterSyntax(option.Parameter, option.Mandatory);
    }

    void Cli::printOption(const Option& option)
    {
        if (option.ParseType != _detail::OptionParseType::Unidentified)
            *m_outputStream << std::left << std::setw(m_maxOptionParameterSize + 9) << parameterSyntax(option.Parameter, option.Mandatory) << std::left << std::setw(m_maxSecondOptionParameterSize + 10) << option.SecondParameter << option.Description << std::endl;
    }

    void Cli::unexpectedArgumentsAcceptable()
    {
        m_exitOnUnexpectedValue = false;
    }

    std::string Cli::parameterSyntax(std::string value, bool mandatory)
    {
        std::string str;
        if (mandatory)
            str.append("<").append(value).append(">");
        else
            str.append("[").append(value).append("]");
        return str;
    }
    namespace _detail
    {
        class OptionMismatchException
            : public std::exception
        {
            const char* err = "Tagless options cannot be combined with other options!";
            virtual const char* what() const noexcept override { return err; };
        };
    }
    void Cli::throwOnMismatch(bool tagless)
    {
        _detail::OptionMismatchException e;
        if ((ICli::tagless() && normal()) || (!ICli::tagless() && tagless) || (ICli::normal() && tagless))
            throw e;
    }
}
#endif