#include <cdbuiltin.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <linux/limits.h>

int cd(char **arguments, int count, char *home_directory){
	if(count > 2){
		fprintf(stderr, "Error: Invalid Usage\n");
		return 1;
	} 
	else{
		char destinationDirectory[PATH_MAX + 1];
		if(count == 1){
			if(strcpy(destinationDirectory, home_directory) == NULL) {
				fprintf(stderr, "Strcpy Error\n");
				return 1;
			}
		}
		else{
			if(strcpy(destinationDirectory, arguments[1]) == NULL) {
				fprintf(stderr, "Strcpy Error\n");
				return 1;
			}
		}
		if(count > 1 && strcmp(arguments[1], "~") == 0) {
			if(strcpy(destinationDirectory, home_directory) == NULL) {
				fprintf(stderr, "Strcpy Error\n");
				return 1;
			}
		}
		else if(count > 1 && arguments[1][0] == '~' && arguments[1][1] == '/') {
			if(strcpy(destinationDirectory, home_directory) == NULL) {
				fprintf(stderr, "Strcpy Error\n");
				return 1;
			}
			if(strcat(destinationDirectory, &arguments[1][1]) == NULL) {
				fprintf(stderr, "Strcat Error\n");
				return 1;
			}
		}
		int val = chdir(destinationDirectory);
		if(val == -1){
			perror("Chdir Error");
			return 1;
		}
		return 0;
	} 
}