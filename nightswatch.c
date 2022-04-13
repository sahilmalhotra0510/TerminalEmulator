#include <nightswatch.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>

/* Initialize new terminal i/o settings */
static struct termios old, new1;
void initTermios(int echo) {
	tcgetattr(0, &old); /* grab old terminal i/o settings */
	new1 = old; /* make new settings same as old settings */
	new1.c_lflag &= ~ICANON; /* disable buffered i/o */
	new1.c_lflag &= echo ? ECHO : ~ECHO; /* set echo mode */
	tcsetattr(0, TCSANOW, &new1); /* use these new terminal i/o settings now */
}

/* Restore old terminal i/o settings */
void resetTermios(void) {
	tcsetattr(0, TCSANOW, &old);
}

int wasKeyPressed() {
	int bytesWaiting;
	ioctl(STDIN_FILENO, FIONREAD, &bytesWaiting);
	return bytesWaiting;
}

int interrupt() {
	int fd_file = open("/proc/interrupts", O_RDONLY);
	if(fd_file<0){
		perror("Open Error");
		return 1;
	}
	else{
		char buf[100001];
		read(fd_file, buf, 100000);
		char *p=strtok(buf, "\n");
		printf("%s\n", p + 4);
		while(p!=NULL){
			if(strstr (p, "i8042") != NULL){
				char *infoLine = p;
				while(infoLine) {
					if(('a' <= *infoLine && *infoLine <= 'z') || ('A' <= *infoLine && *infoLine <= 'Z')){
						*infoLine = '\0';
						break;
					}
					++infoLine;
				}
				printf("%s\n", p + 4);
				break;
			}
			p=strtok(NULL, "\n");
		}
	}
	close(fd_file);
	return 0;
}

int dirty() {
	int fd_file = open("/proc/meminfo", O_RDONLY);
	if(fd_file<0){
		perror("Open Error");
		return 1;
	}
	else{
		char buf[100001];
		read(fd_file, buf, 100000);
		char *p=strtok(buf, "\n");
		while(p!=NULL){
			if(strstr (p, "Dirty:") != NULL){
				while(p) {
					if('0' <= *p && *p <= '9') break;
					++p;
				}
				printf("%s\n", p);
				break;
			}
			p=strtok(NULL, "\n");
		}
	}
	close(fd_file);
	return 0;
}

struct nightswatchCommands{
	char *command;
	int (*commandFunction)();
} nightswatchCommandsList[] = {{"interrupt", interrupt}, {"dirty", dirty}};

int nightswatch(char **arguments, int count, char *home_directory){
	double timeInterval;
	if(count != 4){
		fprintf(stderr, "Error: Invalid Usage\n");
		return 1;
	}
	if(strcmp(arguments[1], "-n") != 0){
		fprintf(stderr, "Error: Invalid Usage\n");
		return 1;
	}
	timeInterval = atof(arguments[2]);
	if(timeInterval <= 0.0){
		fprintf(stderr, "Error: Invalid Usage\n");
		return 1;
	}
	int lenNightswatchCommands = sizeof(nightswatchCommandsList)/sizeof(nightswatchCommandsList[0]);
	for(int i=0; i<lenNightswatchCommands; ++i) {
		if(strcmp(arguments[3], nightswatchCommandsList[i].command) == 0){
			time_t now, prev;
			int firstRun = 1;
			initTermios(0);
			while(1){
				time(&now);
				if(firstRun || difftime(now, prev) >= timeInterval){
					if(nightswatchCommandsList[i].commandFunction()){
						return 1;
					}
					prev = now;
					firstRun = 0;
				}
				while(wasKeyPressed()){
					char ch;
					scanf("%c", &ch);
					if(ch == 'q'){
						resetTermios();
						while(wasKeyPressed()) scanf("%c", &ch);
						return 0;
					}
				}
			}
		}
	}
	fprintf(stderr, "Error: Invalid Command\n");
	return 1;
}