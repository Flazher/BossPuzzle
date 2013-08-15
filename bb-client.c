#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include "bb.h"
#include "dialog.h"
#include "output.h"

void fillMatrixWithData();

char host[16];
int port = 0;
char buffer[1024];
char request[16];
char *binaryname;

struct server_response {
	unsigned short int status;
	unsigned short int datatype;
	unsigned short int args;
};

int bytesReceived;

int **field;

int main(int argc, char** argv)
{
	binaryname = argv[0];
	if(argc == 3 && (port = atoi(argv[2])) && strlen(argv[1]) <= 15)
		strncpy(host, argv[1], strlen(argv[1]));
	else
		usage(binaryname);
	
		
	struct sockaddr_in server_addr;
	int sock;

	if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		fatalError(BB_DIALOG_SOCKET_CREATION_ERROR);

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = inet_addr(host);
	inet_pton(AF_INET, host, (void *)(&(server_addr.sin_addr.s_addr)));
	if(connect(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) < 0)
		fatalError(BB_DIALOG_CONNECTION_FAIL);	
	printf(BB_DIALOG_CONNECTION_SUCCESSFUL);
	getchar();

	initscr();
	start_color();
	init_pair(1, COLOR_WHITE, COLOR_BLACK);
	init_pair(3, COLOR_RED, COLOR_BLACK);
	bkgd(COLOR_PAIR(1));
	attron(COLOR_PAIR(1));

	drawIntroScreen();
	getch();
	clear();
	int key;
	printw(BB_DIALOG_SIZE_REQUEST);

	while(!scanw("%d", &size) || size > 15 || size < 1) {} 
	request[0] = BB_COMMAND_INIT | size;
	send(sock, request, 1, 0);
	
	struct server_response response;
	for(;;)
	{
		bytesReceived = recv(sock, buffer, 1024, 0);
		response.status = (unsigned short int)buffer[0]&0xc0;
		response.datatype = (unsigned short int)buffer[0]&0x30;
		response.args = (unsigned short int)buffer[0]&0xf;
		
		if (response.status == BB_STATUS_IGNORED)
			notify(BB_DIALOG_WARNING, BB_DIALOG_REQUEST_IGNORED);
		else if((unsigned short int)((buffer[bytesReceived-2]<<8)&0xff00|buffer[bytesReceived-1]) != BB_SIGNATURE)
			notify(BB_DIALOG_WARNING, BB_DIALOG_SIGNATURE_MISMATCH);
		else
		{	
		switch (response.datatype)
		{
			case BB_DATATYPE_RAW_DATA:
			{
				if((unsigned short int)(buffer[0]&0xf) != size)
					fatalErrorCurses(BB_DIALOG_FIELD_SIZE_MISMATCH);
				printw(BB_DIALOG_FIELD_RECEIVED);
					if (field == NULL)
					{
						field = (int**)malloc(sizeof(int*)*size);
						for (int i = 0; i < size; i++)
							field[i]=(int*)malloc(sizeof(int)*size);
					}
					fillMatrixWithData();
					getch();
					drawField();
				break;	
			}
			case BB_DATATYPE_EVENT:
			{
				switch (response.args)
				{
					case BB_EVENT_MOVED: fillMatrixWithData(); drawField(); break;
					case BB_EVENT_CANT_MOVE: drawField(); break;
					case BB_EVENT_WIN: clear(); drawWinMsg(); getch(); endwin(); exit(0); break;
				}
				break;
			}
		}
			}
	refresh();
	key = getch();
	if(key < 0x5B && key > 0x40) key += 0x14;
	switch (key) {
		case 0x77: request[0] = BB_COMMAND_MOVE | BB_MOVE_UP; break;
		case 0x61: request[0] = BB_COMMAND_MOVE | BB_MOVE_LEFT; break;
		case 0x73: request[0] = BB_COMMAND_MOVE | BB_MOVE_DOWN; break;
		case 0x64: request[0] = BB_COMMAND_MOVE | BB_MOVE_RIGHT; break;
		case 0x71: endwin(); exit(0); break;
		case 0x72: request[0] = BB_COMMAND_RAND; break;
	}
		send(sock, request, 1, 0);
	}
	
	refresh();
	endwin();
	return 0;
}

void fillMatrixWithData()
{
	for (int i = 0; i < size; i++)
		for (int j = 0; j < size; j++)
			field[i][j] = buffer[1+i*size+j];
}
