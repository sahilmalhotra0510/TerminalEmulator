#include <echobuiltin.h>
#include <stdio.h>

int echo(char **arguments, int count, char *home_directory){
	for(int i=1; i<count; ++i){
		printf("%s", arguments[i]);
		if(i < count-1) printf(" ");
	}
	printf("\n");
	return 0;
}