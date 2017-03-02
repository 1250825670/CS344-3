#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

#define MAX_LENGTH 70001

void sendData(int, char*);
void readData(int, char*);

void error(const char *msg) { perror(msg); exit(1); } // Error function used for reporting issues

int main(int argc, char *argv[])
{
	int socketFD, portNumber, charsWritten, charsRead;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	char buffer[MAX_LENGTH],keyBuffer[MAX_LENGTH],textBuffer[MAX_LENGTH];
	
	if (argc < 4) { fprintf(stderr,"USAGE: %s plaintext key port\n", argv[0]); exit(1); } // Check usage & args
	
	int plaintextFD = open(argv[1],O_RDONLY);
	if (plaintextFD < 0) error("CLIENT: ERROR plaintext file open failed");
	int keyFD = open(argv[2],O_RDONLY);
	if (keyFD < 0) error("CLIENT: ERROR key file open failed");
	
	memset(keyBuffer, '\0', sizeof(keyBuffer)); // Clear out the buffer array
	memset(textBuffer, '\0', sizeof(textBuffer)); // Clear out the buffer array
	charsRead = read(keyFD,keyBuffer,sizeof(keyBuffer)-1);	//get the cipher key
	if (charsRead <= 0) error("CLIENT: ERROR reading key file");
	charsRead = read(plaintextFD,textBuffer,sizeof(textBuffer)-1);	//get the plaintext information
	if (charsRead <= 0) error("CLIENT: ERROR reading plaintext file");
	
	if (len(keyBuffer) < len(textBuffer)) error("CLIENT: ERROR key shorter than text");
	textBuffer[len(buffer)-1] = '\0';
	
	close(plaintextFD);
	close(keyFD);
	
	// Set up the server address struct
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[3]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverHostInfo = gethostbyname('localhost'); // Convert the machine name into a special form of address
	if (serverHostInfo == NULL) { fprintf(stderr, "CLIENT: ERROR, no such host\n"); exit(0); }
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address
	
	// Set up the socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (socketFD < 0) error("CLIENT: ERROR opening socket");
	
	// Connect to server
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to address
		error("CLIENT: ERROR connecting");
	
	// Send program name to the server to ensure access is allowed
	memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer array
	strcpy(buffer, "otp_enc");	//send program name to check access
	sendData(socketFD,buffer);
	
	readData(socketFD,buffer);
	if (strcmp(buffer,"REJECTED") == 0) error("Server has denied access");
	
	sendData(socketFD,keyBuffer);
	
	readData(socketFD,buffer);
	if (strcmp(buffer,"key received") != 0) error("Key not received by server");
	
	sendData(socketFD,textBuffer);
	
	readData(socketFD,buffer);
	buffer[len(buffer)-1] = '\n';
	printf(buffer);
	
	close(socketFD); // Close the socket
	return 0;
}

void sendData(int socketFD, char* buffer){
	int charsRead;
	charsWritten = send(socketFD, buffer, strlen(buffer), 0); // Write to server
	if (charsWritten < 0) error("CLIENT: ERROR writing to socket");
	if (charsWritten < strlen(buffer)) printf("CLIENT: WARNING: Not all data written to socket!\n");
}
void readData(int socketFD, char* buffer){
	int charsRead;
	memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer
	charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0); // Read data from the socket, leaving \0 at end
	if (charsRead < 0) error("CLIENT: ERROR reading from socket");
}