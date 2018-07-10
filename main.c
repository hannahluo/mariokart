#include <LPC17xx.h>
#include <stdlib.h>
#include <stdio.h>
#include <RTL.h>
#include <stdint.h>
#include <math.h>
#include "timer.h"
#include "GLCD.h"
#include "background.c"
#include "car1.c"
#include "car2.c"
#include "canister.c"
#include "finish_line.c"

/* MACROS */
#define MAPWIDTH 320
#define MAPHEIGHT 240
#define PATHSIZE 800
#define MAXCANS 3
#define CANWIDTH 9
#define CANHEIGHT 12
#define OUTTRACKMINX 10
#define OUTTRACKMINY 10
#define OUTTRACKMAXX 310
#define OUTTRACKMAXY 230
#define INTRACKMINX 70
#define INTRACKMINY 70
#define INTRACKMAXX 250
#define INTRACKMAXY 170
// Q:\eng\ece\util\find-com

// Car struct - Holds position and orientation of cars in the game
typedef struct{
	uint32_t x;
	uint32_t y;
	int angle;
	int speed;
	uint8_t height;
	uint8_t width;
	uint8_t lap;
	uint8_t orientation;
	unsigned char * image;
} Car;

// Player struct - Holds data for player in game
typedef struct{
	uint8_t canisters;
	Car* car;
} Player;

// Canister struct - Holds data for powerup canisters in game
typedef struct{
	uint32_t x;
	uint32_t y;
	uint8_t collected;
} Canister;

// GLOBALS // 
int path[PATHSIZE][2] = { 0 };
uint8_t CARWIDTH = 13;
uint8_t CARHEIGHT = 25;
Car* CPU;
Player* player;
Canister* cans[MAXCANS];

// SEMAPHORES //
OS_MUT cpuMut;
OS_MUT playerSpdMut;
OS_MUT playerPosMut;
OS_MUT playerAngMut;
OS_MUT canMut;

// Function to allocate memory space for a car and initialize its values
Car* makeCar(uint32_t x, uint32_t y, int angle, int speed, uint8_t height, uint8_t width, unsigned char * image, uint8_t lap, uint8_t orientation) {
	Car* car = malloc(sizeof(Car));
	car->x = x;
	car->y = y;
	car->angle = angle;
	car->speed = speed;
	car->height = height;
	car->width = width;
	car->lap = lap;
	car->orientation = orientation;
	car->image = image;
	return car;
}

// Rounds a decimal value to an integer and returns it
int round2Int(float n) {
	if(n >= 0) {
		// If the decimal portion of the number is less than .5, then rounds down
		if( (int)n + 0.5 > n ) return (int)n;
		// Otherwise round up
		else return (int)n + 1;
	}
	else {
		// If the decimal portion of the number is less than .5, then rounds down
		if( (int)n - 0.5 < n ) return (int)n;
		// Otherwise round up
		else return (int)n - 1;
	}
}

// Function to display an integer number in binary on the LEDs
void setLeds(uint8_t litNum) {
	int i;
	uint32_t gpio1Val = 0, gpio2Val = 0;
	
	LPC_GPIO1->FIOCLR |= 0xB0000000;
	LPC_GPIO2->FIOCLR |= 0x0000007C;
	
	// Checks which bits in GPIO1 should be on for the number by checking if the number at each bit is divisible by 2 or there is a remainder(indicating LED on)
	if(litNum%2)
		gpio1Val += pow(2,28);
	litNum = litNum >> 1;
	if(litNum%2)
		gpio1Val += pow(2,29);
	litNum = litNum >> 1;
	if(litNum%2)
		gpio1Val += pow(2,31);
	litNum = litNum >> 1;
	
	// Checks which bits in GPIO2 should be on for the number the same way as for GPIO1
	for(i = 0; i < 5; i++) {
		if(litNum%2) {
			gpio2Val += pow(2,i+2);
		}	
		litNum = litNum >> 1;
	}
	
	// Sets all bits in GPIO1 and GPIO2 that should be turned on for the number to be displayed correctly
	LPC_GPIO1->FIOSET |= gpio1Val;
	LPC_GPIO2->FIOSET |= gpio2Val;
}

// Create an array of positions in the centre of the track
void populatePath(void){
	int i=0, j;
	int midDist = (INTRACKMINX-OUTTRACKMINX)/2;
	int jx1 = OUTTRACKMINX + midDist, jx2 = OUTTRACKMAXX - midDist;
	int jy1 = OUTTRACKMINY + midDist, jy2 = OUTTRACKMAXY - midDist;
	// Writes all points along the top x-directional portion of the track to the array
	for(j = jx1; j < jx2; j++){
		path[i][0] = j - (CARWIDTH/2);
		path[i][1] = OUTTRACKMINY + midDist - (CARHEIGHT/2);
		i++;
	}
	// Writes all points along the right y-directional portion of the track to the array
	for(j = jy1; j < jy2; j++){
		path[i][0] = OUTTRACKMAXX - midDist - (CARWIDTH/2);
		path[i][1] = j - (CARHEIGHT/2);
		i++;
	}
	// Writes all points along the bottom x-directional portion of the track to the array
	for(j = jx2; j > jx1; j--){
		path[i][0] = j - (CARWIDTH/2);
		path[i][1] = OUTTRACKMAXY - midDist - (CARHEIGHT/2);
		i++;
	}
	// Writes all points along the left y-directional portion of the track to the array
	for(j = jy2; j > jy1; j--){
		path[i][0] = OUTTRACKMINX + midDist - (CARWIDTH/2);
		path[i][1] = j - (CARHEIGHT/2);
		i++;
	}
}

// Draws an unfilled rectangle at the positions specified
void drawRectangle( int xMin, int yMin, int xMax, int yMax ){
	int i;
	// Draws the horizontal lines of the rectangle
	for(i=xMin;i<xMax;i++){
		GLCD_PutPixel(i,yMin);
		GLCD_PutPixel(i,yMax);
	}
	// Draws the vertical lines of the rectangle
	for(i=yMin;i<yMax;i++){
		GLCD_PutPixel(xMin,i);
		GLCD_PutPixel(xMax,i);
	}
}

// Draws a car based on its orientation
void drawCar( Car* car ){
	// Checks for the angle of the car and then calls the right function to read the bitmap array correctly
	if(car->orientation == 1){
		GLCD_Bitmap_Sideways_Flipped(car->x,car->y,car->width, car->height, car->image);
	}
	else if(car->orientation == 0){
		GLCD_Bitmap_Flipped(car->x,car->y,car->width, car->height, car->image);
	}
	else if(car->orientation == 2){
		GLCD_Bitmap(car->x,car->y,car->width, car->height, car->image);
	}
	else{
		GLCD_Bitmap_Sideways(car->x,car->y,car->width, car->height, car->image);
	}
}

// Handle the canister powerups
__task void CanisterTask(void){
	int i, randNum;
	// Set a new seed based on the initial value of the potentiometer(unknown) to avoid the same seed causing identical spawns each time
	srand(player->car->angle);
	
	// Initialize the canisters to random positions on the track
	for(i=0;i<MAXCANS;i++){
		// Protect canister data
		os_mut_wait(&canMut,0xffff);
		
		cans[i] = malloc(sizeof(Canister));
		randNum = rand() % PATHSIZE;
		cans[i]->x = path[randNum][0];
		cans[i]->y = path[randNum][1];
		cans[i]->collected = 0;
		
		//Release canister data
		os_mut_release(&canMut);
	}
	
	while(1){
		// Protect canister data
		os_mut_wait(&canMut,0xffff);
		
		// Iterate through each canister to check whether they have been picked up by the player
		for(i=0;i<MAXCANS;i++){	
			// If the canister is currently not collected by the player and is on the map, check if the player is picking it up
			if(!cans[i]->collected){
				// Protect player dimension data
				os_mut_wait(&playerPosMut, 0xffff);
				
				// Check for collision with player
				if(cans[i]->x + CANWIDTH >= player->car->x && cans[i]->x <= player->car->x + player->car->width &&  cans[i]->y + CANHEIGHT >= player->car->y && cans[i]->y <= player->car->y + player->car->height ){
					// If the player does not have the maximum amount of canisters, add to their total when they pick the new one up
					if( player->canisters < 8 ) player->canisters++;
					cans[i]->collected = 1;
				}
				
				// Release player dimension data
				os_mut_release(&playerPosMut);
			}
			// If the canister has been collected by the player previously, set a new x and y position to respawn it somewhere else as uncollected
			else{
				randNum = rand() % PATHSIZE;
				cans[i]->x = path[randNum][0];
				cans[i]->y = path[randNum][1];
				cans[i]->collected = 0;
			}
		}
		// Print out the amount of canisters the player currently has
		setLeds(pow(2, player->canisters)-1);
		
		//Release canister data
		os_mut_release(&canMut);
		
		os_tsk_pass();
	}
}

// Draw all graphics to LCD screen
__task void DrawTask(void){
	int i, j, xPlayerPrev = player->car->x, yPlayerPrev = player->car->y, xCPUPrev = CPU->x, yCPUPrev = CPU->y;
	int playerHeightPrev = player->car->height, playerWidthPrev = player->car->width, CPUHeightPrev = CPU->height, CPUWidthPrev = CPU->width;
	char playerLap[25];
	char cpuLap[25];
	
	GLCD_SetTextColor(0xFFE0);
	GLCD_Clear(0x07E0);

	// Drawing the boundary of the track
	drawRectangle(OUTTRACKMINX-1,OUTTRACKMINY-1,OUTTRACKMAXX+1,OUTTRACKMAXY+1);
	drawRectangle(INTRACKMINX+1,INTRACKMINY+1,INTRACKMAXX-1,INTRACKMAXY-1);
	
	// Colour the whole track grey with a bitmap to be faster than colouring each individual pixel
	// Colours the horizontal lines of the track
	for(i = OUTTRACKMINX; i < OUTTRACKMAXX; i += 10) {
		for(j = OUTTRACKMINY; j < INTRACKMINY; j += 10) {
			GLCD_Bitmap(i, j, 10, 10, (unsigned char *)&BACKGROUND_pixel_data);
		}	
		for(j = INTRACKMAXY; j < OUTTRACKMAXY; j += 10) {
			GLCD_Bitmap(i, j, 10, 10, (unsigned char *)&BACKGROUND_pixel_data);
		}	
	}
	// Colours the vertical lines of the track
	for(j = INTRACKMINY; j < INTRACKMAXY; j += 10) {
		for(i = OUTTRACKMINX; i < INTRACKMINX; i += 10) {
			GLCD_Bitmap(i, j, 10, 10, (unsigned char *)&BACKGROUND_pixel_data);
		}
		for(i = INTRACKMAXX; i < OUTTRACKMAXX; i += 10) {
			GLCD_Bitmap(i, j, 10, 10, (unsigned char *)&BACKGROUND_pixel_data);
		}
	}
	// Draw text as black
	GLCD_SetTextColor(0x0000);
	GLCD_SetBackColor(0xFFE0);
	
	while(1){
		// Cover the previous position of the player and the CPU with grey so that the track appears unchanged once a car moves off of it
		GLCD_Bitmap(xPlayerPrev,yPlayerPrev, playerWidthPrev, playerHeightPrev, (unsigned char *)&BACKGROUND_pixel_data);
		GLCD_Bitmap(xCPUPrev,yCPUPrev, CPUWidthPrev, CPUHeightPrev, (unsigned char *)&BACKGROUND_pixel_data);
		
		//Draw the finish line
		for(i = 0; i < 15; i++){
			GLCD_Bitmap(156, OUTTRACKMINY + 4*i, 8, 4, (unsigned char *)&FINISH_LINE_pixel_data);
		}
		
		// Draw the canisters on the track
		for(i=0;i<MAXCANS;i++){
			// Protect canister data
			os_mut_wait(&canMut,0xffff);
			
			// If the canister has been collected, then recolour the track grey where it used to be
			if(cans[i]->collected){
				GLCD_Bitmap(cans[i]->x,cans[i]->y, CANWIDTH, CANHEIGHT, (unsigned char *)&BACKGROUND_pixel_data);
			}
			// If the canister has not been collected, then draw the canister image at its location
			else{
				GLCD_Bitmap(cans[i]->x,cans[i]->y, CANWIDTH, CANHEIGHT, (unsigned char *)&CANISTER_pixel_data);
			}
			
			//Release canister data
			os_mut_release(&canMut);
		}
		
		// If the player has passed 5 laps then display the win screen
		if( ( player->car->lap - 1 )/4 == 5 ){
			GLCD_Clear(0x0000);
			GLCD_SetTextColor(0xFFFF);
			GLCD_SetBackColor(0x0000);
			while(1){
				GLCD_DisplayString(15, 21, 0, "YOU WIN!");
			}
		}
		
		// Overwrite and display the player lap text with the current lap they are on
    sprintf(playerLap, "PLAYER LAP: %i/5", (player->car->lap - 1)/4);
		GLCD_DisplayString(14, 20, 0, playerLap);
		
		// Draw the player at its current position and store its data to use in the next iteration
		
		// Protect player dimension data
		os_mut_wait(&playerPosMut, 0xffff);
		
		drawCar(player->car);		
		xPlayerPrev = player->car->x;
		yPlayerPrev = player->car->y;
		playerHeightPrev = player->car->height;
		playerWidthPrev = player->car->width;
		
		// Release player dimension data
		os_mut_release(&playerPosMut);
		
		// Wait if the CPU's values are being updated by another task
		os_mut_wait(&cpuMut, 0xFFFF);
		
		// If the CPU has passed 5 laps before the player display the lose screen
		if( ( CPU->lap - 1 )/4 == 5 ){
			GLCD_Clear(0x0000);
			GLCD_SetTextColor(0xFFFF);
			GLCD_SetBackColor(0x0000);
			while(1){
				GLCD_DisplayString(15, 20, 0, "YOU LOSE...");
			}
		}
		
		// Overwrite and display the CPU lap text with the current lap it is on
		sprintf(cpuLap,    "CPU LAP:    %i/5", (CPU->lap - 1)/4);
		GLCD_DisplayString(15, 20, 0, cpuLap);
			
		// Draw the CPU at its current position and store its data to use in the next iteration
		drawCar(CPU);
		xCPUPrev = CPU->x;
		yCPUPrev = CPU->y;
		CPUHeightPrev = CPU->height;
		CPUWidthPrev = CPU->width;
		
		// Release CPU mutex
		os_mut_release(&cpuMut);

		os_tsk_pass(); 
	}
}

// Reads the potentiometer values and sets them to the player angle
__task void ReadPotentiometerTask(void) {
	LPC_PINCON->PINSEL1 &= ~(0x03<<18);
	LPC_PINCON->PINSEL1 |= (0x01<<18);
	LPC_SC->PCONP |= (0x01<<12);
	LPC_ADC->ADCR = (1<<2) | (4<<8) | (1<<21); 
	
	while(1) {
		LPC_ADC->ADCR |= (0x01<<24);
		while(!(LPC_ADC->ADGDR & (unsigned int)0x01<<31));
		
		// Protect player angle data
		os_mut_wait(&playerAngMut, 0xffff);
		
		// Convert angle to a value between -180 to +180 degrees
		player->car->angle = (int)(((LPC_ADC->ADGDR & 0x0000FFF0) >> 4) - 2048)*180/2048;
		
		// Release player angle data
		os_mut_release(&playerAngMut);
		
		os_tsk_pass();
	}
}

// Task to move all players in the game, including CPU
__task void MovePlayersTask(void){
	int newX, newY, newCPUX, newCPUY, newWidth, newHeight, newCPUWidth, newCPUHeight, newOrientation, newCPUOrientation;
	// The new location for the player and CPU will be calculated, and they will only be moved to the new positions if they are not moving into a collision
	while(1){
		// PLAYER MOVEMENT //
		
		// Protect speed from updating while moving
		os_mut_wait(&playerSpdMut, 0xffff);
		
		// Protect player angle data
		os_mut_wait(&playerAngMut, 0xffff);
		
		// The player moves forward based on their speed and angle
		newX = player->car->x + player->car->speed*round2Int(cos(player->car->angle * 3.14/180));
		newY = player->car->y + player->car->speed*round2Int(sin(player->car->angle * 3.14/180));
		
		// Release speed data
		os_mut_release(&playerSpdMut);
		
		// Turn the player car depending on the angle their car is at
		if((player->car->angle >= -45 && player->car->angle < 45) || player->car->angle > 135 || player->car->angle< -135){
			newHeight = CARWIDTH;
			newWidth = CARHEIGHT;
			if( player->car->angle < 45 && player->car->angle >= -45 ) newOrientation = 1;
			else newOrientation = 3;
		}
		else{
			newHeight = CARHEIGHT;
			newWidth = CARWIDTH;
			if( player->car->angle < 135 && player->car->angle >= 45 ) newOrientation = 0;
			else newOrientation = 2;
		}
		
		// Release player angle data
		os_mut_release(&playerAngMut);
		
		// Check for collision with CPU car
		if(!( newX + newWidth >= CPU->x && newX <= CPU->x + CPU->width &&  newY + newHeight >= CPU->y && newY <= CPU->y + CPU->height )){
			// Check for collision with outer walls
			if( newX >= OUTTRACKMINX && newX <= OUTTRACKMAXX - newWidth && newY >= OUTTRACKMINY && newY <= OUTTRACKMAXY - newHeight ){
				// Check for collision with the inner walls
				if( !( newX + newWidth >= INTRACKMINX && newX <= INTRACKMAXX &&  newY + newHeight >= INTRACKMINY && newY <= INTRACKMAXY ) ){
					// If no collision will occur at the new x and y position, update the player's position
					
					// Protect player dimension data
					os_mut_wait(&playerPosMut, 0xffff);
					
					player->car->x = newX;
					player->car->y = newY;
					player->car->height = newHeight;
					player->car->width = newWidth;
					player->car->orientation = newOrientation;
					
					// Release player dimension data
					os_mut_release(&playerPosMut);
				}
			}
		}
		
		// Increment the player lap if they have passed the correct checkpoint on the track
		if(( player->car->lap%4 == 0 && player->car->x > 160 ) || ( player->car->lap%4 == 1 && player->car->y > 120 ) || 
			( player->car->lap%4 == 2 && player->car->x < 160 ) || ( player->car->lap%4 == 3 && player->car->y < 120 )) {
			player->car->lap++;
		}
		
		// CPU MOVEMENT //
		// Move the CPU along a straight line in each direction to follow the track. Changes the CPU car angle depending on the direction it is moving in
		if(CPU->y <= 40 && CPU->x < 270){
			newCPUX = CPU->x + CPU->speed;
			newCPUY = CPU->y;
			newCPUOrientation = 1;
			CPU->angle = 0;
		}
		else if(CPU->x >= 270 && CPU->y < 195){
			newCPUY = CPU->y + CPU->speed;
			newCPUX = CPU->x;
			newCPUOrientation = 0;
			CPU->angle = 90;
		}
		else if(CPU->y >= 195 && CPU->x > 40){
			newCPUX = CPU->x - CPU->speed;
			newCPUY = CPU->y;
			newCPUOrientation = 3;
			CPU->angle = 180;
		}
		else if(CPU->x <= 40 && CPU->y > 40){
			newCPUY = CPU->y - CPU->speed;
			newCPUX = CPU->x;
			newCPUOrientation = 2;
			CPU->angle = -90;
		}
		
		// Change CPU data based on its current orientation
		if((CPU->angle > -45 && CPU->angle < 45) || CPU->angle > 135 || CPU->angle < -135){
			newCPUHeight = CARWIDTH;
			newCPUWidth = CARHEIGHT;
		}
		else{
			newCPUHeight = CARHEIGHT;
			newCPUWidth = CARWIDTH;
		}
		
		// Protect CPU data from being read by DrawTask
		os_mut_wait(&cpuMut, 0XFFFF);
		
		// Check whether the CPU will not be colliding with the player
		if(!( newCPUX + newCPUWidth >= player->car->x && newCPUX <= player->car->x + player->car->width &&  newCPUY + newCPUHeight >= player->car->y && newCPUY <= player->car->y + player->car->height )){
			// If no collision occurs, then update the CPU position and orientation
			CPU->x = newCPUX;
			CPU->y = newCPUY;
			CPU->height = newCPUHeight;
			CPU->width = newCPUWidth;
			CPU->orientation = newCPUOrientation;
		}
		
		// Increment the CPU lap if they have passed the correct checkpoint on the track
		if(( CPU->lap%4 == 0 && CPU->x > 160 ) || ( CPU->lap%4 == 1 && CPU->y > 120 ) || 
			( CPU->lap%4 == 2 && CPU->x < 160 ) || ( CPU->lap%4 == 3 && CPU->y < 120 )) {
			CPU->lap++;
		}
				
		// Release mutex protecting CPU data
		os_mut_release(&cpuMut);
		
		os_tsk_pass();
	}
}

// Task to check whether the player has used their boost powerup by pushing the button
__task void PushButtonTask(void){
	int boostActive = 0;
	int buttonCurr;
	uint32_t limit = 3;
	uint32_t diff = 0;
	uint32_t initial;
	
	// Store the previous state of the button to prevent the task continually reading a button push and only activating upon a change in button state
	int buttonPrev = LPC_GPIO2->FIOPIN & (0x01 << 10);

	while(1) {
		buttonCurr = LPC_GPIO2->FIOPIN & (0x01 << 10);
		// Check if the button is pressed and has not been registered as already pressed
		if((buttonCurr != buttonPrev) && (buttonCurr & (0x01 << 10)) ) {
			//Protect canister data
			os_mut_wait(&canMut, 0xffff);
			
			// If the player has a canister available, activates the boost and sets a new inital time the boost was activated
			if( player->canisters > 0 ){
				initial = timer_read()/1E6;
				boostActive = 1;
				player->canisters--;
			}
			
			//Release canister data
			os_mut_release(&canMut);
		}
		// Calculates the time elapsed since the boost was activated
		diff = (timer_read()/1E6 - initial);
		
		// If the boost is activated, increase the player speed
		if(boostActive) {
			// Protect speed data from being read while also being altered
			os_mut_wait(&playerSpdMut, 0xffff);
			
			// If the boost time has elapsed, reset the player speed and deactivate the boost
			if(diff > limit) {
				player->car->speed = 2;
				boostActive = 0;
			}	
			else player->car->speed = 4;
			
			// Release speed data
			os_mut_release(&playerSpdMut);
		}
		// Store the button press state to use in the next iteration of the task
		buttonPrev = buttonCurr;
		os_tsk_pass();
	}
}

// Starts all tasks and initializes semaphores.
__task void StartTasks(void){
	// Initialize semaphores
	os_mut_init(&cpuMut);
	os_mut_init(&playerSpdMut);
	os_mut_init(&playerPosMut);
	os_mut_init(&playerAngMut);
	os_mut_init(&canMut);
	
	// Create each task
	os_tsk_create(PushButtonTask,1);
	os_tsk_create(ReadPotentiometerTask,1);
	os_tsk_create(MovePlayersTask,1);
	os_tsk_create(CanisterTask,1);
	os_tsk_create(DrawTask,1);
	
	while(1);
}

// Main function called at start of program
int main(void){
	// Allocate data for the player and initialize their car and other values
	player = malloc(sizeof(Player));
	player->car = makeCar(160 - 2*CARWIDTH,20,90,2,CARWIDTH,CARHEIGHT,CAR1_pixel_data,0,1);
	player->canisters = 0;
	
	// Initialize the CPU car values
	CPU = makeCar(160 - 2*CARWIDTH,40,90,2,CARWIDTH,CARHEIGHT,CAR2_pixel_data,0,1);
	
	// Initialize LCD Screen
	GLCD_Init();
	
	// Initalize bits to be input
	LPC_GPIO1->FIODIR |= 0xB0000000;
	LPC_GPIO2->FIODIR |= 0x0000007C;
	
	// Fill the path array
	populatePath();
	
	// Initalize timer 
	timer_setup();
	
	// Start the tasks.
	os_sys_init(StartTasks);
}
