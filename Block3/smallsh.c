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

int backgroundAllowed;
int parentPID;

//function declarations
void print(char* text);
char** getInput(int* background, int* counter);
char** parseInput(char input[], int* background, int* counter);
void executeCommand(char** commandArguments, int* exitVal, int* background, int* counter);
void freeAll(char** commandArguments,int *counter);
void checkChildren();
void catchSigInt(int sigNum);
void catchSigTSTP(int sigNum);

int main(int argc, char *argv[]){
	char** commandArguments;
	int exitVal,background,counter;
	backgroundAllowed = 1;
	parentPID = getpid();
	
	struct sigaction SIGINT_action = {0}, SIGTSTP_action = {0}, ignore_action = {0};
	SIGINT_action.sa_handler = catchSigInt;
	sigfillset(&SIGINT_action.sa_mask);
	//sigaddset(&SIGINT_action.sa_mask,SIGINT);
	SIGINT_action.sa_flags = 0;
	SIGTSTP_action.sa_handler = catchSigTSTP;
	sigfillset(&SIGTSTP_action.sa_mask);
	//sigaddset(&SIGTSTP_action,SIGTSTP.sa_mask);
	SIGTSTP_action.sa_flags = 0;
	ignore_action.sa_handler = SIG_IGN;
	
	sigaction(SIGINT,&SIGINT_action,NULL);
	sigaction(SIGTSTP,&SIGTSTP_action,NULL);
	sigaction(SIGTERM,&ignore_action,NULL);
	sigaction(SIGHUP,&ignore_action,NULL);
	sigaction(SIGQUIT,&ignore_action,NULL);
	
	while(1){
		background = 0;
		counter = 0;
		commandArguments = getInput(&background,&counter);
		if(counter > 0 && strcmp(commandArguments[0],"exit") == 0){
			print("exit\n");
			exit(0);
		}
		else if(counter != 0){
			executeCommand(commandArguments, &exitVal, &background,&counter);
		}
		checkChildren();
		freeAll(commandArguments,&counter);
	}
}

//checks for background process completion
void checkChildren(){
	char printOutput[MAX_INPUT];
	int childExitMethod = -5, childPID = -5;
	int exitStatus;
	
	while((childPID = waitpid(-1,&childExitMethod,WNOHANG)) > 0){
		if(WIFEXITED(childExitMethod)){
			exitStatus = WEXITSTATUS(childExitMethod);
			memset(printOutput,'\0',sizeof(printOutput));
			sprintf(printOutput,"background pid %d is done: exit value %d\n",childPID,exitStatus);
			print(printOutput);
		}
		else if(WIFSIGNALED(childExitMethod)){
			exitStatus = WTERMSIG(childExitMethod);
			memset(printOutput,'\0',sizeof(printOutput));
			sprintf(printOutput,"background pid %d is done: terminated by signal %d\n",childPID,exitStatus);
			print(printOutput);
		}
	}
}

//execute the user's command
void executeCommand(char** commandArguments, int* exitVal, int* background, int* counter){  
	char printOutput[MAX_INPUT];
	int i, childExitMethod = -5;
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
		return;
	}
	else if(strcmp(commandArguments[0],"cd")==0){
		char newDir[MAX_INPUT];
		memset(newDir,'\0',sizeof(newDir));
		if(*counter == 1)
			strcpy(newDir,getenv("HOME"));
		else
			strcpy(newDir,commandArguments[1]);
		chdir(newDir);
		*exitVal = 0;
		return;
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
			inFile = open(commandArguments[inSlot+1],O_RDONLY);
			if(inFile == -1){
				memset(printOutput,'\0',sizeof(printOutput));
				sprintf(printOutput,"cannot open %s for input\n",commandArguments[inSlot+1]);
				print(printOutput);
				*exitVal = 1;
			}
		}
		if(outSlot != 0){
			outFile = open(commandArguments[outSlot+1],O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IXUSR);
			if(outFile == -1){
				memset(printOutput,'\0',sizeof(printOutput));
				sprintf(printOutput,"cannot open %s for output\n",commandArguments[outSlot+1]);
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
	}
	else if(childPID == 0){
		int min = 0;
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
		if(*background == 0 && backgroundAllowed == 1){
			childPID = waitpid(childPID,&childExitMethod,0);
			if(WIFEXITED(childExitMethod)){
				*exitVal = WEXITSTATUS(childExitMethod);
			}
			else if(WIFSIGNALED(childExitMethod)){
				*exitVal = WTERMSIG(childExitMethod);
			}
		}
		else{
			memset(printOutput,'\0',sizeof(printOutput));
			sprintf(printOutput,"background pid is %d\n",childPID);
			print(printOutput);
		}
	}
}

//request command input from the user
char** getInput(int* background, int* counter){
	char input[MAX_INPUT];
	memset(input,'\0',sizeof(input));
	print(": ");
	fflush(stdin);
	fgets(input,sizeof(input),stdin);
	
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
	if(*c == '#' || *c == ' ' || *c == '\t' || strlen(input) == 1){
		return commandArguments;
	}
	strcpy(command, input);
	token = strtok(command, " \n");
	c = NULL;
	while(token != NULL){
		commandArguments[*counter] = (char *)malloc(sizeof(token)+1);
		if(strcmp(token,"$$")==0){
			sprintf(commandArguments[*counter],"%d",getpid());
		}
		else if((c = strstr(token,"$$")) != NULL){
			char temp[100];
			int cnt = 0;
			memset(temp,'\0',sizeof(temp));
			char *ch;
			for(ch=token; ch != c; ch++){
			//while(ch != c){
				temp[cnt] = *ch;
				cnt++;
				//ch++;
			}
			sprintf(commandArguments[*counter],"%s%d",temp,getpid());
			ch++;
			ch++;
			cnt = 0;
			memset(temp,'\0',sizeof(temp));
			while(*ch != 0){
				temp[cnt] = *ch;
				cnt++;
				ch++;
			}
			strcat(commandArguments[*counter],temp);
		}
		else
			strcpy(commandArguments[*counter],token);
		*counter = *counter + 1;
		token = strtok(NULL, " \n");
	}
	if(strcmp(commandArguments[*counter-1],"&") == 0){
		*background = 1;
		strcpy(commandArguments[*counter-1],"\0");
	}
	return commandArguments;
}

void print(char* text){
	fflush(stdout);
	printf(text);
	fflush(stdout);
}

void freeAll(char** commandArguments,int *counter){
	int i;
	for(i=0;i<*counter;i++)
		free(commandArguments[i]);
	free(commandArguments);
}

//write codes: 0=stdin, 1=stdout, 2=stderr
void catchSigInt(int sigNum){	//kill child processes
	int pid = getpid();
	if(pid != parentPID){
		//kill(pid,SIGTERM);
		raise(SIGTERM);
	}
}
void catchSigTSTP(int sigNum){	//block background processes
	char* entering = "Entering foreground-only mode (& is now ignored)\n";
	char* exiting = "Exiting foreground-only mode\n";
	if(backgroundAllowed == 1){
		backgroundAllowed = 0;
		write(1,entering,50);
	}
	else{
		backgroundAllowed = 1;
		write(1,exiting,30);
	}
	fflush(stdout);
}
