#ifndef BAZ_PO_HPP
#define BAZ_PO_HPP

/*
BazPO is a flexible C++14 single header program argument parsing library.
Copyright (c) 2022 Baris Tanyeri
https://github.com/karusb/BazPO
MIT License

Copyright (c) 2022 Baris Tanyeri

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

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
    class Constraint;
    class MultiConstraint;
    class ICli
    {
    public:
        ICli() = default;
        virtual ~ICli() = default;
        ICli(const ICli&) = delete;

        // Value or Multi Option
        virtual Option& add(const std::string& option, const std::string& secondOption = "", const std::string& description = "", const std::string& defaultValue = "", bool multipleOptions = false, size_t maxValueCount = 1) = 0;
        // Tagless Option
        virtual Option& add(size_t valueCount = 1, const std::string& description = "", const std::string& defaultValue = "") = 0;
        // Function Tagless Option
        virtual Option& add(const std::function<void(const Option&)>& onExists, size_t valueCount = 1, const std::string& description = "", const std::string& defaultValue = "") = 0;
        // Function Value or Function Multi Option
        virtual Option& add(const std::string& option, const std::function<void(const Option&)>& onExists, const std::string& secondOption = "", const std::string& description = "", const std::string& defaultValue = "", bool multipleOptions = false, size_t maxValueCount = 1) = 0;
        // Any Option Add
        virtual void add(Option& option) = 0;
        // Prioritize Option
        virtual Option& prioritize(const std::string& key) = 0;
        // Mandatory Option
        virtual Option& mandatory(const std::string& key) = 0;

    protected:
        // Program exit
        virtual void exitWithCode(int code) = 0;
        virtual void printOptionUsage(const Option& option) = 0;
        virtual void printOption(const Option& option) = 0;
        virtual std::string parameterSyntax(const std::string& value, bool mandatory) const = 0;
        virtual void conversionError(const std::string& value, const std::string& parameter) = 0;

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
        friend class MultiConstraint;
    };

    namespace _detail
    {
        enum class OptionParseType
        {
            // Single Value
            Value,
            // Multiple Values Possible, via single tag "-o val1 val2" 
            MultiValue,
            // Tagless
            Unidentified
        };
        template <typename T>
        std::pair<T, bool> valueAs(const std::string& value)
        {
            std::stringstream ss;
            T v;
            ss << value;
            ss >> v;
            if (ss.fail())
                return { v, true };
            return { v, false };
        }
        template <>
        std::pair<bool, bool> valueAs(const std::string& value) { return { (value == "1" || value == "True" || value == "true" || value == "t" || value == "y"), false }; }
        template <>
        std::pair<std::string, bool> valueAs(const std::string& value) { return { value, false }; }
        template <typename T>
        std::pair<std::deque<T>, bool> valuesAs(const std::deque<const char*>& values)
        {
            std::deque<T> ret;
            for (auto it : values)
            {
                auto val = valueAs<T>(it);
                ret.push_back(val.first);
                if (val.second)
                    return { ret, true };
            }
            return { ret, false };
        }

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
        Option& prioritize()
        {
            if (ParseType == _detail::OptionParseType::Unidentified)
                throw _detail::PrioritizationOptionMismatch();
            Prioritized = true;
            po->prioritize(Parameter);
            return *this;
        }
        Option& withMaxValueCount(size_t count) { MaxValueCount = count; return *this; }
        Option& mandatory() { Mandatory = !Mandatory; return *this; }
        Option& constrain(std::deque<std::string> stringConstraints);
        template<typename T>
        Option& constrain(std::pair<T, T> minMaxConstraints);
        Option& constrain(const std::function<bool(const Option&)>& isSatisfied, const std::string& errorMessage);
        Option& constrain(Constraint& contraint) { Constrained = &contraint; return *this; };

        template <typename T>
        inline T valueAs() const
        {
            auto valPair = _detail::valueAs<T>(Value);
            if (valPair.second)
                po->conversionError(Value, Parameter);
            return valPair.first;
        }
        template <typename T>
        inline std::deque<T> valuesAs() const
        {
            auto valPair = _detail::valuesAs<T>(Values);
            if (valPair.second)
                po->conversionError(Values[valPair.first.size() - 1], Parameter);
            return valPair.first;
        }

    protected:
        void setValue(const char* value) { Value = value; Values.push_back(value); };
        virtual void execute(const Option&) const { /* there is nothing to execute by default */ };
        size_t maxValueCount() const { return MaxValueCount; }

    private:
        void setCli(ICli& cli) { po = &cli; }

        std::string Parameter;
        std::string SecondParameter;
        std::string Description;
        _detail::OptionParseType ParseType;
        bool Exists = false;
        int ExistsCount = 0;
        bool Mandatory = false;
        bool Prioritized = false;
        Constraint* Constrained = nullptr;
        MultiConstraint* MultiConstrained = nullptr;
        std::shared_ptr<Constraint> ConstraintStorage;

        ICli* po;
        friend class Cli;
        friend class Constraint;
        friend class MultiConstraint;

        const char* Value = "";
        std::deque<const char*> Values;
        size_t MaxValueCount = 1;
    };

    class Constraint
    {
    protected:
        explicit Constraint(Option& option)
            : option(option)
        {
            option.Constrained = this;
        }
    public:
        Constraint() = delete;
        Constraint(const Constraint&) = delete;
        virtual ~Constraint() = default;

        virtual bool satisfied() const = 0;
        virtual std::string what() const = 0;
    protected:
        Option& option;
    };

    class StringConstraint
        : public Constraint
    {
    public:
        StringConstraint(Option& option, const std::deque<std::string>& stringConstraints)
            : Constraint(option)
            , constraints(stringConstraints)
        {}

        virtual bool satisfied() const override
        {
            for (auto& constraint : constraints)
                if (constraint == option.value())
                    return true;
            return false;
        };
        virtual std::string what() const override
        {
            std::string str;
            str.append("value either to be ");
            for (auto& constraint : constraints)
                str.append(constraint).append(", ");
            return str;
        };
    private:
        std::deque<std::string> constraints;
    };
    class FunctionConstraint
        : public Constraint
    {
    public:
        FunctionConstraint(Option& option, const std::function<bool(const Option&)>& isSatisfied, const std::string& errorMessage)
            : Constraint(option)
            , isSatisfied(isSatisfied)
            , errorMessage(errorMessage)
        {}

        virtual bool satisfied() const override { return isSatisfied(option); };
        virtual std::string what() const override { return errorMessage; };
    private:
        std::function<bool(const Option&)> isSatisfied;
        std::string errorMessage;
    };
    template <typename T>
    class MinMaxConstraint
        : public Constraint
    {
    public:
        MinMaxConstraint(Option& option, const std::pair<T, T>& minMaxConstraint)
            : Constraint(option)
            , constraint(minMaxConstraint)
        {}

        virtual bool satisfied() const override
        {
            T val = option.valueAs<T>();
            if (val >= constraint.first && val <= constraint.second)
                return true;
            return false;
        };
        virtual std::string what() const override
        {
            std::string str;
            str.append("values to be between ");
            str.append(std::to_string(constraint.first)).append(", ").append(std::to_string(constraint.second));
            return str;
        };

    private:
        std::pair<T, T> constraint;
    };

    class MultiConstraint
    {
    public:
        MultiConstraint() = delete;
        MultiConstraint(const MultiConstraint&) = delete;
        virtual ~MultiConstraint() = default;

    protected:
        template <typename... Options>
        explicit MultiConstraint(ICli* po, Option& option1, Option& option2, Options&... rest)
            : cli(po)
        {
            addOptions(option1, option2, rest...);
        }

        virtual std::deque<Option*> satisfiedOptions()
        {
            std::deque<Option*> ret;
            for (auto& pair : relativeOptions)
                if (pair.second)
                    ret.push_back(pair.first);
            return ret;
        }

        virtual bool satisfied(Option& foundOption) = 0;
        virtual std::string what() = 0;

        std::string parameterSyntax(const Option& option) const { return cli->parameterSyntax(option.Parameter, option.Mandatory); }

        std::deque<std::pair<Option*, bool>> relativeOptions;
        ICli* cli = nullptr;
    private:
        void addOptions(Option& option1) { option1.MultiConstrained = this; relativeOptions.emplace_back(&option1, false); }
        template <typename... Options>
        void addOptions(Option& option1, Options&... rest) { addOptions(option1); addOptions(rest...); }

        friend class Cli;
    };

    Option& Option::constrain(std::deque<std::string> stringConstraints) { ConstraintStorage = std::make_shared<StringConstraint>(*this, stringConstraints); return *this; };
    template<typename T>
    Option& Option::constrain(std::pair<T, T> minMaxConstraints) { ConstraintStorage = std::make_shared<MinMaxConstraint<T>>(*this, minMaxConstraints); return *this; };
    Option& Option::constrain(const std::function<bool(const Option&)>& isSatisfied, const std::string& errorMessage) { ConstraintStorage = std::make_shared<FunctionConstraint>(*this, isSatisfied, errorMessage); return *this; };

    class EitherMandatory
        : public MultiConstraint
    {
    public:
        template <typename... Options>
        explicit EitherMandatory(ICli* po, Option& option1, Option& option2, Options&... rest)
            : MultiConstraint(po, option1, option2, rest...)
        {
            makeMandatory(option1, option2, rest...);
        }

        Option* satisfiedOption() const { return chosenOption; }
    protected:
        virtual bool satisfied(Option& foundOption) override
        {
            auto totalExist = 0;
            for (auto& option : relativeOptions)
            {
                totalExist += option.first->exists();
                if (totalExist > 1)
                    return false;
                else if (option.first == &foundOption && chosenOption == nullptr && foundOption.exists())
                {
                    option.second = true;
                    chosenOption = option.first;
                    setRelativesNotMandatory();
                }
            }
            return chosenOption != nullptr;
        }

        virtual std::string what() override
        {
            std::string msg;
            msg.append("Only one of the ");
            for (auto& option : relativeOptions)
            {
                msg.append(parameterSyntax(*option.first));
                msg.append(", ");
            }
            msg.append(" parameters must be provided");
            return msg;
        }

    private:
        void makeMandatory(Option& option1) const { option1.mandatory(); }
        template <typename... Options>
        void makeMandatory(Option& option1, Options&... rest) const { makeMandatory(option1); makeMandatory(rest...); }
        void setRelativesNotMandatory()
        {
            for (auto& option : relativeOptions)
                option.first->mandatory();
        }

        Option* chosenOption = nullptr;
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
        {
            withMaxValueCount(maxValueCount);
        };
    };

    class TaglessOption
        : public Option
    {
    public:
        TaglessOption(ICli* po, size_t valueCount = 1, const std::string& description = "", const std::string& defaultValue = "", bool mandatory = false)
            : Option("", "", description, defaultValue, mandatory, _detail::OptionParseType::Unidentified, po)
        {
            withMaxValueCount(valueCount);
        };
    };
    namespace _detail
    {
        class FunctionExecutor
        {
        public:
            explicit FunctionExecutor(const std::function<void(const Option&)>& onExists)
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
        FunctionOption(ICli* po, const std::string& parameter, const std::function<void(const Option&)>& onExists, const std::string& secondParameter = "", const std::string& description = "", const std::string& defaultValue = "", bool mandatory = false)
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
        FunctionMultiOption(ICli* po, const std::string& parameter, const std::function<void(const Option&)>& onExists, const std::string& secondParameter = "", const std::string& description = "", const std::string& defaultValue = "", bool mandatory = false, size_t maxValueCount = SIZE_MAX)
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
        FunctionTaglessOption(ICli* po, const std::function<void(const Option&)>& onExists, size_t valueCount = 1, const std::string& description = "", const std::string& defaultValue = "", bool mandatory = false)
            : TaglessOption(po, valueCount, description, defaultValue, mandatory)
            , _detail::FunctionExecutor(onExists)
        {}
        virtual void execute(const Option& option) const override { f(option); }
    };

    class Cli
        : public ICli
    {
    public:
        Cli(int argc, const char* argv[], const char* programDescription = "")
            : m_argc(argc)
            , m_argv(argv)
            , m_programDescription(programDescription)
        {
#ifndef BazPO_DISABLE_AUTO_HELP_MESSAGE
            add("-h", [this](const Option&) { exitWithCode(0); }, "--help", "Prints this help message").prioritize();
            setUndefined();
#endif
        };
        // Implementation of ICli
        virtual Option& add(const std::string& option, const std::string& secondOption = "", const std::string& description = "", const std::string& defaultValue = "", bool multipleOptions = false, size_t maxValueCount = 1) override;
        virtual Option& add(const std::string& option, const std::function<void(const Option&)>& onExists, const std::string& secondOption = "", const std::string& description = "", const std::string& defaultValue = "", bool multipleOptions = false, size_t maxValueCount = 1) override;
        virtual Option& add(size_t valueCount = 1, const std::string& description = "", const std::string& defaultValue = "") override;
        virtual Option& add(const std::function<void(const Option&)>& onExists, size_t valueCount = 1, const std::string& description = "", const std::string& defaultValue = "") override;
        virtual void add(Option& option) override;
        virtual Option& prioritize(const std::string& key) final
        {
            auto mainKey = getKey(key);
            auto& option = m_refMap.at(mainKey);
            if (option.ParseType == _detail::OptionParseType::Unidentified)
                throw _detail::PrioritizationOptionMismatch();

            m_priorityMap.emplace(mainKey, option);
            option.Prioritized = true;
            return option;
        };
        virtual Option& mandatory(const std::string& key) final { return m_refMap.at(getKey(key)).mandatory(); }
        inline const Option& option(const std::string& option) const { return m_refMap.at(getKey(option)); }
        template <typename T>
        inline T valueAs(const std::string& option) const { return m_refMap.at(getKey(option)).valueAs<T>(); }
        inline bool exists(const std::string& option) const { return m_refMap.at(getKey(option)).Exists; }
        inline bool existsCount(const std::string& option) const { return m_refMap.at(getKey(option)).ExistsCount; }
        void askInput(Option& option);
        inline void askInput(const std::string& key) { askInput(m_refMap.at(getKey(key))); }
        void printOptions();
        void parse();
        inline void changeIO(std::ostream* ostream, std::istream* istream = &std::cin) { m_inputStream = istream; m_outputStream = ostream; }
        inline void userInputRequired() { m_askInputForMandatoryOptions = true; }
        inline void unexpectedArgumentsAcceptable() { m_exitOnUnexpectedValue = false; }
        template<typename... Options>
        EitherMandatory& eitherMandatory(Options&... options) { m_multiConstraintStorage.push_back(std::make_shared<EitherMandatory>(this, m_refMap.at(getKey(options))...)); return reinterpret_cast<EitherMandatory&>(*m_multiConstraintStorage.back()); }
        Option& constraint(const std::string& key, std::deque<std::string> stringConstraints) { return m_refMap.at(getKey(key)).constrain(stringConstraints); };
        template<typename T>
        Option& constraint(const std::string& key, std::pair<T, T> minMaxConstraints) { return m_refMap.at(getKey(key)).constrain<T>(minMaxConstraints); };
        Option& constraint(const std::string& key, const std::function<bool(const Option&)>& isSatisfied, const std::string& errorMessage) { return m_refMap.at(getKey(key)).constrain(isSatisfied, errorMessage); };

    private:
        virtual void conversionError(const std::string& value, const std::string& parameter) override;
        virtual void exitWithCode(int code) override { printOptions(); exit(code); };
        virtual void printOptionUsage(const Option& option) override;
        virtual void printOption(const Option& option) override;
        virtual std::string parameterSyntax(const std::string& value, bool mandatory) const override;
        std::string sizeSyntax(size_t value) const;

        void parsePriority();
        void parseNormal();
        void parseTagless();
        void checkMandatoryOptions();
        inline std::string getKey(const std::string& option) const { return (m_aliasMap.find(option) != m_aliasMap.end()) ? m_aliasMap.at(option) : option; }
        void registerOptionSizes(size_t optionSize, size_t secondOptionSize, size_t descriptionSize);
        void registerAlias(const std::string& option, const std::string& secondOption);
        void multiArgParseError(const std::string& key, const std::string& value);
        void unknownArgParsingError(const std::string& value);
        void constraintError(const std::string& constraints, const std::string& value, const std::string& parameter);
        void multiConstraintError(const std::string& message);
        void throwOnMismatch(bool tagless) const;

        size_t m_maxOptionParameterSize = 0;
        size_t m_maxSecondOptionParameterSize = 0;
        size_t m_maxDescriptionSize = 0;
        const int m_argc;
        const char** m_argv;
        const char* m_programDescription;

        std::deque<std::shared_ptr<Option>> m_optionStorage;
        std::deque<std::shared_ptr<MultiConstraint>> m_multiConstraintStorage;
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

    Option& Cli::add(const std::string& option, const std::string& secondOption, const std::string& description, const std::string& defaultValue, bool multipleOptions, size_t maxValueCount)
    {
        setNormal();
        throwOnMismatch(false);
        registerOptionSizes(option.size(), secondOption.size(), description.size());
        if (multipleOptions)
        {
            if (maxValueCount == 1)
                maxValueCount = SIZE_MAX;
            m_optionStorage.push_back(std::make_shared<MultiOption>(nullptr, option, secondOption, description, defaultValue, false, maxValueCount));
        }
        else
            m_optionStorage.push_back(std::make_shared<ValueOption>(nullptr, option, secondOption, description, defaultValue, false));

        m_refMap.emplace(option, *m_optionStorage.back()).first->second.setCli(*this);
        registerAlias(option, secondOption);
        return *m_optionStorage.back();
    }

    Option& Cli::add(const std::string& option, const std::function<void(const Option&)>& onExists, const std::string& secondOption, const std::string& description, const std::string& defaultValue, bool multipleOptions, size_t maxValueCount)
    {
        setNormal();
        throwOnMismatch(false);
        registerOptionSizes(option.size(), secondOption.size(), description.size());
        if (multipleOptions)
        {
            if (maxValueCount == 1)
                maxValueCount = SIZE_MAX;
            m_optionStorage.push_back(std::make_shared<FunctionMultiOption>(nullptr, option, onExists, secondOption, description, defaultValue, false, maxValueCount));
        }
        else
            m_optionStorage.push_back(std::make_shared<FunctionOption>(nullptr, option, onExists, secondOption, description, defaultValue, false));

        m_refMap.emplace(option, *m_optionStorage.back()).first->second.setCli(*this);
        registerAlias(option, secondOption);
        return *m_optionStorage.back();
    }

    Option& Cli::add(size_t valueCount, const std::string& description, const std::string& defaultValue)
    {
        setTagless();
        throwOnMismatch(true);
        registerOptionSizes(getNextId() % 10 + 1, 0, description.size());
        m_optionStorage.push_back(std::make_shared<TaglessOption>(nullptr, valueCount, description, defaultValue, false));

        m_refMap.emplace(std::to_string(getCurrentId()), *m_optionStorage.back()).first->second.setCli(*this);
        return *m_optionStorage.back();
    }

    Option& Cli::add(const std::function<void(const Option&)>& onExists, size_t valueCount, const std::string& description, const std::string& defaultValue)
    {
        setTagless();
        throwOnMismatch(true);
        registerOptionSizes(getNextId() % 10 + 1, 0, description.size());
        m_optionStorage.push_back(std::make_shared<FunctionTaglessOption>(nullptr, onExists, valueCount, description, defaultValue, false));

        m_refMap.emplace(std::to_string(getCurrentId()), *m_optionStorage.back()).first->second.setCli(*this);
        return *m_optionStorage.back();
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
        if (m_parsed)
            return;

        parsePriority();
        if (!m_parsedPriority)
        {
            if (tagless())
                parseTagless();
            else
                parseNormal();

            checkMandatoryOptions();
            for (const auto& it : m_refMap)
                if (it.second.MultiConstrained != nullptr && !it.second.MultiConstrained->satisfied(it.second))
                    multiConstraintError(it.second.MultiConstrained->what());
            m_parsed = true;

            for (const auto& it : m_refMap)
                if (it.second.Exists)
                    it.second.execute(it.second);
        }
        else
            for (const auto& it : m_priorityMap)
                if (it.second.Exists)
                    it.second.execute(it.second);

    }
    void Cli::parsePriority()
    {
        if (m_priorityMap.empty())
            return;
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
                lastOption = &option->second;
            }
            else if (lastOption != nullptr)
            {
                if (m_refMap.find(key) == m_refMap.end())
                {
                    if (lastOption->maxValueCount() > lastOption->values().size() || lastOption->ParseType == _detail::OptionParseType::Value)
                        lastOption->setValue(m_argv[i]);
                    else
                        break;
                    if (lastOption->ParseType == _detail::OptionParseType::Value)
                        lastOption = nullptr;
                }
                else
                    break;
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
                    if (it.second.Constrained != nullptr && !it.second.Constrained->satisfied())
                        constraintError(it.second.Constrained->what(), it.second.value(), it.second.Parameter);
                    if (it.second.MultiConstrained != nullptr && !it.second.MultiConstrained->satisfied(it.second))
                        multiConstraintError(it.second.MultiConstrained->what());
                }
        }
        if (cursor < m_argc && m_exitOnUnexpectedValue)
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
                lastOption = &option->second;
            }
            else if (lastOption != nullptr)
            {
                if (lastOption->maxValueCount() > lastOption->values().size() || lastOption->ParseType == _detail::OptionParseType::Value)
                    lastOption->setValue(m_argv[i]);
                else if (m_exitOnUnexpectedValue)
                    multiArgParseError(lastOption->Parameter, m_argv[i]);
                if (lastOption->Constrained != nullptr && !lastOption->Constrained->satisfied())
                    constraintError(lastOption->Constrained->what(), lastOption->value(), lastOption->Parameter);
                if (lastOption->MultiConstrained != nullptr && !lastOption->MultiConstrained->satisfied(*lastOption))
                    multiConstraintError(lastOption->MultiConstrained->what());
                if (lastOption->ParseType == _detail::OptionParseType::Value)
                    lastOption = nullptr;
            }
            else if (m_exitOnUnexpectedValue)
                unknownArgParsingError(m_argv[i]);
        }
    }

    void Cli::checkMandatoryOptions()
    {
        for (auto& pair : m_refMap)
        {
            if (pair.second.Mandatory && !pair.second.Exists)
            {
                printOptionUsage(pair.second);
                *m_outputStream << " is a required parameter" << std::endl;
                if (m_askInputForMandatoryOptions)
                    askInput(pair.second);
                else if (m_exitOnUnexpectedValue)
                    exitWithCode(1);
            }
        }
    }

    inline void Cli::askInput(Option& option)
    {
        printOptionUsage(option);
        *m_outputStream << ": ";
        std::string temp;
        std::getline(*m_inputStream, temp);

        if (!temp.empty())
        {
            m_inputStorage.emplace_back(temp);
            option.setValue(m_inputStorage.back().c_str());
            option.Exists = true;
            ++option.ExistsCount;
            if (option.Constrained != nullptr && !option.Constrained->satisfied())
                constraintError(option.Constrained->what(), option.value(), option.Parameter);
            if (option.MultiConstrained != nullptr && !option.MultiConstrained->satisfied(option))
                multiConstraintError(option.MultiConstrained->what());
        }
        else if (option.Mandatory && m_exitOnUnexpectedValue && option.MultiConstrained == nullptr)
            exitWithCode(1);
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

    void Cli::constraintError(const std::string& constraints, const std::string& value, const std::string& parameter)
    {
        *m_outputStream << "Expected " << constraints << " where -> '" << value << "' is not expected for option " << parameter;
        exitWithCode(1);
    }

    void Cli::multiConstraintError(const std::string& message)
    {
        *m_outputStream << message << std::endl;
        exitWithCode(1);
    }

    void Cli::conversionError(const std::string& value, const std::string& parameter)
    {
        *m_outputStream << "Type of value '" << value << "' is not expected for option " << parameter;
        exitWithCode(1);
    }

    void Cli::multiArgParseError(const std::string& key, const std::string& value)
    {
        *m_outputStream << "Expected only one value to argument '" << key << "'" << "where -> '" << value << "' is not expected";
        exitWithCode(1);
    }

    void Cli::unknownArgParsingError(const std::string& value)
    {
        *m_outputStream << "Given value -> '" << value << "' is not expected";
        exitWithCode(1);
    }

    void Cli::printOptionUsage(const Option& option)
    {
        if (option.ParseType == _detail::OptionParseType::Unidentified)
            if (option.Mandatory)
                *m_outputStream << option.Description << sizeSyntax(option.maxValueCount()) << " ";
            else
                *m_outputStream << "[" << option.Description << sizeSyntax(option.maxValueCount()) << "] ";
        else
            *m_outputStream << parameterSyntax(option.Parameter, option.Mandatory);
    }

    void Cli::printOption(const Option& option)
    {
        if (option.ParseType != _detail::OptionParseType::Unidentified)
            *m_outputStream << std::left << std::setw(m_maxOptionParameterSize + 9) << parameterSyntax(option.Parameter, option.Mandatory)
            << std::left << std::setw(m_maxSecondOptionParameterSize + 10) << option.SecondParameter << option.Description << std::endl;
        else
            *m_outputStream << std::left << std::setw(m_maxOptionParameterSize + 9 + m_maxSecondOptionParameterSize + 10) << parameterSyntax(option.Description, option.Mandatory)
            << sizeSyntax(option.maxValueCount()) << std::endl;
    }

    std::string Cli::sizeSyntax(size_t value) const
    {
        std::string str;
        if (value == 1)
            str.append("(").append(std::to_string(value)).append(")");
        else if (value == SIZE_MAX)
            str.append("(").append("*...)");
        else if (value > 1)
            str.append("(").append(std::to_string(value)).append("...)");
        return str;
    }

    std::string Cli::parameterSyntax(const std::string& value, bool mandatory) const
    {
        std::string str;
        if (mandatory)
            str.append("<").append(value).append(">");
        else if (value == "")
            return str;
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
