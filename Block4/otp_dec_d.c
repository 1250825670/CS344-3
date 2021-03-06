#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

#define MAX_LENGTH 70005

void decryptMessage(int);
void error(const char *msg) { perror(msg); exit(1); } // Error function used for reporting issues

int main(int argc, char *argv[]){
	int listenSocketFD, establishedConnectionFD, portNumber, charsRead, childPID, childExitMethod;
	socklen_t sizeOfClientInfo;
	struct sockaddr_in serverAddress, clientAddress;
	
	if (argc < 2) { fprintf(stderr,"USAGE: %s port\n", argv[0]); exit(1); } // Check usage & args
	
	// Set up the address struct for this process (the server)
	memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[1]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverAddress.sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process
	
	// Set up the socket
	listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (listenSocketFD < 0) error("ERROR opening socket");
	
	// Enable the socket to begin listening
	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to port
		error("ERROR on binding");
	listen(listenSocketFD, 5); // Flip the socket on - it can now receive up to 5 connections
	
	while(1){
		// Accept a connection, blocking if one is not available until one connects
		sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect
		establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
		if (establishedConnectionFD < 0) error("ERROR on accept");
		
		decryptMessage(establishedConnectionFD);
		while((childPID = waitpid(-1,&childExitMethod,WNOHANG)) > 0){}
	}
	
	close(listenSocketFD); // Close the listening socket
	return 0; 
}

void decryptMessage(int establishedConnectionFD){
	int childPID = fork();
	if(childPID < 0) error("fork failed");
	else if (childPID == 0){	//this is the child
		char* END = "tuna";
		int charsRead, i, textLength;
		char buffer[MAX_LENGTH], ciphertext[MAX_LENGTH], key[MAX_LENGTH], plaintext[MAX_LENGTH];
		memset(ciphertext, '\0', sizeof(ciphertext));
		memset(key, '\0', sizeof(key));
		memset(plaintext, '\0', sizeof(plaintext));
		
		memset(buffer, '\0', sizeof(buffer));
		charsRead = recv(establishedConnectionFD, buffer, sizeof(buffer)-1, 0); //Get the name of the connecting program
		if (charsRead < 0) error("ERROR reading from socket");
		if(strcmp(buffer,"otp_dec") == 0) send(establishedConnectionFD, "ACCEPTED", 8, 0); // Send success back if opt_enc is connecting
		else send(establishedConnectionFD, "REJECTED", 8, 0); // Send rejection back for anyone else
		
		do{
			memset(buffer, '\0', sizeof(buffer));
			charsRead = recv(establishedConnectionFD, buffer, sizeof(buffer)-1, 0); //Obtain the key
			if (charsRead < 0) error("ERROR reading from socket");
			strcat(key,buffer);
		}while(strstr(key,END)==NULL);
		charsRead = send(establishedConnectionFD, "ACK", 3, 0); // Send success back
		if (charsRead < 0) error("ERROR writing to socket");
		
		do{
			memset(buffer, '\0', sizeof(buffer));
			charsRead = recv(establishedConnectionFD, buffer, sizeof(buffer)-1, 0); //Obtain the original text
			if (charsRead < 0) error("ERROR reading from socket");
			strcat(ciphertext,buffer);
		}while(strstr(ciphertext,END)==NULL);
		textLength = charsRead-4;
		
		for(i=0;i<textLength;i++){
			int ct = ciphertext[i];
			int k = key[i];
			if(ct == 32)
				ct = 91;
			if(k == 32)
				k = 91;
			k -= 64;
			if(ct > 91 || ct < 65)
				error("ERROR invalid input");
			int dif = ct - k;
			if(dif < 65)
				dif += 27;
			if(dif == 91)
				dif = 32;
			plaintext[i] = dif;
		}
		
		strcat(plaintext,END);
		charsRead = send(establishedConnectionFD, plaintext, strlen(plaintext), 0); //Send ciphertext back
		if (charsRead < 0) error("ERROR writing to socket");
		
		_exit(0);
	}
}
