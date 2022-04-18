#include <iostream>
#include "BazPO.hpp"

namespace BazPO
{
    namespace _detail
    {
        extern size_t TaglessOptionIDCounter = 0;
    }
    void Cli::Add(std::string option, std::string secondOption, std::string description, bool mandatory, bool multipleOptions)
    {
        RegisterLargestInput(option.size(), secondOption.size());
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
        RegisterLargestInput(option.size(), secondOption.size());
        Option* optionRef = nullptr;
        m_functionalValues.push_back(FunctionOption(option, action, secondOption, description, mandatory));
        optionRef = &m_functionalValues.back();

        m_refMap.insert({ option, *optionRef });
        RegisterAlias(option, secondOption);
    }

    void Cli::Add(Option& option)
    {
        RegisterLargestInput(option.Parameter.size(), option.SecondParameter.size());
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
        ParseNormal();
        // At this moment we know if options are present or not
        AskUserInputForMandatoryOptions();
        m_parsed = true;
        // Execute Actions
        for (auto it = m_refMap.begin(); it != m_refMap.end(); ++it)
            it->second.Execute(it->second);
    }

    void Cli::ParseTagless()
    {
        if (m_taglessOptions.size() > 0)
        {
            for (int i = 1; i < m_argc; ++i)
            {
                auto key = GetKey(std::to_string(i - 1)); // this line assumes all tagless options have one value
                auto option = m_refMap.find(key);
                option->second.setValue(m_argv[i]);
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
                    if (lastOption->ParseType == OptionParseType::MultiValue)
                        lastOption->setValue(m_argv[i + 1]);
                    else if (lastOption->ParseType == OptionParseType::Value && m_exitOnUnexpectedValue)
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
        // TODO: Print if the option is mandatory
        // TODO: Print the program usage with all arguments
        // TODO: Separate multi option description
        // Define the spacing between Options and description
        size_t secondOptionSpacing = 2;
        size_t descriptionSpacing = 3;
        std::string secondOptionSpaces = CreateSpaces(secondOptionSpacing);
        std::string descriptionSpaces = CreateSpaces(descriptionSpacing);
        // Define required space characters between and option and description
        const std::string optionHeader = "Option";
        const std::string descriptionHeader = "Description";
        std::string optionHeaderSpaces = CreateSpaces(m_maxOptionParameterSize + m_maxSecondOptionParameterSize + secondOptionSpacing + descriptionSpacing - optionHeader.size());
        // Program Description
        *m_outputStream << m_argv[0] << CreateSpaces(7) << m_programDescription << std::endl;
        // Program Usage
        
        //Output Header
        *m_outputStream << optionHeader << optionHeaderSpaces << descriptionHeader << std::endl;
        for (const auto& pair : m_refMap)
        {
            // Calculate required padding for given option
            size_t firstPaddingSize = pair.first.size() < m_maxOptionParameterSize ? m_maxOptionParameterSize - pair.first.size() : 0;
            size_t secondPaddingSize = pair.second.SecondParameter.size() < m_maxSecondOptionParameterSize ? m_maxSecondOptionParameterSize - pair.second.SecondParameter.size() : 0;
            std::string firstPadding = CreateSpaces(firstPaddingSize);
            std::string secondPadding = CreateSpaces(secondPaddingSize);
            // Output the option
            *m_outputStream <<
                pair.first << firstPadding << secondOptionSpaces <<
                pair.second.SecondParameter << secondPadding << descriptionSpaces <<
                pair.second.Description << std::endl;
        }
    }

    void Cli::RegisterLargestInput(size_t optionSize, size_t secondOptionSize)
    {
        // Register largest parameter size to use for padding later on
        m_maxOptionParameterSize = optionSize > m_maxOptionParameterSize ? optionSize : m_maxOptionParameterSize;
        m_maxSecondOptionParameterSize = secondOptionSize > m_maxSecondOptionParameterSize ? secondOptionSize : m_maxSecondOptionParameterSize;
    }

    void Cli::RegisterAlias(std::string option, std::string secondOption)
    {
        // If secondOption is not default value then its registered as an alias to the map
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

    void Cli::NonsenseNotAccepted()
    {
        m_exitOnUnexpectedValue = true;
    }
}
