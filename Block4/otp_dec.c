#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <fcntl.h>

#define MAX_LENGTH 70005

void sendData(int, char*);
void readData(int, char*, size_t);
void removeNewline(char*);

void error(const char *msg) { perror(msg); exit(1); } // Error function used for reporting issues

int main(int argc, char *argv[])
{
	int socketFD, portNumber, charsWritten, charsRead;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	char buffer[MAX_LENGTH],keyBuffer[MAX_LENGTH],textBuffer[MAX_LENGTH], c;
	char* END = "tuna";
	
	if (argc < 4) { fprintf(stderr,"USAGE: %s ciphertext key port\n", argv[0]); exit(1); } // Check usage & args
	
	int ciphertextFD = open(argv[1],O_RDONLY);
	if (ciphertextFD < 0) error("CLIENT: ERROR ciphertext file open failed");
	int keyFD = open(argv[2],O_RDONLY);
	if (keyFD < 0) error("CLIENT: ERROR key file open failed");
	
	memset(keyBuffer, '\0', sizeof(keyBuffer)); // Clear out the buffer array
	memset(textBuffer, '\0', sizeof(textBuffer)); // Clear out the buffer array
	charsRead = read(keyFD,keyBuffer,sizeof(keyBuffer)-1);	//get the cipher key
	if (charsRead <= 0) error("CLIENT: ERROR reading key file");
	charsRead = read(ciphertextFD,textBuffer,sizeof(textBuffer)-1);	//get the ciphertext information
	if (charsRead <= 0) error("CLIENT: ERROR reading ciphertext file");
	
	if (strlen(keyBuffer) < strlen(textBuffer)) error("CLIENT: ERROR key shorter than text");
	removeNewline(textBuffer);
	removeNewline(keyBuffer);
	
	close(ciphertextFD);
	close(keyFD);
	
	// Set up the server address struct
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[3]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverHostInfo = gethostbyname("localhost"); // Convert the machine name into a special form of address
	if (serverHostInfo == NULL) { fprintf(stderr, "CLIENT: ERROR, no such host\n"); exit(0); }
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address
	
	// Set up the socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (socketFD < 0) error("CLIENT: ERROR opening socket");
	
	// Connect to server
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to address
		error("CLIENT: ERROR connecting");
	
	// Send program name to the server to ensure access is allowed
	sendData(socketFD,"otp_dec");	//send program name to check access
	
	readData(socketFD,buffer,8);	//check if access is granted
	if (strcmp(buffer,"REJECTED") == 0) error("Server has denied access");
	
	strcat(keyBuffer,END);
	sendData(socketFD,keyBuffer);	//send key
	readData(socketFD,buffer,3);		//check if received
	if (strcmp(buffer,"ACK") != 0) error("Key not received by server");
	strcat(textBuffer,END);
	sendData(socketFD,textBuffer);	//send ciphertext
	
	readData(socketFD,buffer,sizeof(buffer)-1);	//receive ciphertext
	char* ending = strstr(buffer,END);
	*ending = '\0';
	printf("%s\n", buffer);
	
	close(socketFD); // Close the socket
	return 0;
}

void sendData(int socketFD, char* buffer){
	int charsWritten;
	charsWritten = send(socketFD, buffer, strlen(buffer), 0); // Write to server
	if (charsWritten < 0) error("CLIENT: ERROR writing to socket");
	if (charsWritten < strlen(buffer)) printf("CLIENT: WARNING: Not all data written to socket!\n");
}
void readData(int socketFD, char* buffer, size_t len){
	int charsRead;
	memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer
	charsRead = recv(socketFD, buffer, len, 0); // Read data from the socket, leaving \0 at end
	if (charsRead < 0) error("CLIENT: ERROR reading from socket");
}

void removeNewline(char* buffer){
	int len = strcspn(buffer,"\n");
	buffer[len] = '\0';
}