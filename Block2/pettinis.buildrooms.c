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
	struct Room* connection1;
	struct Room* connection2;
	struct Room* connection3;
	struct Room* connection4;
	struct Room* connection5;
	struct Room* connection6;
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
			rooms[i]->connection(rooms[i]->usedConnections) = rooms[randNum];
			rooms[randNum]->connection(rooms[randNum}->usedConnections) = rooms[i];
		}
	}
}

int main(void){
	time_t t;
	srand((unsigned) time(&t));
	int numRooms = rand() % 8;
	struct Room* rooms[numRooms+3];
	rooms[0] = &makeRoom();
	rooms[0]->type = START_ROOM;
	rooms[1] = &makeRoom();
	rooms[1]->type = END_ROOM;
	int i;
	for (i=2; i<(numRooms+3); i++){
		rooms[i] = &makeRoom();
	}
	makeConnections(rooms, numRooms);
	int PID = getpid();	//get current process ID
	int file_descriptor;
	for (i=1; i<=numRooms; i++){
		file_descriptor = open(strcat(strcat("pettinis.rooms.",PID),strcat(strcat("/room",i),".txt")));
		if ( file_descriptor == -1)
			exit(1);
		
		close(file_descriptor);
	}
}