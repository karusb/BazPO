// ProgramOptionsManual.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "../ProgramOptions/BazPO.hpp"
int main(int argc,const char* argv[])
{
    BazPO::Cli po(argc, argv);
    po.Add("-a", "--alpha", "Option A", true);
    po.Add("-b", "--bravo", "Option B");
    po.PrintOptions();
    po.UserInputRequiredForAbsentMandatoryOptions();
    po.ParseArguments();
    auto aoption = po.GetOption("-a");
    auto boption = po.GetOption("-b");
    std::cout << "EXISTS:" << aoption.Exists << std::endl;
    std::cout << "INT:" << aoption.int_value() << std::endl;
    std::cout << "BOOL:" << aoption.bool_value() << std::endl;
    std::cout << aoption.value();

    while (1);
}
