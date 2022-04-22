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
    po.add("-a", "--alpha", "Option A");
    po.parse();

    auto& a = po.option("-a");

    EXPECT_EQ(true, a.exists());

    EXPECT_EQ(std::string("Aoption"), a.value());
}

TEST_F(ProgramOptionsTest, get_second_option_successful)
{
    int argc = 3;
    const char* argv[3]{ {"programoptions"}, {"-a"}, {"Aoption"} };
    Cli po{ argc, argv };
    po.add("-a", "--alpha", "Option A");
    po.parse();

    auto& a = po.option("--alpha");

    EXPECT_EQ(true, a.exists());

    EXPECT_EQ(std::string("Aoption"), a.value());
}

TEST_F(ProgramOptionsTest, int_conversion_succesful)
{
    int argc = 3;
    const char* argv[3]{ {"programoptions"}, {"-d"}, {"15"} };
    Cli po{ argc, argv };
    po.add("-d", "--delta", "Option D");
    po.parse();

    auto& a = po.option("-d");

    EXPECT_EQ(true, a.exists());
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

    EXPECT_EQ(true, a.exists());
    EXPECT_EQ(15.2156, a.valueAs<double>());
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

    EXPECT_EQ(true, a.exists());
    EXPECT_EQ(true, a.valueBool());

    EXPECT_EQ(true, b.exists());
    EXPECT_EQ(false, b.valueBool());

    EXPECT_EQ(true, c.exists());
    EXPECT_EQ(false, c.valueBool());
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

    auto& a = po.option("-a");
    auto& b = po.option("-b");
    auto& c = po.option("-c");

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
    po.add("-a", "--echo");
    po.add("-b", "--bravo");
    po.add("-c", "--charlie");
    po.parse();

    auto& a = po.option("-a");
    auto& b = po.option("-b");
    auto& c = po.option("-c");

    EXPECT_EQ(true, a.exists());
    EXPECT_EQ(2, a.existsCount());
    EXPECT_EQ(true, b.exists());
    EXPECT_EQ(1, b.existsCount());
    EXPECT_EQ(true, c.exists());
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

TEST_F(ProgramOptionsTest, DISABLED_program_exits_when_help_is_provided_with_mandatory_option)
{
    int argc = 2;
    const char* argv[2]{ {"programoptions"}, {"-h"} };
    Cli po{ argc, argv };
    po.add("-a", "--alpha", "Option A", true);

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

    ASSERT_DEATH(po.parse(), "");
}

TEST_F(ProgramOptionsTest, program_exits_when_unknown_arguments_are_given_in_tagless_mode)
{
    Cli po{ argc, argv };
    po.add();

    ASSERT_DEATH(po.parse(), "");
}

TEST_F(ProgramOptionsTest, program_exits_when_unknown_arguments_are_given_with_legit_ones)
{
    Cli po{ argc, argv };
    po.add("-a", "--alpha", "Option A");

    ASSERT_DEATH(po.parse(), "");
}

TEST_F(ProgramOptionsTest, program_works_when_unknown_arguments_are_given_with_legit_ones_and_UnexpectedArgumentsAcceptable)
{
    Cli po{ argc, argv };
    po.add("-a", "--alpha", "Option A");

    po.unexpectedArgumentsAcceptable();
    po.parse();

    auto& a = po.option("-a");
    EXPECT_EQ(true, a.exists());
    EXPECT_EQ(1, a.existsCount());
    EXPECT_EQ(std::string("Aoption"), a.value());
    EXPECT_EQ(1, a.values().size());
}

TEST_F(ProgramOptionsTest, multi_options_successful)
{
    int argc = 6;
    const char* argv[6]{ {"programoptions"}, {"-a"}, {"value1"}, {"value2"}, {"value3"}, {"value4"} };
    Cli po{ argc, argv };
    po.add("-a", "--echo", "", false, true);
    po.add("-b", "--bravo");
    po.add("-c", "--charlie");
    po.parse();

    auto& a = po.option("-a");
    auto& b = po.option("-b");
    auto& c = po.option("-c");

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
    po.add("-a", "--echo", "", false, true, 3);

    ASSERT_DEATH(po.parse(), "");
}

TEST_F(ProgramOptionsTest, multi_options_successful_with_limited_count)
{
    int argc = 6;
    const char* argv[6]{ {"programoptions"}, {"-a"}, {"value1"}, {"value2"}, {"value3"}, {"value4"} };
    Cli po{ argc, argv };
    po.add("-a", "--echo", "", false, true, 3);
    po.unexpectedArgumentsAcceptable();
    po.parse();

    auto& a = po.option("-a");
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
    po.add("-a", "--echo", "", false, true);
    po.add("-b", "--bravo");
    po.add("-c", "--charlie");
    po.parse();

    auto& a = po.option("-a");
    auto& b = po.option("-b");
    auto& c = po.option("-c");

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
    po.add("-a", "--echo", "", false, true);
    po.parse();

    auto& a = po.option("-a");

    EXPECT_EQ(true, a.exists());

    EXPECT_EQ(4, a.values().size());
    EXPECT_EQ(std::string("value1"), a.values()[0]);
    EXPECT_EQ(std::string("value2"), a.values()[1]);
    EXPECT_EQ(std::string("-d"), a.values()[2]);
    EXPECT_EQ(std::string("value4"), a.values()[3]);
}

TEST_F(ProgramOptionsTest, value_option_with_unknown_tags_successful_with_acceptable_unknown_arguments)
{
    int argc = 6;
    const char* argv[6]{ {"programoptions"}, {"-a"}, {"value1"}, {"value2"}, {"-d"}, {"value4"} };
    Cli po{ argc, argv };
    po.add("-a", "--echo", "", false);
    po.unexpectedArgumentsAcceptable();
    po.parse();

    auto& a = po.option("-a");

    EXPECT_EQ(true, a.exists());

    EXPECT_EQ(1, a.values().size());
    EXPECT_EQ(std::string("value1"), a.values()[0]);
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

    auto& a = po.option("-a");

    EXPECT_EQ(true, a.exists());

    EXPECT_EQ(2, a.values().size());
    EXPECT_EQ(std::string("value1"), a.values()[0]);
    EXPECT_EQ(std::string("value4"), a.values()[1]);
}

TEST_F(ProgramOptionsTest, mandatory_option_exits_when_not_provided) {
    int argc = 1;
    const char* argv[] = { "programoptions" };
    Cli po(argc, argv);
    po.add("-a", "--alpha", "Option A", "", true);

    ASSERT_DEATH(po.parse(), "");
}

TEST_F(ProgramOptionsTest, mandatory_option_exits_when_not_provided_with_multiple_input) {
    Cli po(argc, argv);
    po.add("-z", "--zeta", "Zeta", "", true);

    ASSERT_DEATH(po.parse(), "");
}

TEST_F(ProgramOptionsTest, mandatory_option_asked_when_not_provided) {
    int argc = 1;
    const char* argv[] = { "programoptions" };
    std::stringstream str("input");
    Cli po(argc, argv);
    po.userInputRequired();
    po.add("-a", "--alpha", "Option A", true);
    po.changeIO(&std::cout, &str);

    po.parse();

    auto& a = po.option("-a");
    EXPECT_EQ(true, a.exists());
    EXPECT_EQ(std::string("input"), a.value());
}

TEST_F(ProgramOptionsTest, multi_line_input_parsed_as_whole_line) {
    int argc = 1;
    const char* argv[] = { "programoptions" };
    std::stringstream str("input input2 input3");
    Cli po(argc, argv);
    po.userInputRequired();
    po.add("-a", "--alpha", "Option A", true);
    po.changeIO(&std::cout, &str);

    po.parse();

    auto& a = po.option("-a");
    EXPECT_EQ(true, a.exists());
    EXPECT_EQ(std::string("input input2 input3"), a.value());
}

TEST_F(ProgramOptionsTest, multi_line_double_parsed_correctly) {
    int argc = 1;
    const char* argv[] = { "programoptions" };
    std::stringstream str("15.87396509125677 \r\n");
    Cli po(argc, argv);
    po.userInputRequired();
    po.add("-a", "--alpha", "Option A", true);
    po.changeIO(&std::cout, &str);

    po.parse();

    auto& a = po.option("-a");
    EXPECT_EQ(true, a.exists());
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
    auto& a = po.option("-a");
    EXPECT_EQ(true, a.exists());
    EXPECT_EQ(std::string("input input2 input3"), a.value());
    EXPECT_EQ(std::string("input input2 input3"), a.values()[0]);

    po.askInput("-a");
    EXPECT_EQ(true, a.exists());
    EXPECT_EQ(std::string("input4 input5 input6"), a.value());
    EXPECT_EQ(std::string("input4 input5 input6"), a.values()[1]);
}

TEST_F(ProgramOptionsTest, function_option_executes_successfully) {
    bool executed = false;
    int argc = 2;
    const char* argv[] = { "programoptions", "-a"};
    Cli po(argc, argv);
    po.add("-a", [&](const Option&) { executed = true; }, "--alpha", "Option A");

    po.parse();
    EXPECT_EQ(true, executed);
}

TEST_F(ProgramOptionsTest, function_option_not_executed_when_not_provided) {
    bool executed = false;
    int argc = 1;
    const char* argv[] = { "programoptions"};
    Cli po(argc, argv);
    po.add("-a", [&](const Option&) { executed = true; }, "--alpha", "Option A");

    po.parse();
    EXPECT_EQ(false, executed);
}

TEST_F(ProgramOptionsTest, function_option_contents_correct) {
    bool executed = false;
    int argc = 5;
    const char* argv[5]{ {"programoptions"}, {"-a"}, {"value1"}, {"-a"}, {"value4"} };
    Cli po(argc, argv);
    po.add("-a", [&](const Option& option) {
        EXPECT_EQ(2, option.values().size());
        EXPECT_EQ(std::string("value1"), option.values()[0]);
        EXPECT_EQ(std::string("value4"), option.values()[1]);
        executed = true;
        }, "--alpha", "Option A", true);

    po.parse();
    EXPECT_EQ(true, executed);
}

TEST_F(ProgramOptionsTest, function_multi_options_successful)
{
    bool executed = false;
    int argc = 6;
    const char* argv[6]{ {"programoptions"}, {"-a"}, {"value1"}, {"value2"}, {"-a"}, {"value4"} };
    Cli po{ argc, argv };
    po.add("-a", [&](const Option& option) {
        EXPECT_EQ(3, option.values().size());
        EXPECT_EQ(std::string("value1"), option.values()[0]);
        EXPECT_EQ(std::string("value2"), option.values()[1]);
        EXPECT_EQ(std::string("value4"), option.values()[2]);
        executed = true;
        }, "", "", false, true);

    po.parse();

    auto& a = po.option("-a");

    EXPECT_EQ(true, a.exists());

    EXPECT_EQ(3, a.values().size());
    EXPECT_EQ(std::string("value1"), a.values()[0]);
    EXPECT_EQ(std::string("value2"), a.values()[1]);
    EXPECT_EQ(std::string("value4"), a.values()[2]);
}

TEST_F(ProgramOptionsTest, function_tagless_option_successful)
{
    bool executed = false;
    int argc = 5;
    const char* argv[5]{ {"programoptions"}, {"value1"}, {"value2"}, {"value3"}, {"value4"} };
    Cli po{ argc, argv };
    po.add([&](const Option& option) {
        EXPECT_EQ(4, option.values().size());
        EXPECT_EQ(std::string("value1"), option.values()[0]);
        EXPECT_EQ(std::string("value2"), option.values()[1]);
        EXPECT_EQ(std::string("value3"), option.values()[2]);
        EXPECT_EQ(std::string("value4"), option.values()[3]);
        executed = true;
        }, 4);

    po.parse();

    auto& a = po.option("0");
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

    po.parse();

    EXPECT_EQ(true, option.exists());
    EXPECT_EQ(std::string("Aoption"), option.value());
}

TEST_F(ProgramOptionsTest, multi_option_with_reference_succesful) {
    int argc = 6;
    const char* argv[6]{ {"programoptions"}, {"-a"}, {"value1"}, {"value2"}, {"-a"}, {"value4"} };
    Cli po(argc, argv);
    MultiOption option(&po, "-a", "--alpha", "Option A", false);

    po.parse();

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

    po.parse();
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

    po.parse();
    EXPECT_EQ(true, executed);
    EXPECT_EQ(true, option.exists());
    EXPECT_EQ(3, option.values().size());
    EXPECT_EQ(std::string("value1"), option.values()[0]);
    EXPECT_EQ(std::string("value2"), option.values()[1]);
    EXPECT_EQ(std::string("value4"), option.values()[2]);
}

TEST_F(ProgramOptionsTest, function_tagless_options_with_reference_successful)
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

    po.parse();

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
    po.add(4);

    po.parse();

    auto& a = po.option("0");

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
    po.add();
    po.add();
    po.add();
    po.add();

    po.parse();

    auto& a = po.option("0");
    auto& b = po.option("1");
    auto& c = po.option("2");
    auto& d = po.option("3");

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

    po.parse();

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

    po.parse();

    EXPECT_EQ(true, a.exists());

    EXPECT_EQ(4, a.values().size());
    EXPECT_EQ(std::string("value1"), a.values()[0]);
    EXPECT_EQ(std::string("value2"), a.values()[1]);
    EXPECT_EQ(std::string("value3"), a.values()[2]);
    EXPECT_EQ(std::string("value4"), a.values()[3]);
}

TEST_F(ProgramOptionsTest, tagless_then_other_mismatch_throws) {
    Cli po(argc, argv);
    po.add();

    EXPECT_ANY_THROW(po.add("-a"));
}

TEST_F(ProgramOptionsTest, other_then_tagless_mismatch_throws) {
    Cli po(argc, argv);
    po.add("-a");

    EXPECT_ANY_THROW(po.add());
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
    po.add("-a", "--alpha", "Option A");
    po.add("-b", "--bravo", "Option B");
    po.add("-c", "--charlie", "Option C");
    po.add("-d", "--delta", "Option D");
    po.add("-f", "--foxtrot", "Option F");
    po.add("-e", "--echo", "Option E");
    po.printOptions();
}

TEST_F(ProgramOptionsTest, tagless_options_print) {
    int argc = 1;
    const char* argv[] = { "programoptions" };
    Cli po(argc, argv);
    po.add(4, "My values");
    po.printOptions();
}
