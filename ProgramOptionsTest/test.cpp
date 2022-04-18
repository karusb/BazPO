
#include "gtest/gtest.h"
#include "../ProgramOptions/BazPO.hpp"

using namespace BazPO;

class ProgramOptionsTest
    : public ::testing::Test
{
public:
    int argc = 10;
    const char* argv[10]{{"programoptions"}, {"-a"}, {"Aoption"}, {"--bravo"}, {"Boption"}, {"-c"}, {"-d"}, {"15"}, {"-e"}, {"15.2156"}};
};

TEST_F(ProgramOptionsTest, add_then_get_successful)
{
    Cli po{ argc, argv };
    po.Add("-a", "--alpha", "Option A");
    po.ParseArguments();

    auto a = po.GetOption("-a");

    EXPECT_EQ(true, a.Exists);
    EXPECT_EQ(false, a.Mandatory);
    EXPECT_EQ(std::string("Aoption"), a.value<>());
}

TEST_F(ProgramOptionsTest, get_second_option_successful)
{
    Cli po{ argc, argv };
    po.Add("-a", "--alpha", "Option A");
    po.ParseArguments();

    auto a = po.GetOption("--alpha");

    EXPECT_EQ(true, a.Exists);
    EXPECT_EQ(false, a.Mandatory);
    EXPECT_EQ(std::string("Aoption"), a.value<>());
}
TEST_F(ProgramOptionsTest, int_conversion_succesful)
{
    Cli po{ argc, argv };
    po.Add("-d", "--delta", "Option D");
    po.ParseArguments();

    auto a = po.GetOption("-d");

    EXPECT_EQ(true, a.Exists);
    EXPECT_EQ(15, a.value<int>());
}

TEST_F(ProgramOptionsTest, double_conversion_succesful)
{
    Cli po{ argc, argv };
    po.Add("-e", "--echo", "Option E");
    po.ParseArguments();

    auto a = po.GetOption("-e");

    EXPECT_EQ(true, a.Exists);
    EXPECT_EQ(15.2156, a.value<double>());
}

TEST_F(ProgramOptionsTest, bool_conversion_succesful)
{
    int argc = 6;
    const char* argv[6]{ {"programoptions"}, {"-a"}, {"True"}, {"--bravo"}, {"False"}, {"-c"} };
    Cli po{ argc, argv };
    po.Add("-a", "--echo");
    po.Add("-b", "--bravo");
    po.Add("-c", "--charlie");
    po.ParseArguments();

    auto a = po.GetOption("-a");
    auto b = po.GetOption("-b");
    auto c = po.GetOption("-c");

    EXPECT_EQ(true, a.Exists);
    EXPECT_EQ(false, a.value<bool>());

    EXPECT_EQ(true, b.Exists);
    EXPECT_EQ(false, b.value<bool>());

    EXPECT_EQ(true, c.Exists);
    EXPECT_EQ(false, c.value<bool>());
}

TEST_F(ProgramOptionsTest, all_options_are_found)
{
    int argc = 6;
    const char* argv[6]{ {"programoptions"}, {"-a"}, {"Aoption"}, {"--bravo"}, {"Boption"}, {"-c"} };
    Cli po{ argc, argv };
    po.Add("-a", "--alpha", "Option A");
    po.Add("-b", "--bravo", "Option B");
    po.Add("-c", "--charlie", "Option C");
    po.ParseArguments();

    auto a = po.GetOption("-a");
    auto b = po.GetOption("-b");
    auto c = po.GetOption("-c");

    EXPECT_EQ(true, a.Exists);
    EXPECT_EQ(false, a.Mandatory);
    EXPECT_EQ(std::string("Aoption"), a.value<>());

    EXPECT_EQ(true, b.Exists);
    EXPECT_EQ(false, b.Mandatory);
    EXPECT_EQ(std::string("Boption"), b.value<>());

    EXPECT_EQ(true, c.Exists);
    EXPECT_EQ(false, c.Mandatory);
    EXPECT_EQ(std::string(""), c.value<>());
}
TEST_F(ProgramOptionsTest, multi_options_successful)
{
    int argc = 6;
    const char* argv[6]{ {"programoptions"}, {"-a"}, {"value1"}, {"value2"}, {"value3"}, {"value4"} };
    Cli po{ argc, argv };
    po.Add("-a", "--echo", "", false, true);
    po.Add("-b", "--bravo");
    po.Add("-c", "--charlie");
    po.ParseArguments();

    auto a = po.GetOption("-a");
    auto b = po.GetOption("-b");
    auto c = po.GetOption("-c");

    EXPECT_EQ(true, a.Exists);
    EXPECT_EQ(false, b.Exists);
    EXPECT_EQ(false, c.Exists);

    EXPECT_EQ(4, a.values<>().size());
    EXPECT_EQ(std::string("value1"), a.values<>()[0]);
    EXPECT_EQ(std::string("value2"), a.values<>()[1]);
    EXPECT_EQ(std::string("value3"), a.values<>()[2]);
    EXPECT_EQ(std::string("value4"), a.values<>()[3]);
}

TEST_F(ProgramOptionsTest, multi_options_with_multi_tags_successful)
{
    int argc = 6;
    const char* argv[6]{ {"programoptions"}, {"-a"}, {"value1"}, {"value2"}, {"-a"}, {"value4"} };
    Cli po{ argc, argv };
    po.Add("-a", "--echo", "", false, true);
    po.Add("-b", "--bravo");
    po.Add("-c", "--charlie");
    po.ParseArguments();

    auto a = po.GetOption("-a");
    auto b = po.GetOption("-b");
    auto c = po.GetOption("-c");

    EXPECT_EQ(true, a.Exists);
    EXPECT_EQ(false, b.Exists);
    EXPECT_EQ(false, c.Exists);

    EXPECT_EQ(3, a.values<>().size());
    EXPECT_EQ(std::string("value1"), a.values<>()[0]);
    EXPECT_EQ(std::string("value2"), a.values<>()[1]);
    EXPECT_EQ(std::string("value4"), a.values<>()[2]);
}

TEST_F(ProgramOptionsTest, value_option_with_multi_tags_successful)
{
    int argc = 5;
    const char* argv[5]{ {"programoptions"}, {"-a"}, {"value1"}, {"-a"}, {"value4"} };
    Cli po{ argc, argv };
    po.Add("-a", "--echo", "");
    po.Add("-b", "--bravo");
    po.Add("-c", "--charlie");
    po.ParseArguments();

    auto a = po.GetOption("-a");
    auto b = po.GetOption("-b");
    auto c = po.GetOption("-c");

    EXPECT_EQ(true, a.Exists);
    EXPECT_EQ(false, b.Exists);
    EXPECT_EQ(false, c.Exists);

    EXPECT_EQ(2, a.values<>().size());
    EXPECT_EQ(std::string("value1"), a.values<>()[0]);
    EXPECT_EQ(std::string("value4"), a.values<>()[1]);
}

TEST_F(ProgramOptionsTest, mandatory_option_exits_when_not_provided) {
    int argc = 1;
    const char* argv[] = { "programoptions" };
    Cli po(argc, argv);
    po.Add("-a", "--alpha", "Option A", "", true);

    ASSERT_DEATH(po.ParseArguments(), "");
}

TEST_F(ProgramOptionsTest, mandatory_option_asked_when_not_provided) {
    int argc = 1;
    const char* argv[] = { "programoptions" };
    std::stringstream str("input");
    Cli po(argc, argv);
    po.UserInputRequiredForAbsentMandatoryOptions();
    po.Add("-a", "--alpha", "Option A", true);
    po.ChangeIO(&std::cout, &str);

    po.ParseArguments();

    auto a = po.GetOption("-a");
    EXPECT_EQ(true, a.Exists);
    EXPECT_EQ(std::string("input"), a.value<>());
}

TEST_F(ProgramOptionsTest, function_option_executes_successfully) {
    bool executed = false;
    int argc = 2;
    const char* argv[] = { "programoptions", "-a"};
    Cli po(argc, argv);
    po.Add("-a", [&](const Option&) { executed = true; }, "--alpha", "Option A", true);

    po.ParseArguments();
    EXPECT_EQ(true, executed);
}

TEST_F(ProgramOptionsTest, function_option_contents_correct) {
    bool executed = false;
    int argc = 6;
    const char* argv[6]{ {"programoptions"}, {"-a"}, {"value1"}, {"value2"}, {"-a"}, {"value4"} };
    Cli po(argc, argv);
    po.Add("-a", [&](const Option& option) {
        EXPECT_EQ(3, option.values<>().size());
        EXPECT_EQ(std::string("value1"), option.values<>()[0]);
        EXPECT_EQ(std::string("value2"), option.values<>()[1]);
        EXPECT_EQ(std::string("value4"), option.values<>()[2]);
        executed = true;
        }, "--alpha", "Option A", true);

    po.ParseArguments();
    EXPECT_EQ(true, executed);
}

TEST_F(ProgramOptionsTest, value_option_with_reference_succesful) {
    Cli po(argc, argv);
    ValueOption option("-a", "--alpha", "Option A", false, &po);

    po.ParseArguments();

    EXPECT_EQ(true, option.Exists);
    EXPECT_EQ(false, option.Mandatory);
    EXPECT_EQ(std::string("Aoption"), option.value<>());
}

TEST_F(ProgramOptionsTest, multi_option_with_reference_succesful) {
    int argc = 6;
    const char* argv[6]{ {"programoptions"}, {"-a"}, {"value1"}, {"value2"}, {"-a"}, {"value4"} };
    Cli po(argc, argv);
    MultiOption option("-a", "--alpha", "Option A", false, &po);

    po.ParseArguments();

    EXPECT_EQ(true, option.Exists);
    EXPECT_EQ(false, option.Mandatory);
    EXPECT_EQ(3, option.values<>().size());
    EXPECT_EQ(std::string("value1"), option.values<>()[0]);
    EXPECT_EQ(std::string("value2"), option.values<>()[1]);
    EXPECT_EQ(std::string("value4"), option.values<>()[2]);
}

TEST_F(ProgramOptionsTest, function_option_with_reference_succesful) {
    bool executed = false;
    int argc = 6;
    const char* argv[6]{ {"programoptions"}, {"-a"}, {"value1"}, {"value2"}, {"-a"}, {"value4"} };
    Cli po(argc, argv);
    FunctionOption option("-a", [&](const Option& option) {
        EXPECT_EQ(3, option.values<>().size());
        EXPECT_EQ(std::string("value1"), option.values<>()[0]);
        EXPECT_EQ(std::string("value2"), option.values<>()[1]);
        EXPECT_EQ(std::string("value4"), option.values<>()[2]);
        executed = true;
        }, "--alpha", "Option A", false, &po);

    po.ParseArguments();
    EXPECT_EQ(true, executed);
    EXPECT_EQ(true, option.Exists);
    EXPECT_EQ(false, option.Mandatory);
    EXPECT_EQ(3, option.values<>().size());
    EXPECT_EQ(std::string("value1"), option.values<>()[0]);
    EXPECT_EQ(std::string("value2"), option.values<>()[1]);
    EXPECT_EQ(std::string("value4"), option.values<>()[2]);
}

TEST_F(ProgramOptionsTest, tagless_options_successful_with_multiple_values)
{
    int argc = 5;
    const char* argv[5]{ {"programoptions"}, {"value1"}, {"value2"}, {"value3"}, {"value4"} };
    Cli po{ argc, argv };
    po.Add(4);

    po.ParseArguments();

    auto a = po.GetOption("0");

    EXPECT_EQ(true, a.Exists);

    EXPECT_EQ(4, a.values<>().size());
    EXPECT_EQ(std::string("value1"), a.values<>()[0]);
    EXPECT_EQ(std::string("value2"), a.values<>()[1]);
    EXPECT_EQ(std::string("value3"), a.values<>()[2]);
    EXPECT_EQ(std::string("value4"), a.values<>()[3]);
}
TEST_F(ProgramOptionsTest, tagless_options_successful_with_one_value)
{
    int argc = 5;
    const char* argv[5]{ {"programoptions"}, {"value1"}, {"value2"}, {"value3"}, {"value4"} };
    Cli po{ argc, argv };
    po.Add();
    po.Add();
    po.Add();
    po.Add();

    po.ParseArguments();

    auto a = po.GetOption("0");
    auto b = po.GetOption("1");
    auto c = po.GetOption("2");
    auto d = po.GetOption("3");

    EXPECT_EQ(true, a.Exists);
    EXPECT_EQ(true, b.Exists);
    EXPECT_EQ(true, c.Exists);
    EXPECT_EQ(true, d.Exists);

    EXPECT_EQ(std::string("value1"), a.value<>());
    EXPECT_EQ(std::string("value2"), b.value<>());
    EXPECT_EQ(std::string("value3"), c.value<>());
    EXPECT_EQ(std::string("value4"), d.value<>());
}

TEST_F(ProgramOptionsTest, tagless_option_with_reference_succesful_1) {
    int argc = 5;
    const char* argv[5]{ {"programoptions"}, {"value1"}, {"value2"}, {"value3"}, {"value4"} };
    Cli po(argc, argv);
    TaglessOption a(1, "Option A", false, &po);
    TaglessOption b(1, "Option B", false, &po);
    TaglessOption c(1, "Option C", false, &po);
    TaglessOption d(1, "Option D", false, &po);

    po.ParseArguments();

    EXPECT_EQ(true, a.Exists);
    EXPECT_EQ(true, b.Exists);
    EXPECT_EQ(true, c.Exists);
    EXPECT_EQ(true, d.Exists);

    EXPECT_EQ(std::string("value1"), a.value<>());
    EXPECT_EQ(std::string("value2"), b.value<>());
    EXPECT_EQ(std::string("value3"), c.value<>());
    EXPECT_EQ(std::string("value4"), d.value<>());
}

TEST_F(ProgramOptionsTest, tagless_option_with_reference_succesful_2) {
    int argc = 5;
    const char* argv[5]{ {"programoptions"}, {"value1"}, {"value2"}, {"value3"}, {"value4"} };
    Cli po(argc, argv);
    TaglessOption a(4, "Option A", false, &po);

    po.ParseArguments();

    EXPECT_EQ(true, a.Exists);

    EXPECT_EQ(4, a.values<>().size());
    EXPECT_EQ(std::string("value1"), a.values<>()[0]);
    EXPECT_EQ(std::string("value2"), a.values<>()[1]);
    EXPECT_EQ(std::string("value3"), a.values<>()[2]);
    EXPECT_EQ(std::string("value4"), a.values<>()[3]);
}

TEST_F(ProgramOptionsTest, options_print) {
    int argc = 1;
    const char* argv[] = { "programoptions"};
    Cli po(argc, argv);
    po.Add("-a", "--alpha", "Option A");
    po.Add("-b", "--bravo", "Option B");
    po.Add("-c", "--charlie", "Option C");
    po.Add("-d", "--delta", "Option D");
    po.Add("-f", "--foxtrot", "Option F");
    po.Add("-e", "--echo", "Option E");
    po.PrintOptions();
}
