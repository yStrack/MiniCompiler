# MiniCompiler
- The purpose of this project is to develop in C a function called "geracod", which implements a small code generator (a "micro-compiler") for a very simple programming language called SB.  

# How the SB language works
- The local variables follow the form vi, i identifies the variable (i.e v1,v2,...), the maximum number of variables is 4.   
- The parameters works similar, following the form pi (i.e p1,p2), the maximum number of parameters is 2.  
- Constants are written follow $i (i.e $1,$-1)  
- The file **_testTrab.txt_** represents a function that does the factorial of n (fat(n) -> returns n!).  

# Development enviroment
- Operational System: Fedora 23  
- Language: C  
- Compiler: gcc  

# How to compile and execute
## 1. Compile on linux
```
$ gcc -Wall -Wa,--execstack -o microCompiler mainTrab.c geracod.c
```
## 2. Execute
```
$ ./microCompiler
```
