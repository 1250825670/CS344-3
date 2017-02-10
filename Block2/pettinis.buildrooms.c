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

char* getName(int* usedRooms){
	char *roomNames[10] = {"EDINBURGH","CORNWALL","CARDIFF","BRISTOL","YORK","KENT","OXFORD","SANDFORD","CANTERBURY","LONDON"};
	int randNum = rand() % 10;
	while(1){
		if(usedRooms[randNum] == 0){
			usedRooms[randNum] = 1;
			return roomNames[randNum];
		}
		randNum = rand() % 10;
	}
}

struct Room* makeRoom(int* usedRooms){
	int i;
	struct Room *room = (struct Room *) malloc(sizeof(struct Room));
	sprintf(room->name,getName(usedRooms));
	room->totalConnections = (rand() % 4) + 3;
	room->usedConnections = 0;
	for(i=0;i<6;i++)
		room->connections[i] = 0;
	sprintf(room->type,"MID_ROOM");
	return room;
}

int verifyConnections(struct Room* rooms[], int numRooms){
	int i;
	for(i=0;i<numRooms;i++){
		if(rooms[i]->usedConnections < 3)
			return 1;
	}
	return 0;
}
int makeConnections(struct Room* rooms[], int numRooms){
	int randNum;	//to hold randomly generated numbers
	int i;//,j;
	//for(j=0;j<numRooms;j++)
	//		printf("%s\n",rooms[j]->name);
	for (i=0; i<numRooms; i++){
		//printf("Run %d\n",i);
		while (rooms[i]->usedConnections < rooms[i]->totalConnections){
			randNum = rand() % numRooms;
			printf("Created: %d\n",randNum);
			while (randNum == i)
				randNum = rand() % numRooms;
			printf("Used: %d\n",randNum);
			printf("%s <-> %s\n",rooms[i]->name, rooms[randNum]->name);
			printf("%d <-> %d\n",rooms[i]->usedConnections,rooms[randNum]->usedConnections);
			rooms[i]->connections[rooms[i]->usedConnections] = rooms[randNum];
			rooms[randNum]->connections[rooms[randNum]->usedConnections] = rooms[i];
			rooms[i]->usedConnections++;
			rooms[randNum]->usedConnections++;
			printf("%s <-> %s\n",rooms[i]->name, rooms[randNum]->name);
			printf("%d <-> %d\n",rooms[i]->usedConnections,rooms[randNum]->usedConnections);
		}
	}
	//printf("makeConnections:\n");
	//for(j=0;j<numRooms;j++)
	//	printf("%s\n",rooms[j]->name);
	return verifyConnections(rooms, numRooms);
}

int main(void){
	int numRooms = 7;
	int usedRooms[10], i, j;
	for(i=0;i<10;i++)
		usedRooms[i] = 0;
	time_t t;
	pid_t getpid(void);
	srand((unsigned) time(&t));
	struct Room *rooms[numRooms];
	rooms[0] = makeRoom(usedRooms);
	sprintf(rooms[0]->type,"START_ROOM");
	rooms[1] = makeRoom(usedRooms);
	sprintf(rooms[1]->type,"END_ROOM");
	for (i=2; i<numRooms; i++){
		rooms[i] = makeRoom(usedRooms);
	}
	printf("Rooms created\n");
	char directory[100];
	sprintf(directory, "pettinis.rooms.%d",getpid());
	mkdir(directory,S_IRWXU | S_IRWXG | S_IRWXO);
	if(makeConnections(rooms, numRooms) == 1){
		printf("Error: Not all rooms have 3 connections.\n");
		return 1;
	}
	printf("Connections created\n");
	FILE *file;
	for (i=0; i<numRooms; i++){
		printf("Room %d\n",i+1);
		char fileLocation[100];
		sprintf(fileLocation,"%s/%s_room",directory,rooms[i]->name);
		printf("%s\n",fileLocation);
		file = fopen(fileLocation,"w");
		if(file == 0){
			printf("file open failed\n");
			exit(1);
		}
		char input1[100] = "ROOM NAME: ";
		strcat(input1,rooms[i]->name);
		strcat(input1,"\n");
		fprintf(file,input1);
		for(j=0; j<rooms[i]->usedConnections; j++){
			char input2[100];
			sprintf(input2, "CONNECTION %d: %s",j+1,rooms[i]->connections[j]->name);
			strcat(input2,"\n");
			fprintf(file,input2);
		}
		char input3[100] = "ROOM TYPE: ";
		strcat(input3,rooms[i]->type);
		strcat(input3,"\n");
		fprintf(file,input3);
		fclose(file);
	}
	printf("Files created\n");
	for (i=0; i<numRooms; i++){
		printf("Removing %d\n",i);
		free(rooms[i]);
		rooms[i]=0;
	}
	printf("Rooms deleted\n");
}
