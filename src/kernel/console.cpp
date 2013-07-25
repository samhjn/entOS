#include "os.h"
#define VIDEO_RAM_BASE (0xb8000)
#define GET_ADDR(x,y)	((u8*)VIDEO_RAM_BASE + ((x) + (y) * 80) * 2)
 
typedef struct {
	int cx, cy;
	int w, h;
	void* pdev;
} ConsoleObj;

static ConsoleObj gcon;
int conStage;
u8 conDefaultColor = 0x07;

void conInitStage0() {
	gcon.cx = 0;
	gcon.cy = 0;
	gcon.w = 80;
	gcon.h = 25;
	conStage = 0;
	conClearScreen();
}

int conGetCursor(int *x, int *y) {
	*x = gcon.cx;
	*y = gcon.cy;
	return RESULT_SUCCESS;
}

int conSetCursor(int x, int y) {
	if ((x < 0) || (x >= gcon.w)) return RESULT_ERROR_PARAM;
	if ((y < 0) || (y >= gcon.h)) return RESULT_ERROR_PARAM;
	gcon.cx = x;
	gcon.cy = y;
	return RESULT_SUCCESS;
}

void conScrollNext() {
	int i, j;
	for (i = 0; i < gcon.h - 1; i++) {
		for (j = 0; j < gcon.w; j++) 
			*(u16*)GET_ADDR(j, i) = *(u16*)GET_ADDR(j, i + 1 );
	}
	for (j = 0; j < gcon.w; j++) {
		*GET_ADDR(j, i) = 0x20;		
		*(GET_ADDR(j, i) + 1) = conDefaultColor;	
	}
			
}
void conNewLine() {
	gcon.cx = 0;
	gcon.cy ++;
	if (gcon.cy >= gcon.h) {
		gcon.cy = gcon.h - 1;
		conScrollNext();
	}
}

void conClearScreen() {
	int i, j;
	for (i = 0; i < gcon.h; i++) {
		for (j = 0; j < gcon.w; j++) {
			*GET_ADDR(j, i) = 0x20;		
			*(GET_ADDR(j, i) + 1) = conDefaultColor;	
		}
	}
}

void conFillColor(u8 color) {
	int i, j;
	
	conDefaultColor = color;
	for (i = 0; i < gcon.h; i++) {
		for (j = 0; j < gcon.w; j++) {
			*(GET_ADDR(j, i) + 1) = conDefaultColor;	
		}
	}
}

void conSetColor(u8 color) {
	conDefaultColor = color;
}
void conPutChar(char ch) {
	if (ch == '\r') return;
	if (ch == '\n') {
		conNewLine();
		return; 
	}
	*GET_ADDR(gcon.cx, gcon.cy) = ch;
	*(GET_ADDR(gcon.cx, gcon.cy) + 1) = conDefaultColor;
	gcon.cx++;
	if (gcon.cx >= gcon.w) conNewLine();
}

void conPuts(char* str) {
	while(*str) {
		conPutChar(*(str++));
	}
}


