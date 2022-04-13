#include <execute.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <linux/limits.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <cdbuiltin.h>
#include <pwdbuiltin.h>
#include <echobuiltin.h>
#include <lsbuiltin.h>
#include <nightswatch.h>
#include <jobcontrol.h>
#include <pinfo.h>
#include <errno.h>
#include <signal.h>

typedef struct backgroundCommand {
	struct backgroundCommand* nextCommand;
	char* commandName;
	pid_t processId;
	int position;
} backgroundCommands;

backgroundCommands* startProcess = NULL;
backgroundCommands* foreGroundCommand = NULL;

void childEndHandler(int sig){
	pid_t pid;
	int status;

	while((pid = waitpid(-1, &status, (WNOHANG | WUNTRACED))) > 0){
		backgroundCommands *iterator = startProcess, *prev = NULL;

		while(iterator) {
			if(iterator->processId == pid) {
				if(prev){
					prev->nextCommand = iterator->nextCommand;
				}
				else{
					startProcess = iterator->nextCommand;
				}
				break;
			}
			prev = iterator;
			iterator = iterator->nextCommand;
		}
		
		if(foreGroundCommand && foreGroundCommand->processId == pid) {
			tcsetpgrp(STDIN_FILENO, getpgid(0));
			if(WIFEXITED(status)){
			}
			else if(WIFSTOPPED(status)) {
				printf("%s with pid %d stopped\n", foreGroundCommand->commandName, pid);
				addToBackground(foreGroundCommand->processId, foreGroundCommand->commandName);
			}
			else if(WIFSIGNALED(status)){
				printf("%s with pid %d terminated\n", foreGroundCommand->commandName, pid);
			}
			free(foreGroundCommand->commandName);
			free(foreGroundCommand);
			foreGroundCommand = NULL;
		}
		else{
			if(WIFEXITED(status)){
				int es = WEXITSTATUS(status);

				printf("%s with pid %d exited with status %d\n", iterator->commandName, pid, es);
				free(iterator->commandName);
				free(iterator);
			}
			else if(WIFSTOPPED(status)) {
				printf("%s with pid %d stopped\n", iterator->commandName, pid);
			}
			else if(WIFSIGNALED(status)){
				printf("%s with pid %d terminated\n", iterator->commandName, pid);
				free(iterator->commandName);
				free(iterator);
			}
		}

	}
}

void sigintHandler(int sig) {
	if(foreGroundCommand) {
		kill(foreGroundCommand->processId, SIGINT);
	}
}

void sigtstpHandler(int sig) {
	if(foreGroundCommand) {
		kill(foreGroundCommand->processId, SIGTSTP);
	}
}

void initJobControl() {
	signal(SIGCHLD, childEndHandler);
	signal(SIGINT, sigintHandler);
	signal(SIGTSTP, sigtstpHandler);
	signal(SIGTTOU, SIG_IGN);
}

int recursivePrintJob(backgroundCommands* iterator) {
	if(iterator == NULL) return 0;

	int val = recursivePrintJob(iterator->nextCommand);

	printf("[%d]\t", iterator->position);
	char pathToProcess[PATH_MAX+1];
	sprintf(pathToProcess, "/proc/%d/wchan", iterator->processId);
	
	FILE* fp = fopen(pathToProcess, "r");
	if(fp != NULL){
		char status[PATH_MAX+1];
		fscanf(fp, "%s", status);
	
		if(strcmp(status, "do_signal_stop") == 0) printf("Stopped\t");
		else printf("Running\t");

		fclose(fp);

		sprintf(pathToProcess, "/proc/%d/cmdline", iterator->processId);
		FILE* fp2 = fopen(pathToProcess, "r");

		if(fp2 != NULL){
			char processName[PATH_MAX+1];
			fscanf(fp2, "%s", processName);

			if(strstr(processName, "/") == NULL) printf("\t%s [%d]\n", processName, iterator->processId);
			else printf("\t%s [%d]\n", 1 + strrchr(processName, (int)'/'), iterator->processId);

			fclose(fp2);
		}
	}
	return val;
}

int printJobs(char **arguments, int count, char *home_directory){
	return recursivePrintJob(startProcess);
}

int setenvBuiltin(char **arguments, int count, char *home_directory){
	if(count == 2) setenv(arguments[1], "", 1);	
	else if(count == 3) setenv(arguments[1], arguments[2], 1);
	else {
		fprintf(stderr, "Setenv: Invalid Usage\n");
		return 1;
	}
	return 0;
}

int getenvBuiltin(char **arguments, int count, char *home_directory){
	if(count != 2) {
		fprintf(stderr, "Getenv: Invalid Usage\n");
		return 1;
	}
	else printf("%s\n", getenv(arguments[1]));
	return 0;
}

int unsetenvBuiltin(char **arguments, int count, char *home_directory){
	if(count == 2){
		unsetenv(arguments[1]);
	}
	else fprintf(stderr, "Unsetenv: Invalid Usage\n");
	return 0;
}

int overkill(char **arguments, int count, char *home_directory){
	backgroundCommands *iterator = startProcess;
	while(iterator){
		int val = kill(iterator->processId, SIGKILL);
		if(val == -1){
			perror("Error");
			return 1;
		}
		iterator = iterator->nextCommand;
	}
	return 0;
}

void addToBackground(pid_t PID, char* commandArgument) {
	backgroundCommands* current = malloc(sizeof(backgroundCommands));
	if(startProcess == NULL) current->position = 1;
	else current->position = startProcess->position + 1;
	current->nextCommand = startProcess;
	current->processId = PID;
	current->commandName = malloc(sizeof(char) * (1 + strlen(commandArgument)));
	strcpy(current->commandName, commandArgument);
	startProcess = current;
}

void addToForeground(pid_t PID, char* commandArgument) {
	foreGroundCommand = malloc(sizeof(backgroundCommands));
	foreGroundCommand->position = -1;
	foreGroundCommand->nextCommand = NULL;
	foreGroundCommand->processId = PID;
	foreGroundCommand->commandName = malloc(sizeof(char) * (1 + strlen(commandArgument)));
	strcpy(foreGroundCommand->commandName, commandArgument);
}

int quitBuiltin(char **arguments, int count, char *home_directory){
	overkill(arguments, count, home_directory);
	exit(0);
}

int kjob(char **arguments, int count, char *home_directory){
	if(count != 3){
		fprintf(stderr, "Error: Invalid Usage\n");
		return 1;
	}
	backgroundCommands *iterator = startProcess;
	while(iterator){
		if(iterator->position == atoi(arguments[1])){
			int val = kill(iterator->processId, atoi(arguments[2]));
			if(val == -1) {
				perror("Kill error");
				return 1;
			}
			return 0;
		}
		iterator = iterator->nextCommand;
	}
	fprintf(stderr, "Error: No job with job number %d\n", atoi(arguments[1]));
	return 1;
}

int bgBuiltin(char **arguments, int count, char *home_directory){
	if(count != 2){
		fprintf(stderr, "Error: Invalid Usage\n");
		return 1;
	}
	backgroundCommands *iterator = startProcess;
	while(iterator){
		if(iterator->position == atoi(arguments[1])){
			int val = kill(iterator->processId, SIGCONT);
			if(val == -1) {
				perror("Kill error");
				return 1;
			}
			return 0;
		}
		iterator = iterator->nextCommand;
	}
	fprintf(stderr, "Error: No job with job number %d\n", atoi(arguments[1]));
	return 1;
}

int fgBuiltin(char **arguments, int count, char *home_directory){
	if(count != 2){
		fprintf(stderr, "Error: Invalid Usage\n");
		return 1;
	}
	backgroundCommands *iterator = startProcess, *prev = NULL;
	while(iterator) {
		if(iterator->position == atoi(arguments[1])){
			int val = kill(iterator->processId, SIGCONT);
			if(val == -1) {
				perror("Kill error");
				return 1;
			}
			if(prev){
				prev->nextCommand = iterator->nextCommand;
			}
			else{
				startProcess = iterator->nextCommand;
			}
			addToForeground(iterator->processId, iterator->commandName);
			free(iterator->commandName);
			free(iterator);
			tcsetpgrp(STDIN_FILENO, foreGroundCommand->processId);
			siginfo_t fgStatus;
			waitid(P_PID, foreGroundCommand->processId, &fgStatus, (WUNTRACED | WNOWAIT));
			return 0;
		}
		prev = iterator;
		iterator = iterator->nextCommand;
	}
	fprintf(stderr, "Error: No job with job number %d\n", atoi(arguments[1]));
	return 1;
}