/*****************************************************************************
* Basic Frontenis for GBA.
* Author: Ryoga a.k.a. JDURANMASTER
*
******************************************************************************
*    - Testing GBA mode 4, GBA mode 3, double buffering, keyboard controlling.
*    - No Sound Effects implemented yet.
*
******************************************************************************/  

#include <stdlib.h>
#include <stdio.h>
#include "font.h"

#include "gba.h"         //GBA register definitions.
#include "fade.h"        //background fades.
#include "keypad.h"      //keypad defines
#include "dispcnt.h"     //REG_DISPCNT register defines
#include "dma.h"         //dma defines.
#include "dma.c"         //dma copy function.
#include "util.h"         //utils defines.
#include "util.c"         //utils functions.

//gfx
#include "Rlogo.h"        //8-bit Ryoga Logo
#include "HKlogo.h"       //8-bit HK logo
#include "saludIntro.h"   //8-bit Health logo.
#include "tabfrontenis.h" //main screen

//some useful colors
#define BLACK 0x0000
#define WHITE 0xFFFF
#define BLUE 0xEE00
#define CYAN 0xFF00
#define GREEN 0x0EE0
#define RED 0x00FF
#define MAGENTA 0xF00F
#define BROWN 0x0D0D

//defines for the video system
#define DISPLAY_CONTROLLER *(unsigned long*)0x4000000
#define VIDC_BASE_HALF	((volatile unsigned short int*) 0x04000000)	
#define VCOUNT		(VIDC_BASE_HALF[3])	


unsigned short* ScreenBuffer = (unsigned short*)0x6000000;
unsigned short* ScreenPal = (unsigned short*)0x5000000;

#define VIDEO_MODE_3 0x3
#define BACKGROUND2 0x400
#define SCREEN_W 240
#define SCREEN_H 160

//game constants
#define BALL_SIZE 6
#define PADDLE_WIDTH 8
#define PADDLE_HEIGHT 28
#define PADDLE_SPEED 2

//global variables
int paddleX[3];
int paddleY[3];
int ballX, ballY;
int velX = 2, velY = 1;
int score1=0,score2=0;

//clear VideoBuffer with some unused VRAM
void ClearBuffer()
{
	REG_DM3SAD = 0x06010000;//Source Address - Some unused VRAM
	REG_DM3DAD = 0x06000000;//Destination Address - Front buffer
	REG_DM3CNT = DMA_ENABLE | DMA_SOURCE_FIXED | 19200;
}

//wait for the screen to stop drawing
void WaitForVsync()
{
	while((volatile u16)REG_VCOUNT != 160){}
}

//Wait until the start key is pressed
void WaitForStart()
{
	
	u8 t=0;//used for detecting a single press
	
	while (1)
	if( KEY_DOWN(KEYSTART) )
	{
		t++;
		if(t<2){
			return;
		}
	}
	else{
		t = 0;
	}
}

//Wait until the A key is pressed
void WaitForAButton()
{
	
	u8 t=0;//used for detecting a single press
	
	while (1)
	if( KEY_DOWN(KEYA) )
	{
		t++;
		if(t<2){
			return;
		}
	}
	else{
		t = 0;
	}
}

void setMode(int mode)
{
    DISPLAY_CONTROLLER = mode | BACKGROUND2;
}

inline void drawpixel(int x, int y, unsigned short color)
{
	videoBuffer[y * 240 + x] = color;
}

inline unsigned short getpixel(int x, int y)
{
    return videoBuffer[y * 240 + x];
}

void drawbox(int left, int top, int right, int bottom, unsigned short color)
{
	int x, y;
    for(y = top; y < bottom; y++)
        for(x = left; x < right; x++)
            drawpixel(x, y, color);
}

int buttonPressed(int button)
{
    //pointer to the button interface
    volatile unsigned int *BUTTONS = (volatile unsigned int *)0x04000130;
    
    //see if UP button is pressed
    if (!((*BUTTONS) & button))
        return 1;
    else
        return 0;
}

//draw text using characters contained in font.h
void print(int left, int top, char *str, unsigned short color)
{
    int x, y, draw;
    int pos = 0;
    char letter;
    
    //look at all characters in this string
    while (*str)
    {
        //get current character ASCII code
        letter = (*str++) - 32;
        
        //draw the character
        for(y = 0; y < 8; y++)
            for(x = 0; x < 8; x++)
            {
                //grab a pixel from the font character
                draw = font[letter * 64 + y * 8 + x];
                
                //if pixel = 1, then draw it
                if (draw)
                    drawpixel(left + pos + x, top + y, color);
            }

        //jump over 8 pixels
        pos += 8;
    }
}

void printScores()
{
    char s[5];
    drawbox(0,0,20,10,BLACK);

    sprintf(s,"%i",score1);
    print(5,0,s,WHITE);
}

void eraseBall()
{
	drawbox(ballX, ballY, ballX+BALL_SIZE, ballY+BALL_SIZE, BLACK);
}

void updateBall()
{
    ballX += velX;
    ballY += velY;

    //did ball hit right wall?
    if (ballX > SCREEN_W - BALL_SIZE - 1)
    {
        velX *= -1;
        score1++;
    }

    //did ball hit left wall?
    if (ballX < 1)
    {
        velX *= -1;
        score2++;
    }

    //did ball hit top or bottom walls?
    if (ballY < 12 || ballY > SCREEN_H - BALL_SIZE - 1)
        velY *= -1;

}

void drawBall()
{
	drawbox(ballX, ballY, ballX+BALL_SIZE, ballY+BALL_SIZE, WHITE);
}

void erasePaddle1()
{
	drawbox(paddleX[1], paddleY[1], paddleX[1] + PADDLE_WIDTH, paddleY[1] + PADDLE_HEIGHT, BLACK);
}

// show the game intro.
void showGameIntro()
{
	int loop;
	
	EraseScreen();
	
	// logo - saludIntro
	for(loop=0;loop<256;loop++) {
      	ScreenPal[loop] = saludIntroPalette[loop];     
   	}

   	for(loop=0;loop<19200;loop++) {
      	ScreenBuffer[loop] = saludIntroData[loop];
   	}
	
    WaitForVblank();
	Flip();
	Sleep(2500);
	EraseScreen();
   	
	// logo Hammer Keyboard Studios.
	for(loop=0;loop<256;loop++) {
      	ScreenPal[loop] = HKlogoDataPalette[loop];     
   	}

   	for(loop=0;loop<19200;loop++) {
      	ScreenBuffer[loop] = HKlogoDatadata[loop];
   	}
	
   	WaitForVblank();
	Flip();
	Sleep(2000);
	EraseScreen();
   	
	// logo - Rlogo
	for(loop=0;loop<256;loop++) {
      	ScreenPal[loop] = RlogoPalette[loop];     
   	}

   	for(loop=0;loop<19200;loop++) {
      	ScreenBuffer[loop] = Rlogodata[loop];
   	}
	
   	WaitForVblank();
	Flip();
	Sleep(2000);
	EraseScreen();
	
	// main tab
	for(loop=0;loop<256;loop++) {
      	ScreenPal[loop] = tabfrontenisPalette[loop];     
   	}

   	for(loop=0;loop<19200;loop++) {
      	ScreenBuffer[loop] = tabfrontenisdata[loop];
   	}
	
   	WaitForVblank();
	Flip();
	Sleep(2000);
}

void showIniText()
{
	
	drawbox(0, 0, 239, 159, BLACK);
	
	print(23-16,1,"FROTENIS IS A SPORT THAT IS", GREEN);
	print(23-16,11,"PLAYED IN A 30 METERS PELOTA", GREEN);
	print(23-16,22,"COURT USING RACQUETS AND", GREEN);
	print(23-16,33,"RUBBER BALLS.", GREEN);
	
	print(23-16,55,"THE SPORT WAS DEVELOPED IN", GREEN);
	print(23-16,66,"MEXICO AROUND 1990 AND IS", GREEN);
	print(23-16,77,"ACCREDITED AS A BASQUE", GREEN);
	print(23-16,88,"PELOTA SPECIALITY.", GREEN);
	
	print(60-16,110,"PRESS THE A BUTTON", RED);
	print(80-16,121,"TO CONTINUE", RED);
	
	print(25-16,140,"PROGRAMMED BY JDURANMASTER", WHITE);
	
	WaitForAButton();
}

void drawPaddle1()
{
	drawbox(paddleX[1], paddleY[1], paddleX[1] + PADDLE_WIDTH, paddleY[1] + PADDLE_HEIGHT, RED);
}


void updatePaddle1(){

    //check for UP button press
    if (buttonPressed(64))
    {
        if (paddleY[1] > 10)
            paddleY[1] -= PADDLE_SPEED;
    }
    
    //check for DOWN button press
    if (buttonPressed(128))
    {
        if (paddleY[1] < SCREEN_H - PADDLE_HEIGHT - 1)
            paddleY[1] += PADDLE_SPEED;
    }
}

void checkCollisions()
{
    int x,y;
    
    //see if ball hit a paddle
    x = ballX + BALL_SIZE/2;
    y = ballY + BALL_SIZE/2;
    if (getpixel(x,y) != BLACK)
    {
        //we have a hit! 
        velX *= -1;
        ballX += velX;
    }
}

void waitRetrace()
{
	while (VCOUNT != 160);
	while (VCOUNT == 160);
}

int main(void)
{
    //Enable background 2 and set mode to MODE_4
	setMode(MODE_4 | OBJ_MAP_1D | BG2_ENABLE);
	
	showGameIntro();
	WaitForStart();
	EraseScreen();		

	//clear buffer
	ClearBuffer();
	
	//Change video mode to MODE_3 with background 2 enable.
	setMode(MODE_3 | BG2_ENABLE);
	
	showIniText();
	
    //init the ball
    ballX = SCREEN_W / 2 - BALL_SIZE / 2;
    ballY = 40;
    
    //init the left paddle
    paddleX[1] = 10;
    paddleY[1] = SCREEN_H / 2 - PADDLE_HEIGHT / 2;
    
    //init the right paddle
    paddleX[2] = SCREEN_W - 20;
    paddleY[2] = SCREEN_H / 2 - PADDLE_HEIGHT / 2;

    //clear the screen
    drawbox(0, 0, 239, 159, BLACK);
    
    //display title
    print(55-16,1,"BASIC FRONTENIS - 2016", GREEN);


    //game loop
    while(1)
    {
        waitRetrace();
        eraseBall();
        erasePaddle1();
        updatePaddle1();
        updateBall();

        drawPaddle1();
        checkCollisions();
        drawBall();

        printScores();
    }

    return 0;
}
