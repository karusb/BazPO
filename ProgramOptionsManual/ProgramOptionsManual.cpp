// ProgramOptionsManual.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "../ProgramOptions/BazPO.hpp"
int main(int argc,const char* argv[])
{
    BazPO::Cli po(argc, argv, "asdg asghhm asg hgasoasht asfgally");
    po.Add("-a", "--ammonium", "fdgj sds ajsdjsnium jsdnt jsd jsgle berry", true);
    po.Add("-berk", "--berkytonics", "Simple jumble james");
    po.Add("-gb", "--ginB", "fdgj sds ajsdjsnium jsdnt jsd jsgle berryfdgj sds ajsdjsnium jsdnt jsd jsgle berryfdgj sds ajsdjsnium jsdnt jsd jsgle berryfdgj sds ajsdjsnium js");
    po.PrintOptions();
    po.UserInputRequiredForAbsentMandatoryOptions();
    po.ParseArguments();
    auto aoption = po.GetOption("-a");

    std::cout << "EXISTS:" << aoption.exists() << std::endl;
    std::cout << "INT:" << aoption.value_as<int>() << std::endl;
    std::cout << "BOOL:" << aoption.value_bool() << std::endl;
    std::cout << aoption.value();
    while (1);
}
