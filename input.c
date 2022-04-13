#include <input.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <tokenize.h>
#include <errno.h>
#include <readline/readline.h>
#include <readline/history.h>

void fetchCommands(char *promptString, char **inp, char ***commands){
	*inp = readline(promptString);
	if(*inp) add_history(*inp);
	if(*inp) tokenizeCommands((*inp), (commands));
	else commands = NULL;
}
