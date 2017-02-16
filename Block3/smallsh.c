#include <stdio.h>
#include <stdlib.h>

#define MAX_INPUT 2048

void print(char* text);
char* getInput();
void getFirstWord(char firstWord[], char* input);
runStatus(char* input);
runCD(char* input);
runMisc(char* input);

runMisc(char* input){
  
  else
    print("unknown command\n");
}

runStatus(char* input){
  
}

runCD(char* input){
  
}

char* getInput(){
  char input[MAX_INPUT];
  memset(intput,'\0',sizeof(input));
  print(": ");
  fgets(input, MAX_INPUT, stdin)
  
  char *c = input;
  int spaces=0;
  while (*c == ' ') || (*c == '\t'){
    c++;
    spaces++;
  }
  c = input;
  if(*c == '#' || *c == ' ' || c == '\t' || sizeof(input) == 0){
    return getInput();
  }
  return input;
}

int main(int argc, char *argv[]){
char firstWord[50];
char input[MAX_INPUT];
  do{
    memset(intput,'\0',sizeof(input));
    input = getInput();
    getFirstWord(firstWord, input);
    
    if(strcmp(firstWord,"status")==0){
      runStatus(input);
    }
    else if(strcmp(firstWord,"cd")==0){
      runCD(input);
    }
    else{
      runMisc(input);
    }
  }while(strcmp(input,"exit") != 0);
}

void getFirstWord(char firstWord[], char* input){
  memset(firstWord,'\0',sizeof(firstWord));
  int counter=0;
  char *c = input;
  while(c != ' '){
    firstWord[counter] = *c;
    c++;
    counter++;
  }
}

void print(char* text){
  fprintf(stdout, text);
  fflush();
}
