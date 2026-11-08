#include "gtest/gtest.h"
#include "../include/BazPO.hpp"
#include <stdexcept>
namespace
{
class StrictCli : public BazPO::Cli
{
public:
    StrictCli(int argc, const char** argv) : BazPO::Cli(argc, argv)
    {
        strictParsing();
        changeIO(&diagnostics);
    }
private:
    void exitWithCode(int) override { throw std::invalid_argument(diagnostics.str()); }
    std::ostringstream diagnostics;
};
}
TEST(StrictParsing, DefaultValueRemainsOwnedAfterCallerMutation)
{
    const char* argv[] = {"synthetic"};
    StrictCli cli(1, argv);
    std::string value(4096, 'a');
    cli.option("--value", "", "", value);
    value.assign(8192, 'b');
    cli.parse();
    EXPECT_EQ(std::string(4096, 'a'), cli.valueAs<std::string>("--value"));
}
TEST(StrictParsing, RejectsMissingValueBeforeAnotherOption)
{
    const char* argv[] = {"synthetic", "--file", "--enabled"};
    StrictCli cli(3, argv);
    cli.option("--file"); cli.flag("--enabled");
    EXPECT_THROW(cli.parse(), std::invalid_argument);
}
TEST(StrictParsing, RejectsMissingFinalValue)
{
    const char* argv[] = {"synthetic", "--file"};
    StrictCli cli(2, argv); cli.option("--file");
    EXPECT_THROW(cli.parse(), std::invalid_argument);
}
TEST(StrictParsing, RejectsRepeatedOptionsThroughAliases)
{
    const char* argv[] = {"synthetic", "--file", "one", "-f", "two"};
    StrictCli cli(5, argv); cli.option("--file", "-f");
    EXPECT_THROW(cli.parse(), std::invalid_argument);
}
TEST(StrictParsing, DoesNotConsumeUnknownTagAsValue)
{
    const char* argv[] = {"synthetic", "--file", "--unknown"};
    StrictCli cli(3, argv); cli.option("--file");
    EXPECT_THROW(cli.parse(), std::invalid_argument);
}
TEST(StrictParsing, NumericValueDoesNotAliasInternalPositionalId)
{
    const char* argv[] = {"synthetic", "apply", "--pid", "0"};
    StrictCli cli(4, argv); cli.tagless(); cli.option("--pid");
    cli.parse();
    EXPECT_EQ("apply", cli.valueAs<std::string>("0"));
    EXPECT_EQ(0, cli.valueAs<int>("--pid"));
}
