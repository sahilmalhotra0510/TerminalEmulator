#include <pwdbuiltin.h>
#include <stdio.h>
#include <linux/limits.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

int pwd(char **arguments, int count, char *home_directory){
	if(count > 1){
		fprintf(stderr, "Error: Invalid Usage\n");
		return 1;
	} 
	char present_directory[PATH_MAX + 1];
	if(getcwd(present_directory, PATH_MAX + 1) != NULL){
		printf("%s\n", present_directory);
		return 0;
	}
	else{
		perror("Getcwd Error");
		return 1;
	}
}