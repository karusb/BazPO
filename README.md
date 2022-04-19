# BazPO - Program Options Argument Parser

BazPO takes away all the hassle handling program arguments, all you need is a line per option!

## Features

- Developer friendly and easy to use with advanced features
- Automatic value conversions.
- Automatic function execution with provided argument values.
- Stream selection, to be able to change text input/output.
- Ability to ask input from user or get input from stream
- Help option automatically added to your program with all your added options and descriptions. Can be disabled.
- Automatic program exit on invalid arguments when specified.

## Getting Started

- Download BazPO.hpp source code and add to your project folder
- Include the downloaded header to your main program
  
**Example (1):**

```c++
#include "BazPO.hpp"

using namespace BazPO;

int main(int argc,const char* argv[])
{
    Cli po(argc, argv);

    po.Add("-a", "--alpha", "Option A", true);
    po.Add("-b", [&](const Option&) { /* do something */ }, "--bravo", "Option B");

    po.UserInputRequiredForAbsentMandatoryOptions();

    po.ParseArguments();

    auto aoption = po.GetOption("-a");

    std::cout << "EXISTS:" << aoption.exists() << std::endl;
    std::cout << "INT:" << aoption.value_as<int>() << std::endl;
    std::cout << "BOOL:" << aoption.value_bool() << std::endl;
}
```

**Example (2):**

```c++
#include "BazPO.hpp"

using namespace BazPO;

int main(int argc,const char* argv[])
{
    Cli po(argc, argv);

    ValueOption optionA("-a", "--alpha", "Option A", false, &po);
    FunctionOption optionB("-b", [&](const Option& option) {
            /* do something */
        }, "--bravo", "Option B", false, &po);

    po.UserInputRequiredForAbsentMandatoryOptions();

    po.ParseArguments();
    
    std::cout << "EXISTS:" << optionA.exists() << std::endl;
    std::cout << "INT:" << optionA.value_as<int>() << std::endl;
    std::cout << "BOOL:" << optionA.value_bool() << std::endl;
}
```

## Options

### **ValueOption**

- Basic argument with/without a value or values provided with multiple tags.

Below argument list is valid and only one value is accepted.

```sh
myprogram -a value1 -a value2
```

### **MultiOption**

- All arguments provided after the tag will be considered values until a valid tag.

Below argument list is valid for MultiOption only.

```sh
myprogram -a value1 value2 value3 -a value4 -b
```

### **FunctionOption**

- Provided function will be executed if the given tag is provided as an argument with or without a value.
- FunctionOption is parsed like MultiOption
  
### **TaglessOption**

- A tag isnt needed, however other options cannot be used in conjunction with this option.
- Given number of arguments are separated into different options or all into one.
  
```sh
myprogram value1 value2 value3 value4
```
