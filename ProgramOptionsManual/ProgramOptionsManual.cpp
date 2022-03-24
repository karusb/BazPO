// ProgramOptionsManual.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "../ProgramOptions/BazPO.hpp"
int main(int argc,char* argv[])
{
    BazPO po(argc, argv);
    po.Add("-a", "--alpha", "Option A",true);
    po.Add("-b", "--bravo", "Option B");
    po.PrintOptions();
    po.ParseArguments();
    auto aoption = po.GetOption("-a");
    auto boption = po.GetOption("-b");

    std::cout << "EXISTS:" << aoption.Exists << std::endl;
    std::cout << "INT:" << po.GetValueAs<int>("-a") << std::endl;
    std::cout << "BOOL:" << po.GetValueAs<bool>("-a") << std::endl;
    std::cout << aoption.Value;
}
