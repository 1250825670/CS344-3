//sstring to parse fgets data
//use read/write for re-entrance?

/* Sammy Pettinichi
 * CS344 - Operating Systems
 * Project 3 - smallsh
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

//define constants
#define MAX_INPUT 2048
#define MAX_COMMANDS 512

//function declarations
void print(char* text);
char** getInput(int* background, int* counter);
char** parseInput(char input[], int* counter);
void executeCommand(char** commandArguments, int* exitVal, int* background, int* counter);
void checkChildren();

//checks for background process completion
void checkChildren(){
  char printOutput[MAX_INPUT];
  int childExitMethod = -5, childPID = -5;
  int exitStatus;
  
  do{
    childPID = waitpid(-1,&childExitMethod,0);
    if(childPID > 0){
      if(WIFEXITED(childExitMethod) != 0){
        exitStatus = WEXITSTATUS(childExitMethod);
        memset(printOutput,'\0',sizeof(printOutput));
        sprintf(printOutput,"background pid %d is done: exit value %d\n",childPID,exitStatus);
        print(printOutput);
      }
      else if(WIFSIGNALED(childExitMethod){
        exitStatus = WTERMSIG(childExitMethod);
        memset(printOutput,'\0',sizeof(printOutput));
        sprintf(printOutput,"background pid %d is done: terminated by signal %d\n",childPID,exitStatus);
        print(printOutput);
      }
    }
  }while(childPID != 0);
}

//execute the user's command
void executeCommand(char** commandArguments, int* exitVal, int* background, int* counter){  
  char printOutput[MAX_INPUT];
  int i, childExitMethod = -5, waitResult;
  pid_t childPID = -5;
  int outSlot=0,inSlot=0,inFile=-1,outFile=-1;
  memset(printOutput,'\0',sizeof(printOutput));
  if(strcmp(commandArguments[0],"status")==0){
    if(*exitVal == 0 || *exitVal == 1){
      memset(printOutput,'\0',sizeof(printOutput));
      sprintf(printOutput,"exit value %d\n",*exitVal);
      print(printOutput);
    }
    else{
      memset(printOutput,'\0',sizeof(printOutput));
      sprintf(printOutput,"terminated by signal %d\n",*exitVal);
      print(printOutput);
    }
    *exitVal = 0;
  }
  else if(strcmp(commandArguments[0],"cd")==0){
    if(commandArguments[1] == 0)
      chdir(getenv("HOME"));
    else
      chdir(commandArguments[1]);
    *exitVal = 0;
  }
  else if(*counter > 1){
    for(i=0;i<*counter;i++){
      if(strcmp(commandArguments[i],"<")==0){
        inSlot = i;
      }
      else if(strcmp(commandArguments[i],">")==0){
        outSlot = i;
      }
    }
    if(inSlot != 0){
      inFile = open(commandArugments[inSlot+1],O_RDONLY);
      if(inFile == -1){
        memset(printOutput,'\0',sizeof(printOutput));
        spritnf(printOutput,"cannot open %s for input\n",commandArguments[inSlot+1]);
        print(printOutput);
        *exitVal = 1;
      }
    }
    if(outSlot != 0){
      outFile = open(commandArguments[outSlot+1],O_CREAT);
      if(outFile == -1){
        memset(printOutput,'\0',sizeof(printOutput));
        spritnf(printOutput,"cannot open %s for output\n",commandArguments[outSlot+1]);
        print(printOutput);
        *exitVal = 1;
      }
    }
  }
  
  childPID = fork();
  if(childPID == -1){
    memset(printOutput,'\0',sizeof(printOutput));
    sprintf(printOutput,"fork failed\n");
    print(printOutput);
    exit(1);
    break;
  }
  else if(childPID == 0){
    int minSlot = 0;
    if(outSlot != 0){
      dup2(outFile,1);
      min = outSlot;
    }
    if(inSlot != 0){
      dup2(inFile,0);
      min = inSlot;
    }
    if(min == 0){
      min = *counter;
    }
    commandArguments[min] = NULL;
    
    if(execvp(commandArguments[0],commandArguments) < 0){
      memset(printOutput,'\0',sizeof(printOutput));
      sprintf(printOutput,"%s: no such file or directory\n",commandArguments[0]);
      print(printOutput);
      _exit(1);
    }
  }
  else{
    if(inSlot != 0)
      close(inFile);
    if(outSlot != 0)
      close(outFile);
    if(*background == 0){
      waitResult = waitpid(childPID,&childExitMethod,0);
    }
    else{
      memset(printOutput,'\0',sizeof(printOutput));
      sprintf(printOutput,"background pid is %d\n",childPID);
      print(printOutput);
    }
  }
}

int main(int argc, char *argv[]){
  char** commandArguments;
  int exitVal,background,counter;
  do{
    background = 0;
    counter = 0;
    commandArguments = getInput(input, &background,&counter);
    
    if(counter != 0 && strcmp(commandArguments[0],"exit") != 0){
      executeCommand(commandArguments, &exitVal, &background,&counter);
    }
    free(commandArguments);
    checkChildren();
  }while(strcmp(commandArguments[0],"exit") != 0);
}

//request command input from the user
char** getInput(int* background, int* counter){
  char input[MAX_INPUT];
  memset(intput,'\0',sizeof(input));
  print(": ");
  fflush(stdin);
  read(0,input,sizeof(input));
  fgets(input, MAX_INPUT, stdin)
  
  return parseInput(input,background,counter);
}

//parse the input into an array for easier access
char** parseInput(char input[], int* background, int* counter){
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
  token = strtok(command, " ");
  while(token != NULL){
    if(strcmp(token,"&&")==0){
      strcpy(token,pid());
    }
    strcpy(commandArguments[counter],token);
    *counter++;
    token = strtok(NULL, " ");
  }
  if(strcmp(commandArugments[*counter-1],"&")==0){
    *background = 1;
    strcpy(commandArguments[*counter-1],"\0");
  }
  return commandArguments;
}

//write codes: 0=stdin, 1=stdout, 2=stderr
void print(char* text){
  write(1,text,sizeof(text));
  fflush(stdout);
}
