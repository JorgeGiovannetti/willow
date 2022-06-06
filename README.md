# willow
[![Build](https://github.com/JorgeGiovannetti/willow/actions/workflows/build.yml/badge.svg)](https://github.com/JorgeGiovannetti/willow/actions/workflows/build.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)


A concise, statically typed, object-oriented programming language

## Milestones

- [x] Lexical and Syntactic Analysis
- [x] Basic Variable Semantics: Procedure Directory and Symbol Table
- [x] Basic Variable Semantics: Semantic Considerations Table
- [x] Code Generation of Arithmetic Expressions and Sequential Statements: Assignment, Input, etc.
- [x] Code Generation of Conditional Statements: Decisions/Loops
- [ ] Code Generation of Functions
- [x] Runtime Memory Mapping for Virtual Machine
- [ ] Virtual Machine: Arithmetic Expression and Sequential Statement Execution
- [ ] Code Generation for Arrays/Structured Types
- [ ] Virtual Machine: Conditional Statement Execution
- [ ] Documentation (1st Draft)
- [ ] Code Generation and Virtual Machine
- [ ] Final Compiler and Documentation

## Requirements
This project requires C++17, supporting the following compilers:
* GCC >= 8.3.0
* Clang >= 7.0.0
* MSVC >= 14.16

## Example
The following code demonstrates a simple willow program:
```rust
fn main() {
    myName: string = read(); 
    write("Hello, " + myName + "!");
    x: int = 3;
    y: int = 5;
    
    write(x + y);
}
```

## Developed by
[Jorge Giovannetti](https://github.com/JorgeGiovannetti) \
[Mariafernanda Salas](https://github.com/MariferSalas)

## License
The project is available under the [MIT](https://opensource.org/licenses/MIT) license.
