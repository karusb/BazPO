#include <iostream>
#include "BazPO.hpp"

void BazPO::Add(std::string option, std::string secondOption,std::string description, bool mandatory )
{
    // Register largest parameter size to use for padding later on
    m_maxOptionParameterSize = option.size() > m_maxOptionParameterSize ? option.size() : m_maxOptionParameterSize;
    m_maxSecondOptionParameterSize = secondOption.size() > m_maxSecondOptionParameterSize ? secondOption.size() : m_maxSecondOptionParameterSize;

    m_map.insert(std::make_pair(option, Option(option, "", secondOption, description, mandatory)));
    // If secondOption is not default value then its registered as an alias to the map
    if (secondOption != "")
        m_aliasMap.insert(std::make_pair(secondOption, option));
}

BazPO::Option BazPO::GetOption(std::string option)
{
    if (!m_parsed)
        ParseArguments();
    auto key = GetKey(option);
    return m_map.at(key);
}

std::string BazPO::GetValue(std::string option)
{  
    if (!m_parsed)
        ParseArguments();
    auto key = GetKey(option);
    return m_map.at(key).Exists ? m_map.at(key).Value : "";
}

bool BazPO::Exists(std::string option)
{
    if (!m_parsed)
        ParseArguments();
    auto key = GetKey(option);
    return m_map.at(key).Exists;
}

void BazPO::ParseArguments()
{
    for (int i = 0; i < m_argc; ++i)
    {
        // Set the key, if secondOption i.e an alias is used set it to original parameter, else set it to argument
        auto key = GetKey(m_argv[i]);
        if (m_map.find(key) != m_map.end() )
        {
            m_map.at(key).Exists = true;
            if ((i + 1) < m_argc) // Value might exist if its within bounds and not in the options map otherwise it's empty
                m_map.at(key).Value = m_map.find(GetKey(m_argv[i + 1])) != m_map.end() ? "" : (m_argv[i + 1]);
        }
    }
    // At this moment we know if options are present or not
    AskUserInputForMandatoryOptions();
    m_parsed = true;
}

void BazPO::AskUserInputForMandatoryOptions()
{
    for (auto& pair : m_map)
    {
        if (pair.second.Mandatory && !pair.second.Exists)
        {         
            std::string temp;
            std::cout << pair.second.Parameter << " is a required parameter" << std::endl;
            std::cout << pair.second.Parameter << ": " << std::endl;
            std::cin >> temp;           
            
            m_optionStorage.push_back(temp);
            pair.second.Value = m_optionStorage.back().c_str();
            pair.second.Exists = true;
        }
    }
}

inline std::string BazPO::CreateSpaces(size_t spaceCount)
{
    return std::string(" ", spaceCount);
}

inline std::string BazPO::GetKey(std::string option)
{
    return (m_aliasMap.find(option) != m_aliasMap.end()) ? m_aliasMap.at(option) : std::string(option);
}

void BazPO::PrintOptions()
{
    // Define the spacing between Options and description
    size_t secondOptionSpacing = 2;
    size_t descriptionSpacing = 3;
    std::string secondOptionSpaces = CreateSpaces(secondOptionSpacing);
    std::string descriptionSpaces = CreateSpaces(descriptionSpacing);
    // Define required space characters between and option and description
    const std::string optionHeader = "Option";
    const std::string descriptionHeader = "Description";
    std::string optionHeaderSpaces = CreateSpaces(m_maxOptionParameterSize + m_maxSecondOptionParameterSize + secondOptionSpacing + descriptionSpacing - optionHeader.size());
    //Output Header
    std::cout << optionHeader << optionHeaderSpaces << descriptionHeader << std::endl;
    for (const auto& pair : m_map)
    {
        // Calculate required padding for given option
        size_t firstPaddingSize = pair.first.size() < m_maxOptionParameterSize ? m_maxOptionParameterSize - pair.first.size() : 0;
        size_t secondPaddingSize = pair.second.SecondParameter.size() < m_maxSecondOptionParameterSize ? m_maxSecondOptionParameterSize - pair.second.SecondParameter.size() : 0;
        std::string firstPadding = CreateSpaces(firstPaddingSize);
        std::string secondPadding = CreateSpaces(secondPaddingSize);
        // Output the option
        std::cout <<
            pair.first << firstPadding << secondOptionSpaces <<
            pair.second.SecondParameter << secondPadding << descriptionSpaces <<
            pair.second.Description << std::endl;
    }
}
