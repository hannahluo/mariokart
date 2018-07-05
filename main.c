#include <LPC17xx.h>
#include <stdlib.h>
#include <stdio.h>
#include <RTL.h>
#include <stdint.h>
#include "GLCD.h"
#include <math.h>
#include "background.c"
#define MAPWIDTH 320
#define MAPHEIGHT 240
#define CARWIDTH 18
#define CARHEIGHT 32
#define PATHSIZE 780
#define MAXCANS 5
// Q:\eng\ece\util\find-com

// CONSTANTS //
uint8_t screenWidth = 10;
uint8_t screenHeight = 10;
uint8_t screenBottomX = 0;
uint8_t screenBottomY = 0;
//int map[MAPWIDTH][MAPHEIGHT] = { 0 };
int path[PATHSIZE][2] = { 0 };

static const unsigned char CAR1_pixel_data[18 * 32 * 3 + 1] =
("MkMkMkMk,c\313Z\313Z\313Z\313Z\313Z\313Z\313Z,cMkMkMkMkMkMkMk,c\212R\206)"
 "\306\020\305\020\305\020\305\020\305\020\245\020\006\031\247\061\313ZMkMkMkMkMk,ciJ"
 "'\031J\031\254!\214!\214!\214!\214!\214!\214!)\031G!\252R,cMkMk,c)B\312\061\314"
 ")\253)\211)H\031H\031H\031H\031\007\031H\031\253!\253)\312\061\252RMkMk,cF!\314)\312"
 "\061mB\062S\362J\362J\362J\322B\221B\016\062i\031I!\212!\347\071MkMk,c\004!\014:\360"
 "J\063S\362J\362J\362J\362J\362J\022K\322Bq:\254!H\031\307\071MkMk,cf)\360RSS\362"
 "J\362J\362J\362J\362J\362J\362J\362J\322BP\062\213!\307\071MkMk,cf)\021S\063S"
 "\362J\362J\362J\362J\362J\362J\362J\362J\362J\221:\254!\307\071MkMk,cf)\021"
 "S\063S\362J\362J\362J\362J\362J\362J\362J\362J\362Jq:\254!\307\071MkMk,cf)\021"
 "S\063S\322B\321B\217:o:o:n:\220:\321B\362J\221:\254!\307\071MkMk,cf)\320J\320"
 "Jn\062\215\062.\063n\063n\063n\063\016\063m\062N\062\016\062k!\307\071MkMk,c%!\311).;r"
 "L\024M\034o\377w\337w\377w\333f\363L\257\063L\"\346\020\307\071MkMk,c\343\030\010"
 "\032\232n\377\227\377\177\377\177\377\177\377\177\377\177\377\177\377\177<"
 "W\226=D\011\206\061MkMkjJ\304\020\212\"\276\207\377\267\377\207\377w\377\177"
 "\377\177\377\177\377\177\377w\276_\272N\246\021%!\252RMk\247\061g\031\252\"\226"
 "]\337\227<\207\327U\064M\065M\024M\030^<g\235Wq\064I\"G\031\307\071MkjJ\350)QL\357"
 ";\364T\320K\215\062\013*,*\013*\315\062\217\063\323\064\256+\060L\307)\212RMk,c("
 "\062\030\206\257Km:N:\260B\322J\322J\362J\220:\314)\211\031QT\226}(:MkMk,c(\062"
 "\272~\260S\260J\063S\362J\362J\362J\362J\362J\322BJ!\061\\\027vIBMkMk,c(*\064"
 "M\255:\022S\362J\362J\362J\362J\362J\362J\362J\016\062l*\363LI:MkMk,ce!\354*"
 "\312\061TS\362J\362J\362J\362J\362J\362J\362J\261B\007\021\313*\347\071MkMk,c\246"
 "!M\063\313\061\063S\362J\362J\362J\362J\362J\362J\362J\262B'\031-+\010:MkMk,c\010"
 "\062Q\\\353\061\023K\362J\362J\362J\362J\362J\362J\362J\261Bh!Q\\(BMkMk,c\010"
 "\062\222\\\353\061\063S\362J\362J\362J\362J\362J\362J\362J\261BH\031r\\(BMkMk"
 ",c\010*qD\013:T[\362J\362J\362J\362J\362J\362J\362J\261Bi\031QDI:MkMk,cE!\355"
 ":\353\061\021S\023S\362J\362J\362J\362J\362J\362J\015\062\007\031k*\347\071MkMk,cF"
 ")\216B,:\360Rt[\022K\362J\362J\362J\322B\262B\212!I\031j!\307\071MkMk,cf)L:\345"
 "\030g!\312)o:\321B\321B\322B.\062H\031\244\010\245\010I\031\307\071MkMk,cF)\013:\211"
 ")\257J-:\252)i!i!i!\253)-\062\314)\346\020(\031\307\071MkMk,cF)\013:\353\061s[\362"
 "J\321B\321B\321B\321B\322J\322BP\062\007\021(\031\307\071MkMk,cF)\014:\353\061t[\022"
 "K\362J\362J\362J\362J\362J\322BP\062\007\021(\031\307\071MkMk,c\010:\251)\013:R[\063"
 "S\362J\362J\362J\261B\221:p:\315)(\031\007\031IJMkMkMk\014c\307\061%!g!\210)g!G"
 "!G!&\031\006\031\006\031\304\020\345\030\350\071,cMkMk");

static const unsigned char CAR2_pixel_data[18 * 32 * 4 + 1] =
("MkMkMky\316\060\214EQa\070a\070a\070a\070a\070a\070EQ\060\214y\316MkMkMkMkMk\232\326"
 "\312\202\003q\002\231\002\241\002\241\002\241\002\241\002\241\002\241\002\231\342p\212z\232"
 "\326MkMkMk\232\326\215\243\345\221\006\222\204q\342P\302P\302P\302P\302PaH\241"
 "`#\201\302\200\353\212\232\326MkMk\020\214\305\201\346y\006z\310\272&\302\304"
 "\271\304\271\304\271\204\261\342\240@\200\040X\201`\241h\357\203MkMk\256s\245"
 "i\210\242\310\322F\312\345\311\345\311\345\311\345\311\345\311\345\311C\261"
 "a\230\000paP\216sMkMk\357\203G\222\351\322\345\311\345\311\345\311\345\311\345"
 "\311\345\311\345\311\345\311\345\311C\261@\230\000`\216{MkMk\357\203\250\252"
 "\351\322\345\311\345\311\345\311\345\311\345\311\345\311\345\311\345\311\345"
 "\311\345\311\302\240\000p\216{MkMk\357\203\250\252\351\322\345\311\345\311\345"
 "\311\345\311\345\311\345\311\345\311\345\311\345\311\345\311\302\240\000p\216"
 "{MkMk\357\203\250\252\351\322\345\311\345\311\345\301\304\301\304\301\304"
 "\301\304\301\345\301\345\311\345\311\302\240\000p\216{MkMk\357\203\250\252\351"
 "\322\304\261\244\251\245y\246I\246I\246I\246I\305y\244\251\244\251\241\240"
 "\000p\216{MkMk\317\203\305iG\212\352b\013C\064U\034o\034o\034o\034o\064U\013C\010J\040"
 "`\000H\216{MkMk\256s\004\021qD\333v\337w\377w\377\177\377\177\377\177\377\177\377"
 "w\277wyN\020\064\343\010\216sMk\060\204\313Ze\021]w\337\257\377\217\377\177\377"
 "\177\377\177\377\177\377\177\377\177\337w}W\333Ne\021\313Z\262\224,{\003Y$!\065"
 "U\337\247\337\247\337w\276w\276w\276w\276w\277w\276o]W\363\064$!\003YL{\256{"
 "\306QL;\317;\373n\272~\221TiBiBiBiBqTyVyN\216+,;\246Q\317\203Mk\317s\024mQ"
 "DL[\007Z\345\211\304\261\304\261\304\261\304\261Cy\303\070(:\020<\024m\317sMkM"
 "k\020|\030\216\323lG\222\011\323&\312\345\311\345\311\345\311\345\311\204\271"
 "\002\251\040`\262l\030\216\020|MkMk\020|\367]\357[h\242f\312\345\311\345\311\345"
 "\311\345\311\345\311\345\311\244\301@pmS\367]\020|MkMk\020|qDHbJ\323&\312\345"
 "\311\345\311\345\311\345\311\345\311\345\311\304\301\302\240\343HqD\020|Mk"
 "Mk\256s\252*dY\250\322\345\311\345\311\345\311\345\311\345\311\345\311\345"
 "\311\345\311C\261\000\070\252*\216sMkMk\020|\020<\007bg\312\345\311\345\311\345\311"
 "\345\311\345\311\345\311\345\311\345\311C\261\242@\020D\020|MkMk\020\204\262"
 "l\007jg\312\345\311\345\311\345\311\345\311\345\311\345\311\345\311\345\311"
 "C\261\242H\262l\020\204MkMk\020|\262\\\007b\310\322\345\311\345\311\345\311\345"
 "\311\345\311\345\311\345\311\345\311C\261\242@\262\\\020|MkMk\020|QD'jJ\323"
 "&\312\345\311\345\311\345\311\345\311\345\311\345\311\345\311#\261\242HQD"
 "\020|MkMk\256s\211Z\245i\210\242\310\322\345\311\345\311\345\311\345\311\345"
 "\311\345\311\244\301\201x\000HeA\216sMkMk\357\203h\232'\202G\212*\323F\312\345"
 "\311\345\311\345\311\345\311\204\271\002\251\040`\000X\000`\216{MkMk\357\203G\212"
 "#A$Ida\204\211\244\261\244\261\244\261\244\261\002y@H\000\060\000(\000X\216{MkMk\357"
 "\203\006\202\205a\351\272\345\261C\201\302X\302X\302X\302XC\201c\241@\200\000"
 "@\000P\216{MkMk\357\203\006\202\245iJ\323&\312\345\301\344\301\344\301\344\301"
 "\344\301\345\301\244\301@\220\000@\000P\216{MkMk\357\203\006\202\245iJ\323g\312"
 "\345\311\345\311\345\311\345\311\345\311\345\311c\271@\220\000@\000P\216{MkMk"
 "\226\265Hr\306q\351\262\011\323&\312\345\301\345\301\304\301C\261\002\251\201"
 "\230\000xaX\004YU\265MkMkMk\060\214E\071\343\060DI\343@\242@\242@\242@!\060\040\060"
 "\000\060\000\040\004\071\060\214MkMk");
 
typedef struct{
	uint32_t x;
	uint32_t y;
	int angle;
	int speed;
	uint8_t lap;
} Car;

typedef struct{
	uint8_t canisters;
	Car* car;
} Player;

typedef struct{
	uint8_t collected;
	uint8_t time;
	uint32_t x;
	uint32_t y;
} Canister;

Car* CPU;
Player* player;
OS_MUT steer;
Canister* cans[MAXCANS];

Car* makeCar(uint32_t x, uint32_t y, int angle, int speed, uint8_t lap) {
	Car* car = malloc(sizeof(Car));
	car->x = x;
	car->y = y;
	car->angle = angle;
	car->speed = speed;
	car->lap = lap;
	return car;
}

void setLeds(uint8_t litNum) {
	int i;
	uint32_t gpio1Val = 0, gpio2Val = 0;
	
	LPC_GPIO1->FIOCLR |= 0xB0000000;
	LPC_GPIO2->FIOCLR |= 0x0000007C;
	
	if(litNum%2)
		gpio1Val += pow(2,28);
	litNum = litNum >> 1;
	
	if(litNum%2)
		gpio1Val += pow(2,29);
	litNum = litNum >> 1;
	
	if(litNum%2)
		gpio1Val += pow(2,31);
	litNum = litNum >> 1;
			
	for(i = 0; i < 5; i++) {
		if(litNum%2) {
			// printf("%i, gpio 2 val being changed\n",gpio2Val);
			gpio2Val += pow(2,i+2);
		}	
		litNum = litNum >> 1;
	}
			
	LPC_GPIO1->FIOSET |= gpio1Val;
	LPC_GPIO2->FIOSET |= gpio2Val;
}

void initCanisters(void){ 
	int i;
	for(i=0;i<MAXCANS;i++){
		cans[i] = malloc(sizeof(Canister));
		cans[i]->x = 100;
		cans[i]->y = 100;
		cans[i]->collected = 1;
		cans[i]->time = 0; 
	}		
}

void populatePath(void){
	//start w rectangle
	int i=0, j, k;
	for(j=0;j<240;j++){
		path[i][0] = 40 + j;
		path[i][1] = 30;
		i++;
	}
	for(j=0;j<150;j++){
		path[i][0] = 280;
		path[i][1] = 30 + j;
		i++;
	}
	for(j=0;j<240;j++){
		path[i][0] = 280 - j;
		path[i][1] = 180;
		i++;
	}
	for(j=0;j<150;j++){
		path[i][0] = 40;
		path[i][1] = 180 - j;
		i++;
	}
	/*
	for(i=0;i<PATHSIZE;i++){
		for(j=-2;j<=2;j++){
			for(k=-2;k<=2;k++){
				map[path[i][0] + j][path[i][1] + k] = 1;
			}
		}
	}*/
}

__task void CanisterTask(void){
	int i, randNum, numCans = 0;
	srand(player->car->angle);
	while(1){
		for(i=0;i<MAXCANS;i++){
			if(cans[i]->collected == 1){
				randNum = rand() % PATHSIZE;
				cans[i]->x = path[randNum][0];
				cans[i]->y = path[randNum][1];;
				cans[i]->collected =0;
			}
		}
		os_tsk_pass();
	}
}

__task void DrawTask(void){
	int i, xPlayerPrev = player->car->x, yPlayerPrev = player->car->y, xCPUPrev = CPU->x, yCPUPrev = CPU->y;
	GLCD_SetTextColor(0xFFE0);
	GLCD_SetBackColor(0X001F);
	GLCD_Clear(0xFFFF);
	for(i=0;i<PATHSIZE;i++){
		GLCD_Bitmap(path[i][0],path[i][1], 40, 30, (unsigned char *)&GIMP_IMAGE_pixel_data);
	}
	while(1){
		GLCD_Bitmap(xPlayerPrev,yPlayerPrev, 40, 30, (unsigned char *)&GIMP_IMAGE_pixel_data);
		GLCD_Bitmap(xCPUPrev,yCPUPrev, 40, 30, (unsigned char *)&GIMP_IMAGE_pixel_data);
		GLCD_Bitmap(player->car->x,player->car->y,CARWIDTH,CARHEIGHT, (unsigned char *)&CAR1_pixel_data);
		GLCD_Bitmap(CPU->x,CPU->y,CARWIDTH,CARHEIGHT, (unsigned char *)&CAR2_pixel_data);
		for(i=0;i<MAXCANS;i++){
			GLCD_Bitmap(cans[i]->x,cans[i]->y, 25, 18, (unsigned char *)&CAR2_pixel_data);
		}
		xPlayerPrev = player->car->x;
		yPlayerPrev = player->car->y;
		xCPUPrev = CPU->x;
		yCPUPrev = CPU->y;
		os_tsk_pass(); 
	}
}

__task void ReadPotentiometerTask(void) {
	LPC_PINCON->PINSEL1 &= ~(0x03<<18);
	LPC_PINCON->PINSEL1 |= (0x01<<18);
	LPC_SC->PCONP |= (0x01<<12);
	LPC_ADC->ADCR = (1<<2) | (4<<8) | (1<<21); 
	
	while(1) {
		LPC_ADC->ADCR |= (0x01<<24);
		while(!(LPC_ADC->ADGDR & (unsigned int)0x01<<31));
		
		// divide number of possible direction settings by 10 to avoid too much directional noise
		player->car->angle = (int)(((LPC_ADC->ADGDR & 0x0000FFF0) >> 4) - 2048)*36/2048;
		player->car->angle *= 10;
		os_tsk_pass();
	}
}

__task void MovePlayersTask(void){
	int newX, newY;
	int i = 40, j = 30;
	CPU->y = j;
	CPU->x = i;
	while(1){
		/*
		if( map[player->car->x][player->car->y] == 0 ){
			player->car->speed = 1;
		}
		else{
			player->car->speed = 3;
		}*/
		newX = player->car->x + player->car->speed*cos(player->car->angle * 3.14/180);
		newY = player->car->y + player->car->speed*sin(player->car->angle * 3.14/180);
		// Check for collision with wall or CPU car in either direction
		if(!( newX + CARWIDTH >= CPU->x && newX <= CPU->x + CARWIDTH &&  newY + CARHEIGHT >= CPU->y && newY <= CPU->y + CARHEIGHT )){
			if( newX >= 0 && newX <= (MAPWIDTH-CARWIDTH) && newY >= 0 && newY <= (MAPHEIGHT-CARHEIGHT) ){
				player->car->x = newX;
				player->car->y = newY;
			}
		}
		if(CPU->y <= 30 && CPU->x < 280) CPU->x += CPU->speed;
		else if(CPU->x >= 280 && CPU->y < 180) CPU->y += CPU->speed;
		else if(CPU->y >= 180 && CPU->x > 40) CPU->x -= CPU->speed;
		else if(CPU->x <= 40 && CPU->y > 30) CPU->y -= CPU->speed;

		// MODULO BY SCREEN SIZE SO THE PLAYER FITS IN THE SCREEN
		setLeds(player->car->lap);
		os_tsk_pass();
	}
}
	
__task void PushButtonTask(void){
	int boolBoostActive;
	int buttonCurr;
	int buttonPrev = LPC_GPIO2->FIOPIN & (0x01 << 10);
	LPC_GPIO1->FIODIR |= 0xB0000000;
	LPC_GPIO2->FIODIR |= 0x0000007C;
	while(1) {
		buttonCurr = LPC_GPIO2->FIOPIN & (0x01 << 10);
		if((buttonCurr != buttonPrev) && (buttonCurr & (0x01 << 10)) ) {
			if( player->car->speed == 3 ) {
				player->car->speed = 6;
			}
			else {
				player->car->speed = 3;
			}
			player->car->lap++;
		}
		buttonPrev = buttonCurr;
		os_tsk_pass();
	}
}

// Starts all tasks and initializes semaphores.
__task void StartTasks(void){
	// Create each task.
	os_tsk_create(PushButtonTask,1);
	os_tsk_create(ReadPotentiometerTask,1);
	os_tsk_create(MovePlayersTask,1);
	os_tsk_create(CanisterTask,1);
	os_tsk_create(DrawTask,1);
	
	while(1);
}

int main(void){
	player = malloc(sizeof(Player));
	player->car = makeCar(5,5,0,3,0);
	player->canisters = 0;
	
	CPU = makeCar(100,100,0,2,0);
	GLCD_Init();
	printf("Initialize Print \n");
	
	LPC_GPIO1->FIODIR |= 0xB0000000;
	LPC_GPIO2->FIODIR |= 0x0000007C;
	
	populatePath();
	initCanisters();
	// Start the tasks.
	os_sys_init(StartTasks);
}
