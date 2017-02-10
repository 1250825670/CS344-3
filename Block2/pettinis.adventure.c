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
	char* connectionNames[6];
	char type[25];
};
struct Map {
	struct Room* start;
	struct Room* current;
	struct Room* end;
};

void makeConnections(int numRooms, struct Room* rooms[]){
	int i,j,slot=0;
	for(i=0; i<numRooms; i++){
		for(j=i+1;j<numRooms; j++){
			slot=0;
			while(slot < rooms[i]->totalConnections){
				if(strcmp(rooms[i]->connectionNames[slot],rooms[j]->name) == 0){
					rooms[i]->connections[rooms[i]->usedConnections] = rooms[j];
					rooms[j]->connections[rooms[j]->usedConnections] = rooms[i];
					rooms[i]->usedConnections++;
					rooms[j]->usedConnections++;
				}
				slot++;
			}
		}
	}
}
void makeRooms(char* directory, int numRooms, struct Room* rooms[], struct Map* map){
	int i;
	FILE *file;
	for(i=0,i<numRooms;i++){
		struct Room* room = (struct Room*) malloc(sizeof(struct Room));
		fileLocation[100];
		sprintf(fileLocation,"%s%s_room",directory,rooms[i]->name); //INCORRECT Needs to find files
		file = fopen(fileLocation, "r");
		char buf[25];
		fgets(buf,11,file);
		fgets(buf,25,file);
		room->name = buf;
		int connections = 0;
		while(fgetc(file) == 'C'){
			fgets(buf,13,file);
			fgets(buf,25,file);
			room->connectionNames[connections] = buf;
			connections++;
		}
		room->totalConnections = connections;
		room->usedConnections = 0;
		fgets(buf,10,file);
		fgets(buf,25,file);
		room->type = buf;
		if(strcmp(room->type,"START_ROOM") == 0){
			map->start = room;
			map->current = room;
		}
		else if(strcmp(room->type,"END_ROOM") == 0){
			map->end = room;
		}
		fclose(file);
		rooms[i] = room;
	}
	makeConnections(rooms);
}

void print(struct Map map){
	printf("CURRENT LOCATION: %s\n",map->current->name);
	char* connectionList[100];
	int i;
	sprintf(connectionList, "POSSIBLE CONNECTIONS: ");
	sprintf(connectionList,map->current->connections[0]->name);
	for(i=0;i<map->current->totalConnections;i++){
		sprintf(connectionList,", ");
		sprintf(connectionList,map->current->connections[i]->name);
	}
	printf(connectionList);
	printf("Where to? >")
}

void playGame(struct Map map){
	char* path[100];
	char input[100];
	int i=0;
	int completed = 0, steps = 0;
	while(completed == 0){
		print(map);
		scanf("%s",input);
		path[steps] = input;
		steps++;
		struct Room* temp = map->current;
		for(i=0;i<map->current->totalConnections;i++){
			if(strcmp(map->current->connections[i]->name,input)==0){
				i=10;
				map->current = map->current->connections[i];
			}
		}
		if(temp == map->current){
			printf("\nHUH? I DON’T UNDERSTAND THAT ROOM. TRY AGAIN.\n\n");
		}
		else if(map->current == map->end){
			printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");
			printf("YOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n",steps);
			for(i=0;i<steps;i++){
				printf("%s\n",path[i]);
			}
		}
	}
}

int main(void){
	struct stat statVar;
	DIR *dir;
	struct dirent *current, newest;
	int temp, curr, numRooms = 7;
	char directory[100];
	dir = opendir(".");
	if(dir){
		while ((current = readdir(dir)) != NULL){
			if(strstr(current->d_name,"pettinis.rooms") != 0){
				stat(current->d_name, &statVar);
				curr = ctime(&statVar.st_mtime);
				it(curr > temp){
					newest = current;
					directory = newest->d_name;
					temp = curr;
				}
			}
			printf("%s", ctime(&statVar.st_mtime)); 
		}
	}
	closedir(dir);
	
	struct Room* rooms[numRooms];
	struct Map map;
	makeRooms(directory, numRooms, rooms, &map);
	
	playGame(map);
}
