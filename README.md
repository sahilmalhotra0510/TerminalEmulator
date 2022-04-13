# TerminalEmulator
This project consists of an interactive custom made shell program to create and manage new processes. The code is compatible with UNIX based Operating Systems.

This code is easily extensible. The separate files are as follows:
1. shell.c : The main file to initialize variables and control the main flow.
2. input.c : Accept input from user, call tokenize function to split it into multiple commands.
3. tokenize.c : Tokenize given input string into multiple commands.
4. execute.c : This executes a pipeline along with handling whether it has to be executed in the foreground or background.
5. jobcontrol.c : This file has signal handlers for SIGCHLD, SIGINT, SIGTSTP, SIGTTOU. Has functions like jobs, overkill, kjob, fg and bg.

Execute on UNIX terminal as:
$ make
$ ./shell
$ make clean
