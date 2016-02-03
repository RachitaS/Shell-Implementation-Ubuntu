# Shell-Implementation-Ubuntu
Implemented a Linux shell for running all types of commands (that supports piping,
redirection, history, environment variables,external-internal commands etc) in C++ using family of
system calls.

Programming Language: C++
Runnning Platform: Ubuntu

# Makefile included

Functionalities Implemented:

1) Execution all the commands (ls, clear, vi etc) 
2) Shell built­ins (cd, pwd, export)
3) Echo with or without quotes (Odd number of quotes handled)
4) Grep with or without quotes
5) Multilevel pipe
6) I/O redirection
7) History
8) !<int>, !<string>, and !! cases
9) Pipes with redirection (output redirection always occurs in the end)
10) Environment and local variables echoing.
11) Exit the program on command "exit".
12) Interrupt Signal handled:(On pressing "Ctrl+C")
