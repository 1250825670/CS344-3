#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>

struct Room {
	char name[25];
	int totalConnections;
	int usedConnections;
	struct Room* connections[6];
	char type[25];
};

char* getName(int randNum){
	if (randNum == 0)
		return "EDINBURGH\0";
	else if (randNum == 1)
		return "CORNWALL\0";
	else if (randNum == 2)
		return "CARDIFF\0";
	else if (randNum == 3)
		return "BRISTOL\0";
	else if (randNum == 4)
		return "YORK\0";
	else if (randNum == 5)
		return "KENT\0";
	else if (randNum == 6)
		return "OXFORD\0";
	else if (randNum == 7)
		return "SANDFORD\0";
	else if (randNum == 8)
		return "CANTERBURY\0";
	else if (randNum == 9)
		return "LONDON\0";
}

struct Room* makeRoom(){
	printf("making room\n");
	struct Room* room = (struct Room*) malloc(sizeof(struct Room));
	sprintf(room->name,getName(rand() % 10));
	room->totalConnections = (rand() % 4) + 3;
	room->usedConnections = 0;
	sprintf(room->type,"MID_ROOM");
	printf("room created\n");
	return room;
}

void makeConnections(struct Room* rooms[], int numRooms){
	printf("making connections\n");
	int randNum;	//to hold randomly generated numbers
	int i;
	for (i=0; i<numRooms; i++){
		while (rooms[i]->usedConnections < rooms[i]->totalConnections){
			randNum = rand() % numRooms;
			while (randNum == i && rooms[randNum]->usedConnections < rooms[randNum]->totalConnections)
				randNum = rand() % numRooms;
			rooms[i]->usedConnections++;
			rooms[randNum]->usedConnections++;
			rooms[i]->connections[rooms[i]->usedConnections] = rooms[randNum];
			rooms[randNum]->connections[rooms[randNum]->usedConnections] = rooms[i];
		}
	}
	printf("connections made\n");
}

int main(void){
	time_t t;
	pid_t getpid(void);
	srand((unsigned) time(&t));
	int numRooms = rand() % 8;
	struct Room* rooms[numRooms+3];
	rooms[0] = makeRoom();
	sprintf(rooms[0]->type,"START_ROOM");
	rooms[1] = makeRoom();
	sprintf(rooms[1]->type,"END_ROOM");
	int i, j;
	printf("numrooms: %d\n",numRooms);
	for (i=2; i<(numRooms+3); i++){
		rooms[i] = makeRoom();
	}
	printf("after makerooms\n");
	char directoryLocation[1000];
	sprintf(directoryLocation, "pettinis.rooms.%d./",getpid());
	makeConnections(rooms, numRooms);
	FILE *file;
	int file_descriptor;
	printf("before saving\n");
	for (i=1; i<=numRooms+3; i++){
		char fileLocation[1000];
		sprintf(fileLocation,"%sroom%d.txt",directoryLocation,i);
		printf("opening file\n");
		file = fopen(fileLocation,"w");
		if(file == 0)
			printf("file open failed\n");
		printf("getting name\n");
		char input1[1000] = "ROOM NAME: ";
		strcat(input1,rooms[i]->name);
		printf("saving name\n");
		fprintf(file,input1);
		//fputs(input1, file);
		//fputs("\n", file);
		printf("getting connections\n");
		for(j=1; j<=rooms[i]->totalConnections; j++){
			char input2[1000];
			sprintf(input2, "CONNECTION %d: %s",j,rooms[i]->connections[j]->name);
			fputs(input2, file);
			fputs("\n", file);
		}
		printf("getting type\n");
		char input3[1000] = "ROOM TYPE: ";
		strcat(input3,rooms[i]->type);
		fputs(input3, file);
		fputs("\n", file);
		fclose(file);
	}
	printf("after saving\n");
	for (i=0; i<(numRooms+3); i++){
		free(rooms[i]);
	}
}