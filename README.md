# C-LC3-Compiler

C-LC3-Compiler is a modern, student built C compiler targeting the LC3 Assembly language, described in *Introduction to Computing* by Dr. Yale Patt and Dr. Sanjay Patel. This tool is mainly for educational purposes, and is specifically meant to help students taking ECE 220 - Computer Systems & Programming at UIUC, however it should be relevant and useful to any student learning LC3 Assembly. The calling conventions implemented by the compiler mirror the conventions descibred in *Introduction to Computing*, any discrepencies are bugs.

This repository contains the source code for the compiler, as well as various tests. Currently, only a subset of the C language is supported. Some important features we are working on include:

- Global Variables
- Static Variables
- Arrays
- Structs and Unions
- Typedefs
- Function Pointers

### Build Instructions
To build the compiler locally,
1. Ensure you have CMake, and clang installed. 
2. `git clone https://github.com/xavierrouth/C-LC3-Compiler.git`
3. `cd C-LC3-Compiler`
4. `mkdir build`
5. `cd build`
6. `cmake ..`
7. `make`

To use the compiler without building from source we recommend using https://godbolt.org/.

### Usage Gudie
To run the compiler, use `lc3-compile`:

```
Usage: lc3-compile [OPTION...] file...
A C to LC3 Compiler built for students at the University of Illinois
Urbana-Champaign by HKN (https://hkn-alpha.netlify.app).

  -g, --debug                Enable debugging information
  -o, --output=FILE          Output path
      --sandbox              Disables most semantic analysis errors (type
                             checking)
  -S, --asm                  Enable assembly output
  -v, --verbose              Produce verbose output
  -?, --help                 Give this help list
      --usage                Give a short usage message
  -V, --version              Print program version

Mandatory or optional arguments to long options are also mandatory or optional
for any corresponding short options.

Report bugs to <xrouth2@illinois.edu>.
```

To run the tests, run the `test` executable from the build directory.

### LC3Tools
This project uses the [LC3Tools](https://github.com/chiragsakhuja/lc3tools) repository in order to assemble and verify the assembly generation from the compiler. Students working with LC3 may find tools from there helpful as well.

### Credits
The C-LC3-Compiler is developed and maintained by students from [HKN-Alpha](https://hkn-alpha.netlify.app/) at UIUC.

### Contributing and Bug Reporting
If you find a bug, please notify \<xrouth2 at illinois dot edu\>. We are always open to contributions, feel free to submit PRs or open discussion about what can be improved.
