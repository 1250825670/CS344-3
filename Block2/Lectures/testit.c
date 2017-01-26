#include <stdio.h>

int Add6(int in);

void main()
{
	char* temp = "CS344";

	int i;
	i=0;

	temp[2]='F';

	for (i = 0; i < 5 ; i++ )
		printf("%c\n", temp[i]);

	printf("Adding 6 to 3: %d\n", Add6(3));	
}

int Add6(int in)
{
	int six = 7;

	return six + in;
}
