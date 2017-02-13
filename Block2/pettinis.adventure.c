/* Sammy Pettinichi
 * CS344 - Operating Systems
 * Block 2 - Adventure Game
 * pettinis.adventure.c
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <dirent.h>

//mutex for the threads
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

struct Room {	//room struct to hold all room information
	char name[15];
	int totalConnections;
	int usedConnections;
	struct Room* connections[6];
	char connectionNames[6][15];
	char type[15];
};
struct Map {	//map struct to know current, start, and finish rooms while playing
	struct Room* start;
	struct Room* current;
	struct Room* end;
};

void makeConnections(int numRooms, struct Room* rooms[]){	//links room connections
	int i,j,counter;
	for(i=0;i<numRooms;i++){	//loops through all rooms
		counter = rooms[i]->usedConnections;
		for(j=0; j < rooms[i]->totalConnections; j++){	//loops through how many connections it has
			if(strcmp(rooms[i]->connectionNames[counter],rooms[j]->name)==0){
				rooms[i]->connections[counter] = rooms[j];
				counter++;
			}
		}
		rooms[i]->usedConnections = counter;	//adds up used connections
	}
}
int checkFileExists(char *filename){	//checks if room file exists
	struct stat buf;
	return stat(filename, &buf);
}
void makeRooms(char* directory, int numRooms, struct Room* rooms[], struct Map* map){
	int i,j;
	char c;
	FILE *file;	//file pointer
	char *roomNames[10] = {"Edinburgh","Cornwall","Cardiff","Bristol","York","Kent","Oxford","Sandford","Canterbury","London"};
	int roomsUsed[10];
	for(i=0;i<10;i++)	//sets all rooms as unused
		roomsUsed[i] = 0;
	for(i=0;i<numRooms;i++){
		struct Room* room = (struct Room*) malloc(sizeof(struct Room)); //allocates heap space
		for(j=0;j<6;j++)	//sets all connections to null
			room->connections[j] = 0;
		char fileLocation[100];
		j=0;
		do{
			sprintf(fileLocation,"%s/%s_room",directory,roomNames[j]);
			j++;
		}while(checkFileExists(fileLocation)==-1 || roomsUsed[j] == 1); //loops until an unused room is found
		file = fopen(fileLocation, "r");	//opens to read
		roomsUsed[j] = 1;	//sets room used
		if(file == 0){
			printf("File open failed.\n");	//reports file failure
			exit(1);
		}
		char buf[15];
		do{
			c = fgetc(file);
		}while(c != ':');	//gets characters until colon to then read the name
		fscanf(file,"%s",buf);	//reads the name
		strcpy(room->name,buf);	//copies it to the Room struct
		int connections = 0;
		c = fgetc(file);	//advances to next line
		c = fgetc(file);
		do{
			do{
				c = fgetc(file);
			}while(c != ':');	//loops until colon to read connections
			c = fgetc(file);	//clears space
			fscanf(file,"%s",buf);	//reads name
			sprintf(room->connectionNames[connections],"%s",buf);	//saves it as a connection name
			connections++;
			c = fgetc(file);	//advances to next line
			c = fgetc(file);	//gets first char of new line
		}while(c == 'C');	//continues while each new line begins with C for Connections
		room->totalConnections = connections;	//sets totalConnections
		room->usedConnections = 0;
		do{
			c = fgetc(file);
		}while(c != ':');	//loops until colon to get room type
		c = fgetc(file);	//clears space
		fscanf(file,"%s",buf);	//gets room type
		sprintf(room->type,"%s",buf);	//saves room type
		if(strcmp(buf,"START_ROOM") == 0){	//sets start and end rooms for the map
			map->start = room;
			map->current = room;
		}
		else if(strcmp(buf,"END_ROOM") == 0){
			map->end = room;
		}
		fclose(file);	//closes file
		rooms[i] = room;	//saves room to Room array
	}
	//makeConnections(numRooms,rooms);
}

void* printTime(){
	pthread_mutex_lock(&mutex);	//checks for mutex to be available
	FILE *file;
	file = fopen("currentTime.txt","w");	//opens curentTime.txt file
	if(file == 0){
		printf("File open failed...\n");
		exit(1);
	}
	time_t currTime;
	struct tm* timeInfo;
	char timeBuf[50];
	
	time (&currTime);	//gets current time
	timeInfo = localtime(&currTime);
	
	strftime(timeBuf,50, "%l:%M%P, %A, %B %d, %Y", timeInfo); //formats time
	fprintf(file,timeBuf);	//saves time to file
	
	fclose(file);	//closes file
	pthread_mutex_unlock(&mutex);	//unlocks mutex
}

void print(struct Map *map){
	int i;
	printf("\nCURRENT LOCATION: %s\n",map->current->name);	//prints current location name
	char connectionList[100] = "POSSIBLE CONNECTIONS: ";
	strcat(connectionList,map->current->connectionNames[0]);
	for(i=1;i<map->current->totalConnections;i++){	//lists all locations
		strcat(connectionList,", ");
		strcat(connectionList,map->current->connectionNames[i]);
	}
	printf(connectionList);
}

void* playGame(struct Map *map, struct Room* rooms[], int numRooms){
	pthread_mutex_lock(&mutex);	//locks mutex
	pthread_t timeID;
	pthread_create(&timeID,NULL,printTime,NULL);	//creats time thread
	char path[100][15];
	char input[100];
	int i=0,j;
	int completed = 0, steps = 0;
	print(map);	//prints starting location
	while(completed == 0){
		printf("\nWhere to? >");
		scanf("%s",input);	//asks user where to go
		if(strcmp(input,"time")==0){	//if user requests the time
			pthread_mutex_unlock(&mutex);	//unlocks mutex
			sleep(1);	//pauses 1 second to allow time to lock mutex
			pthread_mutex_lock(&mutex);	//waits for time to unlock the mutex to continue
			char * currTime = 0;
			size_t len = 100;
			FILE *file = fopen("currentTime.txt","r");	//opens currentTime.txt file
			if(file == 0){
				printf("File open failed...\n");
				exit(1);
			}
			getline(&currTime,&len,file);	//reads in time
			printf("\n%s\n",currTime);	//prints time
			fclose(file);	//closes file
		}
		else{	//for any other request
			struct Room* temp = map->current;
			for(i=0;i<map->current->totalConnections;i++){
				if(strcmp(input,map->current->connectionNames[i])==0){	//checks if input matches room name
					for(j=0;j<numRooms;j++){
						if(strcmp(map->current->connectionNames[i],rooms[j]->name)==0){
							map->current = rooms[j];
							break;
						}
					}
					sprintf(path[steps],"%s",input);
					steps++;
				}
			}
			if(strcmp(temp->name, map->current->name) == 0){	//if temp equals current then no valid answer was provided, so loop
				printf("\nHUH? I DON’T UNDERSTAND THAT ROOM. TRY AGAIN.\n");
			}
			else if(map->current == map->end){	//if the current room is the ending, then celebrate and end the game
				printf("\nYOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");
				printf("YOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n",steps);	//print total steps
				for(i=0;i<steps;i++){	//print path list
					printf("%s\n",path[i]);
				}
				pthread_cancel(timeID);	//end time thread
				break;
			}
			print(map);	//print map to loop
		}
	}
}

int main(void){
	pthread_t gameThread;
	struct stat statVar;
	DIR *dir;
	struct dirent *current, *newest;	//to hold directory info
	int i, temp, curr, numRooms=7;
	char directory[100];
	dir = opendir(".");	//opens curren directory
	if(dir){
		while((current = readdir(dir)) != 0){
			if(strstr(current->d_name,"pettinis.rooms") != 0){	//looks for directories with this substring
				stat(current->d_name, &statVar);
				curr = *ctime(&statVar.st_mtime);
				if(curr > temp){	//checks if it is newer than the last
					newest = current;
					strcpy(directory,newest->d_name);	//copies name to directory as most current
					temp = curr;
				}
			}
		}
	}
	closedir(dir);	//closes directory
	struct Room* rooms[numRooms];
	struct Map map;
	makeRooms(directory, numRooms, rooms, &map);	//creates rooms from files
	
	pthread_create(&gameThread,NULL,playGame(&map,rooms,numRooms),NULL);	//creates game thread
	for (i=0; i<numRooms; i++){	//frees all rooms
		free(rooms[i]);
		rooms[i]=0;
	}
	return 0;
}