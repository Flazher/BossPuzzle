#include "output.h"
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>

void drawField()
{
	clear();
	move(0,0);
	addch(ACS_ULCORNER);
	move(size<<1,0);
	addch(ACS_LLCORNER);

	for (int i = 1; i < size<<1; i++) {
		move(i, 0);
		addch(i % 2 ? ACS_VLINE : ACS_LTEE);
		move(i, (size<<2)-1);
		addch(i % 2 ? ACS_VLINE : ACS_RTEE);
	}
	for (int i = 1; i < (size<<2)-1; i++) {
		move(0, i);
		addch(i % 4 ? ACS_HLINE : ACS_TTEE);
		move(size<<1, i);
		addch(i % 4 ? ACS_HLINE : ACS_BTEE);
	}

	for (int i = 1; i < size<<1; i++)
		for (int j = 1; j < (size<<2)-1; j++)
		{
			move(i, j);
			if (!(i % 2)) if (!(j % 4)) addch(ACS_PLUS); else addch(ACS_HLINE); else if (!(j % 4)) addch(ACS_VLINE); 
		}

	move(0, (size<<2)-1);
	addch(ACS_URCORNER);
	move(size<<1, (size<<2)-1);
	addch(ACS_LRCORNER);
	for (int i = 0; i < size; i++)
	{
		for (int j = 0; j < size; j++)
		{
			move((i<<1)+1, (j<<2)+1);
			if(field[i][j]) printw("%i", field[i][j]);
		}
		printf("\n");
	}
}

void fatalError(char *msg)
{
	printf(msg);
	getch();
	exit(1);
}

void fatalErrorCurses(char *msg)
{
	printw(msg);
	getch();
	exit(1);
}

void notify(char *title, char *msg)
{
	attron(A_BOLD);
	printw("[");
	attron(COLOR_PAIR(3));
	printw(title);
	attron(COLOR_PAIR(1));
	printw("]");
	attroff(A_BOLD);
	printw("%s\n", msg);
}

void usage(char *binname)
{
	printf("Usage: %s <host> <port>\n", binname);
	exit(1);
}
