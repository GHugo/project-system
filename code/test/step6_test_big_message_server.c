#include "libc.c"

#define MAX_SIZE 128


int main()
{
	char buffer [MAX_SIZE];
	int listening_id = Listen(1);
	if(listening_id != 0)
	{
		PutString("Start");
	}
	int socket_id = Accept(listening_id);
	if(socket_id != 0)
	{
		PutString("Connect");
	}
	int i;
	Receive(socket_id,buffer,MAX_SIZE,1);
	
	for(i=0;i<MAX_SIZE;i++)
	{
		if(buffer[i] != i)
		{
			PutString("Corrupting Message\n");
			return -1;
		}
		
	}
	PutString("Message Arrived\n");
	return 0;
	
}