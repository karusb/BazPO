// ProgramOptionsManual.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "../include/BazPO.hpp"
using namespace BazPO;


int main(int argc, const char* argv[])
{
    Cli po(argc, argv);

    ValueOption optionA(&po, "-a", "--alpha", "Option A", "", true);
    FunctionOption optionB(&po, "-b", [&](const Option& option) {
        /* do something */
        }, "--bravo", "Option B");

    po.userInputRequired();

    po.parse();

    std::cout << "EXISTS:" << optionA.exists() << std::endl;
    std::cout << "INT:" << optionA.valueAs<int>() << std::endl;
    std::cout << "BOOL:" << optionA.valueAs<bool>() << std::endl;
}
