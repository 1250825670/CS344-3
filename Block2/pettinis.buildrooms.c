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

char* getName(int randNum, int usedRooms[]){
	char name[25];
	memset(name,'0',sizeof(name));
	if (randNum == 0 && usedRooms[0] == 0)
		name = "EDINBURGH";
	else if (randNum == 1 && usedRooms[1] == 0)
		name = "CORNWALL";
	else if (randNum == 2 && usedRooms[2] == 0)
		name = "CARDIFF";
	else if (randNum == 3 && usedRooms[3] == 0)
		name = "BRISTOL";
	else if (randNum == 4 && usedRooms[4] == 0)
		name = "YORK";
	else if (randNum == 5 && usedRooms[5] == 0)
		name = "KENT";
	else if (randNum == 6 && usedRooms[6] == 0)
		name = "OXFORD";
	else if (randNum == 7 && usedRooms[7] == 0)
		name = "SANDFORD";
	else if (randNum == 8 && usedRooms[8] == 0)
		name = "CANTERBURY";
	else if (randNum == 9 && usedRooms[9] == 0)
		name = "LONDON";
	else{
		printf("Error, no names available or invalid input.\n");
		name = "ERROR";
	}
	usedRooms[randNum] = 1;
	return name;
}

struct Room* makeRoom(){
	int usedRooms[10], i;
	for(i=0;i<10;i++)
		usedRooms[i] = 0;
	struct Room* room = (struct Room*) malloc(sizeof(struct Room));
	sprintf(room->name,getName(rand() % 10),usedRooms);
	room->totalConnections = (rand() % 4) + 3;
	room->usedConnections = 0;
	sprintf(room->type,"MID_ROOM");
	return room;
}

int verifyConnections(struct Room* rooms[], int numRooms)){
	int i;
	for(i=0;i<numRooms;i++){
		if(rooms[i]->usedConnections < 3)
			return 1;
	}
	return 0;
}
int makeConnections(struct Room* rooms[], int numRooms){
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
	return verifyConnections(rooms, numRooms);
		
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
	for (i=2; i<(numRooms+3); i++){
		rooms[i] = makeRoom();
	}
	char directory[100];
	sprintf(directory, "pettinis.rooms.%d",getpid());
	mkdir(directory,S_IRWXU | S_IRWXG | S_IRWXO);
	if(makeConnections(rooms, numRooms) == 1){
		printf("Error: Not all rooms have 3 connections.\n");
		return 1;
	}
	FILE *file;
	for (i=1; i<=numRooms+3; i++){
		char fileLocation[100];
		sprintf(fileLocation,"./%s/%s_room",directory,rooms[i]->name);
		char* location = fileLocation;
		file = fopen(location,"w");
		if(file == 0){
			printf("file open failed\n");
			exit(1);
		}
		char input1[100] = "ROOM NAME: ";
		strcat(input1,rooms[i]->name);
		fprintf(file,input1);
		//fputs(input1, file);
		//fputs("\n", file);
		for(j=1; j<=rooms[i]->usedConnections; j++){
			char input2[100];
			sprintf(input2, "CONNECTION %d: %s",j,rooms[i]->connections[j]->name);
			fputs(input2, file);
			fputs("\n", file);
		}
		char input3[100] = "ROOM TYPE: ";
		strcat(input3,rooms[i]->type);
		fputs(input3, file);
		fputs("\n", file);
		fclose(file);
	}
	for (i=0; i<(numRooms+3); i++){
		free(rooms[i]);
	}
}
