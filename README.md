# **BazPO - Program Options Argument Parser**

*BazPO takes away all the hassle handling program arguments, all you need is a line per option!*

## **Features**

- Developer friendly and easy to use with advanced features.
- Automatic value conversions.
- Automatic function execution with provided argument values.
- Stream selection, to be able to change text input/output.
- Ability to ask input from user or get input from stream.
- Help option automatically added to your program with all your added options and descriptions. Can be disabled.
- Automatic program exit on invalid arguments when specified.
- Type conversion extensibility

## **Getting Started**

- Download BazPO.hpp source code and add to your project folder
- Include the downloaded header to your main program
- Library will be compiled along with your program
- **[Download BazPO Here](www.google.com)**

### **Functions / Usage**

- Instantiate BazPO::Cli
- Add your options
- Call parse
- Read your values

- **Adding an option**
  - Using add() [`see example 1`](#Example(1):)
  - Defining the option by yourself [`see example 2`](#Example(2):)
  
- **Reading the values**
  - Calling option(tag) with the tag you specified to get the option [`see example 1`](#Example(1):)
  - When using tagless options specify the order number as a string tag [`see example 2`](#Example(2):)
  - Calling value(tag)/values(tag) with the relevant tag to get option values directly [`see example 2`](#Example(2):)
  - Directly reading from the defined object [`see example 3`](#Example(3):)

### **Examples**

#### **Example (1):**

```c++
#include "BazPO.hpp"

using namespace BazPO;

int main(int argc, const char* argv[])
{
    Cli po(argc, argv);

    po.add("-a", "--alpha", "Option A", true);
    po.add("-b", [&](const Option&) { /* do something */ }, "--bravo", "Option B");

    po.parse();

    auto aoption = po.option("-a");

    std::cout << "EXISTS:" << aoption.exists() << std::endl;
    std::cout << "INT:" << aoption.valueAs<int>() << std::endl;
    std::cout << "BOOL:" << aoption.valueBool() << std::endl;
}
```

- Output

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

#### **Example(2):**

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

- Output
  
```txt
./myProgram input1 input2 input3 input4 input5 input6 input7
Given value -> 'input7' is not expected
BazPOManual
usage: BazPOManual  [-h] [(5)]  [(1)]
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

#### **Example(3):**

```c++
#include "BazPO.hpp"

using namespace BazPO;

int main(int argc, const char* argv[])
{
    Cli po(argc, argv);

    ValueOption optionA(&po, "-a", "--alpha", "Option A", true);
    FunctionOption optionB(&po, "-b", [&](const Option& option) {
        /* do something */
        }, "--bravo", "Option B");

    po.userInputRequired();

    po.parse();

    std::cout << "EXISTS:" << optionA.exists() << std::endl;
    std::cout << "INT:" << optionA.valueAs<int>() << std::endl;
    std::cout << "BOOL:" << optionA.valueBool() << std::endl;
}
```

- Output
  
```txt
./myProgram
<-a> is a required parameter
<-a>: True
EXISTS:1
INT:0
BOOL:1
```

## **Options**

### **ValueOption**

- Basic argument with/without a value or values provided with multiple tags.

Below argument list is valid and only one value is accepted.

```sh
myprogram -a value1 -a value2 -b
```

### **MultiOption**

- All arguments provided after the tag will be considered values until a valid tag.
- Max value count can be provided to limit the number of values that can be provided.

Below argument list is valid for MultiOption only.

```sh
myprogram -a value1 value2 value3 -a value4 -b
```

### **TaglessOption**

- A tag isnt needed, however other options cannot be used in conjunction with this option.
- Given number of arguments are separated into different options or all into one.
  
```sh
myprogram value1 value2 value3 value4
```

### **FunctionOption/FunctionMultiOption/FunctionTaglessOption**

- Provided function will be executed if the given tag is provided as an argument with or without a value.
- FunctionOption is parsed like [`ValueOption`](#ValueOption).
- FunctionMultiOption is parsed like [`MultiOption`](#MultiOption).
- FunctionTaglessOption is parsed like [`TaglessOption`](#TaglessOption).

### **Option Functions**

- Option
  - **exists()** -> option is present in the arguments list
  - **existsCount()** -> returns the number the option is present i.e -a -a -a will return 3
  - **value()** -> returns the raw argument value
  - **valueAs<T>** -> converts the raw argument to the given type
  - **valueBool()** -> returns true if argument is "1" or "True" or "true" or "t" or "y"
  - **execute()** -> only available with [`Function Options`](#FunctionOption/FunctionMultiOption/FunctionTaglessOption)
  - **values()** / **valuesAs<T>()** / **valuesBool()** -> same as their value counterpart but returns all provided values

## Customizations

### Asking For User Input When Mandatory Options Are Not Provided

- Call **userInputRequired()** in your program, user will be asked to input values for your mandatory values.
- For inputs, input stream could be changed to your choosen stream via **changeIO()**

```c++
    Cli po(argc, argv);
    po.userInputRequired();
```

### Making Invalid/Expanded Arguments Acceptable

- For example; when using tagless options, only 3 arguments are needed but calling the program with 4 arguments will cause to exit due to the unexpected argument.
- This behaviour could be used to accept expanded files provided to the program such as "myprogram -a *.txt"
- When used with other options, valid options will be parsed and program won't exit.

```c++
    Cli po(argc, argv);
    po.add(3);
    po.unexpectedArgumentsAcceptable();
```

### Disabling Auto Help

- You can simply disable "-h" option by defining the value below before the header definition.

```c++
#define BazPO_DISABLE_AUTO_HELP_MESSAGE
```
