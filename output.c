#include "output.h"
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dialog.h"

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

void fillLine(int Y, int colorpair)
{
	move(Y, 0);
	clrtoeol();	
	attron(COLOR_PAIR(colorpair));
	attron(A_BOLD);
	for (int i = 0; i < COLS; i++) printw(" ");
	move(Y, 0);
	refresh();
}

void drawIntroScreen()
{
	char credits[] = "Written by flazhik <flazhik@gmail.com>";
	char credits2[] = "No one actually cares";
	char anykey[] = "Any key, plz";
	int sizeOfMatrix;
	int greetWinWid = 50;
	int greetWinHgh = 20;
	int logoStartX = (greetWinWid-9)/3;
	WINDOW *greet;
	greet = subwin(stdscr, greetWinHgh, greetWinWid, 3, (COLS-greetWinWid)/2);
	box(greet, 0, 0);	
	wmove(greet, 0, 3);
	wprintw(greet, "Boss Puzzle client");
	for(int y=3;y<8;y++)
	{
		for(int x=1+logoStartX;x<3+logoStartX;x++){wmove(greet,y,x);waddch(greet,ACS_BLOCK);}
		for(int x=5+logoStartX;x<10+logoStartX;x++){wmove(greet,y,x);if(y%2||(x>4+logoStartX&&x<7+logoStartX&&y==4)||(x>7+logoStartX&&x<10+logoStartX&&y==6)) waddch(greet,ACS_BLOCK);}
	}
	wmove(greet, 3, logoStartX+11);
	wprintw(greet,"Ultimate bicycle");
	wmove(greet, 4, logoStartX+11);
	wprintw(greet,"aka Boss Puzzle");
	wmove(greet, 7, logoStartX+11);
	wprintw(greet,"ROFL");
	wmove(greet, greetWinHgh-5, (greetWinWid-strlen(credits))/2);
	wprintw(greet,credits);
	wmove(greet, greetWinHgh-4, (greetWinWid-strlen(credits2))/2);
	wprintw(greet,credits2);
	wmove(greet,greetWinHgh-2,greetWinWid-strlen(anykey)-3);
	wprintw(greet,anykey);
	wrefresh(greet);
	// Deprecated	
	/* wmove(greet, 10, 5);
	wprintw(greet,BB_DIALOG_SIZE);
	wmove(greet,10,strlen(BB_DIALOG_SIZE)+6);
	while(!wscanw(greet,"%d",&sizeOfMatrix))
	{
		wmove(greet,12,(greetWinWid-strlen(BB_DIALOG_SIZE_REQUEST))/2);
		wprintw(greet,BB_DIALOG_SIZE_REQUEST);
		wmove(greet,10,strlen(BB_DIALOG_SIZE)+6);
	} */
}

void drawPopup(char *title, char *msg)
{
	WINDOW *popup;
	popup = subwin(stdscr, 5, strlen(msg)+15, (LINES-5)/2, (COLS-strlen(msg)-15)/2);
	box(popup, 0, 0);
	wmove(popup,0,3);
	wprintw(popup,title);
	wmove(popup,2,(strlen(msg)+15-strlen(msg))/2);
	wprintw(popup,msg);
	wrefresh(popup);
	getch();
	delwin(popup);
}
