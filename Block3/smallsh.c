//ssting to parse fgets data

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

#define MAX_INPUT 2048
#define MAX_COMMANDS 512

void print(char* text);
char** getInput();
char** parseInput(char input[]);
void executeCommand(char** commandArguments);
void runStatus(char input[]);
void runCD(char* input);
void runMisc(char* input);

void executeCommand(char** commandArguments){
  if(strcmp(commandArguments[0],"status")==0){
    runStatus(commandArguments);
  }
  else if(strcmp(commandArguments[0],"cd")==0){
    runCD(commandArguments);
  }
  else{
    runMisc(commadArguments); 
  }
}
void runMisc(char* input){
  //exec() family of commands
  else
    print("unknown command\n");
}

void runStatus(char* input){
  
}

void runCD(char* input){
  
}

int main(int argc, char *argv[]){
  do{
    char** commandArguments = getInput(input);
    
    if(commandArguments[0] != 0 && strcmp(commandArguments[0],"exit") != 0){
      executeCommand(commandArguments);
    }
  }while(strcmp(commandArguments[0],"exit") != 0);
}

char** getInput(){
  char input[MAX_INPUT];
  memset(intput,'\0',sizeof(input));
  print("smallsh: ");
  fgets(input, MAX_INPUT, stdin)
  
  return parseInput(input);
}

char** parseInput(char input[]){
  int counter, i;
  char** commandArguments = malloc(sizeof(char *) * MAX_COMMANDS);
  char command[MAX_INPUT];
  char* token;
  memset(commandArguments,'\0',sizeof(commandArguments));
  memset(command,'\0',sizeof(command));
  char *c = input;
  if(*c == '#' || *c == ' ' || c == '\t' || strlen(input) == 0){
    commandArguments[0] = 0;
    return commandArguments;
  }
  strcpy(command, input);
  counter = 0;
  token = strtok(command, " ");
  while(token != NULL){
    strcpy(commandArguments[counter],token);
    counter++;
    token = strtok(NULL, " ");
  }
  for(i=1; i<strlen(input); i++){
   
  }
  return commandArguments;
}

void print(char* text){
  fprintf(stdout, text);
  fflush();
}
