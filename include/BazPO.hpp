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
        ICli() = default;
        virtual ~ICli() = default;
        ICli(const ICli&) = delete;

        // Value or Multi Option
		virtual void add(const std::string& option, const std::string& secondOption = "", const std::string& description = "", const std::string& defaultValue = "", bool mandatory = false, bool multipleOptions = false, size_t maxValueCount = 1) = 0;
		// Tagless Option
        virtual void add(size_t valueCount = 1, const std::string& description = "", const std::string& defaultValue = "", bool mandatory = false) = 0;
		// Function Tagless Option
        virtual void add(std::function<void(const Option&)> onExists, size_t valueCount = 1, const std::string& description = "", const std::string& defaultValue = "", bool mandatory = false) = 0;
        // Function Value or Function Multi Option
		virtual void add(const std::string& option, std::function<void(const Option&)> onExists, const std::string& secondOption = "", const std::string& description = "", const std::string& defaultValue = "", bool mandatory = false, bool multipleOptions = false, size_t maxValueCount = 1) = 0;
        // Any Option Add
        virtual void add(Option& option) = 0;

        virtual void prioritize(const std::string& key) = 0;

    protected:
		virtual void setTagless() { m_taglessMode = true; }
		virtual void setNormal() { m_normalMode = true; }
		void setUndefined() { m_normalMode = false; m_taglessMode = false; }
		bool tagless() const { return m_taglessMode; }
		bool normal() const { return m_normalMode; }
        size_t getNextId() { ++m_taglessOptionNextId; return m_taglessOptionNextId; }
        size_t getCurrentId() const { return m_taglessOptionNextId; }

	private: 
		bool m_taglessMode = false;
		bool m_normalMode = false;
        size_t m_taglessOptionNextId = -1;
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
        template <typename T>
        T valueAs(const std::string& value)
        {
            std::stringstream ss;
            T v;
            ss << value;
            ss >> v;
            return v;
        }
        template <>
        bool valueAs(const std::string& value) { return (value == "1" || value == "True" || value == "true" || value == "t" || value == "y"); }
        template <>
        std::string valueAs(const std::string& value) { return value; }
        template <typename T>
        std::deque<T> valuesAs(const std::deque<const char*>& values)
        {
            std::deque<T> ret;
            for (auto it : values)
                ret.push_back(valueAs<T>(it));
            return ret;
        }
    }
    namespace _detail
    {
        class PrioritizationOptionMismatch
            : public std::exception
        {
            const char* err = "Tagless options cannot be prioritized!";
            const char* what() const noexcept override { return err; };
        };
    }
	class Option
	{
	protected:
        Option(const std::string& parameter, const std::string& secondParameter, const std::string& description, const std::string& defaultValue, bool mandatory, _detail::OptionParseType parser, ICli* po = nullptr)
            : Parameter(parameter)
            , SecondParameter(secondParameter)
            , Description(description)
            , ParseType(parser)
            , Mandatory(mandatory)
            , po(po)
		{
            if (defaultValue != "")
                Value = defaultValue.c_str();
            if (po != nullptr)
            {
                if (Parameter == "")
                    Parameter = std::to_string(po->getNextId());
                if (ParseType == _detail::OptionParseType::Unidentified)
                    po->setTagless();
                else
                    po->setNormal();
                po->add(*this);
            }
        }
	public:
		Option() = delete;
		Option(const Option&) = delete;
		virtual ~Option() = default;

		inline bool exists() const { return Exists; }
		inline int existsCount() const { return ExistsCount; }
		inline const char* value() const { return Value; }
        inline const std::deque<const char*>& values() const { return Values; }
        void prioritize() 
        {
            if (ParseType == _detail::OptionParseType::Unidentified)
                throw _detail::PrioritizationOptionMismatch();
            Prioritized = true;
            po->prioritize(Parameter);
        }
		template <typename T>
        inline T valueAs() const { return _detail::valueAs<T>(Value); }
		template <typename T>
        inline std::deque<T> valuesAs() const { return _detail::valuesAs<T>(Values); }

	protected: 
		void setValue(const char* value) { Value = value; Values.push_back(value); };
		virtual void execute(const Option&) const { /* there is nothing to execute by default */ };
		virtual size_t maxValueCount() const { return ParseType == _detail::OptionParseType::Value ? 1 : SIZE_MAX; }

    private:

		std::string Parameter;
		std::string SecondParameter;
		std::string Description;
		_detail::OptionParseType ParseType;
		bool Exists = false;
        int ExistsCount = 0;
		bool Mandatory = false;
        bool Prioritized = false;
        ICli* po;
        friend class Cli;

        const char* Value = "";
        std::deque<const char*> Values;

	};

	class ValueOption
		: public Option
	{
	public:
		ValueOption(ICli* po, const std::string& parameter, const std::string& secondParameter = "", const std::string& description = "", const std::string& defaultValue = "", bool mandatory = false)
			: Option(parameter, secondParameter, description, defaultValue, mandatory, _detail::OptionParseType::Value, po)
		{};
	};

	class MultiOption
		: public Option
	{
	public:
        MultiOption(ICli* po, const std::string& parameter, const std::string& secondParameter = "", const std::string& description = "", const std::string& defaultValue = "", bool mandatory = false, size_t maxValueCount = SIZE_MAX)
            : Option(parameter, secondParameter, description, defaultValue, mandatory, _detail::OptionParseType::MultiValue, po)
            , valueCount(maxValueCount)
        {};

    protected:
        virtual size_t maxValueCount() const override { return valueCount; }

    private:
        size_t valueCount;
	};

	class TaglessOption
		: public Option
	{
	public:
		TaglessOption(ICli* po, size_t valueCount = 1, const std::string& description = "", const std::string& defaultValue = "", bool mandatory = false)
			: Option("", "", description, defaultValue, mandatory, _detail::OptionParseType::Unidentified, po)
			, valueCount(valueCount)
		{};
	protected:
		virtual size_t maxValueCount() const override { return valueCount; }

	private:
		size_t valueCount;
	};
    namespace _detail
    {
        class FunctionExecutor
        {
        public:
            explicit FunctionExecutor(std::function<void(const Option&)> onExists)
                : f(onExists)
            {}
        protected:
            std::function<void(const Option&)> f;
        };
    }
	class FunctionOption
		: public ValueOption
        , protected _detail::FunctionExecutor
	{
	public:
        FunctionOption(ICli* po, const std::string& parameter, std::function<void(const Option&)> onExists, const std::string& secondParameter = "", const std::string& description = "", const std::string& defaultValue = "", bool mandatory = false)
			: ValueOption(po, parameter, secondParameter, description, defaultValue, mandatory)
			, _detail::FunctionExecutor(onExists)
		{}
        virtual void execute(const Option& option) const override { f(option); }
	};

    class FunctionMultiOption
        : public MultiOption
        , protected _detail::FunctionExecutor
    {
    public:
        FunctionMultiOption(ICli* po, const std::string& parameter, std::function<void(const Option&)> onExists, const std::string& secondParameter = "", const std::string& description = "", const std::string& defaultValue = "", bool mandatory = false, size_t maxValueCount = SIZE_MAX)
            : MultiOption(po, parameter, secondParameter, description, defaultValue, mandatory, maxValueCount)
            , _detail::FunctionExecutor(onExists)
        {}
        virtual void execute(const Option& option) const override { f(option); }
    };

    class FunctionTaglessOption
        : public TaglessOption
        , protected _detail::FunctionExecutor
    {
    public:
        FunctionTaglessOption(ICli* po, std::function<void(const Option&)> onExists, size_t valueCount = 1, const std::string& description = "", const std::string & defaultValue = "", bool mandatory = false)
            : TaglessOption(po, valueCount, description, defaultValue, mandatory)
            , _detail::FunctionExecutor(onExists)
        {}
        virtual void execute(const Option& option) const override { f(option); }
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
            prioritize("-h");
            setUndefined();
#endif
		};
		// Implementation of ICli
		virtual void add(const std::string& option, const std::string& secondOption = "", const std::string& description = "", const std::string& defaultValue = "", bool mandatory = false, bool multipleOptions = false, size_t maxValueCount = 1) override;
		virtual void add(const std::string& option, std::function<void(const Option&)> onExists, const std::string& secondOption = "", const std::string& description = "", const std::string& defaultValue = "", bool mandatory = false, bool multipleOptions = false, size_t maxValueCount = 1) override;
		virtual void add(size_t valueCount = 1, const std::string& description = "", const std::string& defaultValue = "", bool mandatory = false) override;
        virtual void add(std::function<void(const Option&)> onExists, size_t valueCount = 1, const std::string& description = "", const std::string& defaultValue = "", bool mandatory = false) override;
		virtual void add(Option& option) override;

        inline const Option& option(const std::string& option) const { return m_refMap.at(getKey(option)); }
		template <typename T>
		inline T valueAs(const std::string& option) const { return m_refMap.at(getKey(option)).valueAs<T>(); }
        inline bool exists(const std::string& option) const { return m_refMap.at(getKey(option)).Exists; }
        inline bool existsCount(const std::string& option) const { return m_refMap.at(getKey(option)).ExistsCount; }
        void askInput(Option& option);
        inline void askInput(const std::string& key) { askInput(m_refMap.at(getKey(key))); }
		void printOptions();
		void parse();
        virtual void prioritize(const std::string& key) final 
        { 
            auto mainKey = getKey(key);
            auto& option = m_refMap.at(mainKey);
            if (option.ParseType == _detail::OptionParseType::Unidentified)
                throw _detail::PrioritizationOptionMismatch();
            m_priorityMap.emplace(mainKey, option);
            option.Prioritized = true;
        };
		inline void changeIO(std::ostream* ostream, std::istream* istream = &std::cin) { m_inputStream = istream; m_outputStream = ostream; }
		inline void userInputRequired() { m_askInputForMandatoryOptions = true; }
		inline void unexpectedArgumentsAcceptable() { m_exitOnUnexpectedValue = false; }

	private:
		void parsePriority();
		void parseNormal();
		void parseTagless();
		void askUserInput();
		inline std::string getKey(const std::string& option) const { return (m_aliasMap.find(option) != m_aliasMap.end()) ? m_aliasMap.at(option) : option; }
		void registerOptionSizes(size_t optionSize, size_t secondOptionSize, size_t descriptionSize);
		void registerAlias(const std::string& option, const std::string& secondOption);
		void multiArgParseError(const std::string& key, const std::string& value);
        void unknownArgParsingError(const std::string& value);
		void printOptionUsage(const Option& option);
		void printOption(const Option& option);
		std::string parameterSyntax(const std::string& value, bool mandatory) const;
        void throwOnMismatch(bool tagless) const;

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
		std::map<std::string, Option&> m_priorityMap;
		std::map<std::string, std::string> m_aliasMap;
		std::vector<std::string> m_inputStorage;
		bool m_parsed = false;
		bool m_parsedPriority = false;
		bool m_askInputForMandatoryOptions = false;
		bool m_exitOnUnexpectedValue = true;

		std::istream* m_inputStream = &std::cin;
		std::ostream* m_outputStream = &std::cout;
	};
    void Cli::add(const std::string& option, const std::string& secondOption, const std::string& description, const std::string& defaultValue, bool mandatory, bool multipleOptions, size_t maxValueCount)
    {
        setNormal();
        throwOnMismatch(false);
        registerOptionSizes(option.size(), secondOption.size(), description.size());
        Option* optionRef = nullptr;
        if (multipleOptions)
        {
            if (maxValueCount == 1)
                maxValueCount = SIZE_MAX;
            m_multiValues.emplace_back(nullptr, option, secondOption, description, defaultValue, mandatory, maxValueCount);
            optionRef = &m_multiValues.back();
        }
        else
        {
            m_values.emplace_back(nullptr, option, secondOption, description, defaultValue, mandatory);
            optionRef = &m_values.back();
        }

        m_refMap.emplace(option, *optionRef);
        registerAlias(option, secondOption);
    }

    void Cli::add(const std::string& option, std::function<void(const Option&)> onExists, const std::string& secondOption, const std::string& description, const std::string& defaultValue, bool mandatory, bool multipleOptions, size_t maxValueCount)
    {
        setNormal();
        throwOnMismatch(false);
        registerOptionSizes(option.size(), secondOption.size(), description.size());
        Option* optionRef = nullptr;
        if (multipleOptions)
        {
            if (maxValueCount == 1)
                maxValueCount = SIZE_MAX;
            m_functionalMultiValues.emplace_back(nullptr, option, onExists, secondOption, description, defaultValue, mandatory, maxValueCount);
            optionRef = &m_functionalMultiValues.back();
        }
        else
        {
            m_functionalValues.emplace_back(nullptr, option, onExists, secondOption, description, defaultValue, mandatory);
            optionRef = &m_functionalValues.back();
        }

        m_refMap.emplace(option, *optionRef);
        registerAlias(option, secondOption);
    }

    void Cli::add(size_t valueCount, const std::string& description, const std::string& defaultValue, bool mandatory)
    {
        setTagless();
        throwOnMismatch(true);
        registerOptionSizes(getNextId() % 10 + 1, 0, description.size());
        m_taglessOptions.emplace_back(nullptr, valueCount, description, defaultValue, mandatory);

        auto id = std::to_string(getCurrentId());
        m_refMap.emplace(id , m_taglessOptions.back());
    }

    void Cli::add(std::function<void(const Option&)> onExists, size_t valueCount, const std::string& description, const std::string& defaultValue, bool mandatory)
    {
        setTagless();
        throwOnMismatch(true);
        registerOptionSizes(getNextId() % 10 + 1, 0, description.size());
        m_functionalTaglessValues.emplace_back(nullptr, onExists, valueCount, description, defaultValue, mandatory);

        auto id = std::to_string(getCurrentId());
        m_refMap.emplace(id , m_functionalTaglessValues.back());
    }

    void Cli::add(Option& option)
    {
        throwOnMismatch(option.ParseType == _detail::OptionParseType::Unidentified);
        registerOptionSizes(option.Parameter.size(), option.SecondParameter.size(), option.Description.size());
        m_refMap.emplace(option.Parameter, option);
        registerAlias(option.Parameter, option.SecondParameter);
    }

    void Cli::parse()
    {
        if (!m_parsed)
        {
            parsePriority();
            if (!m_parsedPriority)
            {
                if (tagless())
                    parseTagless();
                else
                    parseNormal();
                // At this moment we know if options are present or not
                askUserInput();
                m_parsed = true;
                // execute actions
                for (const auto& it : m_refMap)
                    if (it.second.Exists)
                        it.second.execute(it.second);
            }
            else 
                for(const auto& it : m_priorityMap)
                    if (it.second.Exists)
                        it.second.execute(it.second);
        }
    }
    void Cli::parsePriority()
    {
        if (m_priorityMap.empty()) return;
        Option* lastOption = nullptr;
        for (int i = 1; i < m_argc; ++i)
        {
            auto key = getKey(m_argv[i]);
            auto option = m_priorityMap.find(key);
            if (option != m_priorityMap.end())
            {
                m_parsedPriority = true;
                option->second.Exists = true;
                ++option->second.ExistsCount;
                if (option->second.maxValueCount() == 0)
                    break;
                if ((i + 1) < m_argc && m_priorityMap.find(getKey(m_argv[i + 1])) == m_priorityMap.end() && m_refMap.find(getKey(m_argv[i + 1])) == m_refMap.end())
                    option->second.setValue(m_argv[i + 1]);
                if (option->second.ParseType == _detail::OptionParseType::Value)
                    break;
                lastOption = &option->second;
            }
            else if (lastOption != nullptr)
            {
                if ((i + 1) < m_argc && m_priorityMap.find(getKey(m_argv[i + 1])) == m_priorityMap.end() && m_refMap.find(getKey(m_argv[i + 1])) == m_refMap.end() && lastOption->ParseType == _detail::OptionParseType::MultiValue)
                {
                    if (lastOption->maxValueCount() > lastOption->values().size())
                        lastOption->setValue(m_argv[i + 1]);
                    else
                        break;
                }
            }
        }
    }
    void Cli::parseTagless()
    {
        int cursor = 1;
        for (auto& it : m_refMap)
        {
            if (it.second.ParseType == _detail::OptionParseType::Unidentified)
                for (size_t i = 0; cursor < m_argc && i < it.second.maxValueCount(); ++cursor, ++i)
                {
                    it.second.Exists = true;
                    ++it.second.ExistsCount;
                    it.second.setValue(m_argv[cursor]);
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
                    askInput(pair.second);
                else
                {
                    printOptions();
                    exit(-1);
                }
            }
        }
    }

    inline void Cli::askInput(Option& option)
    {
        printOptionUsage(option);
        *m_outputStream << ": ";
        std::string temp;
        std::getline(*m_inputStream, temp);

        m_inputStorage.emplace_back(temp);
        option.setValue(m_inputStorage.back().c_str());
        option.Exists = true;
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

    void Cli::registerAlias(const std::string& option, const std::string& secondOption)
    {
        if (secondOption != "")
            m_aliasMap.emplace(secondOption, option);
    }

    void Cli::multiArgParseError(const std::string& key, const std::string& value)
    {
        *m_outputStream << "Expected only one value to argument '" << key << "'" << "where -> '" << value << "' is not expected";
        printOptions();
        exit(1);
    }

    void Cli::unknownArgParsingError(const std::string& value)
    {
        *m_outputStream << "Given value -> '" << value << "' is not expected";
        printOptions();
        exit(1);
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

    std::string Cli::parameterSyntax(const std::string& value, bool mandatory) const
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
            const char* what() const noexcept override { return err; };
        };
    }
    void Cli::throwOnMismatch(bool tagless) const
    {
        _detail::OptionMismatchException e;
        if ((ICli::tagless() && normal()) || (!ICli::tagless() && tagless) || (ICli::normal() && tagless))
            throw e;
    }
}
#endif
