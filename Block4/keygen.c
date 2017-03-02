#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

int main(int argc, char *argv[]){
	time_t t;
	srand(time(&t));
	int i;
	int keyLength = atoi(argv[1]);
	
	for(i=0;i<keyLength;i++){
		int randNum = rand() % 27 + 65;
		if(randNum == 91)
			randNum = 32;
		printf("%c",randNum);
	}
	printf("\n");
	return 0;
}