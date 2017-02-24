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
#include <readline/readline.h>
#include <readline/history.h>

//define constants
#define MAX_INPUT 2048
#define MAX_COMMANDS 512

//declare global variables
int backgroundAllowed;
int parentPID;

//struct to hold the user's input history
struct CommandHistory {
	char** commandList;
	int size;
	int maxSize;
};

//function declarations
void print(char* text);
char** getInput(int* background, int* counter, struct CommandHistory *commHist);
char** parseInput(char input[], int* background, int* counter);
void executeCommand(char** commandArguments, int* exitVal, int* background, int* counter);
void freeAll(char** commandArguments,int *counter);
void checkChildren();
void catchSigInt(int sigNum);
void catchSigTSTP(int sigNum);
void catchSigTerm(int sigNum);
void quitShell(struct CommandHistory *commHist);
struct CommandHistory* initializeHistory();
void addToHistory(struct CommandHistory *commHist);

//reserves memory for the struct and initializes the size and maxsize
struct CommandHistory* initializeHistory(){
	struct CommandHistory *commandHistory = malloc(sizeof(struct CommandHistory));
	commandHistory->commandList = malloc(sizeof(char *) * MAX_COMMANDS);
	commandHistory->size = 0;
	commandHistory->maxSize = MAX_COMMANDS;
	return commandHistory;
}
//outputs command history to .smallsh_history then frees memory used by history
void quitShell(struct CommandHistory *commHist){
	int i;
	int hist_file = open(".smallsh_history", O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);	//open history file
	for(i=0;i<commHist->size;i++){	//free history
		write(hist_file,commHist->commandList[i],strlen(commHist->commandList[i]));
		free(commHist->commandList[i]);
	}
	close(hist_file);	//close file
	free(commHist->commandList);	//free history listing
	free(commHist);	//free struct
	exit(0);
}

//loops through all commands until user chooses to exit
int main(int argc, char *argv[]){
	char** commandArguments;
	int exitVal,background,counter;
	backgroundAllowed = 1;
	parentPID = getpid();
	struct CommandHistory *commHist = initializeHistory();
	
	//declare signal handlers
	struct sigaction SIGINT_action = {0}, SIGTSTP_action = {0}, SIGTERM_action = {0}, ignore_action = {0};
	//sigint handling
	SIGINT_action.sa_handler = catchSigInt;
	sigfillset(&SIGINT_action.sa_mask);
	SIGINT_action.sa_flags = SA_RESTART;
	//sigtstp handling
	SIGTSTP_action.sa_handler = catchSigTSTP;
	sigfillset(&SIGTSTP_action.sa_mask);
	SIGTSTP_action.sa_flags = SA_RESTART;
	//sigterm handling
	SIGTERM_action.sa_handler = catchSigTerm;
	sigfillset(&SIGTERM_action.sa_mask);
	SIGTERM_action.sa_flags = SA_RESTART;
	//signal ignore
	ignore_action.sa_handler = SIG_IGN;
	//initialize
	sigaction(SIGINT,&SIGINT_action,NULL);
	sigaction(SIGTSTP,&SIGTSTP_action,NULL);
	sigaction(SIGTERM,&SIGTERM_action,NULL);
	sigaction(SIGHUP,&ignore_action,NULL);
	sigaction(SIGQUIT,&ignore_action,NULL);
	
	while(1){
		background = 0;	//resets whether its a background command
		counter = 0;	//resets counter for input length
		commandArguments = getInput(&background,&counter,commHist);	//gets input from user and parses into an array
		if(counter > 0 && strcmp(commandArguments[0],"exit") == 0){	//checks if user wants to exit
			print("exit\n");
			quitShell(commHist);
			//exit(0);
		}
		else if(counter != 0){	//otherwise execute returned command
			executeCommand(commandArguments, &exitVal, &background,&counter);
		}
		checkChildren();	//check if any children have completed
		freeAll(commandArguments,&counter);	//free memory where last input was stored
	}
}

//checks for background process completion
void checkChildren(){
	char printOutput[MAX_INPUT];
	int childExitMethod = -5, childPID = -5;
	int exitStatus;
	
	while((childPID = waitpid(-1,&childExitMethod,WNOHANG)) > 0){	//loops until waitpid returns with nothing
		if(WIFEXITED(childExitMethod)){	//checks if child succeeded
			exitStatus = WEXITSTATUS(childExitMethod);
			memset(printOutput,'\0',sizeof(printOutput));
			sprintf(printOutput,"background pid %d is done: exit value %d\n",childPID,exitStatus);	//informs user of exit and status
			print(printOutput);
		}
		else if(WIFSIGNALED(childExitMethod)){	//checks if child was terminated
			exitStatus = WTERMSIG(childExitMethod);
			memset(printOutput,'\0',sizeof(printOutput));
			sprintf(printOutput,"background pid %d is done: terminated by signal %d\n",childPID,exitStatus); //informs user of termination
			print(printOutput);
		}
	}
}

//execute the user's command
void executeCommand(char** commandArguments, int* exitVal, int* background, int* counter){  
	char printOutput[MAX_INPUT];	//declares output buffer
	int i, childExitMethod = -5;
	pid_t childPID = -5;
	int outSlot=0,inSlot=0,inFile=-1,outFile=-1;
	memset(printOutput,'\0',sizeof(printOutput));
	if(strcmp(commandArguments[0],"status")==0){	//checks if command was for the status
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
	else if(strcmp(commandArguments[0],"cd")==0){	//checks if command is to change directory
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
	else if(*counter > 1){	//otherwise runs various commands
		for(i=0;i<*counter;i++){	//checks if stdin or stdout must be redirected
			if(strcmp(commandArguments[i],"<")==0){
				inSlot = i;
			}
			else if(strcmp(commandArguments[i],">")==0){
				outSlot = i;
			}
		}
		if(inSlot != 0){	//if there is stdin redirection required
			inFile = open(commandArguments[inSlot+1],O_RDONLY);	//opens file
			if(inFile == -1){
				memset(printOutput,'\0',sizeof(printOutput));
				sprintf(printOutput,"cannot open %s for input\n",commandArguments[inSlot+1]);
				print(printOutput);
				*exitVal = 1;
				return;
			}
		}
		if(outSlot != 0){	//if there is stdout redirection required
			outFile = open(commandArguments[outSlot+1],O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IXUSR);	//opens file
			if(outFile == -1){
				memset(printOutput,'\0',sizeof(printOutput));
				sprintf(printOutput,"cannot open %s for output\n",commandArguments[outSlot+1]);
				print(printOutput);
				*exitVal = 1;
				return;
			}
		}
	}
	childPID = fork();	//forks off child
	if(childPID == -1){	//if fork errors
		memset(printOutput,'\0',sizeof(printOutput));
		sprintf(printOutput,"fork failed\n");
		print(printOutput);
		exit(1);
	}
	else if(childPID == 0){	//if this is the child
		int dev_null = open("/dev/null",O_RDWR);	//opens /dev/null for read and write
		int min = 0;
		if(outSlot != 0){
			dup2(outFile,1);	//redirects stdout to the output file
			min = outSlot;	//sets how far to read in array
		}
		else if(*background == 1 && backgroundAllowed == 1){
			dup2(dev_null,1);	//redirects stdout to /dev/null if the process is backgrounded
		}
		if(inSlot != 0){
			dup2(inFile,0);	//redirects stdin to the input file
			min = inSlot;	//sets earlier if both stdin and stdout are redirected
		}
		else if(*background == 1 && backgroundAllowed == 1){
			dup2(dev_null,0);	//redirects stdin to /dev/null if the process is backgrounded
		}
		if(min == 0){
			min = *counter;	//sets to read while input if no redirection
		}
		commandArguments[min] = NULL;	//sets null to stop reading at this point once passed
		
		if(execvp(commandArguments[0],commandArguments) < 0){	//runs command
		  memset(printOutput,'\0',sizeof(printOutput));
		  sprintf(printOutput,"%s: no such file or directory\n",commandArguments[0]);	//informs user if execvp errors 
		  print(printOutput);
		  _exit(1);
		}
	}
	else{	//if it is the parent
		if(inSlot != 0)
			close(inFile);	//close the input file if opened
		if(outSlot != 0)
			close(outFile);	//close the output file if opened
		if(*background == 0 || backgroundAllowed == 0){	//if process is not background or background is blocked
			childPID = waitpid(childPID,&childExitMethod,0);	//wait for completion
			if(WIFEXITED(childExitMethod)){
				*exitVal = WEXITSTATUS(childExitMethod);	//set exit status if success
			}
			else if(WIFSIGNALED(childExitMethod)){
				*exitVal = WTERMSIG(childExitMethod);	//set termination status if error
			}
		}
		else{	//if backgrounded
			memset(printOutput,'\0',sizeof(printOutput));
			sprintf(printOutput,"background pid is %d\n",childPID);	//print backgrounded and childPID
			print(printOutput);
		}
	}
}

void advancedParsing(char* input){
	int count = 0;
	char c;
	while(count < MAX_INPUT){
		//c = getchar();
		c = fgetc(stdin);
		write(1,&c,1);
		if(c == EOF || c == '\n')
			break;
		if(c == 27){
			//c = getchar();
			c = fgetc(stdin);
			if(c == 91){
				//c = getchar();
				c = fgetc(stdin);
				if(c == 65 && histSlot > 0){
					histSlot--;
					fputs(commHist->commandList[histSlot],stdin);
					/*for(i=0;i<strlen(commHist->commandList[histSlot]);i++){
						fputc(commHist->commandList[histSlot][i],stdin);
					}*/
				}
				else if(c == 66 && histSlot < commHist->size){
					histSlot++;
					if(histSlot == commHist->size - 1)
						fputs(input,stdin);
					else
						fputs(commHist->commandList[histSlot],stdin);
					/*for(i=0;i<strlen(commHist->commandList[histSlot]);i++){
						fputc(commHist->commandList[histSlot][i],stdin);
					}*/
				}
			}
		}
		else if(c == 8){
			count--;
			input[count] = '\0';
			fputs(input,stdin);
		}
		else{
			input[count] = c;
			count++;
		}
	}
	input[count] = '\0';
}

//request command input from the user
char** getInput(int* background, int* counter, struct CommandHistory *commHist){
	char input[MAX_INPUT];
	memset(input,'\0',sizeof(input));
	int i, histSlot = commHist->size;	//sets history slot to begin at the commandList size
	print(": ");	//prints colon to indicate awaiting user input
	fflush(stdout);
	fgets(input,sizeof(input),stdin);	//read user input from stdin
	
	/*char* input;	//implementing readline()
	input = readline(": ");
	add_history(input);*/
	
	//advancedParsing(input);
	
	if(commHist->size == commHist->maxSize){	//if the size is as big as maxsize, reallocate more memory
		commHist->maxSize = commHist->maxSize * 2;
		commHist = realloc(commHist, commHist->maxSize);
	}
	commHist->commandList[commHist->size] = (char *)malloc(sizeof(char *) * strlen(input));	//allocates space for command
	strcpy(commHist->commandList[commHist->size],input);	//copies input to history
	commHist->size++;	//increments the size
	return parseInput(input,background,counter);	//parses input and returns the command array
}

//parse the input into an array for easier access
char** parseInput(char* input, int* background, int* counter){
	char** commandArguments = malloc(sizeof(char *) * MAX_COMMANDS);	//create array to hold all commands
	char command[MAX_INPUT];
	char* token;
	memset(commandArguments,'\0',sizeof(commandArguments));
	memset(command,'\0',sizeof(command));
	char *c = input;
	if(*c == '#' || *c == ' ' || *c == '\t' || *c == '\0' || strlen(input) == 0){	//checks if input is a comment or blank space
		return commandArguments;
	}
	strcpy(command, input);	//copies input to be chopped up my strtok
	token = strtok(command, " \n");	//loops for space or newline
	c = NULL;
	while(token != NULL){	//loops until the entire input has been parsed
		commandArguments[*counter] = (char *)malloc(sizeof(token)+1);	//allocates space for this segment
		if(strcmp(token,"$$")==0){	//if $$ is a whole element expand it into the correct PID and replace string
			sprintf(commandArguments[*counter],"%d",getpid());
		}
		else if((c = strstr(token,"$$")) != NULL){	//if $$ is part of a bigger string
			char temp[100];
			int cnt = 0;
			memset(temp,'\0',sizeof(temp));
			char *ch;
			for(ch=token; ch != c; ch++){	//finds where $$ begins in substring
				temp[cnt] = *ch;
				cnt++;
			}
			sprintf(commandArguments[*counter],"%s%d",temp,getpid());	//prints PID to string
			ch++;
			ch++;
			cnt = 0;
			memset(temp,'\0',sizeof(temp));
			while(*ch != 0){	//adds the rest of the string
				temp[cnt] = *ch;
				cnt++;
				ch++;
			}
			strcat(commandArguments[*counter],temp);	//adds to list
		}
		else
			strcpy(commandArguments[*counter],token);	//if no $$ then simply copy
		*counter = *counter + 1;	//increment counter
		token = strtok(NULL, " \n");	//looks for space or newline
	}
	if(strcmp(commandArguments[*counter-1],"&") == 0){	//checks if command ends with &
		*background = 1;	//sets background for true
		strcpy(commandArguments[*counter-1],"\0");	//replaces & character with null to not parse later
		*counter = *counter - 1;	//decrements counter
		
	}
	free(input);
	return commandArguments;	//returns command array
}

//function to cleanly print text and flush stdout
void print(char* text){
	fflush(stdout);
	printf(text);
	fflush(stdout);
}

//frees the command argument array
void freeAll(char** commandArguments,int *counter){
	int i;
	for(i=0;i<*counter;i++)	//loops through all arguments
		free(commandArguments[i]);
	free(commandArguments);
}

//write codes: 0=stdin, 1=stdout, 2=stderr
void catchSigInt(int sigNum){
	int pid = getpid();
	if(pid != parentPID){	//only kills the child processes
		exit(sigNum);	//exits with signal number
	}
	write(1,"\n: ",3);
	fflush(stdout);
}
void catchSigTerm(int sigNum){
	int pid = getpid();
	char* message1 = "\nbackground pid ";
	char* message2 = "is done: terminated by signal ";
	if(pid != parentPID){	//checks if it is parent or child, and terminates only if child
		write(1,message1,16);
		write(1,&pid,sizeof(int));
		write(1,message2,30);
		write(1,&sigNum,sizeof(int));
		write(1,"\n: ",3);
		exit(sigNum);	//exits with signal number
	}
}
void catchSigTSTP(int sigNum){	//toggles allowing background processes
	char* entering = "\nEntering foreground-only mode (& is now ignored)\n";
	char* exiting = "\nExiting foreground-only mode\n";
	if(backgroundAllowed == 1){	//if background is currently allowed, block it
		backgroundAllowed = 0;
		write(1,entering,50);
	}
	else{	//if background is currently blocked, allow it
		backgroundAllowed = 1;
		write(1,exiting,30);
	}
	fflush(stdout);
	write(1,": ",2);
	fflush(stdout);
}
