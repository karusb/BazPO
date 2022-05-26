#include "gtest/gtest.h"
#include "../include/BazPO.hpp"
#include <cmath>

using namespace BazPO;

class ProgramOptionsTest
    : public ::testing::Test
{
public:
    int argc = 10;
    const char* argv[10]{{"programoptions"}, {"-a"}, {"Aoption"}, {"--bravo"}, {"Boption"}, {"-c"}, {"-d"}, {"15"}, {"-e"}, {"15.2156"}};
    void ExpectValues(const Option& option, std::deque<std::string> expectedValues);
    void ExpectOptionExistsWithValue(Cli& cli, std::string option, std::string expectedValue);
    void ExpectOptionExistsWithValue(const Option& option, std::string expectedValue);
    void ExpectOptionExistsWithValues(Cli& cli, std::string optionKey, std::deque<std::string> expectedValues);
    void ExpectOptionExistsWithValues(const Option& option, std::deque<std::string> expectedValues);
};


void ProgramOptionsTest::ExpectValues(const Option& option, std::deque<std::string> expectedValues)
{
    EXPECT_EQ(expectedValues.size(), option.values().size());
    for (int i = 0; i < expectedValues.size(); ++i)
        EXPECT_EQ(expectedValues[i], option.values()[i]);
}

void ProgramOptionsTest::ExpectOptionExistsWithValue(Cli& cli, std::string optionKey, std::string expectedValue)
{
    auto& option = cli.option(optionKey);
    EXPECT_TRUE(option.exists());
    EXPECT_EQ(expectedValue, option.value());
}

void ProgramOptionsTest::ExpectOptionExistsWithValue(const Option& option, std::string expectedValue)
{
    EXPECT_TRUE(option.exists());
    EXPECT_EQ(expectedValue, option.value());
}

void ProgramOptionsTest::ExpectOptionExistsWithValues(Cli& cli, std::string optionKey, std::deque<std::string> expectedValues)
{
    auto& option = cli.option(optionKey);
    EXPECT_TRUE(option.exists());
    ExpectValues(option, expectedValues);
}

void ProgramOptionsTest::ExpectOptionExistsWithValues(const Option& option, std::deque<std::string> expectedValues)
{
    EXPECT_TRUE(option.exists());
    ExpectValues(option, expectedValues);
}

TEST_F(ProgramOptionsTest, add_then_get_successful)
{
    int argc = 3;
    const char* argv[3]{ {"programoptions"}, {"-a"}, {"Aoption"} };
    Cli po{ argc, argv };
    po.add("-a", "--alpha", "Option A");
    po.parse();

    ExpectOptionExistsWithValue(po, "-a", "Aoption");
}

TEST_F(ProgramOptionsTest, default_value_returned_when_option_doesnt_exist)
{
    int argc = 1;
    const char* argv[1]{ {"programoptions"}};
    std::string defaultValue = "my default value";
    Cli po{ argc, argv };
    po.add("-a", "--alpha", "Option A", defaultValue);
    po.parse();

    auto& a = po.option("-a");

    EXPECT_FALSE(a.exists());
    EXPECT_EQ(defaultValue, a.value());
}

TEST_F(ProgramOptionsTest, default_value_not_reachable_when_option_exist)
{
    int argc = 3;
    const char* argv[3]{ {"programoptions"}, {"-a"}, {"Aoption"} };
    Cli po{ argc, argv };
    po.add("-a", "--alpha", "Option A", "my default value");
    po.parse();

    ExpectOptionExistsWithValue(po, "-a", "Aoption");
    ExpectOptionExistsWithValues(po, "-a", { "Aoption"});
}

TEST_F(ProgramOptionsTest, get_second_option_successful)
{
    int argc = 3;
    const char* argv[3]{ {"programoptions"}, {"-a"}, {"Aoption"} };
    Cli po{ argc, argv };
    po.add("-a", "--alpha", "Option A");
    po.parse();

    ExpectOptionExistsWithValue(po, "--alpha", "Aoption");
}

TEST_F(ProgramOptionsTest, int_conversion_succesful)
{
    int argc = 3;
    const char* argv[3]{ {"programoptions"}, {"-d"}, {"15"} };
    Cli po{ argc, argv };
    po.add("-d", "--delta", "Option D");
    po.parse();

    auto& a = po.option("-d");

    EXPECT_TRUE(a.exists());
    EXPECT_EQ(15, a.valueAs<int>());
}

TEST_F(ProgramOptionsTest, double_conversion_succesful)
{
    int argc = 3;
    const char* argv[3]{ {"programoptions"}, {"-e"}, {"15.2156"} };
    Cli po{ argc, argv };
    po.add("-e", "--echo", "Option E");
    po.parse();

    auto& a = po.option("-e");

    EXPECT_TRUE(a.exists());
    EXPECT_EQ(15.2156, a.valueAs<double>());
}

TEST_F(ProgramOptionsTest, string_conversion_succesful)
{
    int argc = 3;
    const char* argv[3]{ {"programoptions"}, {"-a"}, {"some text with multiple spaces"} };
    Cli po{ argc, argv };
    po.add("-a", "--alpha", "Option A");
    po.parse();

    auto& a = po.option("-a");

    EXPECT_TRUE(a.exists());
    EXPECT_EQ("some text with multiple spaces", a.valueAs<std::string>());
}

TEST_F(ProgramOptionsTest, bool_conversion_succesful)
{
    int argc = 6;
    const char* argv[6]{ {"programoptions"}, {"-a"}, {"True"}, {"--bravo"}, {"False"}, {"-c"} };
    Cli po{ argc, argv };
    po.add("-a", "--echo");
    po.add("-b", "--bravo");
    po.add("-c", "--charlie");
    po.parse();

    auto& a = po.option("-a");
    auto& b = po.option("-b");
    auto& c = po.option("-c");

    EXPECT_TRUE(a.exists());
    EXPECT_TRUE(a.valueAs<bool>());

    EXPECT_TRUE(b.exists());
    EXPECT_FALSE(b.valueAs<bool>());

    EXPECT_TRUE(c.exists());
    EXPECT_FALSE(c.valueAs<bool>());
}

TEST_F(ProgramOptionsTest, all_options_are_found)
{
    int argc = 6;
    const char* argv[6]{ {"programoptions"}, {"-a"}, {"Aoption"}, {"--bravo"}, {"Boption"}, {"-c"} };
    Cli po{ argc, argv };
    po.add("-a", "--alpha", "Option A");
    po.add("-b", "--bravo", "Option B");
    po.add("-c", "--charlie", "Option C");
    po.parse();

    ExpectOptionExistsWithValue(po, "-a", "Aoption");
    ExpectOptionExistsWithValue(po, "-b", "Boption");
    ExpectOptionExistsWithValue(po, "-c", "");
}

TEST_F(ProgramOptionsTest, values_returns_correct_contents)
{
    int argc = 7;
    const char* argv[7]{ {"programoptions"}, {"-a"}, {"Aoption"}, {"-a"}, {"Boption"}, {"-a"}, {"Coption"} };
    Cli po{ argc, argv };
    po.add("-a", "--alpha", "Option A", "", false, 3);
    po.add("-b", "--bravo", "Option B");
    po.add("-c", "--charlie", "Option C");
    po.parse();

    auto& a = po.option("-a");

    EXPECT_TRUE(a.exists());
    std::deque<const char*> expected{ argv[2], argv[4], argv[6]};
    EXPECT_EQ(expected, a.values());
}

TEST_F(ProgramOptionsTest, values_returns_correct_string_contents)
{
    int argc = 7;
    const char* argv[7]{ {"programoptions"}, {"-a"}, {"Aoption"}, {"-a"}, {"Boption"}, {"-a"}, {"Coption"} };
    Cli po{ argc, argv };
    po.add("-a", "--alpha", "Option A");
    po.add("-b", "--bravo", "Option B");
    po.add("-c", "--charlie", "Option C");
    po.parse();

    auto& a = po.option("-a");

    EXPECT_TRUE(a.exists());
    std::deque<std::string> expected{ "Aoption", "Boption", "Coption" };
    EXPECT_EQ(expected, a.valuesAs<std::string>());
}

TEST_F(ProgramOptionsTest, values_returns_correct_bool_contents)
{
    int argc = 7;
    const char* argv[7]{ {"programoptions"}, {"-a"}, {"True"}, {"-a"}, {"y"}, {"-a"}, {"Coption"} };
    Cli po{ argc, argv };
    po.add("-a", "--alpha", "Option A");
    po.add("-b", "--bravo", "Option B");
    po.add("-c", "--charlie", "Option C");
    po.parse();

    auto& a = po.option("-a");

    EXPECT_TRUE(a.exists());
    std::deque<bool> expected{ true, true, false };
    EXPECT_EQ(expected, a.valuesAs<bool>());
}

TEST_F(ProgramOptionsTest, exist_count_successful)
{
    int argc = 8;
    const char* argv[8]{ {"programoptions"}, {"-a"}, {"value1"}, {"-b"}, {"-a"}, {"value4"}, {"-c"} , {"value2"} };
    Cli po{ argc, argv };
    po.add("-a", "--echo");
    po.add("-b", "--bravo");
    po.add("-c", "--charlie");
    po.parse();

    auto& a = po.option("-a");
    auto& b = po.option("-b");
    auto& c = po.option("-c");

    EXPECT_TRUE(a.exists());
    EXPECT_EQ(2, a.existsCount());
    EXPECT_TRUE(b.exists());
    EXPECT_EQ(1, b.existsCount());
    EXPECT_TRUE(c.exists());
    EXPECT_EQ(1, c.existsCount());

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

    po.add("-a", "--echo");
    po.add("-b", "--bravo");
    po.add("-c", "--charlie");
}

TEST_F(ProgramOptionsTest, program_exits_when_help_is_provided)
{
    int argc = 2;
    const char* argv[2]{ {"programoptions"}, {"-h"}};
    Cli po{ argc, argv };
    po.add("-a", "--alpha", "Option A");

    EXPECT_EXIT(po.parse(), testing::ExitedWithCode(0), "");
}

TEST_F(ProgramOptionsTest, program_exits_when_help_is_provided_with_mandatory_option)
{
    int argc = 2;
    const char* argv[2]{ {"programoptions"}, {"-h"} };
    Cli po{ argc, argv };
    po.add("-a", "--alpha", "Option A", "").mandatory();

    EXPECT_EXIT(po.parse(), testing::ExitedWithCode(0), "");
}

TEST_F(ProgramOptionsTest, program_exits_when_help_is_provided_in_tagless_mode)
{
    int argc = 2;
    const char* argv[2]{ {"programoptions"}, {"-h"} };
    Cli po{ argc, argv };
    po.add();

    EXPECT_EXIT(po.parse(), testing::ExitedWithCode(0), "");
}

TEST_F(ProgramOptionsTest, program_exits_when_unknown_arguments_are_given)
{
    Cli po{ argc, argv };

    EXPECT_EXIT(po.parse(), testing::ExitedWithCode(1), "");
}

TEST_F(ProgramOptionsTest, program_exits_when_unknown_arguments_are_given_in_tagless_mode)
{
    Cli po{ argc, argv };
    po.add();

    EXPECT_EXIT(po.parse(), testing::ExitedWithCode(1), "");
}

TEST_F(ProgramOptionsTest, program_exits_when_unknown_arguments_are_given_with_legit_ones)
{
    Cli po{ argc, argv };
    po.add("-a", "--alpha", "Option A");

    EXPECT_EXIT(po.parse(), testing::ExitedWithCode(1), "");
}

TEST_F(ProgramOptionsTest, program_works_when_unknown_arguments_are_given_with_legit_ones_and_UnexpectedArgumentsAcceptable)
{
    Cli po{ argc, argv };
    po.add("-a", "--alpha", "Option A");

    po.unexpectedArgumentsAcceptable();
    po.parse();

    auto& a = po.option("-a");
    ExpectOptionExistsWithValues(a, { "Aoption" });
    EXPECT_EQ(1, a.existsCount());
}

TEST_F(ProgramOptionsTest, option_prioritized_after_adding_works)
{
    Cli po{ argc, argv };
    auto& a = po.add("-a");
    auto& b = po.add("-b");
    auto& c = po.add("-c").prioritize();

    po.parse();

    ExpectOptionExistsWithValues(c, { "-d" });
    EXPECT_EQ(1, c.existsCount());
    EXPECT_FALSE(a.exists());
    EXPECT_FALSE(b.exists());
}

TEST_F(ProgramOptionsTest, prioritized_option_is_defined_and_provided_different_args_not_parsed_cli)
{
    {
        Cli po{ argc, argv };
        po.add("-a", "--echo", "");
        po.add("-b", "--bravo");
        po.add("-c", "--charlie");

        po.prioritize("-c");
        po.parse();

        auto& c = po.option("-c");
        ExpectOptionExistsWithValues(c, { "-d" });
        EXPECT_EQ(1, c.existsCount());

        auto& a = po.option("-a");
        EXPECT_FALSE(a.exists());
        EXPECT_EQ(0, a.existsCount());
        EXPECT_EQ(std::string(""), a.value());
        EXPECT_EQ(0, a.values().size());
    }
    {
        Cli po{ argc, argv };
        ValueOption optionA(&po, "-a", "--alpha");
        ValueOption optionB(&po, "-b", "--bravo");
        ValueOption optionC(&po, "-c", "--charlie");

        optionC.prioritize();
        po.parse();

        ExpectOptionExistsWithValues(optionC, { "-d" });
        EXPECT_EQ(1, optionC.existsCount());

        EXPECT_FALSE(optionA.exists());
        EXPECT_EQ(0, optionA.existsCount());
        EXPECT_EQ(std::string(""), optionA.value());
        EXPECT_EQ(0, optionA.values().size());
    }
}

TEST_F(ProgramOptionsTest, multi_options_successful)
{
    int argc = 7;
    const char* argv[7]{ {"programoptions"}, {"-a"}, {"value1"}, {"value2"}, {"-b"}, {"value1"}, {"value2"} };
    Cli po{ argc, argv };
    po.add("-a", "--echo", "", "", true);
    po.add("-b", "--bravo", "", "", true);
    po.add("-c", "--charlie", "", "", true);

    po.parse();

    EXPECT_FALSE(po.option("-c").exists());

    ExpectOptionExistsWithValues(po, "-a", { "value1", "value2" });
    ExpectOptionExistsWithValues(po, "-b", { "value1", "value2" });
}

TEST_F(ProgramOptionsTest, prioritized_multi_options_successful)
{
    int argc = 6;
    const char* argv[6]{ {"programoptions"}, {"-a"}, {"value1"}, {"value2"}, {"value3"}, {"value4"} };
    Cli po{ argc, argv };
    po.add("-a", "--echo", "", "", true);
    po.add("-b", "--bravo", "", "", true);
    po.add("-c", "--charlie", "", "", true);

    po.prioritize("-a");

    po.parse();

    EXPECT_FALSE(po.option("-b").exists());
    EXPECT_FALSE(po.option("-c").exists());

    ExpectOptionExistsWithValues(po, "-a", { "value1", "value2", "value3", "value4" });
}

TEST_F(ProgramOptionsTest, multi_options_unsuccessful_with_limited_count)
{
    int argc = 6;
    const char* argv[6]{ {"programoptions"}, {"-a"}, {"value1"}, {"value2"}, {"value3"}, {"value4"} };
    Cli po{ argc, argv };
    po.add("-a", "--echo", "", "", true, 3);

    ASSERT_DEATH(po.parse(), "");
}

TEST_F(ProgramOptionsTest, multi_options_successful_with_limited_count)
{
    int argc = 6;
    const char* argv[6]{ {"programoptions"}, {"-a"}, {"value1"}, {"value2"}, {"value3"}, {"value4"} };
    Cli po{ argc, argv };
    po.add("-a", "--echo", "", "", true, 3);

    po.unexpectedArgumentsAcceptable();
    po.parse();

    ExpectOptionExistsWithValues(po, "-a", { "value1", "value2", "value3" });
}

TEST_F(ProgramOptionsTest, multi_options_with_multi_tags_successful)
{
    int argc = 6;
    const char* argv[6]{ {"programoptions"}, {"-a"}, {"value1"}, {"value2"}, {"-a"}, {"value4"} };
    Cli po{ argc, argv };
    po.add("-a", "--echo", "", "", true);
    po.add("-b", "--bravo");
    po.add("-c", "--charlie");
    po.parse();

    EXPECT_FALSE(po.option("-b").exists());
    EXPECT_FALSE(po.option("-c").exists());

    ExpectOptionExistsWithValues(po, "-a", { "value1", "value2", "value4" });
}

TEST_F(ProgramOptionsTest, multi_options_with_unknown_tags_successful)
{
    int argc = 6;
    const char* argv[6]{ {"programoptions"}, {"-a"}, {"value1"}, {"value2"}, {"-d"}, {"value4"} };
    Cli po{ argc, argv };
    po.add("-a", "--echo", "", "", true);
    po.parse();

    ExpectOptionExistsWithValues(po, "-a", { "value1", "value2", "-d", "value4" });
}

TEST_F(ProgramOptionsTest, value_option_with_unknown_tags_successful_with_acceptable_unknown_arguments)
{
    int argc = 6;
    const char* argv[6]{ {"programoptions"}, {"-a"}, {"value1"}, {"value2"}, {"-d"}, {"value4"} };
    Cli po{ argc, argv };
    po.add("-a", "--echo");
    po.unexpectedArgumentsAcceptable();
    po.parse();

    ExpectOptionExistsWithValues(po, "-a", { "value1"});
}

TEST_F(ProgramOptionsTest, value_option_with_multi_tags_successful)
{
    int argc = 5;
    const char* argv[5]{ {"programoptions"}, {"-a"}, {"value1"}, {"-a"}, {"value4"} };
    Cli po{ argc, argv };
    po.add("-a", "--echo", "");
    po.add("-b", "--bravo");
    po.add("-c", "--charlie");
    po.parse();

    ExpectOptionExistsWithValues(po, "-a", { "value1", "value4" });
}

TEST_F(ProgramOptionsTest, mandatory_option_exits_when_not_provided) {
    int argc = 1;
    const char* argv[] = { "programoptions" };
    Cli po(argc, argv);
    po.add("-a", "--alpha", "Option A").mandatory();

    ASSERT_DEATH(po.parse(), "");
}

TEST_F(ProgramOptionsTest, mandatory_option_exits_when_not_provided_with_multiple_input) {
    Cli po(argc, argv);
    po.add("-z", "--zeta", "Zeta").mandatory();

    ASSERT_DEATH(po.parse(), "");
}

TEST_F(ProgramOptionsTest, mandatory_option_asked_when_not_provided) {
    int argc = 1;
    const char* argv[] = { "programoptions" };
    std::stringstream str("input");
    Cli po(argc, argv);
    po.userInputRequired();
    po.add("-a", "--alpha", "Option A").mandatory();
    po.changeIO(&std::cout, &str);

    po.parse();

    ExpectOptionExistsWithValue(po, "-a", "input");
}

TEST_F(ProgramOptionsTest, multi_line_input_parsed_as_whole_line) {
    int argc = 1;
    const char* argv[] = { "programoptions" };
    std::stringstream str("input input2 input3");
    Cli po(argc, argv);
    po.userInputRequired();
    po.add("-a", "--alpha", "Option A").mandatory();
    po.changeIO(&std::cout, &str);

    po.parse();

    ExpectOptionExistsWithValue(po, "-a", "input input2 input3");
}

TEST_F(ProgramOptionsTest, multi_line_double_parsed_correctly) {
    int argc = 1;
    const char* argv[] = { "programoptions" };
    std::stringstream str("15.87396509125677 \r\n");
    Cli po(argc, argv);
    po.userInputRequired();
    po.add("-a", "--alpha", "Option A").mandatory();
    po.changeIO(&std::cout, &str);

    po.parse();

    auto& a = po.option("-a");
    EXPECT_TRUE(a.exists());
    EXPECT_EQ(15.87396509125677, a.valueAs<double>());
}

TEST_F(ProgramOptionsTest, multiple_inputs_can_be_provided_when_requested_via_cli) {
    int argc = 1;
    const char* argv[] = { "programoptions" };
    std::stringstream str("input input2 input3\ninput4 input5 input6\n");
    Cli po(argc, argv);
    po.add("-a", "--alpha", "Option A");
    po.changeIO(&std::cout, &str);

    po.parse();

    po.askInput("-a");

    ExpectOptionExistsWithValues(po, "-a", { "input input2 input3" });

    po.askInput("-a");

    ExpectOptionExistsWithValues(po, "-a", { "input input2 input3", "input4 input5 input6" });
}

TEST_F(ProgramOptionsTest, empty_user_input_recognized_does_not_exist) 
{
    int argc = 1;
    const char* argv[] = { "programoptions" };
    std::stringstream str("\ninput4 input5 input6\n");
    Cli po(argc, argv);
    po.add("-a", "--alpha", "Option A").mandatory();
    po.add("-b", "--bravo", "Option B").mandatory();
    po.changeIO(&std::cout, &str);
    po.userInputRequired();

    EXPECT_EXIT(po.parse(), testing::ExitedWithCode(1), "");
}

TEST_F(ProgramOptionsTest, function_option_executes_successfully) 
{
    bool executed = false;
    int argc = 2;
    const char* argv[] = { "programoptions", "-a"};
    Cli po(argc, argv);
    po.add("-a", [&](const Option&) { executed = true; }, "--alpha", "Option A");

    po.parse();
    EXPECT_TRUE(executed);
}

TEST_F(ProgramOptionsTest, function_option_not_executed_when_not_provided) 
{
    bool executed = false;
    int argc = 1;
    const char* argv[] = { "programoptions"};
    Cli po(argc, argv);
    po.add("-a", [&](const Option&) { executed = true; }, "--alpha", "Option A");

    po.parse();
    EXPECT_FALSE(executed);
}

TEST_F(ProgramOptionsTest, function_option_contents_correct) 
{
    int argc = 5;
    const char* argv[5]{ {"programoptions"}, {"-a"}, {"value1"}, {"-a"}, {"value4"} };
    {
        bool executed = false;
        Cli po(argc, argv);
        po.add("-a", [&](const Option& option) {
            ExpectOptionExistsWithValues(option, { "value1", "value4" });
            executed = true;
            }, "--alpha", "Option A", "", true);

        po.parse();
        EXPECT_TRUE(executed);
    }
    {
        bool executed = false;
        Cli po(argc, argv);
        FunctionOption option(&po, "-a", [&](const Option& option) {
            ExpectOptionExistsWithValues(option, { "value1", "value4" });
            executed = true;
            }, "--alpha", "Option A");

        po.parse();

        EXPECT_TRUE(executed);
        ExpectOptionExistsWithValues(option, { "value1", "value4" });
    }
}

TEST_F(ProgramOptionsTest, function_multi_options_successful)
{
    int argc = 6;
    const char* argv[6]{ {"programoptions"}, {"-a"}, {"value1"}, {"value2"}, {"-a"}, {"value4"} };
    {
        bool executed = false;
        Cli po{ argc, argv };
        po.add("-a", [&](const Option& option) {
            ExpectOptionExistsWithValues(option, { "value1", "value2", "value4" });
            executed = true;
            }, "", "", "", true);

        po.parse();
        EXPECT_TRUE(executed);
        ExpectOptionExistsWithValues(po, "-a", { "value1", "value2", "value4" });
    }
    {
        bool executed = false;
        Cli po(argc, argv);
        FunctionMultiOption option(&po, "-a", [&](const Option& option) {
            ExpectOptionExistsWithValues(option, { "value1", "value2", "value4" });
            executed = true;
            }, "--alpha", "Option A");

        po.parse();
        EXPECT_TRUE(executed);
        ExpectOptionExistsWithValues(option, { "value1", "value2", "value4" });
    }
}

TEST_F(ProgramOptionsTest, function_tagless_option_successful)
{
    int argc = 5;
    const char* argv[5]{ {"programoptions"}, {"value1"}, {"value2"}, {"value3"}, {"value4"} };
    {
        bool executed = false;
        Cli po{ argc, argv };
        po.add([&](const Option& option) {
            ExpectOptionExistsWithValues(option, { "value1", "value2", "value3", "value4" });
            executed = true;
            }, 4);

        po.parse();
        EXPECT_TRUE(executed);
        ExpectOptionExistsWithValues(po, "0", { "value1", "value2", "value3", "value4" });
    }
    {
        bool executed = false;
        Cli po{ argc, argv };
        FunctionTaglessOption option(&po, [&](const Option& option) {
            ExpectOptionExistsWithValues(option, { "value1", "value2", "value3", "value4" });
            executed = true;
            }, 4);

        po.parse();
        EXPECT_TRUE(executed);
        ExpectOptionExistsWithValues(option, { "value1", "value2", "value3", "value4" });
    }
}

TEST_F(ProgramOptionsTest, value_option_with_reference_succesful) 
{
    int argc = 3;
    const char* argv[3]{ {"programoptions"}, {"-a"}, {"Aoption"} };
    Cli po(argc, argv);
    ValueOption option(&po, "-a", "--alpha", "Option A");

    po.parse();

    ExpectOptionExistsWithValue(option, "Aoption");
}

TEST_F(ProgramOptionsTest, multi_option_with_reference_succesful) 
{
    int argc = 6;
    const char* argv[6]{ {"programoptions"}, {"-a"}, {"value1"}, {"value2"}, {"-a"}, {"value4"} };
    Cli po(argc, argv);
    MultiOption option(&po, "-a", "--alpha", "Option A");

    po.parse();

    ExpectOptionExistsWithValues(option, { "value1", "value2", "value4" });
}

TEST_F(ProgramOptionsTest, tagless_options_successful_with_multiple_values)
{
    int argc = 5;
    const char* argv[5]{ {"programoptions"}, {"value1"}, {"value2"}, {"value3"}, {"value4"} };
    Cli po{ argc, argv };
    po.add(4);

    po.parse();

    ExpectOptionExistsWithValues(po, "0", {"value1", "value2", "value3", "value4"});
}

TEST_F(ProgramOptionsTest, tagless_options_successful_with_one_value)
{
    int argc = 5;
    const char* argv[5]{ {"programoptions"}, {"value1"}, {"value2"}, {"value3"}, {"value4"} };
    Cli po{ argc, argv };
    po.add();
    po.add();
    po.add();
    po.add();

    po.parse();

    ExpectOptionExistsWithValue(po, "0", "value1");
    ExpectOptionExistsWithValue(po, "1", "value2");
    ExpectOptionExistsWithValue(po, "2", "value3");
    ExpectOptionExistsWithValue(po, "3", "value4");
}

TEST_F(ProgramOptionsTest, tagless_options_with_single_value_reference) {
    int argc = 5;
    const char* argv[5]{ {"programoptions"}, {"value1"}, {"value2"}, {"value3"}, {"value4"} };
    Cli po(argc, argv);
    TaglessOption a(&po, 1, "Option A");
    TaglessOption b(&po, 1, "Option B");
    TaglessOption c(&po, 1, "Option C");
    TaglessOption d(&po, 1, "Option D");

    po.parse();

    ExpectOptionExistsWithValue(a, "value1");
    ExpectOptionExistsWithValue(b, "value2");
    ExpectOptionExistsWithValue(c, "value3");
    ExpectOptionExistsWithValue(d, "value4");
}

TEST_F(ProgramOptionsTest, tagless_option_with_multiple_value_reference) 
{
    int argc = 5;
    const char* argv[5]{ {"programoptions"}, {"value1"}, {"value2"}, {"value3"}, {"value4"} };
    Cli po(argc, argv);
    TaglessOption a(&po, 4, "Option A");

    po.parse();

    ExpectOptionExistsWithValues(a, { "value1", "value2", "value3", "value4" });
}

TEST_F(ProgramOptionsTest, all_option_combinations_with_reference_succesful)
{
    {
        int argc = 7;
        const char* argv[7]{ {"programoptions"}, {"taglessValue1"}, {"-a"}, {"Aoption1"}, {"-b"}, {"value1"}, {"value2"} };
        Cli po(argc, argv);
        ValueOption option(&po, "-a", "--alpha", "Option A");
        MultiOption multiOption(&po, "-b", "--bravo", "Option B");
        TaglessOption taglessOption(&po, 1, "Option C");
        po.parse();

        ExpectOptionExistsWithValue(taglessOption, "taglessValue1");
        ExpectOptionExistsWithValue(option, "Aoption1");
        ExpectOptionExistsWithValues(multiOption, { "value1", "value2" });
    }
    {
        int argc = 7;
        const char* argv[7]{ {"programoptions"}, {"-a"}, {"Aoption2"}, {"taglessValue2"}, {"-b"}, {"value3"}, {"value4"} };
        Cli po(argc, argv);
        ValueOption option(&po, "-a", "--alpha", "Option A");
        MultiOption multiOption(&po, "-b", "--bravo", "Option B");
        TaglessOption taglessOption(&po, 1, "Option C");
        po.parse();

        ExpectOptionExistsWithValue(taglessOption, "taglessValue2");
        ExpectOptionExistsWithValue(option, "Aoption2");
        ExpectOptionExistsWithValues(multiOption, { "value3", "value4" });
    }
    {
        int argc = 7;
        const char* argv[7]{ {"programoptions"}, {"-a"}, {"Aoption3"}, {"-b"}, {"value5"}, {"value6"}, {"taglessValue3"} };
        Cli po(argc, argv);
        ValueOption option(&po, "-a", "--alpha", "Option A");
        MultiOption multiOption(&po, "-b", "--bravo", "Option B", "", false, 2);
        TaglessOption taglessOption(&po, 1, "Option C");
        po.parse();

        ExpectOptionExistsWithValue(taglessOption, "taglessValue3");
        ExpectOptionExistsWithValue(option, "Aoption3");
        ExpectOptionExistsWithValues(multiOption, { "value5", "value6" });
    }
}

TEST_F(ProgramOptionsTest, either_mandatory_runs_normally_when_one_mandatory_option_is_provided)
{
    int argc = 7;
    const char* argv[7]{ {"programoptions"}, {"-c"}, {"Aoption"}, {"-c"}, {"Boption"}, {"-c"}, {"Coption"} };

    {
        Cli po{ argc, argv };
        po.add("-a", "--alpha", "Option A");
        po.add("-b", "--bravo", "Option B");
        po.add("-c", "--charlie", "Option C");
        po.add("-d", "--delta", "Option D");

        auto& eithers = po.eitherMandatory("-a", "-b", "-c");

        po.parse();

        EXPECT_FALSE(po.option("-b").exists());
        EXPECT_FALSE(po.option("-a").exists());
        ExpectOptionExistsWithValues(po, "-c", { "Aoption", "Boption", "Coption" });

        EXPECT_EQ(&po.option("-c"), eithers.satisfiedOption());
    }
    {
        Cli po{ argc, argv };
        ValueOption optiona(&po, "-a", "--alpha", "Option A");
        ValueOption optionb(&po, "-b", "--bravo", "Option B");
        ValueOption optionc(&po, "-c", "--charlie", "Option C");
        ValueOption optiond(&po, "-d", "--delta", "Option D");

        EitherMandatory eithers(&po, optiona, optionb, optionc);

        po.parse();

        EXPECT_FALSE(optiona.exists());
        EXPECT_FALSE(optionb.exists());
        ExpectOptionExistsWithValues(optionc, { "Aoption", "Boption", "Coption" });

        EXPECT_EQ(&optionc, eithers.satisfiedOption());
    }
}

TEST_F(ProgramOptionsTest, either_mandatory_exits_if_none_of_the_parameters_given)
{
    int argc = 3;
    const char* argv[3]{ {"programoptions"}, {"-c"}, {"Coption"} };
    Cli po{ argc, argv };
    ValueOption optiona(&po, "-a", "--alpha", "Option A");
    ValueOption optionb(&po, "-b", "--bravo", "Option B");
    ValueOption optionc(&po, "-c", "--charlie", "Option C");

    EitherMandatory eithers(&po, optiona, optionb);

    EXPECT_EXIT(po.parse(), testing::ExitedWithCode(1), "");
}

TEST_F(ProgramOptionsTest, either_mandatory_exits_if_all_of_the_parameters_given)
{
    int argc = 7;
    const char* argv[7]{ {"programoptions"}, {"-a"}, {"val"}, {"-b"}, {"val"}, {"-c"}, {"val"} };
    Cli po{ argc, argv };
    ValueOption optiona(&po, "-a", "--alpha", "Option A");
    ValueOption optionb(&po, "-b", "--bravo", "Option B");
    ValueOption optionc(&po, "-c", "--charlie", "Option C");

    EitherMandatory eithers(&po, optiona, optionb, optionc);

    EXPECT_EXIT(po.parse(), testing::ExitedWithCode(1), "");
}

TEST_F(ProgramOptionsTest, either_mandatory_exits_if_one_of_the_parameters_given_but_another_mandatory_not)
{
    int argc = 7;
    const char* argv[7]{ {"programoptions"}, {"-a"}, {"Aoption"}, {"-a"}, {"Boption"}, {"-a"}, {"Coption"} };
    Cli po{ argc, argv };
    ValueOption optiona(&po, "-a", "--alpha", "Option A");
    ValueOption optionb(&po, "-b", "--bravo", "Option B");
    ValueOption optionc(&po, "-c", "--charlie", "Option C", "", true);

    EitherMandatory eithers(&po, optiona, optionb);

    EXPECT_EXIT(po.parse(), testing::ExitedWithCode(1), "");
}

TEST_F(ProgramOptionsTest, either_mandatory_does_not_exit_when_user_input_empty_for_one) 
{
    int argc = 1;
    const char* argv[] = { "programoptions" };
    std::stringstream str("\ninput4 input5 input6\n");
    Cli po(argc, argv);
    auto& a = po.add("-a", "--alpha", "Option A");
    auto& b = po.add("-b", "--bravo", "Option B");
    EitherMandatory eitherMandatory(&po, a, b);

    po.changeIO(&std::cout, &str);
    po.userInputRequired();
    po.parse();

    ExpectOptionExistsWithValues(b, { "input4 input5 input6"});
    EXPECT_FALSE(a.exists());
}

TEST_F(ProgramOptionsTest, either_mandatory_exits_when_user_input_empty_for_both) 
{
    int argc = 1;
    const char* argv[] = { "programoptions" };
    std::stringstream str("\n\n");
    Cli po(argc, argv);
    auto& a = po.add("-a", "--alpha", "Option A");
    auto& b = po.add("-b", "--bravo", "Option B");
    EitherMandatory eitherMandatory(&po, a, b);

    po.changeIO(&std::cout, &str);
    po.userInputRequired();

    EXPECT_EXIT(po.parse(), testing::ExitedWithCode(1), "");
}

TEST_F(ProgramOptionsTest, constrained_string_values_value)
{
    int argc = 9;
    const char* argv[9]{ {"programoptions"}, {"-a"}, {"Aoption"}, {"-a"}, {"Boption"}, {"-a"}, {"Coption"}, {"-b"}, {"Boption"} };
    Cli po{ argc, argv };
    po.add("-a", "--alpha", "Option A");
    auto& b = po.add("-b").constrain({ "Boption" });
    po.constraint("-a", { "Aoption", "Boption", "Coption" });

    po.parse();

    ExpectOptionExistsWithValues(po.option("-a"), { "Aoption", "Boption", "Coption" });
    ExpectOptionExistsWithValues(b, {"Boption"});
}

TEST_F(ProgramOptionsTest, constrained_string_values_tagless)
{
    int argc = 4;
    const char* argv[4]{ {"programoptions"}, {"Aoption"}, {"Boption"}, {"Coption"} };
    Cli po{ argc, argv };

    StringConstraint constraint(po.add(3), { "Aoption", "Boption", "Coption" });

    po.parse();

    ExpectOptionExistsWithValues(po.option("0"), {"Aoption", "Boption", "Coption"});
}

TEST_F(ProgramOptionsTest, constrained_string_values_not_found_exits_value)
{
    int argc = 7;
    const char* argv[7]{ {"programoptions"}, {"-a"}, {"Aoption"}, {"-a"}, {"Boption"}, {"-a"}, {"Coption"} };
    Cli po{ argc, argv };
    ValueOption optiona(&po, "-a", "--alpha", "Option A");

    StringConstraint constraint(optiona, { "aoption", "boption", "coption"});

    EXPECT_EXIT(po.parse(), testing::ExitedWithCode(1), "");
}

TEST_F(ProgramOptionsTest, constrained_string_values_not_found_exits_tagless)
{
    int argc = 4;
    const char* argv[4]{ {"programoptions"}, {"Aoption"}, {"Boption"}, {"Coption"} };
    Cli po{ argc, argv };
    TaglessOption optiona(&po, 3);

    StringConstraint constraint(optiona, { "aoption", "boption", "coption" });

    EXPECT_EXIT(po.parse(), testing::ExitedWithCode(1), "");
}

TEST_F(ProgramOptionsTest, constrained_min_max_values_value)
{
    int argc = 9;
    const char* argv[9]{ {"programoptions"}, {"-a"}, {"0.1"}, {"-a"}, {"1.161782354"}, {"-a"}, {"1.941287457"} ,{"-b"}, {"1.941287457"} };
    Cli po{ argc, argv };
    po.add("-a", "--alpha", "Option A");
    auto& b = po.add("-b").constrain<double>({ 0.00001, 1.95 });
    po.constraint<double>("-a", { 0.00001, 1.95 });

    po.parse();

    ExpectOptionExistsWithValues(po.option("-a"), {"0.1", "1.161782354", "1.941287457"});
    ExpectOptionExistsWithValues(b, {"1.941287457"});
}

TEST_F(ProgramOptionsTest, constrained_min_max_values_tagless)
{
    int argc = 4;
    const char* argv[4]{ {"programoptions"}, {"0.1"}, {"1.161782354"}, {"1.941287457"} };
    Cli po{ argc, argv };
    TaglessOption optiona(&po, 3);

    MinMaxConstraint<double> constraint(optiona, { 0.00001, 1.95 });
    po.parse();

    ExpectOptionExistsWithValues(optiona, { "0.1", "1.161782354", "1.941287457" });
}

TEST_F(ProgramOptionsTest, constrained_min_max_values_exits_on_invalid_conversion)
{
    int argc = 3;
    const char* argv[3]{ {"programoptions"}, {"-a"}, {"Aoption"}};
    Cli po{ argc, argv };
    ValueOption optiona(&po, "-a", "--alpha", "Option A");

    MinMaxConstraint<double> constraint(optiona, { 0.00001, 1.95 });

    EXPECT_EXIT(po.parse(), testing::ExitedWithCode(1), "");
}

TEST_F(ProgramOptionsTest, constrained_under_min_value_exits)
{
    int argc = 3;
    const char* argv[3]{ {"programoptions"}, {"-a"}, {"0"} };
    Cli po{ argc, argv };
    ValueOption optiona(&po, "-a", "--alpha", "Option A");

    MinMaxConstraint<double> constraint(optiona, { 0.000011, 1 });

    EXPECT_EXIT(po.parse(), testing::ExitedWithCode(1), "");
}

TEST_F(ProgramOptionsTest, constrained_under_max_value_exits)
{
    int argc = 3;
    const char* argv[3]{ {"programoptions"}, {"-a"}, {"1.00000001"}};
    Cli po{ argc, argv };
    ValueOption optiona(&po, "-a", "--alpha", "Option A");

    MinMaxConstraint<double> constraint(optiona, { 0.000011, 1 });

    EXPECT_EXIT(po.parse(), testing::ExitedWithCode(1), "");
}

TEST_F(ProgramOptionsTest, constrained_value_exits_on_incorrect_user_input)
{
    int argc = 1;
    const char* argv[1]{ {"programoptions"}};
    std::stringstream str("input4 input5 input6\n");
    Cli po{ argc, argv };

    ValueOption optiona(&po, "-a", "--alpha", "Option A");
    StringConstraint constraint(optiona, { "aoption", "boption", "coption" });
    po.mandatory("-a");
    po.changeIO(&std::cout, &str);
    po.userInputRequired();

    EXPECT_EXIT(po.parse(), testing::ExitedWithCode(1), "");
}

TEST_F(ProgramOptionsTest, multiple_constrained_value_exits_when_second_not_satisfied)
{
    int argc = 3;
    const char* argv[3]{ {"programoptions"}, {"-a"}, {"1.00000001"} };
    Cli po{ argc, argv };
    ValueOption optiona(&po, "-a", "--alpha", "Option A");

    MinMaxConstraint<double> minMax(optiona, { 0, 1.75 });
    FunctionConstraint divisibleBy2(optiona, [](const Option& option) -> bool {
        if (std::fmod(option.valueAs<double>(), 2) != 0)
            return false;
        return true;
        }, "value must be divisible by 2");

    EXPECT_EXIT(po.parse(), testing::ExitedWithCode(1), "");
}

TEST_F(ProgramOptionsTest, custom_constraint_exits)
{
    class CustomConstraint
        : public Constraint
    {
    public:
        CustomConstraint(Option& option)
            : Constraint(option)
        {}

        virtual bool satisfied() const override
        {
            if (isupper(option.value()[0]))
                return true;
            return false;
        };
        virtual std::string what() const override
        {
            return "value to start with a capital letter ";
        };
    };
    int argc = 3;
    const char* argv[3]{ {"programoptions"}, {"-a"}, {"aoption"} };
    Cli po{ argc, argv };
    ValueOption optiona(&po, "-a", "--alpha", "Option A");

    CustomConstraint constraintB(po.add("-b", "--bravo", "Option B"));
    CustomConstraint constraint(optiona);

    EXPECT_EXIT(po.parse(), testing::ExitedWithCode(1), "");
}

TEST_F(ProgramOptionsTest, function_constraint_exits_when_not_satisfied)
{
    int argc = 5;
    const char* argv[5]{ {"programoptions"}, {"-a"}, {"abcd"}, {"ab"}, {"abc"} };
    {
        Cli po{ argc, argv };
        MultiOption optiona(&po, "-a", "--alpha", "Option A");

        FunctionConstraint constraint(optiona, [](const Option& option) -> bool {
            if ("abc" != option.valueAs<std::string>())
                return false;
            return true;
            }, "value must be abc");

        EXPECT_EXIT(po.parse(), testing::ExitedWithCode(1), "");
    }
    {
        Cli po{ argc, argv };
        MultiOption optiona(&po, "-a", "--alpha", "Option A");

        po.constraint("-a", [](const Option& option) -> bool {
            if ("abc" != option.valueAs<std::string>())
                return false;
            return true;
            }, "value must be abc");

        EXPECT_EXIT(po.parse(), testing::ExitedWithCode(1), "");
    }
}

TEST_F(ProgramOptionsTest, conversion_error_exits_automatically_with_constraint)
{
    int argc = 3;
    const char* argv[3]{ {"programoptions"}, {"-a"}, {"abcd"} };
    Cli po{ argc, argv };
    ValueOption optiona(&po, "-a", "--alpha", "Option A");

    MinMaxConstraint<double> constraint(optiona, { 0.000011, 1 });

    EXPECT_EXIT(po.parse(), testing::ExitedWithCode(1), "");
}

TEST_F(ProgramOptionsTest, conversion_error_exits_when_value_is_converted)
{
    int argc = 3;
    const char* argv[3]{ {"programoptions"}, {"-a"}, {"abcd"} };
    Cli po{ argc, argv };
    ValueOption optiona(&po, "-a", "--alpha", "Option A");
    po.parse();

    EXPECT_EXIT(optiona.valueAs<double>(), testing::ExitedWithCode(1), "");
    EXPECT_EXIT(optiona.valuesAs<double>(), testing::ExitedWithCode(1), "");
}

TEST_F(ProgramOptionsTest, tagless_throws_if_prioritized) 
{  
    {
        Cli po(argc, argv);
        po.add();
        EXPECT_THROW(po.prioritize("0"), _detail::PrioritizationOptionMismatch);
    }
    {
        Cli po(argc, argv);
        TaglessOption a(&po);

        EXPECT_THROW(a.prioritize(), _detail::PrioritizationOptionMismatch);
    }
}

TEST_F(ProgramOptionsTest, options_print) 
{
    int argc = 1;
    const char* argv[] = { "programoptions" };
    Cli po(argc, argv);
    po.add("-a", "--alpha", "Option A").mandatory();
    po.add("-b", "--bravo", "Option B").mandatory().withMaxValueCount(3);
    po.add("-c", "--charlie", "Option C");
    po.add("-d", "--delta", "Option D");
    po.add("-f", "--foxtrot", "Option F");
    po.add("-e", "--echo", "Option E");
    po.printOptions();
}

TEST_F(ProgramOptionsTest, tagless_options_print) 
{
    int argc = 1;
    const char* argv[] = { "programoptions" };
    Cli po(argc, argv);
    po.add(1, "First set of values").mandatory();
    po.add(2, "Second set of values");
    po.add(3, "Third set of values");
    po.add(4, "Fourth set of values").withMaxValueCount(SIZE_MAX);
    po.printOptions();
}
