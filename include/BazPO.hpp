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
#include <algorithm>

namespace BazPO
{
    class Option;
    class Constraint;
    class MultiConstraint;
    enum class OptionType
    {
        Value,
        MultiValue
    };
    class ICli
    {
    public:
        ICli() = default;
        virtual ~ICli() = default;
        ICli(const ICli&) = delete;

        // Value or Multi Option
        virtual Option& option(const std::string& option, const std::string& secondOption = "", const std::string& description = "", const std::string& defaultValue = "", OptionType optionType = OptionType::Value, size_t maxValueCount = SIZE_MAX) = 0;
        // Function Value or Function Multi Option
        virtual Option& option(const std::string& option, const std::function<void(const Option&)>& onExists, const std::string& secondOption = "", const std::string& description = "", const std::string& defaultValue = "", OptionType optionType = OptionType::Value, size_t maxValueCount = SIZE_MAX) = 0;
        // Flag Option
        virtual Option& flag(const std::string& option, const std::string& description = "", const std::string& secondOption = "") = 0;
        // Function Flag Option
        virtual Option& flag(const std::string& option, const std::function<void(const Option&)>& onExists, const std::string& description = "", const std::string& secondOption = "") = 0;
        // Tagless Option
        virtual Option& tagless(size_t valueCount = 1, const std::string& description = "", const std::string& defaultValue = "") = 0;
        // Function Tagless Option
        virtual Option& tagless(const std::function<void(const Option&)>& onExists, size_t valueCount = 1, const std::string& description = "", const std::string& defaultValue = "") = 0;
        // Any Option Add
        virtual void option(Option& option) = 0;
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

        int getNextId() { ++m_taglessOptionNextId; return m_taglessOptionNextId; }
        int getCurrentId() const { return m_taglessOptionNextId; }

    private:
        int m_taglessOptionNextId = -1;

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
            return { v, ss.fail() };
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
                ret.emplace_back(val.first);
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

                po->option(*this);
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
        Option& mandatory() { Mandatory = true; return *this; }
        Option& constrain(std::deque<std::string> stringConstraints);
        template<typename T>
        Option& constrain(std::pair<T, T> minMaxConstraints);
        Option& constrain(const std::function<bool(const Option&)>& isSatisfied, const std::string& errorMessage);
        Option& constrain(Constraint& contraint) { Constrained.emplace_back(&contraint); return *this; };

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
        void setValue(const char* value) { Value = value; Values.emplace_back(value); };
        virtual void execute(const Option&) const { /* there is nothing to execute by default */ };
        size_t maxValueCount() const { return MaxValueCount; }
        void notMandatory() { Mandatory = false; }

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
        std::deque<Constraint*> Constrained;
        std::deque<MultiConstraint*> MultiConstrained;
        std::deque<std::shared_ptr<Constraint>> ConstraintStorage;

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
            option.constrain(*this);
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

        virtual bool satisfied() const override { return std::any_of(constraints.begin(), constraints.end(), [this](const std::string& constraint) { return constraint == option.value(); });};
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
            return val >= constraint.first && val <= constraint.second;
        };
        virtual std::string what() const override { return std::string("values to be between ").append(std::to_string(constraint.first)).append(", ").append(std::to_string(constraint.second)); };

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
                    ret.emplace_back(pair.first);
            return ret;
        }

        virtual bool satisfied(Option& foundOption) = 0;
        virtual std::string what() = 0;

        bool isMandatory(const Option& option) const { return option.Mandatory; }
        std::string parameterSyntax(const Option& option) const { return cli->parameterSyntax(option.Parameter, option.Mandatory); }

        std::deque<std::pair<Option*, bool>> relativeOptions;
        ICli* cli = nullptr;
    private:
        void addOptions(Option& option1) { option1.MultiConstrained.emplace_back(this); relativeOptions.emplace_back(&option1, false); }
        template <typename... Options>
        void addOptions(Option& option1, Options&... rest) { addOptions(option1); addOptions(rest...); }

        friend class Cli;
    };

    Option& Option::constrain(std::deque<std::string> stringConstraints) { ConstraintStorage.emplace_back(std::make_shared<StringConstraint>(*this, stringConstraints)); return *this; };
    template<typename T>
    Option& Option::constrain(std::pair<T, T> minMaxConstraints) { ConstraintStorage.emplace_back(std::make_shared<MinMaxConstraint<T>>(*this, minMaxConstraints)); return *this; };
    Option& Option::constrain(const std::function<bool(const Option&)>& isSatisfied, const std::string& errorMessage) { ConstraintStorage.emplace_back(std::make_shared<FunctionConstraint>(*this, isSatisfied, errorMessage)); return *this; };

    class MutuallyExclusive
        : public MultiConstraint
    {
    public:
        template <typename... Options>
        explicit MutuallyExclusive(ICli* po, Option& option1, Option& option2, Options&... rest)
            : MultiConstraint(po, option1, option2, rest...)
        {}

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
                }
            }
            return chosenOption != nullptr || !isAnyMandatory();
        }

        virtual std::string what() override
        {
            std::string msg;
            msg.append("Only one of the ");
            for (const auto& option : relativeOptions)
            {
                msg.append(parameterSyntax(*option.first));
                if(option.first != relativeOptions.back().first)
                    msg.append(", ");
            }
            msg.append(" parameters must be provided");
            return msg;
        }

    private:
        bool isAnyMandatory(){ return std::any_of(relativeOptions.begin(), relativeOptions.end(), [this](const auto& optionPair) { return isMandatory(*optionPair.first); });}

        Option* chosenOption = nullptr;
    };

    class ValueOption
        : public Option
    {
    public:
        ValueOption(ICli* po, const std::string& parameter, const std::string& secondParameter = "", const std::string& description = "", const std::string& defaultValue = "", bool mandatory = false, size_t maxValueCount = SIZE_MAX)
            : Option(parameter, secondParameter, description, defaultValue, mandatory, _detail::OptionParseType::Value, po)
        {
            withMaxValueCount(maxValueCount);
        };
    };

    class FlagOption
        : public Option
    {
    public:
        FlagOption(ICli* po, const std::string& parameter, const std::string& description = "", const std::string& secondParameter = "", bool mandatory = false)
            : Option(parameter, secondParameter, description, "", mandatory, _detail::OptionParseType::Value, po)
        {
            withMaxValueCount(0);
        };
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
        FunctionOption(ICli* po, const std::string& parameter, const std::function<void(const Option&)>& onExists, const std::string& secondParameter = "", const std::string& description = "", const std::string& defaultValue = "", bool mandatory = false, size_t maxValueCount = SIZE_MAX)
            : ValueOption(po, parameter, secondParameter, description, defaultValue, mandatory, maxValueCount)
            , _detail::FunctionExecutor(onExists)
        {}
        virtual void execute(const Option& option) const override { f(option); }
    };

    class FunctionFlag
        : public FlagOption
        , protected _detail::FunctionExecutor
    {
    public:
        FunctionFlag(ICli* po, const std::string& parameter, const std::function<void(const Option&)>& onExists, const std::string& description = "", const std::string& secondParameter = "", bool mandatory = false)
            : FlagOption(po, parameter, description, secondParameter, mandatory)
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
            flag("-h", [this](const Option&) { exitWithCode(0); }, "Prints this help message", "--help").prioritize();
#endif
        };
        // Implementation of ICli
        virtual Option& option(const std::string& option, const std::string& secondOption = "", const std::string& description = "", const std::string& defaultValue = "", OptionType optionType = OptionType::Value, size_t maxValueCount = SIZE_MAX) override;
        virtual Option& option(const std::string& option, const std::function<void(const Option&)>& onExists, const std::string& secondOption = "", const std::string& description = "", const std::string& defaultValue = "", OptionType optionType = OptionType::Value, size_t maxValueCount = SIZE_MAX) override;
        virtual Option& flag(const std::string& option, const std::string& description = "", const std::string& secondOption = "") override;
        virtual Option& flag(const std::string& option, const std::function<void(const Option&)>& onExists, const std::string& description = "", const std::string& secondOption = "") override;
        virtual Option& tagless(size_t valueCount = 1, const std::string& description = "", const std::string& defaultValue = "") override;
        virtual Option& tagless(const std::function<void(const Option&)>& onExists, size_t valueCount = 1, const std::string& description = "", const std::string& defaultValue = "") override;
        virtual void option(Option& option) override;
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
        inline const Option& getOption(const std::string& option) const { return m_refMap.at(getKey(option)); }
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
        MutuallyExclusive& mutuallyExclusive(Options&... options) { m_multiConstraintStorage.emplace_back(std::make_shared<MutuallyExclusive>(this, m_refMap.at(getKey(options))...)); return reinterpret_cast<MutuallyExclusive&>(*m_multiConstraintStorage.back()); }
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
        void parseOptions();
        void checkMandatoryOptions();
        inline void crossCheckMultiConstraints();
        inline void checkOptionConstraints(Option& option);
        inline void executeExistingOptions() const;
        inline void executePriorityOptions() const;
        inline std::string getKey(const std::string& option) const { return (m_aliasMap.find(option) != m_aliasMap.end()) ? m_aliasMap.at(option) : option; }
        void registerOptionSizes(size_t optionSize, size_t secondOptionSize, size_t descriptionSize);
        void registerAlias(const std::string& option, const std::string& secondOption);
        void unknownArgParsingError(const std::string& value);
        void constraintError(const std::string& constraints, const std::string& value, const std::string& parameter);
        void multiConstraintError(const std::string& message);

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
        std::deque<std::string> m_inputStorage;
        bool m_parsed = false;
        bool m_parsedPriority = false;
        bool m_askInputForMandatoryOptions = false;
        bool m_exitOnUnexpectedValue = true;

        std::istream* m_inputStream = &std::cin;
        std::ostream* m_outputStream = &std::cout;
    };

    Option& Cli::option(const std::string& option, const std::string& secondOption, const std::string& description, const std::string& defaultValue, OptionType optionType, size_t maxValueCount)
    {
        registerOptionSizes(option.size(), secondOption.size(), description.size());
        if (optionType == OptionType::MultiValue)
            m_optionStorage.emplace_back(std::make_shared<MultiOption>(nullptr, option, secondOption, description, defaultValue, false, maxValueCount));
        else
            m_optionStorage.emplace_back(std::make_shared<ValueOption>(nullptr, option, secondOption, description, defaultValue, false, maxValueCount));

        m_refMap.emplace(option, *m_optionStorage.back()).first->second.setCli(*this);
        registerAlias(option, secondOption);
        return *m_optionStorage.back();
    }

    Option& Cli::option(const std::string& option, const std::function<void(const Option&)>& onExists, const std::string& secondOption, const std::string& description, const std::string& defaultValue, OptionType optionType, size_t maxValueCount)
    {
        registerOptionSizes(option.size(), secondOption.size(), description.size());
        if (optionType == OptionType::MultiValue)
            m_optionStorage.emplace_back(std::make_shared<FunctionMultiOption>(nullptr, option, onExists, secondOption, description, defaultValue, false, maxValueCount));
        else
            m_optionStorage.emplace_back(std::make_shared<FunctionOption>(nullptr, option, onExists, secondOption, description, defaultValue, false, maxValueCount));

        m_refMap.emplace(option, *m_optionStorage.back()).first->second.setCli(*this);
        registerAlias(option, secondOption);
        return *m_optionStorage.back();
    }

    Option& Cli::flag(const std::string& option, const std::string& description, const std::string& secondOption)
    {
        registerOptionSizes(option.size(), secondOption.size(), description.size());
        m_optionStorage.emplace_back(std::make_shared<FlagOption>(nullptr, option, description, secondOption, false));
        m_refMap.emplace(option, *m_optionStorage.back()).first->second.setCli(*this);
        registerAlias(option, secondOption);
        return *m_optionStorage.back();
    }

    Option& Cli::flag(const std::string& option, const std::function<void(const Option&)>& onExists, const std::string& description, const std::string& secondOption)
    {
        registerOptionSizes(option.size(), secondOption.size(), description.size());
        m_optionStorage.emplace_back(std::make_shared<FunctionFlag>(nullptr, option, onExists, description, secondOption, false));
        m_refMap.emplace(option, *m_optionStorage.back()).first->second.setCli(*this);
        registerAlias(option, secondOption);
        return *m_optionStorage.back();
    }

    Option& Cli::tagless(size_t valueCount, const std::string& description, const std::string& defaultValue)
    {
        registerOptionSizes(getNextId() % 10 + 1, 0, description.size());
        m_optionStorage.emplace_back(std::make_shared<TaglessOption>(nullptr, valueCount, description, defaultValue, false));
        m_refMap.emplace(std::to_string(getCurrentId()), *m_optionStorage.back()).first->second.setCli(*this);
        return *m_optionStorage.back();
    }

    Option& Cli::tagless(const std::function<void(const Option&)>& onExists, size_t valueCount, const std::string& description, const std::string& defaultValue)
    {
        registerOptionSizes(getNextId() % 10 + 1, 0, description.size());
        m_optionStorage.emplace_back(std::make_shared<FunctionTaglessOption>(nullptr, onExists, valueCount, description, defaultValue, false));
        m_refMap.emplace(std::to_string(getCurrentId()), *m_optionStorage.back()).first->second.setCli(*this);
        return *m_optionStorage.back();
    }

    void Cli::option(Option& option)
    {
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
            parseOptions();
            checkMandatoryOptions();
            crossCheckMultiConstraints();
            m_parsed = true;

            executeExistingOptions();
        }
        else
            executePriorityOptions();

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

    void Cli::parseOptions()
    {
        Option* lastOption = nullptr;
        int taglessId = 0;
        for (int i = 1; i < m_argc; ++i)
        {
            auto key = getKey(m_argv[i]);
            auto option = m_refMap.find(key);
            if (option != m_refMap.end())
            {
                option->second.Exists = true;
                ++option->second.ExistsCount;
                if(option->second.MaxValueCount > 0)
                    lastOption = &option->second;
            }
            else if (lastOption != nullptr)
            {
                if (lastOption->MaxValueCount > lastOption->Values.size() || lastOption->ParseType == _detail::OptionParseType::Value)
                    lastOption->setValue(m_argv[i]);
                checkOptionConstraints(*lastOption);
                if (lastOption->ParseType == _detail::OptionParseType::Value || lastOption->MaxValueCount == lastOption->Values.size())
                    lastOption = nullptr;
            }
            else if ((option = m_refMap.find(std::to_string(taglessId))) != m_refMap.end() && option->second.ParseType == _detail::OptionParseType::Unidentified)
            {
                option->second.Exists = true;
                ++option->second.ExistsCount;
                option->second.setValue(m_argv[i]);
                checkOptionConstraints(option->second);
                if (option->second.ExistsCount == option->second.MaxValueCount)
                    ++taglessId;
            }
            else if (m_exitOnUnexpectedValue && (taglessId > getCurrentId() || lastOption->MaxValueCount < lastOption->Values.size()))
                unknownArgParsingError(m_argv[i]);
        }
    }

    void Cli::checkMandatoryOptions()
    {
        for (auto& pair : m_refMap)
        {
            if (pair.second.Mandatory && !pair.second.Exists)
            {
                printOption(pair.second);
                printOptionUsage(pair.second);
                *m_outputStream << " is a required parameter" << std::endl;
                if (m_askInputForMandatoryOptions)
                    askInput(pair.second);
                else if (m_exitOnUnexpectedValue && pair.second.MultiConstrained.empty())
                    exitWithCode(1);
            }
        }
    }

    inline void Cli::crossCheckMultiConstraints()
    {
        for (auto& it : m_refMap)
            for (auto& constraint : it.second.MultiConstrained)
                if (!constraint->satisfied(it.second))
                    multiConstraintError(constraint->what());
    }

    inline void Cli::checkOptionConstraints(Option& option)
    {
        for(const auto& constraint : option.Constrained)
            if(!constraint->satisfied())
                constraintError(constraint->what(), option.value(), option.Parameter);
        for(const auto& multiConstraint : option.MultiConstrained)
            if(!multiConstraint->satisfied(option))
                multiConstraintError(multiConstraint->what());
    }

    inline void Cli::executeExistingOptions() const
    {
        for (const auto& it : m_refMap)
            if (it.second.Exists)
                it.second.execute(it.second);
    }

    inline void Cli::executePriorityOptions() const
    {
        for (const auto& it : m_priorityMap)
            if (it.second.Exists)
                it.second.execute(it.second);
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
            checkOptionConstraints(option);
        }
        else if (option.Mandatory && m_exitOnUnexpectedValue && option.MultiConstrained.empty())
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
}

#endif
