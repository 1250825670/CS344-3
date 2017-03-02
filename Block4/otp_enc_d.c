#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

#define MAX_LENGTH 70001

void encryptMessage(int);
void error(const char *msg) { perror(msg); exit(1); } // Error function used for reporting issues

int main(int argc, char *argv[]){
	int listenSocketFD, establishedConnectionFD, portNumber, charsRead, childID, childExitMethod;
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
		
		encryptMessage(establishedConnectionFD);
		while((childPID = waitpid(-1,&childExitMethod,WNOHANG)) > 0){}
	}
	
	close(establishedConnectionFD); // Close the existing socket which is connected to the client
	close(listenSocketFD); // Close the listening socket
	return 0; 
}

void encryptMessage(int establishedConnectionFD){
	int childPID = fork();
	if(childID < 0) error("fork failed");
	else if (childID == 0){	//this is the child
		int charsRead, i, textLength;
		char buffer[MAX_LENGTH], cipher[MAX_LENGTH], key[MAX_LENGTH], plaintext[MAX_LENGTH];
		memset(buffer, '\0', sizeof(buffer));
		memset(cipher, '\0', sizeof(cipher));
		memset(key, '\0', sizeof(key));
		memset(plaintext, '\0', sizeof(plaintext));
		
		charsRead = recv(establishedConnectionFD, buffer, MAX_LENGTH-1, 0); //Get the name of the connecting program
		if (charsRead < 0) error("ERROR reading from socket");
		
		if(strcmp(buffer,"otp_enc") == 0) send(establishedConnectionFD, "ACCEPTED", 8, 0); // Send success back if opt_enc is connecting
		else send(establishedConnectionFD, "REJECTED", 8, 0); // Send rejection back for anyone else
		
		memset(buffer, '\0', sizeof(buffer));
		charsRead = recv(establishedConnectionFD, buffer, MAX_LENGTH-1, 0); //Obtain the key
		if (charsRead < 0) error("ERROR reading from socket");
		strcpy(key,buffer);
		
		charsRead = send(establishedConnectionFD, "key received", 12, 0); // Send success back
		if (charsRead < 0) error("ERROR writing to socket");
		
		memset(buffer, '\0', sizeof(buffer));
		charsRead = recv(establishedConnectionFD, buffer, MAX_LENGTH-1, 0); //Obtain the original text
		if (charsRead < 0) error("ERROR reading from socket");
		strcpy(plaintext,buffer);
		textLength = charsRead;
		
		for(i=0;i<textLength;i++){
			if(plaintext[i] == 32)
				plaintext[i] = 91;
			cipher[i] = plaintext[i] + key[i];
			if(cipher[i] > 91)
				cipher[i] -= 27;
			if(cipher[i] == 91)
				cipher[i] = 32;
		}
		
		charsRead = send(establishedConnectionFD, cipher, textLength, 0); //Send ciphertext back
		if (charsRead < 0) error("ERROR writing to socket");
		
		_exit(0);
	}
}