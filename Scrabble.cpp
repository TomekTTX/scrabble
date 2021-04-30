#include "pch.h"
#include <stdlib.h>
#include <stdio.h>
#include "conio2.h"
#include <string.h>
#include <time.h>

//#define DEBUG
#define BOARDSIZE 15 //size of the board, has a 2-wide frame, should be an odd number
#define BOARD_X 50 //X of board's position, >=3
#define BOARD_Y 5 //Y of board's position, >=2
#define LEGEND_X 1 //X of legend's position
#define LEGEND_Y 1 //Y of legend's position
#define PLAYERS_X 1 //X of player info position
#define PLAYERS_Y 18 //Y of player info position
#define NOTIF_COORDS 38,28 //X and Y for notify function
#define TUT_X 70 //X of tutorial info
#define TUT_Y 5 //Y of tutorial info
#define WORD_INPUT_X 50 //X of word input position
#define WORD_INPUT_Y 25 //Y of word input position
#define MAINCOLOR WHITE //board color
#define FRAMECOLOR LIGHTGRAY //board frame color
#define LEGEND_ATTR 16 * BLACK + LIGHTGRAY //background and text color of legend
#define INPUT_BG BLUE
#define TILE_COLOR YELLOW
#define TEXTCOLOR BLACK //color of characters on frame and tiles
#define HL_BGCOLOR LIGHTGREEN //exchange highlight background color
#define KEY_UP 0x48
#define KEY_LEFT 0x4b
#define KEY_DOWN 0x50
#define KEY_RIGHT 0x4d
#define KEY_ESC 0x1b
#define KEY_ENTER 0x0d
#define KEY_BACKSPACE 0x08
#define ALPH_LEN 'Z' - 'A' + 1
#define SWAP_ITERATIONS 150 //number of random swaps performed by formLetters
#define BLANK_COUNT 2 //amount of blank tiles
#define LEGEND_LENGTH 50 //maximum width of legend
#define LEGEND_SIZE 15 //maximum height of legend
#define MAX_WORDLENGTH 20 //maximum length of a dictionary word
#define DEF_DICT_FILENAME "dictionary.txt" //default name of the file containing dictionary
#define SPECIAL_CELL_TYPES 5 //amount of cell types
#define l_MULT 2
#define L_MULT 3
#define w_MULT 2
#define W_MULT 3


typedef struct {
	char *letterPool;
	int pos;
	int count;
} sack_t;

typedef struct {
	char tiles[BOARDSIZE][BOARDSIZE];
	sack_t letterSack;
	char playerPool[2][8];
	unsigned int playerPoints[2];
	short turn;
} gameState_t;

typedef struct {
	char **dictP;
	int lines;
} dict_t;

typedef enum {
	HORIZONTAL,
	VERTICAL,
} dir_t;

typedef enum {
	_,
	l,
	L,
	w,
	W,
} cell_t;

//startup functions:

int gameInit(gameState_t *game, const int *lettercount);
void newGame(gameState_t *game, const int *lettercount);
void formLetters(gameState_t *game, const int *lettercount);

//drawing functions:

void drawBoard(cell_t cells[BOARDSIZE][BOARDSIZE], const short colors[SPECIAL_CELL_TYPES]);
void drawInnerBoard(cell_t specialcells[BOARDSIZE][BOARDSIZE], const short specialcolors[SPECIAL_CELL_TYPES]);
void placeTiles(gameState_t *game);
void drawLegend(int x, int y, unsigned int mode, const char legend[LEGEND_SIZE][LEGEND_LENGTH]);
void drawPlayers(gameState_t *game);
int showBoardCompat(int x, int y, char *word, dir_t word_dir, char tiles[BOARDSIZE][BOARDSIZE]);
void notify(char *str);
void unnotify();
void readyInput(int mode, const char legend[LEGEND_SIZE][LEGEND_LENGTH]);
void getFileName(char *name);
void clrTut();
void displayExchangeHighlight(gameState_t *game, short choice[7]);

//dictionary functions:

int countLines(FILE *dictF);
dict_t readyDict(char *filename);
void dynamicCheck(char *word, dict_t *dict);
int autoSearch(char tiles[BOARDSIZE][BOARDSIZE], int x, int y, char *word, dir_t word_dir, dict_t *dict);
short checkWord(char *word, dict_t *dict);
short checkSingleLetter(char *pool, char letter, short taken[7], char mode);

//tile & board manipulation functions:

int cursor(int *x, int *y);
int playerGetLetters(gameState_t *game, int player);
void playerLetterExchange(gameState_t *game, int player, short choice[7]);
int placeWord(gameState_t *game, int x, int y, dir_t word_dir, char *word, dict_t *dict);
void doWordPlacement(gameState_t *game, dict_t *dict, dir_t word_dir, const int lettercount[ALPH_LEN], const int letterpoints[ALPH_LEN],
	cell_t specialcells[BOARDSIZE][BOARDSIZE], const short specialcolors[SPECIAL_CELL_TYPES], const char legend[LEGEND_SIZE][LEGEND_LENGTH],
	int x, int y, short tut);
void doLetterExchange(gameState_t *game);
int countPoints(cell_t cells[BOARDSIZE][BOARDSIZE], int x, int y, char *word, dir_t word_dir, const int letterpoints[ALPH_LEN], short tut);

//others

int saveGame(gameState_t *game);
int loadGame(gameState_t *game);
void swap(char *a, char *b);
short checkEndGame(gameState_t *game);



int main(int argc, char **argv) {

	#ifndef __cplusplus
		Conio2_Init();
	#endif
			   

	int c = 0, x = BOARD_X + BOARDSIZE / 2, y = BOARD_Y + BOARDSIZE / 2, tut = 0;
	gameState_t game;	
	dir_t word_dir = HORIZONTAL;
	srand((unsigned int)time(NULL));

	// points and amounts of letters:    A B C D E  F G H I J K L M N O P Q  R S T U V W X Y Z
	const int lettercount[ALPH_LEN]  = { 9,2,2,4,12,2,3,2,9,1,1,4,2,6,8,2,1 ,6,4,6,4,2,2,1,2,1  };
	const int letterpoints[ALPH_LEN] = { 1,3,3,2,1 ,4,2,4,1,8,5,1,3,1,1,3,10,1,1,1,1,4,4,8,4,10 };


	//positions and types of cells with point mults
	cell_t specialcells[BOARDSIZE][BOARDSIZE] = {
		{W,_,_,l,_,_,_,W,_,_,_,l,_,_,W},
		{_,w,_,_,_,L,_,_,_,L,_,_,_,w,_},
		{_,_,w,_,_,_,l,_,l,_,_,_,w,_,_},
		{l,_,_,w,_,_,_,l,_,_,_,w,_,_,l},
		{_,_,_,_,w,_,_,_,_,_,w,_,_,_,_},
		{_,L,_,_,_,L,_,_,_,L,_,_,_,L,_},
		{_,_,l,_,_,_,l,_,l,_,_,_,l,_,_},
		{W,_,_,l,_,_,_,w,_,_,_,l,_,_,W},
		{_,_,l,_,_,_,l,_,l,_,_,_,l,_,_},
		{_,L,_,_,_,L,_,_,_,L,_,_,_,L,_},
		{_,_,_,_,w,_,_,_,_,_,w,_,_,_,_},
		{l,_,_,w,_,_,_,l,_,_,_,w,_,_,l},
		{_,_,w,_,_,_,l,_,l,_,_,_,w,_,_},
		{_,w,_,_,_,L,_,_,_,L,_,_,_,w,_},
		{W,_,_,l,_,_,_,W,_,_,_,l,_,_,W},
	};
	const short specialcolors[SPECIAL_CELL_TYPES] = { WHITE, LIGHTCYAN, LIGHTBLUE, LIGHTMAGENTA, LIGHTRED};


	//content of legend
	const char legend[LEGEND_SIZE][LEGEND_LENGTH] = {
		{"Tomasz Sza\x88kowski, 175630 (a,b,d,f,g,h,i,j,k)"},
		{"q       = exit"},
		{"cursors = moving"},
		{"i       = insert word"},
		{"o       = change word orientation"},
		{"w       = exchange tiles"},
		{"1-7     = choose tiles to exchange"},
		{"n       = new game"},
		{"s       = save game"},
		{"l       = load game"},
		{"t       = tutorial mode"},
		{"esc     = abort"},
		{"enter   = confirm"},
		{'\0'},
	};
	
	//game initialization
	settitle("Tomasz Sza\x88kowski 175630");
	textbackground(BLACK);

	gameInit(&game, lettercount);

	if (game.letterSack.letterPool == NULL) {
		return 1;
	}
	dict_t dict = readyDict(argv[1]);	

	clrscr();
	_setcursortype(_NOCURSOR);
	
	if (dict.dictP == NULL)
		notify("Failed to open dictionary file!");

	drawBoard(specialcells, specialcolors);

// -----------------------------------------  main loop  ----------------------------------------- 
	do {
		textbackground(BLACK);

		drawLegend(x, y, 1962, legend);
		drawPlayers(&game);

		if (checkEndGame(&game))
			break;

		c = getch();
		if (c >= 'A' && c <= 'Z')
			c += 'a' - 'A';

		switch (c)
		{
		case 'n':
			newGame(&game, lettercount);
			drawInnerBoard(specialcells, specialcolors);
			notify("New game started");
			break;
		case 'o':
			word_dir ^= 1;
			break;
		case 'w':
			notify("Choose tiles to exchange");
			drawLegend(x, y, 6208, legend);
			doLetterExchange(&game);
			unnotify();
			break;
		case 's':
			readyInput(1, legend);
			saveGame(&game);
			break;
		case 'l':
			readyInput(1, legend);
			loadGame(&game);
			drawInnerBoard(specialcells, specialcolors);
			placeTiles(&game);
			break;
		case 't':
			tut ^= 1;
			char txt[32];
			sprintf(txt, "Tutorial mode %s", tut ? "on" : "off");
			notify(txt);
			break;
		case 'i':
			unnotify();
			readyInput(0, legend);			
			doWordPlacement(&game, &dict, word_dir, lettercount, letterpoints, specialcells, specialcolors, legend, x, y, tut);
			break;
		}
	} while (c != 'q'); //end main loop

	if (dict.dictP != NULL)
		free(dict.dictP[0]);
	if (game.letterSack.letterPool != NULL)
		free(game.letterSack.letterPool);

	textattr(LEGEND_ATTR);
	_setcursortype(_NORMALCURSOR);
	clrscr();
	return 0;
}
// -----------------------------------------  game end  ----------------------------------------- 



//draws empty board with no frame
void drawInnerBoard(cell_t cells[BOARDSIZE][BOARDSIZE], const short colors[SPECIAL_CELL_TYPES]) {
	for (int i = 0; i < BOARDSIZE; ++i) {
		for (int j = 0; j < BOARDSIZE; ++j) {
			gotoxy(i + BOARD_X, j + BOARD_Y);
			textbackground(colors[(int)cells[j][i]]);
			putch(' ');
		}
	}
}

//draws empty board
void drawBoard(cell_t specialcells[BOARDSIZE][BOARDSIZE], const short specialcolors[SPECIAL_CELL_TYPES])
{
	drawInnerBoard(specialcells, specialcolors);
	textattr(16 * FRAMECOLOR + TEXTCOLOR);
	gotoxy(BOARD_X - 2, BOARD_Y - 1); //draws the frame
	cputs("  ");
	int i = 0;
	char txt[4];
	for (char label = 'A'; i < BOARDSIZE; ++i, ++label) {
		putch(label);
	}
	cputs("  ");
	for (int label = 0; label < BOARDSIZE; ++label) {
		gotoxy(BOARD_X - 2, BOARD_Y + label);
		sprintf(txt, "%2d", label + 1);
		cputs(txt);
	}
	for (int label = 0; label < BOARDSIZE; ++label) {
		gotoxy(BOARDSIZE + BOARD_X, BOARD_Y + label);
		sprintf(txt, "%-2d", label + 1);
		cputs(txt);
	}
	/*for (int label = 0, i = 0; i < BOARDSIZE; ++i, ++label) {
		gotoxy(BOARD_X - 1, BOARD_Y + i);
		putch((char)('0' + ((i + 1) % 10)));
	}
	for (i = 0; i < BOARDSIZE; i++)	{
		gotoxy(BOARD_X - 2, BOARD_Y + i);
		if ((i + 1) / 10) putch((char)('0' + (i + 1) / 10));
		else putch(' ');
	}*/
	gotoxy(BOARD_X - 2, BOARD_Y - 2);
	for (i = 0; i < BOARDSIZE + 4; ++i) putch(' ');
	gotoxy(BOARD_X - 2, BOARD_Y + BOARDSIZE);
	cputs("  ");
	for (char label = 'A', i = 0; i < BOARDSIZE; ++i, ++label) {
		putch(label);
	}
	cputs("  ");
	gotoxy(BOARD_X - 2, BOARD_Y + BOARDSIZE + 1);
	for (i = 0; i < BOARDSIZE + 4; ++i) putch(' ');
}

//fills board with tiles
void placeTiles(gameState_t *game)
{
	gotoxy(BOARD_X, BOARD_Y);
	textattr(16 * TILE_COLOR + TEXTCOLOR);
	for (int i = 0; i < BOARDSIZE; ++i) {
		for (int j = 0; j < BOARDSIZE; ++j) {
			gotoxy(BOARD_X + j, BOARD_Y + i);
			if (game->tiles[j][i] != ' ') putch(game->tiles[j][i]);
		}
	}
}

//displays notification at NOTIF_COORDS
void notify(char *str)
{
	textattr(16 * BLACK + YELLOW);
	gotoxy(NOTIF_COORDS);
	cputs(str);
	cputs("                               ");
}

//initializes game; returns total amount of letter tiles
int gameInit(gameState_t *game, const int *lettercount)
{

	int count = BLANK_COUNT;
	for (int i = 0; i < ALPH_LEN; ++i) {
		count += lettercount[i];
	}
	
	for (int i = 0; i < BOARDSIZE; ++i) {
		for (int j = 0; j < BOARDSIZE; ++j) {
			game->tiles[j][i] = ' ';
		}
	}

	game->letterSack.letterPool = (char *)malloc(count * sizeof(char));
	if (game->letterSack.letterPool == NULL) {
		notify("Memory alloc failed");
		return -1;
	}

	game->letterSack.count = count;
	newGame(game, lettercount);
	return count;
}

//starts new game
void newGame(gameState_t* game, const int *lettercount)
{
	formLetters(game, lettercount);
	game->letterSack.pos = 0;
	game->playerPoints[0] = 0;
	game->playerPoints[1] = 0;
	game->turn = 0;
	for (int j = 0; j <= 1; ++j) {
		game->playerPool[j][7] = '\0';
		for (int i = 0; i < 7; ++i)
			game->playerPool[j][i] = '#';
	}	
	for (int i = 0; i < BOARDSIZE; ++i) {
		for (int j = 0; j < BOARDSIZE; ++j)
			game->tiles[i][j] = ' ';
	}	
	game->letterSack.pos = 0;
	playerGetLetters(game, 0);
	playerGetLetters(game, 1);
}

//displays legend
void drawLegend(int x, int y, unsigned int mode, const char legend[LEGEND_SIZE][LEGEND_LENGTH])
{
	char txt[32];
	int i;
	textattr(LEGEND_ATTR);
	for (i = 0; legend[i][0] != '\0'; ++i) {
		(mode & (1ULL << i)) ? textcolor(LIGHTGREEN) : textcolor(LIGHTGRAY);
		gotoxy(LEGEND_X, LEGEND_Y + i);
		cputs(legend[i]);
	}
	++i;
	gotoxy(LEGEND_X, LEGEND_Y + i);
	sprintf(txt, "Cursor position: %c %d ", (char)(x - BOARD_X + 'A'), y - BOARD_Y + 1);
	(mode & (1ULL << i)) ? textcolor(LIGHTGREEN) : textcolor(LIGHTGRAY);
	cputs(txt);
	textcolor(LIGHTGRAY);
}

//displays player info
void drawPlayers(gameState_t *game)
{
	char txt[32];
	gotoxy(PLAYERS_X, PLAYERS_Y);
	textattr(LEGEND_ATTR);
	sprintf(txt, "Turn: player %d ", game->turn + 1);
	cputs(txt);

	gotoxy(PLAYERS_X, PLAYERS_Y + 4);
	if (game->turn == 0)
		textcolor(LIGHTGREEN);
	cputs("PLAYER 1:");
	gotoxy(PLAYERS_X, PLAYERS_Y + 6);
	sprintf(txt, "points: %d  ", game->playerPoints[0]);
	cputs(txt);
	gotoxy(PLAYERS_X, PLAYERS_Y + 8);
	textattr(16 * TILE_COLOR + BLACK);
	cputs(game->playerPool[0]);

	textattr(LEGEND_ATTR);
	gotoxy(PLAYERS_X, PLAYERS_Y + 12);
	if (game->turn == 1)
		textcolor(LIGHTGREEN);
	cputs("PLAYER 2:");
	gotoxy(PLAYERS_X, PLAYERS_Y + 14);
	sprintf(txt, "points: %d  ", game->playerPoints[1]);
	cputs(txt);
	gotoxy(PLAYERS_X, PLAYERS_Y + 16);
	textattr(16 * TILE_COLOR + BLACK);
	cputs(game->playerPool[1]);
	textattr(LEGEND_ATTR);
}

//refills and scrambles letter pool
void formLetters(gameState_t* game, const int *lettercount)
{
	//put the specified amounts of letters in the pool
	int pos = 0; 
	for (char lett = 'A'; lett <= 'Z'; ++lett) {
		for (int i = 0; i < lettercount[lett - 'A']; ++i) {
			game->letterSack.letterPool[pos] = lett;
			++pos;
		}
	}
	for (int i = 0; i < BLANK_COUNT; ++i) {
		game->letterSack.letterPool[pos] = ' ';
			++pos;
		}

	//scramble the letters
	for (int a, b, i = 0; i < SWAP_ITERATIONS; ++i) {
		a = rand() % (game->letterSack.count);
		b = rand() % (game->letterSack.count);
		swap(&(game->letterSack.letterPool[a]), &(game->letterSack.letterPool[b]));
	}
}

//swaps two chars
void swap(char *a, char *b)
{
	if (*a != *b) {
		*a ^= *b;
		*b ^= *a;
		*a ^= *b;
	}
}

//fills empty slots of player's board; returns amount of letters filled
int playerGetLetters(gameState_t *game, int player)
{
	if (game->letterSack.pos >= game->letterSack.count) return 0;
	int successAmount = 0;
	for (int i = 0; i < 7 && game->letterSack.pos < game->letterSack.count; ++i) {
		if (game->playerPool[player][i] == '#' || game->playerPool[player][i] == '\0') {
			game->playerPool[player][i] = (game->letterSack.letterPool[game->letterSack.pos]);
			game->letterSack.letterPool[game->letterSack.pos] = '#';
			++(game->letterSack.pos);
			++successAmount;
		}
	}
	return successAmount;
}

//displays which tiles are picked for exchange
void displayExchangeHighlight(gameState_t *game, short choice[7])
{
	gotoxy(PLAYERS_X, PLAYERS_Y + 8 * (game->turn + 1) );
	for (int i = 0; i < 7; ++i) {
		if (choice[i]) {
			textattr(16 * HL_BGCOLOR + TEXTCOLOR);
		}
		else {
			textattr(16 * TILE_COLOR + TEXTCOLOR);
		}
		putch(game->playerPool[game->turn][i]);
	}
}

//puts the selected tiles back into the pool; runs playerGetLetters
void playerLetterExchange(gameState_t *game, int player, short choice[7])
{
	char storage[7] = { "#######" };
	int exchAmnt = 0;
	for (int i = 0; i < 7; ++i) {
		if (choice[i]) {
			swap(storage + i, game->playerPool[player] + i);
			++exchAmnt;
		}
	}
	playerGetLetters(game, player);
	game->letterSack.pos -= exchAmnt;	

	for (int i = 0; i < (game->letterSack.count - game->letterSack.pos - exchAmnt); ++i)
		swap(game->letterSack.letterPool + game->letterSack.pos + i, game->letterSack.letterPool + game->letterSack.pos + i + exchAmnt);	   	 

	int exchAmnt2 = exchAmnt;
	for (int i = 0; i < 7; ++i) {
		if (storage[i] != '#' && storage[i] != '\0') {
			swap(storage + i, game->letterSack.letterPool + game->letterSack.count - exchAmnt2);
			--exchAmnt2;
		}
	}
}

//checks if the player has enough letter tiles to create the selected word
int checkPlayerLetters(gameState_t *game, char *word, int x, int y, int length, dir_t word_dir)
{
	if ((word_dir == HORIZONTAL && (x + length <= BOARDSIZE) || word_dir == VERTICAL && (y + length <= BOARDSIZE)) == 0) {
		return 0;
	}
	short j = 0, count = 0, taken[7] = { 0 };
	for (int i = 0; i < length; ++i) {  
		if (word[i] == (word_dir == VERTICAL ? game->tiles[x][y + i] : game->tiles[x + i][y]) ) {
			++count;
			continue;
		}
		for (j = 0; j < 7; ++j) {    
			if (taken[j] == 0 && word[i] == game->playerPool[game->turn][j]) {
				taken[j] = 1;
				++count;
				break;
			}
		}
	}
	if (count != length) {
		for (int i = 0; i < 7; ++i) {
			if (game->playerPool[game->turn][i] == ' ') {
				taken[i] = 1;
				++count;
				if (count == length) {
					break;
				}
			}
		}
	}
	if (count == length) {
		for (int i = 0; i < 7; ++i) {
			if (taken[i] == 1)
				game->playerPool[game->turn][i] = '#';
		}
		return 1;
	}
	return 0;
}

//checks for a single unused(A)/used(D) letter in current player's pool; returns its index (-1 if not found); mode is 'A' or 'D'
short checkSingleLetter(char *pool, char letter, short taken[7], char mode)
{
	for (int i = 0; i < 7; ++i)	{
		if ((taken[i] == 0 && mode == 'A' || taken[i] > 0 && mode == 'D') && pool[i] == letter)
			return i;
	}
	return -1;
}

//checks if the given word is compatible with tiles already placed
int checkBoardLetters(gameState_t *game, int x, int y, dir_t word_dir, char *word)
{
	char c;
	short count = 0;
	for (int i = 0; word[i] != '\0'; ++i) {
		c = (word_dir == VERTICAL ? game->tiles[x][y + i] : game->tiles[x + i][y]);
		if (c != word[i] && c != ' ')
			return 0;
		else if (c == ' ')
			++count;
	}
	if (count == 0)
		return -1;
	else
		return 1;
}

//runs checks and, if successful, places the word on the board; returns 1 on success and 0 on failure
int placeWord(gameState_t *game, int x, int y, dir_t word_dir, char *word, dict_t *dict)
{
	int length;
	for (length = 0; word[length] != '\0'; ++length);
	x -= BOARD_X;
	y -= BOARD_Y;


	int boardCheck = checkBoardLetters(game, x, y, word_dir, word);
	if (boardCheck == 0) {
		notify("This word doesn't fit there");
		return 0;
	}
	if (boardCheck == -1) {
		notify("You need to use at least one tile");
		return 0;
	}
	if (autoSearch(game->tiles, x, y, word, word_dir, dict) == 0)
		return 0;
	if (checkPlayerLetters(game, word, x, y, length, word_dir) == 0) {
		notify("You don't have the letters");
		return 0;
	}


	if (word_dir == HORIZONTAL) {
		for (int i = 0; i < length; ++i) {
			game->tiles[x + i][y] = word[i];
		}
		return 1;
	}
	else {
		for (int i = 0; i < length; ++i) {
			game->tiles[x][y + i] = word[i];
		}
		return 1;
	}

	return 0;
}

//draw the field for typing words
void readyInput(int mode, const char legend[LEGEND_SIZE][LEGEND_LENGTH])
{
	drawLegend(BOARD_X + BOARDSIZE / 2, BOARD_Y + BOARDSIZE / 2, 6144, legend);
	char txt[32];
	sprintf(txt, "Insert %s: ", mode ? "filename" : "word");
	gotoxy(WORD_INPUT_X - strlen(txt), WORD_INPUT_Y);
	textattr(LEGEND_ATTR);
	cputs(txt);
	textbackground(INPUT_BG);
	cputs("          ");
	gotoxy(WORD_INPUT_X, WORD_INPUT_Y);
	_setcursortype(_NORMALCURSOR);
}

//register arrowkey pressing, returns key pressed if it's not an arrow
int cursor(int *x, int *y)
{
	short zero = 0;
	short c = getch();
	if (c == 0) {
		zero = 1;
		c = getch();
		if (c == KEY_UP && (*y) > BOARD_Y) (*y)--;
		else if (c == KEY_DOWN && (*y) < (BOARD_Y + BOARDSIZE - 1)) (*y)++;
		else if (c == KEY_LEFT && (*x) > BOARD_X) (*x)--;
		else if (c == KEY_RIGHT && (*x) < (BOARD_X + BOARDSIZE - 1)) (*x)++;
	}
	else {
		return c;
	}
	return 0;
}

//highlights word letters in green or red depending on compatibility with board tiles; returns 0 if the word doesn't fit on the board, 1 otherwise
int showBoardCompat(int x, int y, char *word, dir_t word_dir, char tiles[BOARDSIZE][BOARDSIZE])
{
	short len, fit = 1;
	for (len = 0; word[len] != '\0'; ++len);
	if (x - BOARD_X + len > BOARDSIZE && word_dir == HORIZONTAL || y - BOARD_Y + len > BOARDSIZE && word_dir == VERTICAL) {
		len = BOARDSIZE + (word_dir == VERTICAL ? BOARD_Y - y : BOARD_X - x);
		fit = 0;
	}
	textcolor(WHITE);
	for (int i = 0; i < len; ++i) {
		(fit && (tiles[x - BOARD_X][y - BOARD_Y] == word[i] || tiles[x - BOARD_X][y - BOARD_Y] == ' ')) ? textbackground(GREEN) : textbackground(RED);
		gotoxy(x, y);
		putch(word[i]);
		word_dir == VERTICAL ? ++y : ++x;		
	}
	gotoxy(word_dir == VERTICAL ? x, y - len : x - len, y); //reset to initial position for the point counter function
	return fit;
}

//returns the amount of lines in a file - 1
int countLines(FILE *dictF)
{
	char buffer[36];
	int lines = 0;

	while (fgets(buffer, 30, dictF)) {
		if (strlen(buffer) <= BOARDSIZE)
			++lines;
	}

	return lines;
}

//places words from dictionary into an array of strings, returns a struct of its pointer and amount of lines
dict_t readyDict(char *filename)
{
	dict_t dict;
	FILE *dictF = NULL;
		
	if (filename != NULL)
		dictF = fopen(filename, "r");

	if (dictF == NULL) {
		notify("Selected dictionary not found, loading default");
		dictF = fopen(DEF_DICT_FILENAME, "r");
	}
	
	if (dictF == NULL) {
		dict.dictP = NULL;
		dict.lines = 0;
		return dict;
	}

	int lines = countLines(dictF);
	fseek(dictF, 0, SEEK_SET);

	char **dictP = (char **)malloc(lines * sizeof(char*));
	char *dictPtr = (char *)malloc((MAX_WORDLENGTH + 1) * lines * sizeof(char));
	char buffer[30];
	int i;
	for (i = 0; i < lines; i++) {
		dictP[i] = dictPtr + (MAX_WORDLENGTH + 1) * i;
		fgets(buffer, MAX_WORDLENGTH + 1, dictF);
		if (strlen(buffer) > BOARDSIZE) {
			--i;
			continue;
		}
		buffer[strlen(buffer) - 1] = '\0';
		strcpy(dictP[i], buffer);
	}
	dictP[lines] = (char *)malloc(sizeof(char));
	dictP[lines][0] = '\0';
	fclose(dictF);

	dict.dictP = dictP;
	dict.lines = lines;
	return dict;
}

//clears notification displayed by notify
void unnotify()
{
	gotoxy(NOTIF_COORDS);
	textbackground(BLACK);
	cputs("                                             ");
}

//colors the cells according to specified point multipliers


//saves game state to selected file
int saveGame(gameState_t *game)
{
	char name[32] = { 0 };
	getFileName(name);
	if (strlen(name) == 0)
		return 0;

	FILE *save = fopen(name, "w+b");
	if (save == NULL) {
		notify("Save file creation failed");
		return 0;
	}
	fwrite(game, sizeof(gameState_t), 1, save);
	fwrite(game->letterSack.letterPool, sizeof(char), game->letterSack.count, save);
	fclose(save);
	char txt[48];
	sprintf(txt, "Game saved to '%s'", name);
	notify(txt);
	return 1;
}

//loads game state from selected file
int loadGame(gameState_t *game)
{
	char name[32] = { 0 };
	getFileName(name);
	if (strlen(name) == 0)
		return 0;

	FILE *save = fopen(name, "rb");
	if (save == NULL) {
		notify("Save file not found");
		return 0;
	}
	gameState_t buffer;
	fread(&buffer, sizeof(gameState_t), 1, save);
	

	for (int i = 0; i < 2; ++i) {
		game->playerPoints[i] = buffer.playerPoints[i];
		for (int j = 0; j < 7; ++j)
			game->playerPool[i][j] = buffer.playerPool[i][j];
	}
	game->turn = buffer.turn;
	game->letterSack.pos = buffer.letterSack.pos;
	for (int i = 0; i < BOARDSIZE; ++i) {
		for (int j = 0; j < BOARDSIZE; ++j) {
			game->tiles[i][j] = buffer.tiles[i][j];
		}
	}

	char *sackBuffer = (char *)malloc(game->letterSack.count * sizeof(char));
	fread(sackBuffer, sizeof(char), game->letterSack.count, save);
	for (int i = 0; i < game->letterSack.count; ++i) {
		game->letterSack.letterPool[i] = sackBuffer[i];
	}

	free(sackBuffer);
	fclose(save);
	notify("Game loaded");
	return 1;
}

//allows player to type in a save file name
void getFileName(char *name)
{
	textattr(16 * INPUT_BG + YELLOW);
	short pos = 0;
	char c;
	do {
		c = getch();
		if (pos < 20 && (c >= 'A' && c <= 'Z' || c >= 'a' && c <= 'z' || c >= '0' && c <= '9')) {
			putch(c);
			name[pos++] = c;
		}
		if (pos > 0 && c == KEY_BACKSPACE) {
			name[pos--] = '\0';
			gotoxy(WORD_INPUT_X + pos, WORD_INPUT_Y);
			putch(' ');
			gotoxy(WORD_INPUT_X + pos, WORD_INPUT_Y);
		}
		if (c == KEY_ESC) {
			name[0] = '\0';
			textbackground(BLACK);
			_setcursortype(_NOCURSOR);
			gotoxy(WORD_INPUT_X - 17, WORD_INPUT_Y);
			cputs("                                       ");
			return;
		}
	} while (name[0] == '\0' || c != KEY_ENTER);
	textbackground(BLACK);
	_setcursortype(_NOCURSOR);
	sprintf(name, "%s.bin", name);
	gotoxy(WORD_INPUT_X - 17, WORD_INPUT_Y);
	cputs("                                       ");
}

//counts points for placing selected word, can display tutorial info
int countPoints(cell_t cells[BOARDSIZE][BOARDSIZE], int x, int y, char *word, dir_t word_dir, const int letterpoints[ALPH_LEN], short tut)
{

#ifdef DEBUG
	gotoxy(x, y);
	putch('X');
#endif // DEBUG

	textattr(LIGHTGREEN);
	x -= BOARD_X;
	y -= BOARD_Y;

	int points = 0, mult = 1, i;
	if (0 && x && y);
	for (i = 0; word[i] != '\0'; ++i) {
		int lettMult = 1;
		cell_t cl = (word_dir == VERTICAL ? cells[y + i][x] : cells[y][x + i]);
		if (cl != _) {
			switch (cl)
			{
			case l:
				lettMult = l_MULT;
				break;
			case L:
				lettMult = L_MULT;
				break;
			case w:
				mult *= w_MULT;
				break;
			case W:
				mult *= W_MULT;
				break;
			}
		}
		points += (letterpoints[word[i] - 'A'] * lettMult);
		if (tut) {		
			if (lettMult > 1) {
				gotoxy(TUT_X, TUT_Y + i);
				putch(lettMult + '0');
				putch('x');
			}
			else {
				gotoxy(TUT_X, TUT_Y + i); 
				cputs("  ");
			}
			char br[16];
			sprintf(br, "%d(%c)  ", letterpoints[word[i] - 'A'], word[i]);
			gotoxy(TUT_X + 3, TUT_Y + i);
			cputs(br);
		}
	}
	if (tut) {
		char txt[48];

		gotoxy(TUT_X, TUT_Y + i + 1);
		sprintf(txt, "%dx word  ", mult);
		cputs(txt);

		gotoxy(TUT_X, TUT_Y + i + 3);		
		sprintf(txt, "Total: %d     ", points * mult);
		cputs(txt);
	}
	return points * mult;
}

//checks if 'word' is present in dictionary's array
short dictSearch(int start, int stop, char *word, char **dict)
{
	if (start > stop) return 0;

	short pos = 0;
	while (word[pos] != '\0') {
		if (word[pos] != dict[(stop + start) / 2][pos])
			break;
		++pos;
	}
	if (word[pos] > dict[(stop + start) / 2][pos])
		return dictSearch((stop + start) / 2 + 1, stop, word, dict);
	if (word[pos] < dict[(stop + start) / 2][pos])
		return dictSearch(start, (stop + start) / 2 - 1, word, dict);

	return 1;
}

//automatically sets parameters of dictSearch to browse the entire dictionary and transforms word to lowercase
short checkWord(char *word, dict_t *dict)
{
	if (strlen(word) <= 1)
		return 0;
	if (dict->dictP == NULL)
		return 1;	
	char wordToCheck[BOARDSIZE + 3];
	strcpy(wordToCheck, word);
	for (int i = 0; wordToCheck[i] != '\0'; ++i)
		wordToCheck[i] += 'a' - 'A';
	return dictSearch(0, dict->lines, wordToCheck, dict->dictP);
}

//clears the tutorial display
void clrTut()
{
	for (int i = 0; i < BOARDSIZE + 4; ++i) { 
		gotoxy(TUT_X, TUT_Y + i);
		cputs("                                ");
	}
}

//forms and checks any additional words that would appear if selected word was placed on the board
int autoSearch(char tiles[BOARDSIZE][BOARDSIZE], int x, int y, char *word, dir_t word_dir, dict_t *dict)
{	
	if (dict->dictP == NULL)
		return 1;

	short control = (tiles[BOARDSIZE / 2][BOARDSIZE / 2] == ' ');

	char txt[BOARDSIZE + 32];
	char tilesC[BOARDSIZE][BOARDSIZE];
	for (int i = 0; i < BOARDSIZE; ++i)
		memcpy(tilesC[i], tiles[i], BOARDSIZE * sizeof(tiles[0][0]) );
	if (word_dir == HORIZONTAL) {
		for (int i = 0; word[i] != '\0'; ++i) {
			tilesC[x + i][y] = word[i];
		}
	}
	else {
		for (int i = 0; word[i] != '\0'; ++i) {
			tilesC[x][y + i] = word[i];
		}
	}

	if (tilesC[BOARDSIZE / 2][BOARDSIZE / 2] == ' ') {
		sprintf(txt, "First word must cover cell %c%d", 'A' + BOARDSIZE / 2, 1 + BOARDSIZE / 2);
		notify(txt);
		return 0;
	}
	
	short invert = (word_dir == VERTICAL);
	char formedWord[BOARDSIZE + 1];
	unsigned int i, pts = 0;
	while (x > 0 && y > 0 && tilesC[x - (invert ^ 1)][y - invert] != ' ') {
		invert ? --y : --x;
	}
	int x0 = x, y0 = y;
	for (i = 0; x < BOARDSIZE && y < BOARDSIZE && tilesC[x][y] != ' '; ++i) {
		formedWord[i] = tilesC[x][y];
		invert ? ++y : ++x;
		if (i >= strlen(word))
			++control;
	}
	formedWord[i] = '\0';
	if (strlen(formedWord) > 0 && checkWord(formedWord, dict) == 0) {
		sprintf(txt, "\x22%s\x22 is not allowed", formedWord);
		notify(txt);
		return 0;
	}
	for (unsigned int j = 0, x = x0, y = y0; j < i; ++j) {
		memset(formedWord, '\0', BOARDSIZE + 1);
		while (x > 0 && y > 0 && tilesC[x - invert][y - (invert ^ 1)] != ' ') {
			invert ? --x : --y;
		}
		int n;
		for (n = 0; x < BOARDSIZE && y < BOARDSIZE && tilesC[x][y] != ' '; ++n) {
			formedWord[n] = tilesC[x][y];
			invert ? ++x : ++y;
		}
		formedWord[n] = '\0';
		if (n > 1)
			++control;
		if (strlen(formedWord) > 1 && checkWord(formedWord, dict) == 0) {
			sprintf(txt, "\x22%s\x22 is not allowed", formedWord);
			notify(txt);
			return 0;
		}
		invert ? ++y : ++x;
	}

	if (control)
		return 1;
	else {
		notify("No connection to other words on the board");		
	}
	return 0;
}

//checks if word is present in dictionary as the player types it in
void dynamicCheck(char *word, dict_t *dict)
{	
	textattr(LIGHTGREEN);
	int x = wherex();
	int y = wherey();
	short check;
	char txt[64];
	gotoxy(NOTIF_COORDS);
	if (word[0] == '\0') {
		cputs("                                      ");
		gotoxy(x, y);
		return;
	}

	if (dict->dictP != NULL)
		check = checkWord(word, dict);
	else
		check = 1;

	if (check == 0)
		textcolor(LIGHTRED);
	sprintf(txt, "\x22%s\x22%s found in dictionary             ", word, check == 1 ? "" : " not");
	cputs(txt);
	gotoxy(x, y);
}

//executes the word placement process
void doWordPlacement(gameState_t *game, dict_t *dict, dir_t word_dir, const int lettercount[ALPH_LEN], const int letterpoints[ALPH_LEN],
	cell_t specialcells[BOARDSIZE][BOARDSIZE], const short specialcolors[SPECIAL_CELL_TYPES], const char legend[LEGEND_SIZE][LEGEND_LENGTH],
	int x, int y, short tut)
{
	short taken[7] = { 0 };
	short sucStor[BOARDSIZE + 3] = { 0 };
	char word[BOARDSIZE + 3] = { 0 };
	short pos = 0, c = 0;
	do {						//start word insert loop
		c = getch();
		if (c >= 'a' && c <= 'z')
			c -= ('a' - 'A');
		if (pos <= (BOARDSIZE + 1) && c >= 'A' && c <= 'Z') {
			word[pos] = (char)c;
			if (tut && dict->dictP != NULL)
				dynamicCheck(word, dict);
			short lettIndex = (checkSingleLetter(game->playerPool[game->turn], (char)c, taken, 'A'));
			if (lettIndex == -1) {
				textattr(16 * RED + WHITE);
				sucStor[pos] = 0;
			}
			else {
				textattr(16 * GREEN + WHITE);
				taken[lettIndex] = 1;
				sucStor[pos] = 1;
			}

			putch(c);
			pos += (pos <= BOARDSIZE + 1);
		}
		if (pos > 0 && c == KEY_BACKSPACE) {
			short freeIndex = checkSingleLetter(game->playerPool[game->turn], word[pos - 1], taken, 'D');
			if (sucStor[pos - 1] == 1 && freeIndex >= 0) {
				taken[freeIndex] = 0;
			}
			word[pos - 1] = '\0';
			gotoxy(WORD_INPUT_X + pos - 1, WORD_INPUT_Y);
			textbackground(INPUT_BG);
			putch(' ');
			pos -= (pos > 0);
			gotoxy(WORD_INPUT_X + pos, WORD_INPUT_Y);
			if (tut && dict->dictP != NULL)
				dynamicCheck(word, dict);
		}
		if (c == KEY_ENTER) {
			drawLegend(x, y, 11284, legend);
			_setcursortype(_NOCURSOR);
			do {						//start word placement loop
				drawInnerBoard(specialcells, specialcolors);
				placeTiles(game);

				if (showBoardCompat(x, y, word, word_dir, game->tiles) && tut)
					countPoints(specialcells, x, y, word, word_dir, letterpoints, 1);
				c = cursor(&x, &y);

				drawLegend(x, y, 23572, legend);

				switch (c)
				{
				case 'o':
					word_dir ^= 1;
					break;
				case 't':
					tut ^= 1;
					if (tut == 0)
						clrTut();
					char txt[32];
					sprintf(txt, "Tutorial mode %s", tut == 1 ? "on" : "off");
					notify(txt);
					break;
				case KEY_ENTER:
					gotoxy(x, y);
					if (placeWord(game, wherex(), wherey(), word_dir, word, dict)) {
						game->playerPoints[game->turn] += countPoints(specialcells, wherex(), wherey(), word, word_dir, letterpoints, 0);
						playerGetLetters(game, game->turn);
						game->turn ^= 1;
						c = KEY_ESC;
						break;
					}
				} //endswitch
			} while (c != KEY_ESC); //end word placement loop		

			clrTut();
			drawInnerBoard(specialcells, specialcolors);
			placeTiles(game);
		} //endif ENTER

	} while (c != KEY_ESC); //end word insert loop
	textbackground(BLACK);
	_setcursortype(_NOCURSOR);
	gotoxy(WORD_INPUT_X - 17, WORD_INPUT_Y);
	cputs("                                       ");
	unnotify();
}

void doLetterExchange(gameState_t *game)
{
	short c, select[7] = { 0,0,0,0,0,0,0 };

	do {
		c = getch();
		if (c >= '1' && c <= '7') {
			select[c - '1'] ^= 1;
		}
		displayExchangeHighlight(game, select);
		if (c == KEY_ENTER || c == 'w') {
			playerLetterExchange(game, game->turn, select);
			game->turn ^= 1;
			break;
		}
	} while (c != KEY_ESC);
}

short checkEndGame(gameState_t *game)
{
	if (game->letterSack.count <= game->letterSack.pos) {
		char txt[32], c;
		if (game->playerPoints[0] != game->playerPoints[1])
			sprintf(txt, "Game finished. Player %d wins!", (game->playerPoints[0] < game->playerPoints[1]) + 1);
		else
			sprintf(txt, "Game finished. Draw!");
		notify(txt);
		do {
			c = getch();
		} while (c == '\0');
		return 1;
	}
	else
		return 0;
}