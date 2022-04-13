#include <pinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/limits.h>

int pinfo(char **arguments, int count, char *home_directory){
	int pid = 0;
	if(count == 1) pid = getpid();
	else if(count == 2){
		for(int i=0; i<strlen(arguments[1]); ++i){
			pid	= pid * 10;
			pid += arguments[1][i] - '0';
		}
	}
	else{
		fprintf(stderr, "Usage Error\n");
		return 1;
	} 

	char PID[100];
	PID[0] = '/'; PID[1] = 'p'; PID[2] = 'r'; PID[3] = 'o'; PID[4] = 'c'; PID[5] = '/';
	int t = pid, tt = pid, c = 0;
	while(t){
		t /= 10;
		++c;
	}
	t = 6 + c - 1;
	while(tt){
		PID[t--] = ('0' + (tt % 10));
		tt /= 10;
	}
	t = 6 + c;
	PID[t] = '/'; PID[t + 1] = 'e'; PID[t + 2] = 'x'; PID[t + 3] = 'e'; PID[t + 4] = '\0';

	char link[4096];
	for(int i=0; i<4096; ++i) link[i] = '\0';

	int val = readlink(PID, link, 4095);

	PID[t + 1] = 's';
	PID[t + 2] = 't';
	PID[t + 3] = 'a';
	PID[t + 4] = 't';
	PID[t + 5] = '\0';

	FILE* fp = fopen(PID, "r");

	if(fp != NULL){
		char  b[PATH_MAX + 1];
		fscanf(fp, " %4096s", b);
		fscanf(fp, " %4096s", b);
		fscanf(fp, " %4096s", b);
		printf("pid -- %d\n", pid);
		printf("Process Status -- %s\n", b);

		for(int i=0; i<20; ++i) fscanf(fp, " %4096s", b);
		printf("Virtual Memory -- %s\n", b);

		fclose(fp);
	}	
	else{
		perror("File Open Error");
		return 1;
	}

	if(val == -1){
		fprintf(stderr, "Readlink Error\n");
		return 1;
	}
	else printf("Executable path -- %s\n", link);
	return 0;
}