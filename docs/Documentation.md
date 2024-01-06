# Hydrogen Documentation

## Chapter 1: What is Hydrogen?
### Chapter 1.1: Basic knowledge
Hydrogen is a interpreted language that compiles to an executable(i think). Please remember that the development of this language is just a hobby.

### Chapter 1.2: Why Hydrogen?
The language is called Hydrogen because its lightweight, simple and it will light on fire when handled improperly.

## Chapter 2: Usage
### Chapter 2.1: Basic usage
The basic usage is simple. Just use 
```bash
hydro <input file>
```
This command will generate an `out` file from the input file which you can execute

### Chapter 2.2: Arguments
You can insert arguments right after `hydro`. All arguments are optional except the input file which is the argument that doesn't start with `-` </br> </br>
The arguments include: 
* `-d` -- Generates the `out.asm` and `out.o` file with the executable.
* `-o` -- Specifies the output file name.
* `-h` -- (Doesn't generate the executable!) Shows the argument and information

## Chapter 3: Installation
### Chapter 3.1: Prerequisites
You need to have these tools installed:
* `g++` or `clang` -- C++ Compiler 
* `nasm` -- Netwide Assembler
* `ld` -- The GNU Linker
* `cmake` -- To build the project
* `git` -- The version-control program to download the repository

### Chapter 3.2: Download the repository
Run the following command in a [`bash`](https://en.wikipedia.org/wiki/Bash_(Unix_shell)) shell.
```bash
git clone https://github.com/orosmatthew/hydrogen-cpp
cd hydrogen-cpp
```
Now you are in the `hydrogen-cpp` directory

### Chapter 3.3: Configure the build
Now run 
```bash
cmake -S . -B build/
```
This will setup everything for the next step.

### Chapter 3.4: Build the compiler executable
Now you need to run 
```bash
cmake --build build/
```

This will create the `hydro` executable in the `build\` directory. For usage instructions please refer to <b> Chapter 2 </b>


## Chapter 4: Syntax 
### Chapter 4.1: Basic Syntax
The Hydrogen language is a semicolon(`;`) based language. That means that after every line/statement you need to have a semicolon. </br> </br>
The language also uses curly brackets (`{`and`}`) for scopes.

### Chapter 4.2: Basic Keywords
There are some basic keywords in the language right now. </br>
These include:
* `let` -- Declare variables.
* `if` -- Add an `if` statement.
* `else` -- Add an `else` branch to your `if statement`.
* `elif` -- Add an `elif` branch to your `if statement`.
### Chapter 4.3: Included Functions
These are functions right build into the language.</br>
These include:
* `exit(EXIT_CODE)` -- Exits the program with the given exit code.

### Chapter 4.4: Comments
There are singe line and multi line comments in the hydrogen language.
#### Chapter 4.4.1: Single Line Comments
You can declare single line comments with `//`.</br>
Example:
```c
// first
```

#### Chapter 4.4.2: Multi Line Comments
You can also declare multi line comments using `/*` to begin and `*/` to end a multi line comment. </br>
Example: 
```c
/* I like to code in C++
Subscribe to Pixeled on YouTube
I hope you have a great day! */
```

### Chapter 4.5: Variables
You can create variables using the `let` keyword.
The formula to create a variable is the following (Everything in curly brackets is a customaizable value) 
```
let {VAR_NAME} =  {VAR_INITIAL_VALUE};
```

### Chapter 4.6: Language Grammar/Language Blueprint
For this information please look into the [Grammar File](grammar.md)

## Chapter 5: Information
### Chapter: 5.1: Social Media
This project is also on YouTube in the Series "[Creating a Compiler](https://www.youtube.com/playlist?list=PLUDlas_Zy_qC7c5tCgTMYq2idyyT241qs)" by Pixeled. </br>
Pixeled's Website: [pixeled.site](https://pixeled.site) </br>
Discord Server: <b><u>NOT AVAILABLE</b></u>

### Chapter 5.2: Contributers
The contributers will be listed down below by thier GitHub usernames along with the pull request/requests that got merged. </br>
These people have contributed to the project so far:
* <b>[orosmatthew](https://github.com/orosmatthew)</b> -- Owner and creator of the series
* <b>[xtay2](https://github.com/xtay2)</b> -- [Fixed Typo in grammar.md](https://github.com/orosmatthew/hydrogen-cpp/pull/6)
* <b>[Ikos3k](https://github.com/Ikos3k)</b> -- [Changed Variable Types from `int` to `size_t`](https://github.com/orosmatthew/hydrogen-cpp/pull/9) </br>
* <b>[mgerhold](https://github.com/mgerhold)</b> -- [Fixed issues in `ArenaAllocator`](https://github.com/orosmatthew/hydrogen-cpp/pull/11) </br>
* <b>[lolguy91](https://github.com/lolguy91)</b> -- [Added the README.md](https://github.com/orosmatthew/hydrogen-cpp/pull/3) </br>
* <b>[RaphtikAtGHG](https://github.com/RaphtikAtGHG)</b> -- [Added Arguments, Added Documentation and created the Discord server(Unofficial)]()