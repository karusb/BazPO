#include "gtest/gtest.h"
#include "../include/BazPO.hpp"
#include <stdexcept>

namespace
{
class CapturingCli : public BazPO::Cli
{
public:
    CapturingCli(int argc, const char** argv) : BazPO::Cli(argc, argv)
    {
        changeIO(&diagnostics);
    }
    std::ostringstream diagnostics;
private:
    void exitWithCode(int) override { throw std::invalid_argument(diagnostics.str()); }
};

class ValueLimits : public ::testing::TestWithParam<bool>
{
protected:
    void configure(BazPO::Option& option)
    {
        if (GetParam()) option.prioritize();
    }
};
}

TEST_P(ValueLimits, ValueOptionAcceptsExactlyItsOccurrenceLimit)
{
    const char* argv[] = {"synthetic", "-a", "one", "-a", "two"};
    CapturingCli cli(5, argv);
    auto& option = cli.option("-a").withMaxValueCount(2);
    configure(option);
    cli.parse();
    EXPECT_EQ(2, cli.existsCount("-a"));
    EXPECT_EQ((std::deque<std::string>{"one", "two"}), option.valuesAs<std::string>());
    EXPECT_EQ("two", option.valueAs<std::string>());
}

TEST_P(ValueLimits, ValueOptionRejectsAnOccurrenceBeyondTheLimit)
{
    const char* argv[] = {"synthetic", "-a", "one", "-a", "two", "-a", "three"};
    CapturingCli cli(7, argv);
    auto& option = cli.option("-a").withMaxValueCount(2);
    configure(option);
    EXPECT_THROW(cli.parse(), std::invalid_argument);
    EXPECT_EQ((std::deque<std::string>{"one", "two"}), option.valuesAs<std::string>());
}

TEST_P(ValueLimits, AliasesShareTheLimitBeforeCallbacksExecute)
{
    const char* argv[] = {"synthetic", "--file", "one", "-f", "two"};
    CapturingCli cli(5, argv);
    int callbacks = 0;
    auto& option = cli.option("--file", [&](const BazPO::Option&) { ++callbacks; }, "-f").withMaxValueCount(1);
    configure(option);
    EXPECT_THROW(cli.parse(), std::invalid_argument);
    EXPECT_EQ(0, callbacks);
}

TEST_P(ValueLimits, OccurrencesWithoutValuesCountTowardTheLimit)
{
    const char* argv[] = {"synthetic", "-a", "-a"};
    CapturingCli cli(3, argv);
    auto& option = cli.option("-a").withMaxValueCount(1);
    configure(option);
    EXPECT_THROW(cli.parse(), std::invalid_argument);
    EXPECT_TRUE(option.values().empty());
}

TEST_P(ValueLimits, ValueOptionStillAllowsAnOptionalValue)
{
    const char* argv[] = {"synthetic", "-a"};
    CapturingCli cli(2, argv);
    auto& option = cli.option("-a", "", "", ".").withMaxValueCount(1);
    configure(option);
    cli.parse();
    EXPECT_TRUE(option.exists());
    EXPECT_TRUE(option.values().empty());
    EXPECT_EQ(".", option.valueAs<std::string>());
}

TEST_P(ValueLimits, DefaultValueOptionsRemainRepeatable)
{
    const char* argv[] = {"synthetic", "-a", "one", "-a", "two", "-a", "three"};
    CapturingCli cli(7, argv);
    auto& option = cli.option("-a");
    configure(option);
    cli.parse();
    EXPECT_EQ(3, cli.existsCount("-a"));
    EXPECT_EQ(3u, option.values().size());
}

TEST_P(ValueLimits, MultiOptionStartsAFreshLimitForEachTag)
{
    const char* argv[] = {"synthetic", "-a", "one", "two", "-a", "three", "four"};
    CapturingCli cli(7, argv);
    auto& option = cli.option("-a", "", "", "", BazPO::OptionType::MultiValue).withMaxValueCount(2);
    configure(option);
    cli.parse();
    EXPECT_EQ(2, option.existsCount());
    EXPECT_EQ((std::deque<std::string>{"one", "two", "three", "four"}), option.valuesAs<std::string>());
}

TEST_P(ValueLimits, MultiOptionResetsAfterAPartialGroupAndAnAlias)
{
    const char* argv[] = {"synthetic", "-a", "one", "--alpha", "two", "three"};
    CapturingCli cli(6, argv);
    auto& option = cli.option("-a", "--alpha", "", "", BazPO::OptionType::MultiValue).withMaxValueCount(2);
    configure(option);
    cli.parse();
    EXPECT_EQ((std::deque<std::string>{"one", "two", "three"}), option.valuesAs<std::string>());
}

TEST_P(ValueLimits, MultiOptionWithLimitOneStillAcceptsRepeatedTags)
{
    const char* argv[] = {"synthetic", "-a", "one", "-a", "two"};
    CapturingCli cli(5, argv);
    auto& option = cli.option("-a", "", "", "", BazPO::OptionType::MultiValue).withMaxValueCount(1);
    configure(option);
    cli.parse();
    EXPECT_EQ((std::deque<std::string>{"one", "two"}), option.valuesAs<std::string>());
}

TEST_P(ValueLimits, NegativeFractionsRemainValidValues)
{
    const char* argv[] = {"synthetic", "-a", "-.5"};
    CapturingCli cli(3, argv);
    auto& option = cli.option("-a").withMaxValueCount(1);
    configure(option);
    cli.parse();
    EXPECT_DOUBLE_EQ(-0.5, option.valueAs<double>());
}

TEST_P(ValueLimits, UnregisteredTagsRemainRawMultiOptionValues)
{
    const char* argv[] = {"synthetic", "-a", "--literal", "-text"};
    CapturingCli cli(4, argv);
    auto& option = cli.option("-a", "", "", "", BazPO::OptionType::MultiValue).withMaxValueCount(2);
    configure(option);
    cli.parse();
    EXPECT_EQ((std::deque<std::string>{"--literal", "-text"}), option.valuesAs<std::string>());
}

TEST_P(ValueLimits, NumericValuesDoNotAliasInternalPositionalIds)
{
    const char* argv[] = {"synthetic", "--pid", "0"};
    CapturingCli cli(3, argv);
    cli.tagless();
    auto& option = cli.option("--pid").withMaxValueCount(1);
    configure(option);
    cli.parse();
    EXPECT_EQ(0, option.valueAs<int>());
    EXPECT_EQ(1u, option.values().size());
}

INSTANTIATE_TEST_SUITE_P(ParsingModes, ValueLimits, ::testing::Values(false, true),
    [](const ::testing::TestParamInfo<bool>& info) { return info.param ? "Priority" : "Normal"; });

TEST(ParsingRegressions, MultiOptionResetsAcrossOtherOptions)
{
    const char* argv[] = {"synthetic", "-a", "one", "two", "-b", "separate", "-a", "three", "four"};
    CapturingCli cli(9, argv);
    auto& option = cli.option("-a", "", "", "", BazPO::OptionType::MultiValue).withMaxValueCount(2);
    cli.option("-b");
    cli.parse();
    EXPECT_EQ((std::deque<std::string>{"one", "two", "three", "four"}), option.valuesAs<std::string>());
    EXPECT_EQ("separate", cli.valueAs<std::string>("-b"));
}

TEST(ParsingRegressions, MultiOptionRejectsAnExtraValueInTheSecondGroup)
{
    const char* argv[] = {"synthetic", "-a", "one", "two", "-a", "three", "four", "extra"};
    CapturingCli cli(8, argv);
    cli.option("-a", "", "", "", BazPO::OptionType::MultiValue).withMaxValueCount(2);
    EXPECT_THROW(cli.parse(), std::invalid_argument);
}

TEST(ParsingRegressions, AcceptableExtraArgumentsDoNotEnterTheNextGroup)
{
    const char* argv[] = {"synthetic", "-a", "one", "extra", "-a", "two", "extra"};
    CapturingCli cli(7, argv);
    auto& option = cli.option("-a", "", "", "", BazPO::OptionType::MultiValue).withMaxValueCount(1);
    cli.unexpectedArgumentsAcceptable();
    cli.parse();
    EXPECT_EQ((std::deque<std::string>{"one", "two"}), option.valuesAs<std::string>());
}

TEST(ParsingRegressions, GroupBoundaryLeavesFollowingValuesForPositionals)
{
    const char* argv[] = {"synthetic", "-a", "one", "two", "positional", "-a", "three", "four"};
    CapturingCli cli(8, argv);
    auto& positional = cli.tagless();
    auto& option = cli.option("-a", "", "", "", BazPO::OptionType::MultiValue).withMaxValueCount(2);
    cli.parse();
    EXPECT_EQ("positional", positional.valueAs<std::string>());
    EXPECT_EQ(4u, option.values().size());
}

TEST(ParsingRegressions, MultiOptionCallbackReceivesAllGroupsOnce)
{
    const char* argv[] = {"synthetic", "-a", "one", "two", "-a", "three", "four"};
    CapturingCli cli(7, argv);
    int callbacks = 0;
    cli.option("-a", [&](const BazPO::Option& option) {
        ++callbacks;
        EXPECT_EQ(4u, option.values().size());
    }, "", "", "", BazPO::OptionType::MultiValue).withMaxValueCount(2);
    cli.parse();
    EXPECT_EQ(1, callbacks);
}

TEST(ParsingRegressions, FlagsRemainRepeatable)
{
    const char* argv[] = {"synthetic", "-v", "--verbose", "-v"};
    CapturingCli cli(4, argv);
    cli.flag("-v", "", "--verbose");
    cli.parse();
    EXPECT_EQ(3, cli.existsCount("-v"));
}

TEST(ParsingRegressions, FlagTerminatesThePreviousValueGroup)
{
    const char* argv[] = {"synthetic", "--file", "--enabled", "unexpected"};
    CapturingCli cli(4, argv);
    cli.option("--file");
    cli.flag("--enabled");
    EXPECT_THROW(cli.parse(), std::invalid_argument);
}

TEST(ParsingRegressions, DefaultValueRemainsOwnedAfterCallerMutation)
{
    const char* argv[] = {"synthetic"};
    CapturingCli cli(1, argv);
    std::string value(4096, 'a');
    cli.option("--value", "", "", value);
    value.assign(8192, 'b');
    cli.parse();
    EXPECT_EQ(std::string(4096, 'a'), cli.valueAs<std::string>("--value"));
}

TEST(ParsingRegressions, ProgramDescriptionRemainsOwnedAfterCallerMutation)
{
    const char* argv[] = {"synthetic"};
    std::string description(4096, 'a');
    BazPO::Cli cli(1, argv, description.c_str());
    description.assign(8192, 'b');
    std::ostringstream output;
    cli.changeIO(&output);
    cli.printOptions();
    EXPECT_NE(std::string::npos, output.str().find(std::string(4096, 'a')));
}

TEST(ParsingRegressions, PositionalCommandAndNumericOptionRemainIndependent)
{
    const char* argv[] = {"synthetic", "apply", "--pid", "0"};
    CapturingCli cli(4, argv);
    auto& command = cli.tagless();
    cli.option("--pid");
    cli.parse();
    EXPECT_EQ("apply", command.valueAs<std::string>());
    EXPECT_EQ(0, cli.valueAs<int>("--pid"));
}
