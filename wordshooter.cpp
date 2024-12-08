//============================================================================
// Name        : cookie-crush.cpp
// Author      : Sibt ul Hussain
// Version     :
// Copyright   : (c) Reserved
// Description : Basic 2D game of Cookie  Crush...
//============================================================================
#ifndef WORD_SHOOTER_CPP
#define WORD_SHOOTER_CPP

//#include <GL/gl.h>
//#include <GL/glut.h>
#include <iostream>
#include <string>
#include <cmath>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include "util.h"
using namespace std;
#define MAX(A,B) ((A) > (B) ? (A):(B)) // defining single line functions....
#define MIN(A,B) ((A) < (B) ? (A):(B))
#define ABS(A) ((A) < (0) ? -(A):(A))
#define FPS 60

string * dictionary;
int dictionarysize = 370099; 
#define KEY_ESC 27 // A

// 20,30,30
const int bradius = 30; // ball radius in pixels...

constexpr int width = 960, height = 660;
int byoffset = bradius;

int nxcells = (width - bradius) / (2 * bradius);
int nycells = (height - byoffset /*- bradius*/) / (2 * bradius);
int nfrows = 2; // initially number of full rows //
float score = 0;
int bwidth = 130;
int bheight = 10;
const int nalphabets = 26;
enum alphabets {
	AL_A, AL_B, AL_C, AL_D, AL_E, AL_F, AL_G, AL_H, AL_I, AL_J, AL_K, AL_L, AL_M, AL_N, AL_O, AL_P, AL_Q, AL_R, AL_S, AL_T, AL_U, AL_V, AL_W, AL_X, AL_Y, AL_Z
};
GLuint texture[nalphabets];
GLuint tid[nalphabets];
string tnames[] = { "a.bmp", "b.bmp", "c.bmp", "d.bmp", "e.bmp", "f.bmp", "g.bmp", "h.bmp", "i.bmp", "j.bmp",
"k.bmp", "l.bmp", "m.bmp", "n.bmp", "o.bmp", "p.bmp", "q.bmp", "r.bmp", "s.bmp", "t.bmp", "u.bmp", "v.bmp", "w.bmp",
"x.bmp", "y.bmp", "z.bmp" };
GLuint mtid[nalphabets];
constexpr int awidth = 60, aheight = 60; // 60x60 pixels cookies...

// Abdul Hadi's Variables
// static array size should be constexpr . constexpr should be knowed at run time else error
constexpr int TOP_PADDING = 30;
constexpr int BOTTOM_PADDING = aheight;
constexpr int COLUMNS = width / awidth;
constexpr int ROWS = (height - TOP_PADDING - BOTTOM_PADDING) / aheight;
constexpr int EMPTY_CELL = -1;
constexpr int MAX_TIME = 150; // In seconds
constexpr int SHOOTER_ALPHABET_X = width / 2 - awidth / 2;
constexpr int SHOOTER_ALPHABET_Y = 0;
constexpr int NEXT_SHOOTER_ALPHABET_X = width - 2 * awidth;
constexpr int NEXT_SHOOTER_ALPHABET_Y = 0;
constexpr int MOVING_DISTANCE = 5;
constexpr int MOVING_DISTANCE_SQUARE = MOVING_DISTANCE * MOVING_DISTANCE;
constexpr int RIGHT = 1, DOWNWARD = 2, RIGHT_DOWNWARD = 3;
constexpr int framesToSkip = 30;
int timeLeft = MAX_TIME;
int movingAlphabetX = SHOOTER_ALPHABET_X;
int movingAlphabetY = SHOOTER_ALPHABET_Y;
int shooterAlphabet, nextShooterAlphabet;
int wordRow, wordColumn, wordLength, wordDirection = 0;
int currentBurstAlphabetRow, currentBurstAlphabetColumn;
int currentFrame = 0;
bool isShooterAlphabetMoving = false;
bool isBursting = false;
double slope;
// time function ke liye ye data type zarori ha pointer wale
time_t start, curr;

ofstream file;

int board[ROWS][COLUMNS]; // 2D-arrays for holding the data...

// Abdul Hadi's Functions
void initializeBoard();
void displayBoard();
void moveShooterAlphabet();
void updateTime();
void burstAlphabet();
void calculateSlope(int mouseX, int mouseY);
void writeWordInFile(string word);
void displayGameOver();
void replaceWord();
char getAplhabeticChar(int alphabet);
bool detectCollision();
bool isInDictionary(string word);
string findLargestWord();
// End

//USED THIS CODE FOR WRITING THE IMAGES TO .bin FILE
void RegisterTextures_Write()
//Function is used to load the textures from the
// files and display
{
	// allocate a texture name
	glGenTextures(nalphabets, tid);
	vector<unsigned char> data;
	ofstream ofile("image-data.bin", ios::binary | ios::out);
	// now load each cookies data...

	for (int i = 0; i < nalphabets; ++i) {

		// Read current cookie
		ReadImage(tnames[i], data);
		if (i == 0) {
			int length = data.size();
			ofile.write((char*)&length, sizeof(int));
		}
		ofile.write((char*)&data[0], sizeof(char) * data.size());

		mtid[i] = tid[i];
		// select our current texture
		glBindTexture(GL_TEXTURE_2D, tid[i]);
		// select modulate to mix texture with color for shading
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		// when texture area is small, bilinear filter the closest MIP map
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
			GL_LINEAR_MIPMAP_NEAREST);
		// when texture area is large, bilinear filter the first MIP map
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// if wrap is true, the texture wraps over at the edges (repeat)
		//       ... false, the texture ends at the edges (clamp)
		bool wrap = true;
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
			wrap ? GL_REPEAT : GL_CLAMP);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
			wrap ? GL_REPEAT : GL_CLAMP);
		// build our texture MIP maps
		gluBuild2DMipmaps(GL_TEXTURE_2D, 3, awidth, aheight, GL_RGB,
			GL_UNSIGNED_BYTE, &data[0]);
	}
	ofile.close();
}

void RegisterTextures()
/*Function is used to load the textures from the
* files and display*/
{
	// allocate a texture name
	glGenTextures(nalphabets, tid);

	vector<unsigned char> data;
	ifstream ifile("image-data.bin", ios::binary | ios::in);

	if (!ifile) {
		cout << " Couldn't Read the Image Data file ";
		//exit(-1);
	}
	// now load each cookies data...
	int length;
	ifile.read((char*)&length, sizeof(int));
	data.resize(length, 0);
	for (int i = 0; i < nalphabets; ++i) {
		// Read current cookie
		//ReadImage(tnames[i], data);
		/*if (i == 0) {
		int length = data.size();
		ofile.write((char*) &length, sizeof(int));
		}*/
		ifile.read((char*)&data[0], sizeof(char)* length);

		mtid[i] = tid[i];
		// select our current texture
		glBindTexture(GL_TEXTURE_2D, tid[i]);
		// select modulate to mix texture with color for shading
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		// when texture area is small, bilinear filter the closest MIP map
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
			GL_LINEAR_MIPMAP_NEAREST);
		// when texture area is large, bilinear filter the first MIP map
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// if wrap is true, the texture wraps over at the edges (repeat)
		//       ... false, the texture ends at the edges (clamp)
		bool wrap = true;
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
			wrap ? GL_REPEAT : GL_CLAMP);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
			wrap ? GL_REPEAT : GL_CLAMP);
		// build our texture MIP maps
		gluBuild2DMipmaps(GL_TEXTURE_2D, 3, awidth, aheight, GL_RGB,
			GL_UNSIGNED_BYTE, &data[0]);
	}
	ifile.close();
}

// ye function bubble banane ke liye ha
void DrawAlphabet(const alphabets &cname, int sx, int sy, int cwidth = 60, int cheight = 60)
	/*Draws a specfic cookie at given position coordinate
	* sx = position of x-axis from left-bottom
	* sy = position of y-axis from left-bottom
	* cwidth= width of displayed cookie in pixels
	* cheight= height of displayed cookiei pixels.
	* */
{
	float fwidth = (float)cwidth / width * 2, fheight = (float)cheight / height * 2;
	float fx = (float)sx / width * 2 - 1, fy = (float)sy / height * 2 - 1;

	glPushMatrix();
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, mtid[cname]);
	glBegin(GL_QUADS);
	glTexCoord2d(0.0, 0.0);
	glVertex2d(fx, fy);
	glTexCoord2d(1.0, 0.0);
	glVertex2d(fx + fwidth, fy);
	glTexCoord2d(1.0, 1.0);
	glVertex2d(fx + fwidth, fy + fheight);
	glTexCoord2d(0.0, 1.0);
	glVertex2d(fx, fy + fheight);
	glEnd();

	glColor4f(1, 1, 1, 1);

	//	glBindTexture(GL_TEXTURE_2D, 0);

	glDisable(GL_TEXTURE_2D);
	glPopMatrix();

	//glutSwapBuffers();
}

// ye fuction random alphabet deta ha through random numbers 0 - 25
int GetAlphabet() {
	return GetRandInRange(0, 26);
}

void Pixels2Cell(int px, int py, int & cx, int &cy)
{
	cx = px / awidth ;
	cy = (py - BOTTOM_PADDING) / aheight;
}

void Cell2Pixels(int cx, int cy, int& px, int& py)
{
	px = cx * awidth;
	py = cy * aheight + BOTTOM_PADDING;
}

// shooter ki patti banane ke liye
void DrawShooter(int sx, int sy, int cwidth = 60, int cheight = 60)
{
	float fwidth = (float)cwidth / width * 2, fheight = (float)cheight / height * 2;
	float fx = (float)sx / width * 2 - 1, fy = (float)sy / height * 2 - 1;

	glPushMatrix();
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, -1);
	glBegin(GL_QUADS);
	glTexCoord2d(0.0, 0.0);
	glVertex2d(fx, fy);
	glTexCoord2d(1.0, 0.0);
	glVertex2d(fx + fwidth, fy);
	glTexCoord2d(1.0, 1.0);
	glVertex2d(fx + fwidth, fy + fheight);
	glTexCoord2d(0.0, 1.0);
	glVertex2d(fx, fy + fheight);
	glEnd();

	glColor4f(1, 1, 1, 1);

	//	glBindTexture(GL_TEXTURE_2D, 0);

	glDisable(GL_TEXTURE_2D);
	glPopMatrix();

	//glutSwapBuffers();
}

// Main Canvas drawing function.
void DisplayFunction() {
	// set the background color using function glClearColor.
	// to change the background play with the red, green and blue values below.
	// Note that r, g and b values must be in the range [0,1] where 0 means dim red and 1 means pure red and so on.
	//#if 0
	glClearColor(1/*Red Component*/, 1.0/*Green Component*/,
		1.0/*Blue Component*/, 0 /*Alpha component*/); // Red==Green==Blue==1 --> White Colour
	glClear(GL_COLOR_BUFFER_BIT); //Update the colors

	if (timeLeft > 0)
	{
		//write your drawing commands here or call your drawing functions...
		updateTime();
		displayBoard();

		if (isShooterAlphabetMoving)
		{
			moveShooterAlphabet();
		}
		else
		{
			DrawAlphabet((alphabets)shooterAlphabet, SHOOTER_ALPHABET_X, SHOOTER_ALPHABET_Y, awidth, aheight);
		}

		DrawAlphabet((alphabets)nextShooterAlphabet, NEXT_SHOOTER_ALPHABET_X, NEXT_SHOOTER_ALPHABET_Y, awidth, aheight);
		DrawString(NEXT_SHOOTER_ALPHABET_X - awidth + 10, NEXT_SHOOTER_ALPHABET_Y + 1.3 * aheight, width, height, "Next Alphabet", colors[DARK_GRAY]);
		
		if (isBursting && currentFrame == 0)
		{
			burstAlphabet();
		}

		currentFrame = (currentFrame + 1) % framesToSkip;
		
		DrawString(40, height - 20, width, height + 5, "Score " + Num2Str(score), colors[BLUE_VIOLET]);
		DrawString(width - 250, height - 25, width, height, "Abdul Hadi 24i-2599", colors[BLACK]);
		DrawString(width / 2 - 30, height - 25, width, height, "Time Left:" + Num2Str(timeLeft) + " secs", colors[RED]);

		// #----------------- Write your code till here ----------------------------#
		//DO NOT MODIFY THESE LINES
		DrawShooter((width / 2) - bwidth / 2, 0, bwidth, bheight);
	}
	else
	{
		displayGameOver();
	}

	glutSwapBuffers();
	//DO NOT MODIFY THESE LINES..
}

/* Function sets canvas size (drawing area) in pixels...
*  that is what dimensions (x and y) your game will have
*  Note that the bottom-left coordinate has value (0,0) and top-right coordinate has value (width-1,height-1)
* */
void SetCanvasSize(int width, int height) {
	/*glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, width, 0, height, -1, 1); // set the screen size to given width and height.*/
}

/*This function is called (automatically) whenever any non-printable key (such as up-arrow, down-arraw)
* is pressed from the keyboard
*
* You will have to add the necessary code here when the arrow keys are pressed or any other key is pressed...
*
* This function has three argument variable key contains the ASCII of the key pressed, while x and y tells the
* program coordinates of mouse pointer when key was pressed.
*
* */

void NonPrintableKeys(int key, int x, int y) {
	if (key == GLUT_KEY_LEFT /*GLUT_KEY_LEFT is constant and contains ASCII for left arrow key*/) {
		// what to do when left key is pressed...
	}
	else if (key == GLUT_KEY_RIGHT /*GLUT_KEY_RIGHT is constant and contains ASCII for right arrow key*/) {
	}
	else if (key == GLUT_KEY_UP/*GLUT_KEY_UP is constant and contains ASCII for up arrow key*/) {
	}
	else if (key == GLUT_KEY_DOWN/*GLUT_KEY_DOWN is constant and contains ASCII for down arrow key*/) {
	}

	/* This function calls the Display function to redo the drawing. Whenever you need to redraw just call
	* this function*/
	/*
	glutPostRedisplay();
	*/
}
/*This function is called (automatically) whenever your mouse moves witin inside the game window
*
* You will have to add the necessary code here for finding the direction of shooting
*
* This function has two arguments: x & y that tells the coordinate of current position of move mouse
*
* */

void MouseMoved(int x, int y) {
	//If mouse pressed then check than swap the balls and if after swaping balls dont brust then reswap the ballss
}

/*This function is called (automatically) whenever your mouse button is clicked witin inside the game window
*
* You will have to add the necessary code here for shooting, etc.
*
* This function has four arguments: button (Left, Middle or Right), state (button is pressed or released),
* x & y that tells the coordinate of current position of move mouse
*
* */

void MouseClicked(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON) // dealing only with left button
	{
		if (state == GLUT_UP)
		{
			if (!isShooterAlphabetMoving && !isBursting)
			{
				calculateSlope(x, height - y);
				isShooterAlphabetMoving = true;
			}
		}
	}
	else if (button == GLUT_RIGHT_BUTTON) // dealing with right button
	{

	}

	glutPostRedisplay();
}
/*This function is called (automatically) whenever any printable key (such as x,b, enter, etc.)
* is pressed from the keyboard
* This function has three argument variable key contains the ASCII of the key pressed, while x and y tells the
* program coordinates of mouse pointer when key was pressed.
* */
void PrintableKeys(unsigned char key, int x, int y) {
	if (key == KEY_ESC/* Escape key ASCII*/) {
		exit(1); // exit the program when escape key is pressed.
	}
}

/*
* This function is called after every 1000.0/FPS milliseconds
* (FPS is defined on in the beginning).
* You can use this function to animate objects and control the
* speed of different moving objects by varying the constant FPS.
*
* */
void Timer(int m) {
	glutPostRedisplay();
	glutTimerFunc(1000.0/FPS, Timer, 0);
}

/*
* our gateway main function
* */
int main(int argc, char*argv[]) {

	InitRandomizer(); // seed the random number generator...

	//Dictionary for matching the words. It contains the 370099 words.
	dictionary = new string[dictionarysize]; 
	ReadWords("words_alpha.txt", dictionary); // dictionary is an array of strings
	
	file.open("words_made.txt");

	shooterAlphabet = GetAlphabet();
	nextShooterAlphabet = GetAlphabet();
	initializeBoard();
	time(&start);

	//Write your code here for filling the canvas with different Alphabets. You can use the Getalphabet function for getting the random alphabets

	glutInit(&argc, argv); // initialize the graphics library...
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA); // we will be using color display mode
	glutInitWindowPosition(50, 50); // set the initial position of our window
	glutInitWindowSize(width, height); // set the size of our window
	glutCreateWindow("ITCs Word Shooter by Abdul Hadi - 24i-2599"); // set the title of our game window
	//SetCanvasSize(width, height); // set the number of pixels...

	// Register your functions to the library,
	// you are telling the library names of function to call for different tasks.
	RegisterTextures();
	glutDisplayFunc(DisplayFunction); // tell library which function to call for drawing Canvas.
	glutSpecialFunc(NonPrintableKeys); // tell library which function to call for non-printable ASCII characters
	glutKeyboardFunc(PrintableKeys); // tell library which function to call for printable ASCII characters
	glutMouseFunc(MouseClicked);
	glutPassiveMotionFunc(MouseMoved); // Mouse

	//// This function tells the library to call our Timer function after 1000.0/FPS milliseconds...
	glutTimerFunc(1000.0/FPS, Timer, 0);

	//// now handle the control to library and it will call our registered functions when
	//// it deems necessary...

	// lines for the music
	SDL_Init(SDL_INIT_AUDIO);
	Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096);
	Mix_Music *bgmusic = Mix_LoadMUS("music.mp3"); // yaha par name likhna ha video ka 
	Mix_PlayMusic(bgmusic, -1);
	
	glutMainLoop();

	file.close();

	return 1;
}

// is function ko aik hi bar call karna ha is liye int main me is ko call kiya ha or ye first two rows me bubbles or bakio ko empty karde ga
void initializeBoard()
{
	int i;
	for (i = 0; i < ROWS - nfrows; i++)
	{
		for (int j = 0; j < COLUMNS; j++)
		{
			board[i][j] = EMPTY_CELL;
		}
    }

	for (; i < ROWS; i++)
	{
		for (int j = 0; j < COLUMNS; j++)
		{
			board[i][j] = GetAlphabet(); // random number generate karne ke liye
		}
    }

	string word;
	while (true)
	{
		word = findLargestWord(); // for popping the bubbles before the start of the game becoming the word of the dictionary

		if (!word.empty())
		{
			writeWordInFile(word);
			replaceWord();
			score += word.length();
		}
		else 
		{
			break;
		}
	}

	// board[3][5] = AL_A;
	// board[2][6] = AL_A;
	// board[2][6] = AL_W;

	// board[2][12] = AL_M;
	// board[2][11] = AL_A;

	// isShooterAlphabetChosen = true;
	// shooterAlphabet = AL_M;
}

void replaceWord()
{
	if (wordDirection == RIGHT)
	{
		for (int j = wordColumn; j < wordLength + wordColumn; j++) // is me hum wordcolumn yani jaha se word bannana shuru howa tha waha se le kar jitne us ke lenght thi waha tak new alphabets dal diye
		{
			board[wordRow][j] = GetAlphabet();
		}
	}
	else if (wordDirection == DOWNWARD)
	{
		for (int i = wordRow; i > wordRow - wordLength; i--)
		{
			board[i][wordColumn] = GetAlphabet();
		}
	}
	else if (wordDirection == RIGHT_DOWNWARD)
	{
		for (int i = wordRow, j = 0; (i > wordRow - wordLength) && (j < wordLength + wordColumn); i--, j++)
		{
			board[currentBurstAlphabetRow][currentBurstAlphabetColumn] = GetAlphabet();
		}
	}
}

void displayBoard()
{
	int alphabet, x, y;
	
	for (int i = 0; i < ROWS; i++)
	{
		for (int j = 0; j < COLUMNS; j++)
		{
			alphabet = board[i][j];
			if (alphabet != EMPTY_CELL)
			{ // j or i is ke rows or column yani us ke cell ke cordinates ha
				Cell2Pixels(j, i, x, y); // draw alphabet ko pixel condinate chaheye kiyuke wo tareke se apne cell me bubble bana de
				DrawAlphabet((alphabets)alphabet, x, y, awidth, aheight);
			}
		}
	}
}

bool detectCollision()
{
	int topCX, topCY, leftCX, leftCY, rightCX, rightCY;
	
	Pixels2Cell(movingAlphabetX + awidth / 2, movingAlphabetY + aheight, topCX, topCY);
	Pixels2Cell(movingAlphabetX, movingAlphabetY + aheight / 2, leftCX, leftCY);
	Pixels2Cell(movingAlphabetX + awidth, movingAlphabetY + aheight / 2, rightCX, rightCY);

	if (board[topCY][topCX] != EMPTY_CELL && topCY > 0)  // row = cy and col = cx because row uper se neeche ate ha or col left se right jata ha
	{
		board[topCY - 1][topCX] = shooterAlphabet;
		return true;
	}
	else if (board[leftCY][leftCX] != EMPTY_CELL)
	{
		board[leftCY][leftCX + 1] = shooterAlphabet; // +1 is waja se ke ball to exist ke right pe touch ho gi or condition hum chech left wali ki kare ge
		return true;
	}
	else if (board[rightCY][rightCX] != EMPTY_CELL)
	{
		board[rightCY][rightCX - 1] = shooterAlphabet; // -1 is waja se ke ball to exist ke left pe touch ho gi or condition hum chech right wali ki kare ge
		return true;
	}

	return false;
}

void moveShooterAlphabet()
{
	if (slope >= 0) 
	{
		movingAlphabetX = (movingAlphabetX + sqrt(MOVING_DISTANCE_SQUARE / (1 + slope * slope)));
		movingAlphabetY = (movingAlphabetY + slope * sqrt(MOVING_DISTANCE_SQUARE / (1 + slope * slope)));
	}
	else 
	{
		movingAlphabetX = (movingAlphabetX - sqrt(MOVING_DISTANCE_SQUARE / (1 + slope * slope)));
		movingAlphabetY = (movingAlphabetY - slope * sqrt(MOVING_DISTANCE_SQUARE / (1 + slope * slope)));
	}
	
	if (movingAlphabetX <= 0 || movingAlphabetX >= width - awidth)
	{
		slope = -slope;
	}

	DrawAlphabet((alphabets)shooterAlphabet, movingAlphabetX, movingAlphabetY, awidth, aheight);

	if (detectCollision())
	{
		isShooterAlphabetMoving = false;
		movingAlphabetX = SHOOTER_ALPHABET_X;
		movingAlphabetY = SHOOTER_ALPHABET_Y;
		shooterAlphabet = nextShooterAlphabet;
		nextShooterAlphabet = GetAlphabet();

		DisplayFunction(); // jaldi se bubble ko sahi jaga par lage de take jab hum dictionary se check kare to bubble galat jaga par phas na jaye

		string word = findLargestWord();
	
		if (!word.empty())
		{
			isBursting = true;
			writeWordInFile(word);
		}
	}
}

// text file me word banane ke liye ye function ha jo 0 - 25 number ko a se yani 97 se add kar ke us ke correspoding alphabets file me store kare
char getAlphabeticChar(int alphabet)
{
	return alphabet + 'a';
}

bool isInDictionary(string word)
{
	for ( int i = 0; i < dictionarysize; i++)
	{
		if (word == dictionary[i])
		{
			return true;
		}
	}

	return false;
}

string findLargestWord()
{
	string word, largestWord;

	for (int i = ROWS - 1; i >= 0; i--)
	{
		for (int j = 0; j < COLUMNS; j++)
		{
			for ( int k = j; k < COLUMNS; k++) // ye right check karne ke liye ha
			{
				if (board[i][k] == EMPTY_CELL)
				{
					word = "";
					break;
				}

				word += getAlphabeticChar(board[i][k]); // char ko bar bar add karke words banana ha

				if (isInDictionary(word) && word.length() > largestWord.length())
				{
					wordDirection = RIGHT;
					wordRow = i;
					wordColumn = j;
					largestWord = word;
				}
			}

			word = "";

			for ( int k = i; k >= 0; k--) // ye down check karne ke liye ha
			{
				if (board[k][j] == EMPTY_CELL)
				{
					word = "";
					break;
				}

				word += getAlphabeticChar(board[k][j]);

				if (isInDictionary(word) && word.length() > largestWord.length())
				{
					wordDirection = DOWNWARD;
					wordRow = i;
					wordColumn = j;
					largestWord = word;
				}
			}
			
			word = "";

			for (int m = i, n = j; m >= 0 && n < COLUMNS; m--, n++) // ye diagonl check karne ke liye hota ha
			{
				if (board[m][n] == EMPTY_CELL)
				{
					word = "";
					break;
				}

				word += getAlphabeticChar(board[m][n]);

				if (isInDictionary(word) && word.length() > largestWord.length())
				{
					wordDirection = RIGHT_DOWNWARD;
					wordRow = i;
					wordColumn = j;
					largestWord = word;
				}
			}
		}
	}

	currentBurstAlphabetRow = wordRow; // bursting kaha se shuru ho gi in se pata chale ga
	currentBurstAlphabetColumn = wordColumn;

	wordLength = largestWord.length();
	return largestWord;
}

void burstAlphabet()
{
	if (wordDirection == RIGHT)
	{
		if (currentBurstAlphabetColumn < wordLength + wordColumn)
		{
			board[wordRow][currentBurstAlphabetColumn] = EMPTY_CELL;
			currentBurstAlphabetColumn++;
			score++;
		}
		else
		{
			isBursting = false;
		}
	}
	else if (wordDirection == DOWNWARD)
	{
		if (currentBurstAlphabetRow > wordRow - wordLength)
		{
			board[currentBurstAlphabetRow][wordColumn] = EMPTY_CELL;
			currentBurstAlphabetRow--;
			score++;
		}
		else
		{
			isBursting = false;
		}
	}
	else if (wordDirection == RIGHT_DOWNWARD)
	{
		if ((currentBurstAlphabetRow > wordRow - wordLength) && (currentBurstAlphabetColumn < wordLength + wordColumn))
		{
			board[currentBurstAlphabetRow][currentBurstAlphabetColumn] = EMPTY_CELL;
			currentBurstAlphabetRow--;
			currentBurstAlphabetColumn++;
			score++;
		}
		else
		{
			isBursting = false;
		}
	}
}


void calculateSlope(int mouseX, int mouseY)
{
	slope = (double)(mouseY - (SHOOTER_ALPHABET_Y + aheight / 2)) / (mouseX - (SHOOTER_ALPHABET_X + awidth / 2));
}

void writeWordInFile(string word)
{
	file << word << '\n';
}

void updateTime()
{
	time(&curr); // jan 1 1970 se ab tak jitne sec guzar gaye ha wo curr me dal de ga
	timeLeft = MAX_TIME - difftime(curr, start);
}

void displayGameOver()
{
	DrawString(width / 2 - 79, height / 2, width, height, "GAME OVER", colors[DARK_GRAY]);
	DrawString(width / 2 - 80, height / 2, width, height, "GAME OVER", colors[DARK_GRAY]);
	DrawString(width / 2 - 81, height / 2, width, height, "GAME OVER", colors[DARK_GRAY]);
	DrawString(width / 2 - 80, height / 2 + 1, width, height, "GAME OVER", colors[DARK_GRAY]);
	DrawString(width / 2 - 80, height / 2 - 1, width, height, "GAME OVER", colors[DARK_GRAY]);

	DrawString(width / 2 - 79, height / 2 - 30, width, height, "SCORE : " + Num2Str(score), colors[DARK_GRAY]);
	DrawString(width / 2 - 80, height / 2 - 30, width, height, "SCORE : " + Num2Str(score), colors[DARK_GRAY]);
	DrawString(width / 2 - 81, height / 2 - 30, width, height, "SCORE : " + Num2Str(score), colors[DARK_GRAY]);
	DrawString(width / 2 - 80, height / 2 - 30, width, height, "SCORE : " + Num2Str(score), colors[DARK_GRAY]);
	DrawString(width / 2 - 80, height / 2 - 31, width, height, "SCORE : " + Num2Str(score), colors[DARK_GRAY]);
	DrawString(width / 2 - 80, height / 2 - 29, width, height, "SCORE : " + Num2Str(score), colors[DARK_GRAY]);
}

#endif /* */