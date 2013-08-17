#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include "bb.h"
#include "dialog.h"
#include "output.h"

int cellMoved(unsigned short int direction);
void prepareRawData();
void randomize();
int isComplete();
void fillLine(int Y, int colorpair);

int bytesToSend;
int **field;
char response[1024];

struct client_request {
	unsigned short int command;
	unsigned short int argument;
};

struct position
{
	int row;
	int column;
};

struct position emptyCell;

int main(int argc, char** argv)
{
	struct sockaddr_in peer;
	char buffer_init[1024];
	char *buffer = buffer_init;
	int bytesReceived;
	unsigned short int client_command;
	unsigned short int client_command_arg;
	struct sockaddr_in remote;
	int listener, sock;
	
	int steps = 0;	

	if((listener = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		fatalError(BB_DIALOG_SOCKET_CREATION_ERROR);

	remote.sin_family = AF_INET;
	remote.sin_port = htons(1337);
	remote.sin_addr.s_addr = htonl(INADDR_ANY);
	if(bind(listener, (struct sockaddr *)&remote, sizeof(struct sockaddr)) < 0)
		fatalError(BB_DIALOG_LISTENER_BIND_ERROR);	
	listen(listener, 1);
	
	initscr();
	
	start_color();
	init_pair(1, COLOR_WHITE, COLOR_BLUE);
	init_pair(2, COLOR_RED, COLOR_WHITE);
	init_pair(3, COLOR_RED, COLOR_BLUE);

	bkgd(COLOR_PAIR(1));
	attron(COLOR_PAIR(2));
	attron(A_BOLD);
	for (int i = 0; i < COLS; i++) printw(" "); 	
	move(0,0);
	printw(BB_DIALOG_AWAIT_FOR_CONNECTION);
	attron(COLOR_PAIR(1));
	curs_set(0);
	refresh();
	struct client_request request;
	for(;;)	
	{
		int claddrlen = sizeof(peer);
		sock = accept(listener, (struct sockaddr*)&peer, (socklen_t*)&claddrlen);
		fillLine(0, 2);
		attroff(A_BOLD);
		printw(BB_DIALOG_CONNECTION_FROM, inet_ntoa(peer.sin_addr));
		attron(COLOR_PAIR(1));
		move(1,0);
		printw(BB_DIALOG_AWAIT_FOR_INIT);
		refresh();	
		while(1)
		{
			bytesReceived = recv(sock, buffer, 1024, 0);
			if(bytesReceived <= 0) break;
			if(bytesReceived > 1)
			{
				notify(BB_DIALOG_WARNING, BB_DIALOG_TOO_LARGE_PACKET);
				close(sock);
			}
			else
			{
				request.command = (unsigned short int)buffer[0]&0xc0;
				request.argument = (unsigned short int)buffer[0]&0x3f;
				bytesToSend = 4;
				switch(request.command)
				{
					case BB_COMMAND_INIT:
					{
						if(!(request.argument < 2 || request.argument > 15))
						{
							size = request.argument;
							response[0] = BB_STATUS_OK | BB_DATATYPE_RAW_DATA | size;
							printw(BB_DIALOG_GENERATING_FIELD, size, size);
							refresh();
							field = (int**)malloc(sizeof(int*)*size);
							for (int i = 0; i < size; i++) field[i] = (int*)malloc(sizeof(int)*size);
							for (int i = 0; i < size; i++)
								for (int j = 0; j < size; j++)
									field[i][j]=!(i==(size-1)&&i==j)?i*size+j+1:0;
							emptyCell.row=size-1;emptyCell.column=emptyCell.row;
							randomize();
							prepareRawData();

							steps = 0;
						}
						break;
					}
					case BB_COMMAND_MOVE:
					{
						response[0] = (BB_STATUS_OK | BB_DATATYPE_EVENT);
						

						if(!cellMoved(request.argument))
							response[0] |= BB_EVENT_CANT_MOVE;
						else
						{
							response[0] |= BB_EVENT_MOVED;
							steps++;
							fillLine(LINES - 1, 2);	
							attroff(A_BOLD);
							if (isComplete())
							{
								printw("Client won with %i steps in %ix%i field.", steps, size, size);	
								// FIX IT!!!
								response[0] = 0x23;
							}
							else
							{
								prepareRawData();
								printw(BB_DIALOG_STEPS, steps);
							}
							attron(COLOR_PAIR(1));
						}
						refresh();
						break;
					}
					case BB_COMMAND_RAND:
					{
						steps = 0;
						randomize();
						prepareRawData();
						break;
					}
					default: notify(BB_DIALOG_WARNING, BB_DIALOG_STRANGE_PACKET); refresh(); break;
				}
			}
			response[bytesToSend-1] =  BB_SIGNATURE&0x0f;
			response[bytesToSend-2] =  (BB_SIGNATURE>>8)&0xf0;
			send(sock, response, bytesToSend, 0);
		}
		if (field != NULL)
		{
			for (int i = 0; i < size; i++) free(field[i]);
			free(field);
			field = NULL;
		}
		erase();
		fillLine(0, 2);	
		printw(BB_DIALOG_CONNECTION_REFUSED, inet_ntoa(peer.sin_addr));
		attroff(A_BOLD);
		attron(COLOR_PAIR(1));
		refresh();
		close(sock);
	}
	refresh();
	endwin();
	return 0;
}

int cellMoved(unsigned short int direction)
{
	int moved = 0;
	switch (direction)
	{
		case BB_MOVE_UP:
		{
			if(emptyCell.row != size-1)
			{
				field[emptyCell.row][emptyCell.column] = field[emptyCell.row+1][emptyCell.column];
				field[emptyCell.row+1][emptyCell.column] = 0;
				emptyCell.row += 1;
				moved = 1;
			}
			break;
		}
		case BB_MOVE_LEFT:
		{
			if(emptyCell.column != size-1)
			{
				field[emptyCell.row][emptyCell.column] = field[emptyCell.row][emptyCell.column+1];
				field[emptyCell.row][emptyCell.column+1] = 0;
				emptyCell.column += 1;
				moved = 1;
			}
			break;
		}
		case BB_MOVE_DOWN:
		{
			if(emptyCell.row)
			{
				field[emptyCell.row][emptyCell.column] = field[emptyCell.row-1][emptyCell.column];
				field[emptyCell.row-1][emptyCell.column] = 0;
				emptyCell.row -= 1;
				moved = 1;
			}
			break;
		}
		case BB_MOVE_RIGHT:
		{
			if (emptyCell.column)
			{
				field[emptyCell.row][emptyCell.column] = field[emptyCell.row][emptyCell.column-1];
				field[emptyCell.row][emptyCell.column-1] = 0;
				emptyCell.column -= 1;
				moved = 1;
			}
			break;
		}
	}
	return moved;
}

void prepareRawData()
{
	
	for (int i = 0; i < size; i++)
		for (int j = 0; j < size; j++)
			response[i*size+j+1] = field[i][j];
		bytesToSend = size*size+3;
}

void randomize()
{
	for (int i = 0; i < BB_RANDOMIZE_POWER*size*size; i++)
		cellMoved(rand() % 4);
}

int isComplete()
{
	int done = 1;
	for (int i = 0; i < size; i++)
		for (int j = 0; j < size; j++)
			if (field[i][j] != i*size+j+1 && (i != size-1 || j!=i))
				done = 0;
	return done;	
}
