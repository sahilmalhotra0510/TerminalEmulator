#include <prompt.h>
#include <input.h>
#include <stdio.h>
#include <execute.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <jobcontrol.h>

void init(){
	initPrompt();
	initExecute();
	initJobControl();
}

int main(){
	init();
	while(1){
		char *promptMemory = NULL;
		int error = showPrompt(&promptMemory);
		if(error) continue;
		int val = fflush(stdout);
		if(val != 0){
			perror("Error in Flushing Output");
			continue;
		}
		char *inputMemory = NULL;
		char **commands = NULL;
		fetchCommands(promptMemory, &inputMemory, &commands);
		int position = 0;
		if(commands == NULL) {
			puts("");
		}
		else {
			while(1){
				if(commands[position] == NULL) break;
				runCommand(commands[position]);
				++position;
			}
		}
		if(promptMemory) free(promptMemory);
		if(commands) free(commands);
		if(inputMemory) free(inputMemory);
	}

	closePrompt();
	return 0;
}