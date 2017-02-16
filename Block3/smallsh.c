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
char** getInput(int* background);
char** parseInput(char input[]);
void executeCommand(char** commandArguments, int* exitVal, int* background);

void executeCommand(char** commandArguments, int* exitVal, int* background){  
  if(strcmp(commandArguments[0],"status")==0){
    if(*exitVal == 0 || *exitVal == 1)
      fprintf(stdout,"exit value %d\n",*exitVal);
    else
      fprintf(stdout,"terminated by signal %d\n",*exitVal);
    fflush();
    *exitVal = 0;
  }
  else if(strcmp(commandArguments[0],"cd")==0){
    if(commandArguments[1] == 0)
      chdir(getenv("HOME"));
    else
      chdir(commandArguments[1]);
    *exitVal = 0;
  }
  else{
    if(
    
    fprintf(stdout,"%s: no such file or directory\n",commandArguments[0]);
    fflush();
  }
}

int main(int argc, char *argv[]){
  char** commandArguments;
  int exitVal, background;
  do{
    background = 0;
    commandArguments = getInput(input, &background);
    
    if(commandArguments[0] != 0 && strcmp(commandArguments[0],"exit") != 0){
      executeCommand(commandArguments, &exitVal, &background);
    }
    free(commandArguments);
  }while(strcmp(commandArguments[0],"exit") != 0);
}

char** getInput(int* background){
  char input[MAX_INPUT];
  memset(intput,'\0',sizeof(input));
  print(": ");
  fgets(input, MAX_INPUT, stdin)
  
  return parseInput(input,background);
}

char** parseInput(char input[], int* background){
  int counter;
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
  if(strcmp(commandArugments[counter-1],"&")==0){
    *background = 1;
  }
  return commandArguments;
}

void print(char* text){
  fprintf(stdout, text);
  fflush();
}
