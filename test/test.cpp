#include "gtest/gtest.h"
#include "../include/BazPO.hpp"

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
    int argc = 3;
    const char* argv[3]{ {"programoptions"}, {"-a"}, {"Aoption"} };
    Cli po{ argc, argv };
    po.Add("-a", "--alpha", "Option A");
    po.ParseArguments();

    auto a = po.GetOption("-a");

    EXPECT_EQ(true, a.exists());

    EXPECT_EQ(std::string("Aoption"), a.value());
}

TEST_F(ProgramOptionsTest, get_second_option_successful)
{
    int argc = 3;
    const char* argv[3]{ {"programoptions"}, {"-a"}, {"Aoption"} };
    Cli po{ argc, argv };
    po.Add("-a", "--alpha", "Option A");
    po.ParseArguments();

    auto a = po.GetOption("--alpha");

    EXPECT_EQ(true, a.exists());

    EXPECT_EQ(std::string("Aoption"), a.value());
}

TEST_F(ProgramOptionsTest, int_conversion_succesful)
{
    int argc = 3;
    const char* argv[3]{ {"programoptions"}, {"-d"}, {"15"} };
    Cli po{ argc, argv };
    po.Add("-d", "--delta", "Option D");
    po.ParseArguments();

    auto a = po.GetOption("-d");

    EXPECT_EQ(true, a.exists());
    EXPECT_EQ(15, a.value_as<int>());
}

TEST_F(ProgramOptionsTest, double_conversion_succesful)
{
    int argc = 3;
    const char* argv[3]{ {"programoptions"}, {"-e"}, {"15.2156"} };
    Cli po{ argc, argv };
    po.Add("-e", "--echo", "Option E");
    po.ParseArguments();

    auto a = po.GetOption("-e");

    EXPECT_EQ(true, a.exists());
    EXPECT_EQ(15.2156, a.value_as<double>());
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

    EXPECT_EQ(true, a.exists());
    EXPECT_EQ(true, a.value_bool());

    EXPECT_EQ(true, b.exists());
    EXPECT_EQ(false, b.value_bool());

    EXPECT_EQ(true, c.exists());
    EXPECT_EQ(false, c.value_bool());
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

    EXPECT_EQ(true, a.exists());
    EXPECT_EQ(std::string("Aoption"), a.value());

    EXPECT_EQ(true, b.exists());
    EXPECT_EQ(std::string("Boption"), b.value());

    EXPECT_EQ(true, c.exists());
    EXPECT_EQ(std::string(""), c.value());
}

TEST_F(ProgramOptionsTest, exist_count_successful)
{
    int argc = 8;
    const char* argv[8]{ {"programoptions"}, {"-a"}, {"value1"}, {"-b"}, {"-a"}, {"value4"}, {"-c"} , {"value2"} };
    Cli po{ argc, argv };
    po.Add("-a", "--echo");
    po.Add("-b", "--bravo");
    po.Add("-c", "--charlie");
    po.ParseArguments();

    auto a = po.GetOption("-a");
    auto b = po.GetOption("-b");
    auto c = po.GetOption("-c");

    EXPECT_EQ(true, a.exists());
    EXPECT_EQ(2, a.exists_count());
    EXPECT_EQ(true, b.exists());
    EXPECT_EQ(1, b.exists_count());
    EXPECT_EQ(true, c.exists());
    EXPECT_EQ(1, c.exists_count());

    EXPECT_EQ(2, a.values().size());
    EXPECT_EQ(std::string("value1"), a.values()[0]);
    EXPECT_EQ(std::string("value4"), a.values()[1]);
    EXPECT_EQ(std::string(""), b.value());
    EXPECT_EQ(std::string("value2"), c.value());
}

TEST_F(ProgramOptionsTest, program_works_with_no_arguments_if_options_optional)
{
    int argc = 1;
    const char* argv[1]{ {"programoptions"} };
    Cli po{ argc, argv };

    po.Add("-a", "--echo");
    po.Add("-b", "--bravo");
    po.Add("-c", "--charlie");
}

TEST_F(ProgramOptionsTest, program_exits_when_help_is_provided)
{
    int argc = 2;
    const char* argv[2]{ {"programoptions"}, {"-h"}};
    Cli po{ argc, argv };
    po.Add("-a", "--alpha", "Option A");

    EXPECT_EXIT(po.ParseArguments(), testing::ExitedWithCode(0), "");
}

TEST_F(ProgramOptionsTest, program_exits_when_help_is_provided_in_tagless_mode)
{
    int argc = 2;
    const char* argv[2]{ {"programoptions"}, {"-h"} };
    Cli po{ argc, argv };
    po.Add();

    EXPECT_EXIT(po.ParseArguments(), testing::ExitedWithCode(0), "");
}

TEST_F(ProgramOptionsTest, program_exits_when_unknown_arguments_are_given)
{
    Cli po{ argc, argv };

    ASSERT_DEATH(po.ParseArguments(), "");
}

TEST_F(ProgramOptionsTest, program_exits_when_unknown_arguments_are_given_in_tagless_mode)
{
    Cli po{ argc, argv };
    po.Add();

    ASSERT_DEATH(po.ParseArguments(), "");
}

TEST_F(ProgramOptionsTest, program_exits_when_unknown_arguments_are_given_with_legit_ones)
{
    Cli po{ argc, argv };
    po.Add("-a", "--alpha", "Option A");

    ASSERT_DEATH(po.ParseArguments(), "");
}

TEST_F(ProgramOptionsTest, program_works_when_unknown_arguments_are_given_with_legit_ones_and_UnexpectedArgumentsAcceptable)
{
    Cli po{ argc, argv };
    po.Add("-a", "--alpha", "Option A");

    po.UnexpectedArgumentsAcceptable();
    po.ParseArguments();

    auto a = po.GetOption("-a");
    EXPECT_EQ(true, a.exists());
    EXPECT_EQ(1, a.exists_count());
    EXPECT_EQ(std::string("Aoption"), a.value());
    EXPECT_EQ(1, a.values().size());
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

    EXPECT_EQ(true, a.exists());
    EXPECT_EQ(false, b.exists());
    EXPECT_EQ(false, c.exists());

    EXPECT_EQ(4, a.values().size());
    EXPECT_EQ(std::string("value1"), a.values()[0]);
    EXPECT_EQ(std::string("value2"), a.values()[1]);
    EXPECT_EQ(std::string("value3"), a.values()[2]);
    EXPECT_EQ(std::string("value4"), a.values()[3]);
}

TEST_F(ProgramOptionsTest, multi_options_unsuccessful_with_limited_count)
{
    int argc = 6;
    const char* argv[6]{ {"programoptions"}, {"-a"}, {"value1"}, {"value2"}, {"value3"}, {"value4"} };
    Cli po{ argc, argv };
    po.Add("-a", "--echo", "", false, true, 3);

    ASSERT_DEATH(po.ParseArguments(), "");
}

TEST_F(ProgramOptionsTest, multi_options_successful_with_limited_count)
{
    int argc = 6;
    const char* argv[6]{ {"programoptions"}, {"-a"}, {"value1"}, {"value2"}, {"value3"}, {"value4"} };
    Cli po{ argc, argv };
    po.Add("-a", "--echo", "", false, true, 3);
    po.UnexpectedArgumentsAcceptable();
    po.ParseArguments();

    auto a = po.GetOption("-a");
    EXPECT_EQ(3, a.values().size());
    EXPECT_EQ(std::string("value1"), a.values()[0]);
    EXPECT_EQ(std::string("value2"), a.values()[1]);
    EXPECT_EQ(std::string("value3"), a.values()[2]);
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

    EXPECT_EQ(true, a.exists());
    EXPECT_EQ(false, b.exists());
    EXPECT_EQ(false, c.exists());

    EXPECT_EQ(3, a.values().size());
    EXPECT_EQ(std::string("value1"), a.values()[0]);
    EXPECT_EQ(std::string("value2"), a.values()[1]);
    EXPECT_EQ(std::string("value4"), a.values()[2]);
}

TEST_F(ProgramOptionsTest, multi_options_with_unknown_tags_successful)
{
    int argc = 6;
    const char* argv[6]{ {"programoptions"}, {"-a"}, {"value1"}, {"value2"}, {"-d"}, {"value4"} };
    Cli po{ argc, argv };
    po.Add("-a", "--echo", "", false, true);
    po.ParseArguments();

    auto a = po.GetOption("-a");

    EXPECT_EQ(true, a.exists());

    EXPECT_EQ(4, a.values().size());
    EXPECT_EQ(std::string("value1"), a.values()[0]);
    EXPECT_EQ(std::string("value2"), a.values()[1]);
    EXPECT_EQ(std::string("-d"), a.values()[2]);
    EXPECT_EQ(std::string("value4"), a.values()[3]);
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

    EXPECT_EQ(true, a.exists());

    EXPECT_EQ(2, a.values().size());
    EXPECT_EQ(std::string("value1"), a.values()[0]);
    EXPECT_EQ(std::string("value4"), a.values()[1]);
}

TEST_F(ProgramOptionsTest, mandatory_option_exits_when_not_provided) {
    int argc = 1;
    const char* argv[] = { "programoptions" };
    Cli po(argc, argv);
    po.Add("-a", "--alpha", "Option A", "", true);

    ASSERT_DEATH(po.ParseArguments(), "");
}

TEST_F(ProgramOptionsTest, mandatory_option_exits_when_not_provided_with_multiple_input) {
    Cli po(argc, argv);
    po.Add("-z", "--zeta", "Zeta", "", true);

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
    EXPECT_EQ(true, a.exists());
    EXPECT_EQ(std::string("input"), a.value());
}

TEST_F(ProgramOptionsTest, function_option_executes_successfully) {
    bool executed = false;
    int argc = 2;
    const char* argv[] = { "programoptions", "-a"};
    Cli po(argc, argv);
    po.Add("-a", [&](const Option&) { executed = true; }, "--alpha", "Option A");

    po.ParseArguments();
    EXPECT_EQ(true, executed);
}

TEST_F(ProgramOptionsTest, function_option_not_executed_when_not_provided) {
    bool executed = false;
    int argc = 1;
    const char* argv[] = { "programoptions"};
    Cli po(argc, argv);
    po.Add("-a", [&](const Option&) { executed = true; }, "--alpha", "Option A");

    po.ParseArguments();
    EXPECT_EQ(false, executed);
}

TEST_F(ProgramOptionsTest, function_option_contents_correct) {
    bool executed = false;
    int argc = 5;
    const char* argv[5]{ {"programoptions"}, {"-a"}, {"value1"}, {"-a"}, {"value4"} };
    Cli po(argc, argv);
    po.Add("-a", [&](const Option& option) {
        EXPECT_EQ(2, option.values().size());
        EXPECT_EQ(std::string("value1"), option.values()[0]);
        EXPECT_EQ(std::string("value4"), option.values()[1]);
        executed = true;
        }, "--alpha", "Option A", true);

    po.ParseArguments();
    EXPECT_EQ(true, executed);
}

TEST_F(ProgramOptionsTest, function_multi_options_successful)
{
    bool executed = false;
    int argc = 5;
    const char* argv[5]{ {"programoptions"}, {"value1"}, {"value2"}, {"value3"}, {"value4"} };
    Cli po{ argc, argv };
    po.Add([&](const Option& option) {
        EXPECT_EQ(4, option.values().size());
        EXPECT_EQ(std::string("value1"), option.values()[0]);
        EXPECT_EQ(std::string("value2"), option.values()[1]);
        EXPECT_EQ(std::string("value3"), option.values()[2]);
        EXPECT_EQ(std::string("value4"), option.values()[3]);
        executed = true;
        }, 4);

    po.ParseArguments();

    auto a = po.GetOption("0");

    EXPECT_EQ(true, a.exists());

    EXPECT_EQ(4, a.values().size());
    EXPECT_EQ(std::string("value1"), a.values()[0]);
    EXPECT_EQ(std::string("value2"), a.values()[1]);
    EXPECT_EQ(std::string("value3"), a.values()[2]);
    EXPECT_EQ(std::string("value4"), a.values()[3]);
}

TEST_F(ProgramOptionsTest, value_option_with_reference_succesful) {
    int argc = 3;
    const char* argv[3]{ {"programoptions"}, {"-a"}, {"Aoption"} };
    Cli po(argc, argv);
    ValueOption option(&po, "-a", "--alpha", "Option A", false);

    po.ParseArguments();

    EXPECT_EQ(true, option.exists());
    EXPECT_EQ(std::string("Aoption"), option.value());
}

TEST_F(ProgramOptionsTest, multi_option_with_reference_succesful) {
    int argc = 6;
    const char* argv[6]{ {"programoptions"}, {"-a"}, {"value1"}, {"value2"}, {"-a"}, {"value4"} };
    Cli po(argc, argv);
    MultiOption option(&po, "-a", "--alpha", "Option A", false);

    po.ParseArguments();

    EXPECT_EQ(true, option.exists());
    EXPECT_EQ(3, option.values().size());
    EXPECT_EQ(std::string("value1"), option.values()[0]);
    EXPECT_EQ(std::string("value2"), option.values()[1]);
    EXPECT_EQ(std::string("value4"), option.values()[2]);
}

TEST_F(ProgramOptionsTest, function_option_with_reference_succesful) {
    bool executed = false;
    int argc = 5;
    const char* argv[5]{ {"programoptions"}, {"-a"}, {"value1"}, {"-a"}, {"value4"} };
    Cli po(argc, argv);
    FunctionOption option(&po, "-a", [&](const Option& option) {
        EXPECT_EQ(2, option.values().size());
        EXPECT_EQ(std::string("value1"), option.values()[0]);
        EXPECT_EQ(std::string("value4"), option.values()[1]);
        executed = true;
        }, "--alpha", "Option A", false);

    po.ParseArguments();
    EXPECT_EQ(true, executed);
    EXPECT_EQ(true, option.exists());
    EXPECT_EQ(2, option.values().size());
    EXPECT_EQ(std::string("value1"), option.values()[0]);
    EXPECT_EQ(std::string("value4"), option.values()[1]);
}

TEST_F(ProgramOptionsTest, function_multi_option_with_reference_succesful) {
    bool executed = false;
    int argc = 6;
    const char* argv[6]{ {"programoptions"}, {"-a"}, {"value1"}, {"value2"}, {"-a"}, {"value4"} };
    Cli po(argc, argv);
    FunctionMultiOption option(&po, "-a", [&](const Option& option) {
        EXPECT_EQ(3, option.values().size());
        EXPECT_EQ(std::string("value1"), option.values()[0]);
        EXPECT_EQ(std::string("value2"), option.values()[1]);
        EXPECT_EQ(std::string("value4"), option.values()[2]);
        executed = true;
        }, "--alpha", "Option A");

    po.ParseArguments();
    EXPECT_EQ(true, executed);
    EXPECT_EQ(true, option.exists());
    EXPECT_EQ(3, option.values().size());
    EXPECT_EQ(std::string("value1"), option.values()[0]);
    EXPECT_EQ(std::string("value2"), option.values()[1]);
    EXPECT_EQ(std::string("value4"), option.values()[2]);
}

TEST_F(ProgramOptionsTest, function_multi_options_with_reference_successful)
{
    bool executed = false;
    int argc = 5;
    const char* argv[5]{ {"programoptions"}, {"value1"}, {"value2"}, {"value3"}, {"value4"} };
    Cli po{ argc, argv };
    FunctionTaglessOption option(&po, [&](const Option& option) {
        EXPECT_EQ(4, option.values().size());
        EXPECT_EQ(std::string("value1"), option.values()[0]);
        EXPECT_EQ(std::string("value2"), option.values()[1]);
        EXPECT_EQ(std::string("value3"), option.values()[2]);
        EXPECT_EQ(std::string("value4"), option.values()[3]);
        executed = true;
        }, 4);

    po.ParseArguments();

    EXPECT_EQ(true, option.exists());

    EXPECT_EQ(4, option.values().size());
    EXPECT_EQ(std::string("value1"), option.values()[0]);
    EXPECT_EQ(std::string("value2"), option.values()[1]);
    EXPECT_EQ(std::string("value3"), option.values()[2]);
    EXPECT_EQ(std::string("value4"), option.values()[3]);
}

TEST_F(ProgramOptionsTest, tagless_options_successful_with_multiple_values)
{
    int argc = 5;
    const char* argv[5]{ {"programoptions"}, {"value1"}, {"value2"}, {"value3"}, {"value4"} };
    Cli po{ argc, argv };
    po.Add(4);

    po.ParseArguments();

    auto a = po.GetOption("0");

    EXPECT_EQ(true, a.exists());

    EXPECT_EQ(4, a.values().size());
    EXPECT_EQ(std::string("value1"), a.values()[0]);
    EXPECT_EQ(std::string("value2"), a.values()[1]);
    EXPECT_EQ(std::string("value3"), a.values()[2]);
    EXPECT_EQ(std::string("value4"), a.values()[3]);
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

    EXPECT_EQ(true, a.exists());
    EXPECT_EQ(true, b.exists());
    EXPECT_EQ(true, c.exists());
    EXPECT_EQ(true, d.exists());

    EXPECT_EQ(std::string("value1"), a.value());
    EXPECT_EQ(std::string("value2"), b.value());
    EXPECT_EQ(std::string("value3"), c.value());
    EXPECT_EQ(std::string("value4"), d.value());
}

TEST_F(ProgramOptionsTest, tagless_option_with_reference_succesful_1) {
    int argc = 5;
    const char* argv[5]{ {"programoptions"}, {"value1"}, {"value2"}, {"value3"}, {"value4"} };
    Cli po(argc, argv);
    TaglessOption a(&po, 1, "Option A");
    TaglessOption b(&po, 1, "Option B");
    TaglessOption c(&po, 1, "Option C");
    TaglessOption d(&po, 1, "Option D");

    po.ParseArguments();

    EXPECT_EQ(true, a.exists());
    EXPECT_EQ(true, b.exists());
    EXPECT_EQ(true, c.exists());
    EXPECT_EQ(true, d.exists());

    EXPECT_EQ(std::string("value1"), a.value());
    EXPECT_EQ(std::string("value2"), b.value());
    EXPECT_EQ(std::string("value3"), c.value());
    EXPECT_EQ(std::string("value4"), d.value());
}

TEST_F(ProgramOptionsTest, tagless_option_with_reference_succesful_2) {
    int argc = 5;
    const char* argv[5]{ {"programoptions"}, {"value1"}, {"value2"}, {"value3"}, {"value4"} };
    Cli po(argc, argv);
    TaglessOption a(&po, 4, "Option A", false);

    po.ParseArguments();

    EXPECT_EQ(true, a.exists());

    EXPECT_EQ(4, a.values().size());
    EXPECT_EQ(std::string("value1"), a.values()[0]);
    EXPECT_EQ(std::string("value2"), a.values()[1]);
    EXPECT_EQ(std::string("value3"), a.values()[2]);
    EXPECT_EQ(std::string("value4"), a.values()[3]);
}

TEST_F(ProgramOptionsTest, tagless_then_other_mismatch_throws) {
    Cli po(argc, argv);
    po.Add();

    EXPECT_ANY_THROW(po.Add("-a"));
}

TEST_F(ProgramOptionsTest, other_then_tagless_mismatch_throws) {
    Cli po(argc, argv);
    po.Add("-a");

    EXPECT_ANY_THROW(po.Add());
}

TEST_F(ProgramOptionsTest, tagless_then_other_reference_mismatch_throws) {
    Cli po(argc, argv);
    TaglessOption a(&po);
    
    EXPECT_ANY_THROW(ValueOption a(&po, ""));
}

TEST_F(ProgramOptionsTest, other_then_tagless_reference_mismatch_throws) {
    Cli po(argc, argv);
    ValueOption a(&po, "");

    EXPECT_ANY_THROW(TaglessOption a(&po));
}

TEST_F(ProgramOptionsTest, options_print) {
    int argc = 1;
    const char* argv[] = { "programoptions" };
    Cli po(argc, argv);
    po.Add("-a", "--alpha", "Option A");
    po.Add("-b", "--bravo", "Option B");
    po.Add("-c", "--charlie", "Option C");
    po.Add("-d", "--delta", "Option D");
    po.Add("-f", "--foxtrot", "Option F");
    po.Add("-e", "--echo", "Option E");
    po.PrintOptions();
}

TEST_F(ProgramOptionsTest, tagless_options_print) {
    int argc = 1;
    const char* argv[] = { "programoptions" };
    Cli po(argc, argv);
    po.Add(4, "My values");
    po.PrintOptions();
}
