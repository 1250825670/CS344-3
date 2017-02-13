/* Sammy Pettinichi
 * CS344 - Operating Systems
 * Block 2 - Adventure Game
 * pettinis.buildrooms.c
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>

//statis element to hold all room names
char *roomNames[10] = {"Edinburgh","Cornwall","Cardiff","Bristol","York","Kent","Oxford","Sandford","Canterbury","London"};

struct Room {	//Room struct to hold all room information
	char name[15];
	int totalConnections;
	int usedConnections[6];
	struct Room* connections[6];
	char connectionNames[6][15];
	char type[15];
};

char* getName(int* usedRooms){
	int randNum = rand() % 10;
	while(1){	//loops indefinitely
		if(usedRooms[randNum] == 0){	//returns room name if unused so far
			usedRooms[randNum] = 1;	//sets room as used
			return roomNames[randNum];
		}
		randNum = rand() % 10;	//generates new number if room already used
	}
}

struct Room* makeRoom(int* usedRooms){
	int i,used=0;
	for(i=0;i<10;i++){	//checks how many rooms have been created
		if(usedRooms[i] == 1)
			used++;
	}
	char* startroom = "START_ROOM\0";
	char* endroom = "END_ROOM\0";
	char* midroom = "MID_ROOM\0";
	struct Room *room = (struct Room *) malloc(sizeof(struct Room));	//allocates heap space
	sprintf(room->name,"%s",getName(usedRooms));	//gets a name at random
	room->totalConnections = 0;
	for(i=0;i<6;i++)	//sets all connections to null
		room->connections[i] = 0;
	if(used == 0)	//sets type as start room if first created
		sprintf(room->type,"%s",startroom);
	else if(used == 1)	//sets type as end room if second created
		sprintf(room->type,"%s",endroom);
	else	//sets the rest as mid rooms
		sprintf(room->type,"%s",midroom);
	return room;
}

void makeConnections(struct Room* rooms[], int numRooms){
	int i,j,k,randNum,randSlot,nameSlot,connectSlot,exists, used;
	for(i=0;i<numRooms;i++){	//loops through all rooms
		randNum = (rand() % 4) + 3;
		for(j=rooms[i]->totalConnections;j<randNum;j++){	//checks how many links have been made so far
			randSlot = rand() % 10;
			exists = 0;
			used = 0;
			for(k=0;k<10;k++){
				if(strcmp(roomNames[k],rooms[i]->name)==0){	//finds which roomName slot the current room is
						nameSlot = k;
				}
			}
			while(exists == 0 || used == 1){
				for(k=0;k<numRooms;k++){
					if(strcmp(roomNames[randSlot],rooms[k]->name)==0 && randSlot != nameSlot){	//ensures sought room has been created and isn't the current room
						exists = 1;
						connectSlot = k;	//finds which roomName slot the linked room is
					}
				}
				for(k=0;k<rooms[i]->totalConnections;k++){
					if(randSlot == rooms[i]->usedConnections[k]){	//checks if this connection has already been used
						used = 1;
					}
				}
				if(used == 1 || exists == 0){	//if this room doesn't exist or was already used
					randSlot++;	//increment it one and check the next r oom
					exists = 0;	//reset trackers
					used = 0;
					if(randSlot==10){	//if it reaches the end of the list restart at the beginning
						randSlot -= 10;
					}
				}
			}
			rooms[i]->usedConnections[rooms[i]->totalConnections] = randSlot;	//save the connection slot of the connection
			rooms[connectSlot]->usedConnections[rooms[connectSlot]->totalConnections] = nameSlot;	//connect back to the current room
			rooms[i]->totalConnections++;	//increment how many connections used on both rooms
			rooms[connectSlot]->totalConnections++;
		}
	}
}

int main(void){
	int numRooms = 7;
	int usedRooms[10];	//variable to hold which rooms have been used
	int i, j;
	for(i=0;i<10;i++)	//sets all rooms to unused
		usedRooms[i] = 0;
	time_t t;
	pid_t getpid(void);
	srand((unsigned) time(&t));	//seeds rand with the time
	struct Room *rooms[numRooms];	//creates array of Room structs
	for (i=0; i<numRooms; i++){	//creates 7 rooms
		rooms[i] = makeRoom(usedRooms);
	}
	makeConnections(rooms, numRooms);	//connects rooms
	char directory[100];
	sprintf(directory, "pettinis.rooms.%d",getpid());	//gets directory name based on PID
	mkdir(directory,S_IRWXU | S_IRWXG | S_IRWXO);	//creates directory
	FILE *file;
	for (i=0; i<numRooms; i++){		//saves room files
		char fileLocation[100];
		sprintf(fileLocation,"%s/%s_room",directory,rooms[i]->name);	//creates location inside directory
		file = fopen(fileLocation,"w");	//opens file
		if(file == 0){
			printf("file open failed\n");	//exits if file failed to open
			exit(1);
		}
		char input1[100] = "ROOM NAME: ";
		strcat(input1,rooms[i]->name);
		strcat(input1,"\n");
		fprintf(file,input1);	//saves room name
		for(j=0; j<rooms[i]->totalConnections; j++){	//loops through all connections
			char input2[100];
			sprintf(input2, "CONNECTION %d: ",j+1);
			strcat(input2,roomNames[rooms[i]->usedConnections[j]]);
			strcat(input2,"\n");
			fprintf(file,input2);	//saves the connection
		}
		char input3[100] = "ROOM TYPE: ";
		strcat(input3,rooms[i]->type);
		strcat(input3,"\n");
		fprintf(file,input3);	//saves room type
		fclose(file);
	}
	for (i=0; i<numRooms; i++){	//frees all the Room structs
		free(rooms[i]);
		rooms[i]=0;	//sets pointer to null
	}
}
