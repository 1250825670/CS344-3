#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

enum nameList {EDINBURGH, CORNWALL, CARDIFF, BRISTOL, YORK, KENT, OXFORD, SANDFORD, CANTERBURY, LONDON};
enum roomTypes {MID_ROOM, START_ROOM, END_ROOM};

struct Room {
	enum nameList name;
	int totalConnections;
	int usedConnections;
	struct Room* connections[6];
	enum roomTypes type;
}

struct Room makeRoom(int roomTypesUsed){
	struct Room room;
	int randNum;	//to hold randomly generated numbers
	room.name = rand() % 10;
	room.totalConnections = (rand() % 4) + 3;
	room.usedConnections = 0;
	room.type = MID_ROOM;
	return room;
}

void makeConnections(struct Room* rooms[], int numRooms){
	int randNum;	//to hold randomly generated numbers
	int i, j;
	for (i=0; i<numRooms; i++){
		while (rooms[i]->usedConnections < rooms[i]->totalConnections){
		for (j=rooms[i]->usedConnections; j<rooms[i]->totalConnections; j++){
			randNum = rand() % numRooms;
			while (randNum == i && rooms[randNum]->usedConnections < rooms[randNum]->totalConnections)
				randNum = rand() % numRooms;
			rooms[i]->usedConnections++;
			rooms[randNum}->usedConnections++;
			rooms[i]->connections[rooms[i]->usedConnections] = rooms[randNum];
			rooms[randNum]->connections[rooms[randNum]->usedConnections] = rooms[i];
		}
	}
}

int main(void){
	time_t t;
	char fileLocation[], input[];
	srand((unsigned) time(&t));
	int numRooms = rand() % 8;
	struct Room* rooms[numRooms+3];
	rooms[0] = &makeRoom();
	rooms[0]->type = START_ROOM;
	rooms[1] = &makeRoom();
	rooms[1]->type = END_ROOM;
	int i, j;
	for (i=2; i<(numRooms+3); i++){
		rooms[i] = &makeRoom();
	}
	fileLocation = strcat(strcat("pettinis.rooms.",getPID()),"/room");
	makeConnections(rooms, numRooms);
	FILE *file;
	int file_descriptor;
	for (i=1; i<=numRooms+3; i++){
		file = fopen(strcat(strcat(fileLocation, i),".txt"),"w");
		input = strcat("ROOM NAME: ",rooms[i]->name);
		fputs(input, file);
		fputs("\n", file);
		
		for(j=1; j<=rooms[i]->totalConnections; j++){
			sprintf(input, "CONNECTION %d: ",j);
			strcat(input,rooms[i]->connections[j]->name);
			fputs(input, file);
			fputs("\n", file);
		}
		
		input = strcat("ROOM TYPE: ",rooms[i]->type);
		fputs(input, file);
		fputs("\n", file);
		fclose(file);
	}
}