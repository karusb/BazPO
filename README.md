# **BazPO - Program Options Argument Parser**

![Version](https://img.shields.io/github/v/release/karusb/BazPO?include_prereleases&style=flat)
![Build Status](https://github.com/karusb/BazPO/actions/workflows/build.yml/badge.svg)
[![Coverage](https://sonarcloud.io/api/project_badges/measure?project=karusb_BazPO&metric=coverage)](https://sonarcloud.io/summary/new_code?id=karusb_BazPO)
[![Maintainability Rating](https://sonarcloud.io/api/project_badges/measure?project=karusb_BazPO&metric=sqale_rating)](https://sonarcloud.io/summary/new_code?id=karusb_BazPO)

BazPO is a flexible C++14 single header program argument parsing library. \
\
BazPO is very easy to use with multiple highly readable syntax that aims to keep your main function simple. \
Apart from other argument parsers BazPO does not enforce static type checking rather types are enforced when used therefore making the option definitions simpler. \
BazPO can simply be included as a header to the program without any linking. \

BEWARE! BazPO is still under development...
**[Download BazPO Here](https://github.com/karusb/BazPO/archive/refs/tags/v0.1.1.zip)**\
*BazPO is licensed under MIT license, so it can be used as is without any restrictions. However, do not remove license and copyright information from the header.*

## **Contents**

- [**BazPO - Program Options Argument Parser**](#bazpo---program-options-argument-parser)
  - [**Contents**](#contents)
  - [**BazPO Features**](#bazpo-features)
  - [**Getting Started**](#getting-started)
    - [Functions / Usage](#functions--usage)
    - [Examples](#examples)
      - [Example (1)](#example-1)
      - [Example (2)](#example-2)
      - [Example (3)](#example-3)
  - [**Options**](#options)
    - [Option Functions](#option-functions)
    - [ValueOption](#valueoption)
    - [MultiOption](#multioption)
    - [TaglessOption](#taglessoption)
    - [FunctionOption/FunctionMultiOption/FunctionTaglessOption](#functionoptionfunctionmultioptionfunctiontaglessoption)
  - [**Customizations**](#customizations)
    - [**Constraints**](#constraints)
      - [StringConstraint](#stringconstraint)
      - [MinMaxConstraint](#minmaxconstraint)
      - [FunctionConstraint](#functionconstraint)
      - [Defining your own constraint](#defining-your-own-constraint)
    - [**MultiConstraints**](#multiconstraints)
      - [EitherMandatory](#eithermandatory)
    - [**Asking For User Input When Mandatory Options Are Not Provided**](#asking-for-user-input-when-mandatory-options-are-not-provided)
    - [**Making Invalid/Expanded Arguments Acceptable**](#making-invalidexpanded-arguments-acceptable)
    - [**Disabling Auto Help**](#disabling-auto-help)
    - [**Option Prioritizing (like -h)**](#option-prioritizing-like--h)

## **BazPO Features**

- Function execution with provided argument values.
- Customizable tags, anything is possible.
- Overridable program exit on invalid arguments.
- Exception handling not needed.
- Stream selection, able to change input/output.
- Ability to ask input from user or get input from a stream.
- Type conversion extensibility.
- On demand value conversion.
- Multiple highly readable usage syntax.

*the usual stuff*

- Automatically generated *pretty* help message.
- Error messages for invalid arguments.

## **Getting Started**

- Download BazPO.hpp source code and add to your project folder
- Include the downloaded header to your main program
- Library will be compiled along with your C++14 program
- **[Download BazPO Here](https://github.com/karusb/BazPO/archive/refs/tags/v0.1.1.zip)**

### Functions / Usage

1. Instantiate BazPO::Cli
2. Add your options
3. Customize your options if needed
4. Call parse
5. Read your values

- **Adding an option**
  - Using add() [`see example 1`](#example-1)
  - Defining the option by yourself [`see example 3`](#example-3)

- **Customizing an option**
  - Refer to [`Customizations`](#customizations) header.
  
- **Reading values**
  - Calling option(tag) with the tag you specified to get the option [`see example 1`](#example-1)
  - Calling value(tag)/values(tag) with the relevant tag to get option values directly [`see example 2`](#example-2)
  - Directly reading from the defined object [`see example 3`](#example-3)
  - Tagless options have an internal tag, which is the order number they are added in the program [`see example 2`](#example-2)

### Examples

#### Example (1)

```c++
#include "BazPO.hpp"

using namespace BazPO;

int main(int argc, const char* argv[])
{
    Cli po(argc, argv);

    po.add("-a", "--alpha", "Option A").mandatory();
    po.add("-b", [&](const Option&) { /* do something */ }, "--bravo", "Option B");

    po.parse();

    auto aoption = po.option("-a");

    std::cout << "EXISTS:" << aoption.exists() << std::endl;
    std::cout << "INT:" << aoption.valueAs<int>() << std::endl;
    std::cout << "BOOL:" << aoption.valueAs<bool>() << std::endl;
}
```

- Outputs

```txt
./myProgram
<-a> is a required parameter

myProgram       
usage: myProgram  <-a> [-b] [-h]
Program Options: 
<-a>       --alpha          Option A
[-b]       --bravo          Option B
[-h]       --help           Prints this help message

./myProgram -a 255
EXISTS:1
INT:255
BOOL:0
```

#### Example (2)

```c++
#include "BazPO.hpp"

using namespace BazPO;

int main(int argc, const char* argv[])
{
    Cli po(argc, argv);
    po.add(5);
    po.add();
    po.parse();

    for(const auto& values : po.option("0").values())
        std::cout << values << std::endl;

    std::cout << po.option("1").value() << std::endl;
}
```

- Outputs
  
```txt
./myProgram input1 input2 input3 input4 input5 input6 input7
Given value -> 'input7' is not expected
myProgram
usage: myProgram  [-h] [(5)]  [(1)]
Program Options:
[-h]       --help          Prints this help message

./myProgram input1 input2 input3 input4 input5 input6
input1
input2
input3
input4
input5
input6
```

#### Example (3)

```c++
#include "BazPO.hpp"

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
```

- Outputs
  
```txt
./myProgram
<-a> is a required parameter
<-a>: True
EXISTS:1
INT:0
BOOL:1
```

## **Options**

### Option Functions

- Option
  - Before Parsing
    - **mandatory()** -> sets the option as mandatory input
    - **withMaxValueCount(size_t)** -> sets a maximum number of accepted values
    - **prioritize()** -> see [`Option prioritizing`](#option-prioritizing-like--h) section.
    - **constrain({"str1", "str2"})** / **constrain<T>(T min, T max)** / **constrain(bool(Option&) isSatisfied, string errorMessage)** -> constrains the option, see [`Constraints`](#constraints) section.
  - After Parsing
    - **exists()** -> option is present in the arguments list
    - **existsCount()** -> returns the number the option is present i.e -a -a -a will return 3
    - **value()** -> returns the raw argument value
    - **valueAs<T>** -> converts the raw argument to the given type
    - **values()** / **valuesAs<T>()** -> same as their value counterpart but returns all provided values
    - **execute()** -> only available with [`Function Options`](#FunctionOptionFunctionMultiOptionFunctionTaglessOption)

### ValueOption

- Basic argument with/without a value or values provided with multiple tags.

Below argument list is valid and only one value is accepted after the tag.

```sh
myprogram -a value1 -a value2 -b
```

### MultiOption

- Use this option if one tag needs multiple inputs after the tag is provided.
- All arguments provided after the tag will be considered values until a valid tag.
- Max value count can be provided to limit the number of values that can be provided.

Below argument list is valid for MultiOption only.

```sh
myprogram -a value1 value2 value3 -a value4 -b
```

### TaglessOption

- Use this option if the program will only use positional arguments.
- Given number of arguments are separated into different options or all into one.
- Provided values will be added to your options in the order you add them.
- Other options cannot be used in conjunction with this option.

```sh
myprogram value1 value2 value3 value4
```

### FunctionOption/FunctionMultiOption/FunctionTaglessOption

- Provided function will be executed if the given tag is provided as an argument with or without a value.
- Other options are already parsed when the function is executed so they can be read in the given function.
- FunctionOption is parsed like [`ValueOption`](#ValueOption).
- FunctionMultiOption is parsed like [`MultiOption`](#MultiOption).
- FunctionTaglessOption is parsed like [`TaglessOption`](#TaglessOption).

## **Customizations**

### **Constraints**

Constraints are used to restrict the values that can be provided for an option.

#### StringConstraint

- Useful when choosing a certain configuration for the program.
- Don't forget, every input is a string :)

**Example (1)**

```c++
    Cli po{ argc, argv };

    StringConstraint constraint(po.add(3).mandatory(), { "Config1", "Config2", "Config3" });

    po.parse();
```

**Example (2)**

```c++
    Cli po{ argc, argv };
    po.add("-a", "--alpha", "Option A").mandatory();
    po.constraint("-a", { "Config1", "Config2", "Config3" });

    po.parse();
```

#### MinMaxConstraint

- When the program expects the provided values within a certain range of values MinMaxConstraint can be used.
- First value of the pair is defined as the minumum value and second pair is defined as the maximum value that a value can take.

**Example (1)**

```c++
    Cli po{ argc, argv };
    ValueOption optiona(&po, "-a", "--alpha", "Option A", "", true);
    MinMaxConstraint<double> constraint(optiona, { 0.00001, 1.95 });
    po.parse();
```

**Example (2)**

```c++
    Cli po{ argc, argv };
    po.add("-a", "--alpha", "Option A").mandatory();
    po.constraint<double>("-a", { 0.00001, 1.95 });

    po.parse();
```

#### FunctionConstraint

- Function constraints can be used whenever a custom check is needed on the provided values.
- Provided function will be called for each value provided for the option it constrains.
- If the function returns false, provided message will be displayed with the value the contraint failed on.
  
**Example**

```c++
    Cli po{ argc, argv };
    MultiOption optiona(&po, "-a", "--alpha", "Option A");

    FunctionConstraint constraint(optiona, [](const Option& option) -> bool {
        if ("abc" != option.valueAs<std::string>())
            return false;
        return true;
        }, "value must be abc");

```

#### Defining your own constraint

- Custom constraints can be added to an option if existing constraints does not meet your needs.
- Before implementing your own constraint, have a look at [`FunctionConstraint`](#FunctionConstraint), which achieves the same functionality.
- Overridden **satisfied()** function is called everytime a value is added.
- Overridden **what()** function should return what was expected, to show user a descriptive message.

**Example**

```c++
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

    Cli po{ argc, argv };
    ValueOption optiona(&po, "-a", "--alpha", "Option A", "", true);

    CustomConstraint constraint(optiona);
    CustomConstraint constraintB(po.add("-b", "--bravo", "Option B"));
    po.parse();

```

### **MultiConstraints**

- MultiConstraints are able to constrain multiple options at once.

#### EitherMandatory

- Either one of the mandatory options are accepted as valid input regardless of the other mandatory options.
- There can only be one option that satisfies EitherMandatory, more than one of these options will not be accepted.
- For example: A program requires either a file input, or a raw data input

**Example (1)**

```c++
    Cli po(argc, argv);
    FunctionOption optiona(&po, "-f",[&](const Option& option) {
        /* do file operations */
        }, "--file", "File input");
    FunctionOption optionb(&po, "-d",[&](const Option& option) {
        /* do data operations */
        }, "--data", "Raw data input");

    EitherMandatory eithers(&po, optiona, optionb);
```

**Example (2)**

```c++
    Cli po(argc, argv);
    ValueOption optiona(&po, "-f", "--file", "File input");
    ValueOption optionb(&po, "-d", "--data", "Raw data input");

    EitherMandatory eithers(&po, optiona, optionb);
    
    po.parse();
    if(&optiona == eithers.satisfiedOption())
    {
        /* do file operations */
    }
    else
    {
        /* do data operations */
    }
```

**Example (3)**

```c++
    Cli po(argc, argv);
    auto& file = po.add("-f", "--file", "File input");
    po.add("-d", "--data", "Raw data input");
    auto& eithers = po.eitherMandatory("-f", "-d");

    po.parse();

    if(&file == eithers.satisfiedOption())
    {
        /* do file operations */
    }
    else
    {
        /* do data operations */
    }
```

### **Asking For User Input When Mandatory Options Are Not Provided**

- Call **userInputRequired()** in your program, user will be asked to input values for your mandatory values.
- For inputs, input stream could be changed to your choosen stream via **changeIO()**

**Example**

```c++
    Cli po(argc, argv);
    /* your options */ 
    po.userInputRequired();
    po.parse();
```

### **Making Invalid/Expanded Arguments Acceptable**

- When used, valid options will be parsed and program won't exit if an unknown argument is provided.

**Example**

```c++
    Cli po(argc, argv);
    po.add(3);
    po.unexpectedArgumentsAcceptable();
    po.parse();
```

### **Disabling Auto Help**

- "-h" option can simply be disabled by defining the value below before the header definition.

```c++
#define BazPO_DISABLE_AUTO_HELP_MESSAGE
```

### **Option Prioritizing (like -h)**

- Prioritized options are parsed first.
- If a prioritized option exists rest of the options won't be parsed even if they are invalid.
- If a prioritized option contains a function it will be executed if it exists regardless of the other provided arguments.
- For example: Providing "-h" will always print the help message as long as it's present in the argument list.
- Example: Adding -a tag as a prioritized option

**Example (1)**

```c++
    Cli po{ argc, argv };
    ValueOption optiona(&po, "-a");
    optiona.prioritize();
    po.parse();
```

**Example (2)**

```c++
    Cli po{ argc, argv };
    po.add("-a").prioritize();
    po.parse();
```
