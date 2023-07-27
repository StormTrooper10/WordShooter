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
	#include "util.h"

	using namespace std;
	#define MAX(A,B) ((A) > (B) ? (A):(B)) // defining single line functions....
	#define MIN(A,B) ((A) < (B) ? (A):(B))
	#define ABS(A) ((A) < (0) ? -(A):(A))
	#define FPS 10

	string * dictionary;
	int dictionarysize = 370099;
	#define KEY_ESC 27 // A

	// 20,30,30
	const int bradius = 30; // ball radius in pixels...

	int width = 930, height = 660;
	int byoffset = bradius;

	int nxcells = (width - bradius) / (2 * bradius);
	int nycells = (height - byoffset /*- bradius*/) / (2 * bradius);
	int nfrows = 2; // initially number of full rows //
	float score = 0;
	int **board; // 2D-arrays for holding the data...
	int bwidth = 130;
	int bheight = 10;
	int bsx, bsy;
	const int nalphabets = 26;
	enum alphabets { // 0..25
		AL_A, AL_B, AL_C, AL_D, AL_E, AL_F, AL_G, AL_H, AL_I, AL_J, AL_K, AL_L, AL_M
		, AL_N, AL_O, AL_P, AL_Q, AL_R, AL_S, AL_T, AL_U, AL_V, AL_W, AL_X, AL_y, AL_Z
	};
	GLuint texture[nalphabets];
	GLuint tid[nalphabets];
	string tnames[] = { "a.bmp", "b.bmp", "c.bmp", "d.bmp", "e.bmp", "f.bmp", "g.bmp", "h.bmp", "i.bmp", "j.bmp",
	"k.bmp", "l.bmp", "m.bmp", "n.bmp", "o.bmp", "p.bmp", "q.bmp", "r.bmp", "s.bmp", "t.bmp", "u.bmp", "v.bmp", "w.bmp",
	"x.bmp", "y.bmp", "z.bmp" };
	GLuint mtid[nalphabets];
	int awidth = 60, aheight = 60; // 60x60 pixels cookies...

	//My Deck
	
	int check = 0;
	
	#define target_lines 4

	//target alphabets, i.e. the alphabets which game user will target. They will cover the top 4 lines of the screen,
	//On hitting any of the bottom row alphebet, match making will occur, and dictionary will be consulted for any possible
	//vertical and diagnal matches.
	//			M N O O I B S A M N O O I B S
	//			J U I M O W S R J U I M O W S 
	//			    N     
	
	int target_alphabets[target_lines][15];
	int gun_alphabet; //The random alphabet ready for shooting
	float gun_alphabet_x, gun_alphabet_y;
	
	//Screen matrix will divide the screen in 60 x 60 pixels blocks, while keeping the gap from top for score row
	//e.g screen_matrix[0][0][0]	= Row 1, Col 1, x position denoted by 0
	//e.g screen_matrix[0][0][1]	= Row 1, Col 1, y position denoted by 1
	int screen_matrix[10][15][2]; //sm
	
	//store value of screen_matrix col where the mouse click is detected
	int mouse_click_sm_c;
	
	
	//1/2-Animation of gun alphabet in progress, 0-No animation at them moment
	//In case of >0, the value remain >0 until the gun_alphabet reaches the clicked block of screen_matrix, then set to 0
	//1: Move to left
	//2: Move to right
	int anim_in_progress = 0;
	
	float anim_target_x, anim_target_y, anim_hops, anim_hop_x, anim_hop_y;
	
	//No of sec left in game.
	int sec_left;
	
	int highest_score;
	
	//game_mode: 0- Menu, 1-Game, 2-Game Over
	int game_mode = 0;
	
	
	//**** All Functions Prototype here ****
	int GetAlphabet();
	void PrintableKeys(unsigned char key, int x, int y);
	void set_game_mode(int mode);
	string str_reverse(string s);
	void MouseClicked(int button, int state, int x, int y);
	
	//**** Prototypes End ****
	
	
	void set_game_mode(int mode) {
	
		InitRandomizer(); // seed the random number generator...
	
		switch(mode) {
			case 0:
			
				game_mode = 0;
			
				break;
				
			case 1:
				score = 0;
						
				sec_left = 60*2;
			
				game_mode = 1;
				
				anim_in_progress = 0;
				
				int r, c;
				for (r=0; r<target_lines; r++) {
					cout<<endl<<"Alphabets At Rows " << r << endl;
					for (c=0; c<15; c++) {
						
						target_alphabets[r][c] = GetAlphabet();
						
						cout<<(char)(65+target_alphabets[r][c])<<" ";
						
					}
				}
				
				gun_alphabet = GetAlphabet();
				gun_alphabet_x = width / 2;
				gun_alphabet_y = 12;
				
				break;
		}
	
	}
	

	string str_reverse(string s) {
	
		string rs = "";
	
		for(int i=0; i < s.length(); i++) {
			rs = s[i] + rs;
		}
		
		return rs;
	
	}	


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
	void DrawAlphabet(const alphabets &cname, int sx, int sy, int cwidth = 60,
		int cheight = 60)
		/*Draws a specfic cookie at given position coordinate
		* sx = position of x-axis from left-bottom
		* sy = position of y-axis from left-bottom
		* cwidth= width of displayed cookie in pixels
		* cheight= height of displayed cookiei pixels.
		* */
	{
		glColor4f(1, 1, 1, 1);

		//	glBindTexture(GL_TEXTURE_2D, 0);

		glDisable(GL_TEXTURE_2D);
		
		float fwidth = (float)cwidth / width * 2, fheight = (float)cheight
			/ height * 2;
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

		
		glPopMatrix();

		//glutSwapBuffers();
	}
	int GetAlphabet() {
		return GetRandInRange(0, 25);
	}

	void Pixels2Cell(int px, int py, int & cx, int &cy) {
	}
	void Cell2Pixels(int cx, int cy, int & px, int &py)
	// converts the cell coordinates to pixel coordinates...
	{
	}
	void DrawShooter(int sx, int sy, int cwidth = 60, int cheight = 60)

	{
	
		float fwidth = (float)cwidth / width * 2, fheight = (float)cheight
			/ height * 2;
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
	/*
	* Main Canvas drawing function.
	* */
	
	void DisplayFunction() {//Automaticaly and continuously being called.
		int r, c;
		
		// set the background color using function glClearColor.
		// to change the background play with the red, green and blue values below.
		// Note that r, g and b values must be in the range [0,1] where 0 means dim red and 1 means pure red and so on.
		//#if 0
		glClearColor(1/*Red Component*/, 1.0/*Green Component*/,
			1.0/*Blue Component*/, 0 /*Alpha component*/); // Red==Green==Blue==1 --> White Colour
		glClear(GL_COLOR_BUFFER_BIT); //Update the colors
		
		
		if (game_mode == 0) { //menu
			
			//Coordinates origin i.e. 0,0 at bottom left corner.
			
			
			//int width = 930, height = 660;
			
			DrawString(width/2-15, 600, width, height + 5, "Word Shooter ", colors[BLUE_VIOLET]);
			
			glColor3fv(colors[GREEN]);
			DrawShooter(420, 430, 200, 80);
			
			DrawString(465, 465, width, height + 5, "Start Game", colors[WHITE]);
			
			//DrawShooter(int sx, int sy, int cwidth = 60, int cheight = 60)
			
			glColor3fv(colors[RED]);
			DrawShooter(420, 330, 200, 80);
			
			DrawString(465, 365, width, height + 5, "Quit Game", colors[WHITE]);
			
			DrawString(450, 250, width, height + 5, "Last Score: " + Num2Str(score), colors[RED]);
			
			DrawString(450, 200, width, height + 5, "Top Score: " + Num2Str(highest_score), colors[RED]);

			
		}
	
		else if (game_mode == 1) { //game display
		
			//Coordinates origin i.e. 0,0 at bottom left corner.
		
			//Game Header
			DrawString(30, height - 20, width, height + 5, "Score " + Num2Str(score), colors[BLUE_VIOLET]);
			DrawString(width / 2 - 130, height - 25, width, height,
				"Time Left:" + Num2Str(sec_left) + " secs", colors[RED]);
			
			DrawString(770, height - 20, width, height + 5, "Top Score: " + Num2Str(highest_score), colors[BLUE_VIOLET]);	


			//	target_alphabets
			//	gun_alphabet
			
			/*
			//write your drawing commands here or call your drawing functions...
			DrawAlphabet((alphabets)0, 0, height / 2, awidth, aheight);
			DrawAlphabet((alphabets)1, 60, height / 2, awidth, aheight);
			DrawAlphabet((alphabets)2, 120, height / 2, awidth, aheight);
			*/
			
			//Alphabets top rows
			for (r=0; r<target_lines; r++) {
				for (c=0; c<15; c++) {
				
					DrawAlphabet(
						(alphabets)target_alphabets[r][c]
						, screen_matrix[r][c][0]
						, screen_matrix[r][c][1]
						, awidth, aheight
						);
					
					
				}
			}
			
			
			DrawAlphabet((alphabets)gun_alphabet, gun_alphabet_x, gun_alphabet_y, awidth, aheight);
			
			glColor3fv(colors[RED]);
			// #----------------- Write your code till here ----------------------------#
			//DO NOT MODIFY THESE LINES
			DrawShooter((width / 2) - 35, 0, bwidth, bheight);
			
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
		//If mouse pressed then check than swap the balls and if after swaping balls dont brust then reswap the balls

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
		int r, c;
		
		y = height - y;
	
		if (button == GLUT_LEFT_BUTTON) // dealing only with left button
		{
			if (state == GLUT_UP)
			{
				
				
				check = 1;
				
				//DrawShooter(420, 330, 200, 80); for quit game button
				
				if (game_mode == 0) { //Menu mode
				
					if (x >= 420 && x <= 420+200 && y >= 330 && y<= 330+80) { //button clicked
					
						exit(27);
						
					}
					
					//DrawShooter(420, 430, 200, 80); for start game button
				
					if (x >= 420 && x <= 420+200 && y >= 430 && y<= 430+80) { //button clicked
					
						set_game_mode(1);
						
					}
				
				}
				else if (game_mode == 1) { //Game mode
				
					if (anim_in_progress == 0) {
						
						//Set row value to zero, as we just require to check column boundries which are same across al the rows.
						r = 0;
						//for (r=0; r<10; r++) {
							//Detect the column index where the mouse click has occured:
							for (c=0; c<15; c++) { //loop through all columns
							
								//x will contain the exact pixel location x coordinate of the point on screen where
								//mouse click has occured.
								//Check x coordinate falls with-in the boundary of any logical block (60x60) stored 
								//in screen_matrix array:
								if ( x >= screen_matrix[r][c][0] && x <=  screen_matrix[r][c][0] + awidth) {
								
									
									//column c has been matched
									//save the column index value. This column is a logincal column in screen_matrix array.
									mouse_click_sm_c = c; 
									
									if (c <= 7) 
										anim_in_progress = 1;	//move left
									else
										anim_in_progress = 2;	//move right
										
										
									//pixel position x & y of bottom left corner of target block of size 60x60
									anim_target_x = screen_matrix[4][c][0]; 
									anim_target_y = screen_matrix[4][c][1];
									
									anim_hops = 20;
									anim_hop_x = abs( (gun_alphabet_x - anim_target_x) / anim_hops );
									anim_hop_y = abs( (gun_alphabet_y - anim_target_y) / anim_hops );
									
									
									cout<<endl<<"Mouse deteted at col "<<c;
									cout<<endl<<"anim_target_x: "<<anim_target_x;
									cout<<endl<<"anim_target_y: "<<anim_target_y;
									cout<<endl<<"anim_hop_x: "<<anim_hop_x;
									cout<<endl<<"anim_hop_y: "<<anim_hop_y;
									cout<<endl;
									
									
									break;
									
								}
							}
							
							//if (anim_in_progress != 0) break;
						//}
						
					}
				
				
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
		if (key == KEY_ESC/* Escape key ASCII*/ || key == 'Q' || key == 'q') {
			//
			if (game_mode == 0)
				exit(1); // exit the program when escape key is pressed in menu.
			else
				set_game_mode(0);
		}
		
		else if (key == 'S' || key == 's') {
			set_game_mode(1);
		}
		
		
	}

	/*
	* This function is called after every 1000.0/FPS milliseconds
	* (FPS is defined on in the beginning).
	* You can use this function to animate objects and control the
	* speed of different moving objects by varying the constant FPS.
	*
	* */
	unsigned long ms = 0;
	void Timer(int m) {//Automaticaly and continuously being called.

		glutPostRedisplay();
		
		glutTimerFunc(1000.0/FPS, Timer, 0);
		
		ms++;
		
		if (game_mode == 1) {
			
			if (ms % 10 == 0) sec_left--;
			
			if (sec_left == 0) {
				set_game_mode(0);
			}
			
		}
		
		//cout<<endl<<ms;
		//glutPostRedisplay();
		
		if (anim_in_progress > 0) {
			
			switch (anim_in_progress) {
				case 1:
					//towards left		
					
					gun_alphabet_x -= anim_hop_x;
					gun_alphabet_y += anim_hop_y;
					break;
				case 2:
					//towards right	
					gun_alphabet_x += anim_hop_x;
					gun_alphabet_y += anim_hop_y;
						
					break;
			}
			
			
			/*
			if (gun_alphabet_x == anim_target_x && gun_alphabet_y == anim_target_y)
				anim_in_progress = 0;
			*/	
				
			anim_hops--;
			
			if (anim_hops == 0) {
			
				anim_in_progress = 0;
				
				gun_alphabet_x = anim_target_x;
				gun_alphabet_y = anim_target_y;
				
				//--
				//*** Collect Letters to form Words ***
				
				//mouse_click_sm_c
				
				//words collected from screen
				string vertical_word_top_2_bot = "", vertical_word_bot_2_top = "";
				string diag_left_word_top_2_bot = "", diag_left_word_bot_2_top = "";
				string diag_right_word_top_2_bot = "", diag_right_word_bot_2_top = "";
				
				//Corresponding dictionary matches with max length within the words collected from screen
				string vertical_match_top_2_bot = "", vertical_match_bot_2_top = "";
				string diag_left_match_top_2_bot = "", diag_left_match_bot_2_top = "";
				string diag_right_match_top_2_bot = "", diag_right_match_bot_2_top = "";
				
				int r, c;
				
				//+ is concatination operator in case of string. e.g "AB" + "XY" = "ABXY" <=== Concatination
				//"5" + "73" = "573"
				//string a="89", b="21". string c = a + b; =====> c = "8921"
				
				
				//Collectl letter on screen: (DATA PART)
				
				//--
				for (r=0; r<4; r++) 
					vertical_word_top_2_bot += (char)(97 + target_alphabets[r][mouse_click_sm_c]);
					
				vertical_word_top_2_bot += (char)(97 + gun_alphabet);
				
				//--
				vertical_word_bot_2_top = str_reverse(vertical_word_top_2_bot);
				
				//-- Diagnol Left
				c = mouse_click_sm_c - 1;
				r = 3;
				
				diag_left_word_bot_2_top = (char)(97 + gun_alphabet);
				
				while (c >= 0 && r >= 0) {
					diag_left_word_bot_2_top += (char)(97 + target_alphabets[r][c]);
					
					c--;
					r--;
				}
				
				//--
				diag_left_word_top_2_bot = str_reverse(diag_left_word_bot_2_top);
				
				//-- Diagnol Right
				c = mouse_click_sm_c + 1;
				r = 3;
				
				diag_right_word_bot_2_top = (char)(97 + gun_alphabet);
				
				while (c <= 14 && r >= 0) {
					diag_right_word_bot_2_top += (char)(97 + target_alphabets[r][c]);
					
					c++;
					r--;
				}
				
				//--
				diag_right_word_top_2_bot = str_reverse(diag_right_word_bot_2_top);
				
				
				cout<<endl<<"Vertical Top 2 Bottom: "<<vertical_word_top_2_bot;
				//cout<<endl<<"Vertical Bottom 2 Top: "<<vertical_word_bot_2_top;
				
				cout<<endl<<"Diagonal Left Top 2 Bottom: "<<diag_left_word_top_2_bot;
				//cout<<endl<<"Diagnol Left Bottom 2 Top: "<<diag_left_word_bot_2_top;
				
				cout<<endl<<"Diagonal Right Top 2 Bottom: "<<diag_right_word_top_2_bot;
				//cout<<endl<<"Diagnol Right Bottom 2 Top: "<<diag_right_word_bot_2_top;
				
				
				//*** Consult the Dictionary for possible matches ***
				
				cout<<endl<<endl<<"All Matches:-"<<endl;
				
				//ANALYSIS AND DECISION PART
				
				int i, j, matches = 0;
				
				//Traverse whole dictionary, each and every word of it.
				for (i=0; i<dictionarysize; i++) {
					
					//dictionary[i] ===> Current dictionary word
					
					for (j=vertical_word_top_2_bot.length(); j >=1 ; j--) {
						if (vertical_word_top_2_bot.substr(0, j) == dictionary[i]) {
						
							cout<<endl<<"Vertical Top 2 Bot Matched: "<<dictionary[i];
							
							if (vertical_match_top_2_bot.length() < dictionary[i].length())
								vertical_match_top_2_bot = dictionary[i];
							
							matches++;
							break;
						}
					}
					
					for (j=vertical_word_bot_2_top.length(); j >=1 ; j--) {
					
						if (vertical_word_bot_2_top.substr(0, j) == dictionary[i]) {
							
							cout<<endl<<"Vertical Bot 2 Top Matched: "<<dictionary[i];
							
							if (vertical_match_bot_2_top.length() < dictionary[i].length())
								vertical_match_bot_2_top = dictionary[i];
							
							matches++;
							break;
						}
					}
					
					
					for (j=diag_left_word_top_2_bot.length(); j >=1 ; j--) {
						if (diag_left_word_top_2_bot.substr(0, j) == dictionary[i]) {
							
							cout<<endl<<"Diagnol Left Top 2 Bot Matched: "<<dictionary[i];
							
							if (diag_left_match_top_2_bot.length() < dictionary[i].length())
								diag_left_match_top_2_bot = dictionary[i];
							
							matches++;
							break;
						}
					}
					
					
					for (j=diag_left_word_bot_2_top.length(); j >=1 ; j--) {
						if (diag_left_word_bot_2_top.substr(0, j) == dictionary[i]) {

							cout<<endl<<"Diagnol Left Bot 2 Top Matched: "<<dictionary[i];
							
							if (diag_left_match_bot_2_top.length() < dictionary[i].length())
								diag_left_match_bot_2_top = dictionary[i];
							
							matches++;
							break;
						}
					}
					
					
					for (j=diag_right_word_top_2_bot.length(); j >=1 ; j--) {
						if (diag_right_word_top_2_bot.substr(0, j) == dictionary[i]) {
							
							cout<<endl<<"Diagnol Right Top 2 Bot Matched: "<<dictionary[i];
							
							if (diag_right_match_top_2_bot.length() < dictionary[i].length())
								diag_right_match_top_2_bot = dictionary[i];
								
							matches++;
							break;
						}
					}
					
					
					for (j=diag_right_word_bot_2_top.length(); j >=1 ; j--) {
						if (diag_right_word_bot_2_top.substr(0, j) == dictionary[i]) {
							
							cout<<endl<<"Diagnol Right Bot 2 Top Matched: "<<dictionary[i];
							
							if (diag_right_match_bot_2_top.length() < dictionary[i].length())
								diag_right_match_bot_2_top = dictionary[i];
								
							matches++;
							break;
						}
					}
					
					
					
				}
				
				if (matches > 0) {
					
					cout<<endl<<endl<<"Selected Matches:-"<<endl;
					
					/*
					score += vertical_match_top_2_bot.length();
					cout<<endl<<"Vertical Top 2 Bot Matched: "<<vertical_match_top_2_bot;
					*/
					
					score += vertical_match_bot_2_top.length();
					cout<<endl<<"Vertical Bot 2 Top Matched: "<<vertical_match_bot_2_top;
					
					/*
					score += diag_left_match_top_2_bot.length();
					cout<<endl<<"Diagnol Left Top 2 Bot Matched: "<<diag_left_match_top_2_bot;
					*/
					
					score += diag_left_match_bot_2_top.length();
					cout<<endl<<"Diagnol Left Bot 2 Top Matched: "<<diag_left_match_bot_2_top;
					
					/*
					score += diag_right_match_top_2_bot.length();
					cout<<endl<<"Diagnol Right Top 2 Bot Matched: "<<diag_right_match_top_2_bot;
					*/
					
					score += diag_right_match_bot_2_top.length();
					cout<<endl<<"Diagnol Right Bot 2 Top Matched: "<<diag_right_match_bot_2_top;
					
					
					if (score > highest_score)
						highest_score = score;
					
					//Bursting of letters in matched words OR new random letters in place of matched letters.
					
					cout<<endl<<endl<<"Bursting Letter Postions:-";
					
					/*
					//-- Vertical Top 2 Bot
					c = mouse_click_sm_c;
					
					for (r=0; r<vertical_match_top_2_bot.length(); r++) {
					
						target_alphabets[r][c] = GetAlphabet();
						
						cout<<endl<<"Vertical, new alphabet at pos: "<<r<<", "<<c;
						
					}
					*/
					
					
					//-- Vertical Bot 2 Top
					
					c = mouse_click_sm_c;
					
					/*
					E.G :
					
					Vertical case 
					
					t << target_alphabets[0][clicked col]
					u << target_alphabets[1][clicked col]
					y << target_alphabets[2][clicked col]			-- one of the matched char
					e << target_alphabets[3][clicked col]			-- one of the matched char
					m << shooted char 	stored in global var gun_alphabet 
					
					
					Character on screen are: "meyut"	stored in vertical_word_bot_2_top
					
					Suppose max dictionary match is "mey" which length is 3	stored in vertical_match_bot_2_top
					
					How many characters in target_alphabet array to change? : 2	>>> Formula: vertical_match_bot_2_top.length()-1
					
					A loop should run vertical_match_bot_2_top.length()-1 times
					
					*/
					
					r = 3;
					c = mouse_click_sm_c;
					for (i=0; i < vertical_match_bot_2_top.length() -1; i++) {
					
						target_alphabets[r][c] = GetAlphabet();
						
						r--;
						
						cout<<endl<<"Vertical, new alphabet at pos: "<<r<<", "<<c;
						
					}
					
					int count;
					
					/*
					//-- Diagnol Left Top 2 Bot
					c = mouse_click_sm_c - (diag_left_word_top_2_bot.length() - 1);
					r = 4 - (diag_left_word_top_2_bot.length() - 1);
					
					count = 0;
					for (i = c; i <= mouse_click_sm_c; i++) {
						
						//leave if count is equal to the no of letters matched in dictionalry.
						if (count == diag_left_match_top_2_bot.length()) break;
					
						if (r == target_lines) break;
						
						target_alphabets[r][i] = GetAlphabet();
						
						cout<<endl<<"Diag Left, new alphabet at pos: "<<r<<", "<<i;
						
						r++;
						
						count++;
					}
					*/
					
					//-- Diagnol Left Bot 2 Top
					c = mouse_click_sm_c - 1;
					r = 3;
					
					for (i = 0; i < diag_left_match_bot_2_top.length() - 1; i++) {
						
						target_alphabets[r][c] = GetAlphabet();
						
						cout<<endl<<"Diag Left, new alphabet at pos: "<<r<<", "<<c;
						
						c--;
						r--;
					}
					
					/*
					//Diagnol Right Top 2 Bot
					c = mouse_click_sm_c + (diag_right_word_top_2_bot.length() - 1);
					r = 4 - (diag_right_word_top_2_bot.length() - 1);
					
					count = 0;
					for (i = c; i >= mouse_click_sm_c; i--) {
						
						//leave if count is equal to the no of letters matched in dictionalry.
						if (count == diag_right_match_top_2_bot.length()) break;
						
						if (r == target_lines) break;
						
						target_alphabets[r][i] = GetAlphabet();
						
						cout<<endl<<"Diag Right, new alphabet at pos: "<<r<<", "<<i;
						
						r++;
						
						count++;
					}
					*/
					
					//-- Diagnol Right Bot 2 Top
					c = mouse_click_sm_c + 1;
					r = 3;
					
					for (i = 0; i < diag_right_match_bot_2_top.length() - 1; i++) {
						
						target_alphabets[r][c] = GetAlphabet();
						
						cout<<endl<<"Diag Right, new alphabet at pos: "<<r<<", "<<c;
						
						c++;
						r--;
					}
					
					//Reset Gun Alphabet
					
					gun_alphabet = GetAlphabet();
		
					gun_alphabet_x = width / 2;
					gun_alphabet_y = 12;
					
				}
				
				
				cout<<endl;
				
				glutPostRedisplay();
			}
		}
	}

	/*
	* our gateway main function
	* */
	int main(int argc, char*argv[]) {
		int r, c;
	
		InitRandomizer(); // seed the random number generator...
		
		//--
		
		
		for (r=0; r<10; r++) {
			for (c=0; c<15; c++) {
				
				screen_matrix[r][c][0] = c*60 + 15; //Left margin		//X pos at row r & col c
				screen_matrix[r][c][1] = (height-r*60)-60 - 40; //Top Margin	//Y pos at row r & col c
				
			}
		}

		cout<<endl;
		
		
		
		//Dictionary for matching the words. It contains the 370099 words.
		dictionary = new string[dictionarysize]; 
		ReadWords("words_alpha.txt", dictionary); // dictionary is an array of strings
		//print first 5 words from the dictionary
		for(int i=0; i < 5; ++i)
			cout<< " word "<< i << " =" << dictionary[i] <<endl;
			
			
		

		//Write your code here for filling the canvas with different Alphabets. 
		//You can use the GetAlphabet function for getting the random alphabets

		cout<<endl;
		cout<<endl;

		glutInit(&argc, argv); // initialize the graphics library...
		glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA); // we will be using color display mode
		glutInitWindowPosition(50, 50); // set the initial position of our window
		glutInitWindowSize(width, height); // set the size of our window
		glutCreateWindow("ITCs Word Shooter"); // set the title of our game window
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

		glutMainLoop();

		
		return 1;
	}
#endif /* */
