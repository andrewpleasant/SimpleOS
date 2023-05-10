#include "page.h"
#include <stdint.h>
#include <string.h>
#include "fat.h"


#define MULTIBOOT2_HEADER_MAGIC         0xe85250d6

unsigned int multiboot_header[] __attribute__((section(".multiboot")))  = {MULTIBOOT2_HEADER_MAGIC, 0, 16, -(16+MULTIBOOT2_HEADER_MAGIC), 0, 12};

uint8_t inb (uint16_t _port) {
    uint8_t rv;
    __asm__ __volatile__ ("inb %1, %0" : "=a" (rv) : "dN" (_port));
    return rv;
}

extern int _end_kernel;

void outb (uint16_t _port, uint8_t val) {
	__asm__ __volatile__ ("outb %0, %1" : : "a" (val), "dN" (_port) );
}

unsigned char keyboard_map[128] =
{
   0,  27, '1', '2', '3', '4', '5', '6', '7', '8',     /* 9 */
 '9', '0', '-', '=', '\b',     /* Backspace */
 '\t',                 /* Tab */
 'q', 'w', 'e', 'r',   /* 19 */
 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', /* Enter key */
   0,                  /* 29   - Control */
 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',     /* 39 */
'\'', '`',   0,                /* Left shift */
'\\', 'z', 'x', 'c', 'v', 'b', 'n',                    /* 49 */
 'm', ',', '.', '/',   0,                              /* Right shift */
 '*',
   0,  /* Alt */
 ' ',  /* Space bar */
   0,  /* Caps lock */
   0,  /* 59 - F1 key ... > */
   0,   0,   0,   0,   0,   0,   0,   0,  
   0,  /* < ... F10 */
   0,  /* 69 - Num lock*/
   0,  /* Scroll Lock */
   0,  /* Home key */
   0,  /* Up Arrow */
   0,  /* Page Up */
 '-',
   0,  /* Left Arrow */
   0,  
   0,  /* Right Arrow */
 '+',
   0,  /* 79 - End key*/
   0,  /* Down Arrow */
   0,  /* Page Down */
   0,  /* Insert Key */
   0,  /* Delete Key */
   0,   0,   0,  
   0,  /* F11 Key */
   0,  /* F12 Key */
   0,  /* All other keys are undefined */
};

int xCoord = 0;
int yCoord = 0;

void putc(int c) {
	// input c put it into the memory slot for memory
	// after putting a character, go to next
	short *mem = 0xb8000;
	if (c == '\n') {
		yCoord++;
		xCoord = 0;
		
		if (yCoord > 24) {
			scrollOne();
			yCoord = 24;
		}		

		return;
	}
	
//	xCoord

	else {	
		mem[xCoord+80*yCoord] = (7 << 8) + c;
	}

	xCoord++;
	
	if (xCoord == 80) {
		xCoord = 0;
		yCoord++;
	}

	if (yCoord > 24) {
		scrollOne();
		xCoord = 0;
		yCoord = 24;
	}
}

void scrollOne() {

	// for each printed, starting at 2nd line, print it
	short *mem = 0xb8000;
	for (int j = 0; j < 80; j++) {
	for (int i = 1; i < 26; i++) {
		mem[j+80*(i-1)] = mem[j+80*i];
	}}
	for (int i = 0; i < 80; i++) {
		mem[i+80*24] = 0;
	}
}


/* FATTY STUFF */


void main() {
	unsigned short *vram = (unsigned short*)0xb8000; // base
        const unsigned char color = 7; // gray on black 
	// use screen 
	// use info mem 
	
	/* HW4 */

	fatInit();
	struct file current;
	fatOpen("/TEST2.TXT", &current);
	fatRead(current);


	/* HW2*/
        map_pages_init();
	init_pfa_list();
        struct ppage *p = allocate_physical_pages(1);
        /* HW 3 */
	map_pages(0x80000000, p, 1);
	free_physical_pages(p);


	/* KEYBOARD */
    	char c = 0;
    	while(1) {
		if(inb(0x60)!=c) {
			c = inb(0x60);
			if (c>0) {
				putc(keyboard_map[c]);
			}
		}
    	}
}
