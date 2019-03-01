/*********************************************************************/
/*                   Music functions for ATMEGA128                   */
/* Originally Written by: Kellen Arb 10.18.08                        */
/* Arpegiator functionality produced by Kamron Ebrahimi	             */
/*                                                                   */
/* This file should include everything you need to set up and use    */
/*songs for your ECE473 alarm clock.  Each function has a description*/
/*so you know how to use it, but all you should need to do is:       */
/*  0)Add the following to your Timer0 overflow interrupt (assuming  */
/*      interrupt is 128 times/second):
  ms++;
  if(ms % 8 == 0) {
    //for note duration (64th notes) 
    beat++;
  }                                                                  */
/*      ms can be changed for any counter variable, if you have one  */
/*      already. beat, however, must stay.                           */
/*  1)Change the #define values below for mute, unmute, and ALARM_PIN*/
/*      to the values needed for your setup.  If you use a different */
/*      port, as well as different pins, you'll have to manually     */
/*      change them throughout this file.                            */
/*  2)In your main function, call music_init().  Check to make sure  */
/*      there aren't any conflicts with the values it sets.          */
/*  4)Set the value of "song" to your liking. You can make this user */
/*      selectable very easily.                                      */
/*  5)Anytime you set off your alarm, add a call to music_on(). This */
/*      will start the interrupt and the song playing.               */
/*  4)Anytime you turn off your alarm, add a call to music_off().    */
/*      this stops the song playing and halts the interrupt.         */
/*                                                                   */
/* That should be all you need!  If you want to create new songs,    */
/*just read the descriptions for play_note and play_rest and copy the*/
/*format of any of the pre-done songs.  Make sure to share any good  */
/*ones with the class! Have fun!                                     */
/*             -Kellen Arb                                           */
/*********************************************************************/
#include <avr/io.h>
#define F_CPU 16000000UL //16Mhz clock
#include <string.h>
#include <avr/sfr_defs.h>
#include "music.h"
#include <avr/interrupt.h>

//Mute is on PORTD
//set the hex values to set and unset the mute pin
//I used PORTD-PIN2
#define mute 0x04
#define unmute 0xFB
//Alarm is also on PORTD
//set the hex value for the alarm pin
//I used PORTD-PIN7
#define ALARM_PIN 0x80
#define NUM_SONGS 4
//set this variable to select the song
//(0-3, unless you add more)

#define ALARM_PIN2 0x40 //PORT D pin 6

uint8_t song;
//uint8_t rest_flag;
/*
//function prototypes defined here
void song0(uint16_t note); //Beaver Fight Song
void song1(uint16_t note); //Tetris Theme (A)
void song2(uint16_t note); //Mario Bros Theme
void song3(uint16_t note); 
void play_song(uint8_t song, uint8_t note);
void play_rest(uint8_t duration);
void play_note(char note, uint8_t flat, uint8_t octave, uint8_t duration);
void music_off(void);
void music_on(void);      
void music_init(void);
*/

#define C0 0x1DDC
#define Db0 0x1C30
#define D0 0x1A9A
#define Eb0 0x1919
#define E0 0x17B2
#define F0 0x165D
#define Gb0 0x151D
#define G0 0x13ED
#define Ab0 0x12CE
#define A0 0x11C0
#define Bb0 0x10C0
#define B0 0x0FD0
#define C1 0x0EED
#define Db1 0x0E16
#define D1 0x0D4C
#define Eb1 0x0C8D
#define E1 0x0BD8
#define F1 0x0B2E
#define Gb1 0x0A8D
#define G1 0x09F6
#define Ab1 0x0967
#define A1 0x08DF
#define Bb1 0x0860
#define B1 0x07E7
#define C2 0x0776
#define Db2 0x070A
#define D2 0x06A5
#define Eb2 0x0646
#define E2 0x05EB
#define F2 0x0596
#define Gb2 0x0546
#define G2 0x04FA
#define Ab2 0x04B2
#define A2 0x046F
#define Bb2 0x042F
#define B2 0x03F3
#define C3 0x03BA
#define Db3 0x0384
#define D3 0x0352
#define Eb3 0x0322
#define E3 0x02F5
#define F3 0x02CA
#define Gb3 0x02A2
#define G3 0x027C
#define Ab3 0x0258
#define A3 0x0237
#define Bb3 0x0217
#define B3 0x01F9
#define C4 0x01DC
#define Db4 0x01C1
#define D4 0x01A8
#define Eb4 0x0190
#define E4 0x017A
#define F4 0x0164
#define Gb4 0x0150
#define G4 0x013D
#define Ab4 0x012B
#define A4 0x011B
#define Bb4 0x010B
#define B4 0x00FC
#define C5 0x00ED
#define Db5 0x00E0
#define D5 0x00D3
#define Eb5 0x00C7
#define E5 0x00BC
#define F5 0x00B1
#define Gb5 0x00A7
#define G5 0x009E
#define Ab5 0x0095
#define A5 0x008D
#define Bb5 0x0085
#define B5 0x007D
#define C6 0x0076
#define Db6 0x006F
#define D6 0x0069
#define Eb6 0x0063
#define E6 0x005D
#define F6 0x0058
#define Gb6 0x0053
#define G6 0x004E
#define Ab6 0x004A
#define A6 0x0046
#define Bb6 0x0042
#define B6 0x003E
#define C7 0x003A
#define Db7 0x0037
#define D7 0x0034
#define Eb7 0x0031
#define E7 0x002E
#define F7 0x002B
#define Gb7 0x0029
#define G7 0x0026
#define Ab7 0x0024
#define A7 0x0022
#define Bb7 0x0020
#define B7 0x001E
#define C8 0x001C
#define Db8 0x001B
#define D8 0x0019
#define Eb8 0x0018
#define E8 0x0015
#define F8 0x0012
#define Gb8 0x0010
#define G8 0x000D
#define Ab8 0x000B
#define A8 0x0009
#define Bb8 0x0007
#define B8 0x0005

//nest this in an array of 12 different key sets
//uint16_t C[42] = {C0, D0, E0, F0, G0, A0, B0, C1, D1, E1, F1, G1, A1, B1, C2, D2, E2, F2, G2, A2, B2, C3, D3, E3, F3, G3, A3, B3, C4, D4, E4, F4, G4, A4, B4, C5, D5, E5, F5, G5, A5, B5, C6, D6, E6, F6, G6, A6, B6, C7, D7, E7, F7, G7, A7, B7, C8, D8, E8, F8, G8, A8, B8};
char C[8] = {'C', 'D', 'E', 'F', 'G', 'A', 'B', 'C'};
char dorian[8] = {'D', 'E', 'F', 'G', 'A', 'B', 'C', 'D'};
char phrygian[8] = {'E', 'F', 'G', 'A', 'B', 'C', 'D', 'E'};
char lydian[8] = {'F', 'G', 'A', 'B', 'C', 'D', 'E', 'F'};
char mixolydian[8] = {'G', 'A', 'B', 'C', 'D', 'E', 'F', 'G'};
char aeolian[8] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'A'};
char locrian[8] = {'B', 'C', 'D', 'E', 'F', 'G', 'A', 'B'};

char C_d[8] = {'C', 'B', 'A', 'G', 'F', 'E', 'D', 'C'};
char dorian_d[8] = {'D', 'C', 'B', 'A', 'G', 'F', 'E', 'D'};
char phrygian_d[8] = {'E', 'D', 'C', 'B', 'A', 'G', 'F', 'E'};
char lydian_d[8] = {'F', 'E', 'D', 'C', 'B', 'A', 'G', 'F'};
char mixolydian_d[8] = {'G', 'F', 'E', 'D', 'C', 'B', 'A', 'G'};
char aeolian_d[8] = {'A', 'G', 'F', 'E', 'D', 'C', 'B', 'A'};
char locrian_d[8] = {'B', 'A', 'G', 'F', 'E', 'D', 'C', 'B'};

//char C_d[8] = {'C', 'B', 'A', 'G', 'F', 'E', 'D', 'C'};

//global control consts
volatile uint8_t switch_ch;

/*********** CHANNNEL ONE ****************/
volatile uint16_t beat;
volatile uint16_t max_beat;
volatile uint8_t notes;

//arpegiator channel 1 tuning controls
volatile uint8_t save1;
volatile uint8_t delete1;

//muscal const ch1
volatile uint8_t attribute1;
volatile uint8_t notes_to_play1;
volatile uint8_t rate1;
volatile uint8_t steps1;
volatile uint8_t octave1;
volatile uint8_t type1;

uint8_t rest_flag;

//UP down and down up behavior                        //added to top of music.c, added to music.h, need to wire
volatile uint8_t p_flag1;
volatile uint8_t p_flag2;

/************ CHANNEL TWO *************/
volatile uint16_t beat2;
volatile uint16_t max_beat2;
volatile uint8_t notes2;

//muscal const ch1
volatile uint8_t attribute2;
volatile uint8_t notes_to_play2;
volatile uint8_t rate2;
volatile uint8_t steps2;
volatile uint8_t octave2;
uint8_t rest_flag2;
volatile uint8_t repeat2;
volatile uint8_t type2;

//control flags for chopping notes
uint8_t chop_top1 = 0;
uint8_t chop_bot1 = 0;
uint8_t chop_top2 = 0;
uint8_t chop_bot2 = 0;

//for chopping double root notes.... Jesus this code has turned into a cluster f
uint8_t octave_flag_up1 = 0;
uint8_t octave_flag_up2 = 0;
uint8_t octave_flag_down1 = 0;
uint8_t octave_flag_down2 = 0;

//arpegiator channel 2 tuning controls
volatile uint8_t play; //starts playing any savaed sequence
volatile uint8_t stop; //stops channel two from playing synth returns to normal mode
volatile uint8_t sequence_to_play[4];
volatile uint8_t sequence_flag;

//mode variables
volatile char *mode1;
volatile char *mode1_d;
volatile uint8_t modal1;

volatile char *mode2;
volatile char *mode2_d;
volatile uint8_t modal2;

void song0(uint16_t note)
{ //beaver fight song (Max and Kellen)
   switch (note)
   {
   case 0:
      play_note('F', 0, 4, 8);
      break;
   case 1:
      play_note('E', 0, 4, 8);
      break;
   case 2:
      play_note('D', 0, 4, 8);
      break;
   case 3:
      play_note('C', 0, 4, 8);
      break;
   case 4:
      play_note('A', 0, 4, 6);
      break;
   case 5:
      play_note('A', 1, 4, 2);
      break;
   case 6:
      play_note('A', 0, 4, 6);
      break;
   case 7:
      play_note('A', 1, 4, 2);
      break;
   case 8:
      play_note('A', 0, 4, 16);
      break;
   case 9:
      play_note('F', 0, 4, 8);
      break;
   case 10:
      play_note('E', 0, 4, 8);
      break;
   case 11:
      play_note('D', 0, 4, 8);
      break;
   case 12:
      play_note('C', 0, 4, 8);
      break;
   case 13:
      play_note('B', 1, 4, 6);
      break;
   case 14:
      play_note('A', 0, 4, 2);
      break;
   case 15:
      play_note('B', 1, 4, 6);
      break;
   case 16:
      play_note('A', 0, 4, 2);
      break;
   case 17:
      play_note('B', 1, 4, 16);
      break;
   case 18:
      play_note('G', 0, 4, 3);
      break;
   case 19:
      play_rest(1); //rest
      break;
   case 20:
      play_note('G', 0, 4, 7);
      break;
   case 21:
      play_rest(1); //rest
      break;
   case 22:
      play_note('G', 1, 4, 4);
      break;
   case 23:
      play_note('G', 0, 4, 6);
      break;
   case 24:
      play_note('A', 0, 4, 2);
      break;
   case 25:
      play_note('B', 1, 4, 8);
      break;
   case 26:
      play_note('A', 0, 4, 2);
      break;
   case 27:
      play_rest(2);
      break;
   case 28:
      play_note('A', 0, 4, 8);
      break;
   case 29:
      play_note('A', 1, 4, 4);
      break;
   case 30:
      play_note('A', 0, 4, 6);
      break;
   case 31:
      play_note('B', 1, 4, 2);
      break;
   case 32:
      play_note('C', 0, 5, 4);
      break;
   case 33:
      play_note('D', 1, 5, 4);
      break;
   case 34:
      play_note('D', 0, 5, 4);
      break;
   case 35:
      play_note('B', 0, 4, 8);
      break;
   case 36:
      play_note('A', 0, 4, 4);
      break;
   case 37:
      play_note('G', 0, 4, 8);
      break;
   case 38:
      play_note('A', 0, 4, 8);
      break;
   case 39:
      play_note('G', 0, 4, 24);
      break;
   case 40:
      play_rest(8);
      break;
   case 41:
      play_note('F', 0, 4, 8);
      break;
   case 42:
      play_note('E', 0, 4, 8);
      break;
   case 43:
      play_note('D', 0, 4, 8);
      break;
   case 44:
      play_note('C', 0, 4, 8);
      break;
   case 45:
      play_note('A', 0, 4, 6);
      break;
   case 46:
      play_note('A', 1, 4, 2);
      break;
   case 47:
      play_note('A', 0, 4, 6);
      break;
   case 48:
      play_note('A', 1, 4, 2);
      break;
   case 49:
      play_note('A', 0, 4, 16);
      break;
   case 50:
      play_note('F', 0, 4, 8);
      break;
   case 51:
      play_note('G', 1, 4, 8);
      break;
   case 52:
      play_note('G', 0, 4, 8);
      break;
   case 53:
      play_note('D', 0, 4, 8);
      break;
   case 54:
      play_note('B', 1, 4, 6);
      break;
   case 55:
      play_note('A', 0, 4, 2);
      break;
   case 56:
      play_note('B', 1, 4, 6);
      break;
   case 57:
      play_note('A', 0, 4, 2);
      break;
   case 58:
      play_note('B', 1, 4, 16);
      break; //phrase
   case 59:
      play_note('D', 0, 4, 16);
      break;
   case 60:
      play_note('D', 0, 5, 16);
      break;
   case 61:
      play_note('A', 0, 4, 16);
      break;
   case 62:
      play_note('C', 0, 5, 16);
      break;
   case 63:
      play_note('B', 1, 4, 8);
      break;
   case 64:
      play_note('C', 0, 5, 4);
      break;
   case 65:
      play_note('D', 0, 5, 4);
      break;
   case 66:
      play_note('A', 0, 4, 8);
      break;
   case 67:
      play_note('G', 0, 4, 8);
      break;
   case 68:
      play_note('F', 0, 4, 24);
      break;
   case 69:
      play_rest(8);
      break;
   default:
      notes = -1;
   }
} //song0

void song1(uint16_t note)
{ //tetris theme (Kellen)
   switch (note)
   {
   case 0:
      play_note('E', 0, 4, 8);
      break;
   case 1:
      play_note('B', 0, 3, 4);
      break;
   case 2:
      play_note('C', 0, 4, 4);
      break;
   case 3:
      play_note('D', 0, 4, 4);
      break;
   case 4:
      play_note('E', 0, 4, 2);
      break;
   case 5:
      play_note('D', 0, 4, 2);
      break;
   case 6:
      play_note('C', 0, 4, 4);
      break;
   case 7:
      play_note('B', 0, 3, 4);
      break; //bar
   case 8:
      play_note('A', 0, 3, 7);
      break;
   case 9:
      play_rest(1);
      break;
   case 10:
      play_note('A', 0, 3, 4);
      break;
   case 11:
      play_note('C', 0, 4, 4);
      break;
   case 12:
      play_note('E', 0, 4, 8);
      break;
   case 13:
      play_note('D', 0, 4, 4);
      break;
   case 14:
      play_note('C', 0, 4, 4);
      break; //bar
   case 15:
      play_note('B', 0, 3, 12);
      break;
   case 16:
      play_note('C', 0, 4, 4);
      break;
   case 17:
      play_note('D', 0, 4, 8);
      break;
   case 18:
      play_note('E', 0, 4, 8);
      break; //bar
   case 19:
      play_note('C', 0, 4, 8);
      break;
   case 20:
      play_note('A', 0, 3, 7);
      break;
   case 21:
      play_rest(1);
      break;
   case 22:
      play_note('A', 0, 3, 16);
      break; //bar phrase
   case 23:
      play_rest(4);
      break;
   case 24:
      play_note('D', 0, 4, 8);
      break;
   case 25:
      play_note('F', 0, 4, 4);
      break;
   case 26:
      play_note('A', 0, 4, 8);
      break;
   case 27:
      play_note('G', 0, 4, 4);
      break;
   case 28:
      play_note('F', 0, 4, 4);
      break; //bar
   case 29:
      play_note('E', 0, 4, 12);
      break;
   case 30:
      play_note('C', 0, 4, 4);
      break;
   case 31:
      play_note('E', 0, 4, 8);
      break;
   case 32:
      play_note('D', 0, 4, 4);
      break;
   case 33:
      play_note('C', 0, 4, 4);
      break; //bar
   case 34:
      play_note('B', 0, 3, 7);
      break;
   case 35:
      play_rest(1);
      break;
   case 36:
      play_note('B', 0, 3, 4);
      break;
   case 37:
      play_note('C', 0, 4, 4);
      break;
   case 38:
      play_note('D', 0, 4, 8);
      break;
   case 39:
      play_note('E', 0, 4, 8);
      break;
   case 40:
      play_note('C', 0, 4, 8);
      break;
   case 41:
      play_note('A', 0, 3, 7);
      break;
   case 42:
      play_rest(1);
      break;
   case 43:
      play_note('A', 0, 3, 8);
      break;
   case 44:
      play_rest(8);
      break; //bar phrase
   case 45:
      play_note('E', 0, 3, 16);
      break;
   case 46:
      play_note('C', 0, 3, 16);
      break; //bar
   case 47:
      play_note('D', 0, 3, 16);
      break;
   case 48:
      play_note('B', 0, 2, 16);
      break; //bar
   case 49:
      play_note('C', 0, 3, 16);
      break;
   case 50:
      play_note('A', 0, 2, 16);
      break; //bar
   case 51:
      play_note('A', 1, 2, 16);
      break;
   case 52:
      play_note('B', 0, 2, 8);
      break;
   case 53:
      play_rest(8);
      break; //bar phrase
   case 54:
      play_note('E', 0, 3, 16);
      break;
   case 55:
      play_note('C', 0, 3, 16);
      break; //bar
   case 56:
      play_note('D', 0, 3, 16);
      break;
   case 57:
      play_note('B', 0, 2, 16);
      break; //bar
   case 58:
      play_note('C', 0, 3, 8);
      break;
   case 59:
      play_note('E', 0, 3, 8);
      break;
   case 60:
      play_note('A', 0, 3, 16);
      break; //bar
   case 61:
      play_note('A', 1, 3, 16);
      break;
   case 62:
      play_rest(16);
      break; //bar phrase
   default:
      notes = -1;
   }
} //song1

void song2(uint16_t note)
{ //Super Mario Bros Theme (Brian)
   switch (note)
   {
   case 0:
      play_note('E', 0, 4, 1);
      break;
   case 1:
      play_rest(1);
      break;
   case 2:
      play_note('E', 0, 4, 3);
      break;
   case 3:
      play_rest(1);
      break;
   case 4:
      play_note('E', 0, 4, 2);
      break;
   case 5:
      play_rest(2);
      break;
   case 6:
      play_note('C', 0, 4, 2);
      break;
   case 7:
      play_note('E', 0, 4, 4);
      break;
   case 8:
      play_note('G', 0, 4, 8);
      break;
   case 9:
      play_note('G', 0, 2, 8);
      break;
   case 10:
      play_rest(8);
      break;
   case 11:
      play_note('C', 0, 4, 5);
      break;
   case 12:
      play_note('G', 0, 3, 2);
      break;
   case 13:
      play_rest(4);
      break;
   case 14:
      play_note('E', 0, 3, 4);
      break;
   case 15:
      play_rest(2);
      break;
   case 16:
      play_note('A', 0, 3, 2);
      break;
   case 17:
      play_rest(2);
      break;
   case 18:
      play_note('B', 0, 3, 2);
      break;
   case 19:
      play_rest(2);
      break;
   case 20:
      play_note('B', 1, 3, 2);
      break;
   case 21:
      play_note('A', 0, 3, 4);
      break;
   case 22:
      play_note('G', 0, 3, 3);
      break;
   case 23:
      play_note('E', 0, 4, 2);
      break;
   case 24:
      play_rest(1);
      break;
   case 25:
      play_note('G', 0, 4, 2);
      break;
   case 26:
      play_note('A', 0, 4, 4);
      break;
   case 27:
      play_note('F', 0, 4, 2);
      break;
   case 28:
      play_note('G', 0, 4, 2);
      break;
   case 29:
      play_rest(2);
      break;
   case 30:
      play_note('E', 0, 4, 2);
      break;
   case 31:
      play_rest(2);
      break;
   case 32:
      play_note('C', 0, 4, 2);
      break;
   case 33:
      play_note('D', 0, 4, 2);
      break;
   case 34:
      play_note('B', 0, 3, 2);
      break;
   case 35:
      play_rest(4);
      break;
   case 36:
      play_note('C', 0, 4, 5);
      break;
   case 37:
      play_rest(2);
      break;
   case 38:
      play_note('G', 0, 3, 2);
      break;
   case 39:
      play_rest(3);
      break;
   case 40:
      play_note('E', 0, 3, 4);
      break;
   case 41:
      play_rest(2);
      break;
   case 42:
      play_note('A', 0, 3, 2);
      break;
   case 43:
      play_rest(2);
      break;
   case 44:
      play_note('B', 0, 3, 2);
      break;
   case 45:
      play_rest(2);
      break;
   case 46:
      play_note('B', 1, 3, 2);
      break;
   case 47:
      play_note('A', 0, 3, 4); //phrase
      break;
   case 48:
      play_note('G', 0, 3, 3);
      break;
   case 49:
      play_note('E', 0, 4, 2);
      break;
   case 50:
      play_rest(1);
      break;
   case 51:
      play_note('G', 0, 4, 2);
      break;
   case 52:
      play_note('A', 0, 4, 4);
      break;
   case 53:
      play_note('F', 0, 4, 2);
      break;
   case 54:
      play_note('G', 0, 4, 2);
      break;
   case 55:
      play_rest(2);
      break;
   case 56:
      play_note('E', 0, 4, 2);
      break;
   case 57:
      play_rest(2);
      break;
   case 58:
      play_note('C', 0, 4, 2);
      break;
   case 59:
      play_note('D', 0, 4, 2);
      break;
   case 60:
      play_note('B', 0, 3, 2);
      break;
   case 61:
      play_rest(8);
      break;
   case 62:
      play_note('G', 0, 4, 2);
      break;
   case 63:
      play_note('G', 1, 4, 2);
      break;
   case 64:
      play_note('F', 0, 4, 2);
      break;
   case 65:
      play_note('E', 1, 4, 2);
      break;
   case 66:
      play_rest(2);
      break;
   case 67:
      play_note('E', 0, 4, 2);
      break;
   case 68:
      play_rest(2);
      break;
   case 69:
      play_note('A', 1, 3, 2);
      break;
   case 70:
      play_note('A', 0, 3, 2);
      break;
   case 71:
      play_note('C', 0, 4, 2);
      break;
   case 72:
      play_rest(2);
      break;
   case 73:
      play_note('A', 0, 3, 2);
      break;
   case 74:
      play_note('C', 0, 4, 2);
      break;
   case 75:
      play_note('D', 0, 4, 2);
      break;
   case 76:
      play_rest(4);
      break;
   case 77:
      play_note('G', 0, 3, 2);
      break;
   case 78:
      play_note('G', 1, 3, 2);
      break;
   case 79:
      play_note('F', 0, 3, 2);
      break;
   case 80:
      play_note('E', 1, 3, 2);
      break;
   case 81:
      play_rest(2);
      break;
   case 82:
      play_note('E', 0, 3, 2);
      break;
   case 83:
      play_rest(2);
      break;
   case 84:
      play_note('G', 0, 4, 2);
      break;
   case 85:
      play_rest(2);
      break;
   case 86:
      play_note('G', 0, 4, 1);
      break;
   case 87:
      play_rest(1);
      break;
   case 88:
      play_note('G', 0, 4, 4);
      break;
   case 89:
      play_rest(8);
      break;
   case 90:
      play_note('G', 0, 4, 2);
      break;
   case 91:
      play_note('G', 1, 4, 2);
      break;
   case 92:
      play_note('F', 0, 4, 2);
      break;
   case 93:
      play_note('E', 1, 4, 2);
      break;
   case 94:
      play_rest(2);
      break;
   case 95:
      play_note('E', 0, 4, 2);
      break;
   case 96:
      play_rest(2);
      break;
   case 97:
      play_note('A', 1, 3, 2);
      break;
   case 98:
      play_note('A', 0, 3, 2);
      break;
   case 99:
      play_note('C', 0, 4, 2);
      break;
   case 100:
      play_rest(2);
      break;
   case 101:
      play_note('A', 0, 3, 2);
      break;
   case 102:
      play_note('C', 0, 4, 2);
      break;
   case 103:
      play_note('D', 0, 4, 2);
      break;
   case 104:
      play_rest(4);
      break;
   case 105:
      play_note('E', 1, 4, 4);
      break;
   case 106:
      play_rest(2);
      break;
   case 107:
      play_note('D', 0, 4, 2);
      break;
   case 108:
      play_rest(4);
      break;
   case 109:
      play_note('C', 0, 4, 4);
      break;
   case 110:
      play_rest(10);
      break;
   case 111:
      play_note('C', 0, 4, 2);
      break;
   case 112:
      play_rest(1);
      break;
   case 113:
      play_note('C', 0, 4, 2);
      break;
   case 114:
      play_rest(2);
      break;
   case 115:
      play_note('C', 0, 4, 2);
      break;
   case 116:
      play_rest(2);
      break;
   case 117:
      play_note('C', 0, 4, 2);
      break;
   case 118:
      play_note('D', 0, 4, 4);
      break;
   case 119:
      play_note('E', 0, 4, 2);
      break;
   case 120:
      play_note('C', 0, 4, 2);
      break;
   case 121:
      play_rest(2);
      break;
   case 122:
      play_note('A', 0, 3, 2);
      break;
   case 123:
      play_note('G', 0, 3, 4);
      break;
   case 124:
      play_rest(4);
      break;
   case 125:
      play_note('C', 0, 4, 2);
      break;
   case 126:
      play_rest(1);
      break;
   case 127:
      play_note('C', 0, 4, 2);
      break;
   case 128:
      play_rest(2);
      break;
   case 129:
      play_note('C', 0, 4, 2);
      break;
   case 130:
      play_rest(2);
      break;
   case 131:
      play_note('C', 0, 4, 2);
      break;
   case 132:
      play_note('D', 0, 4, 2);
      break;
   case 133:
      play_note('E', 0, 4, 2);
      break;
   case 134:
      play_rest(16);
      break;
   case 135:
      play_note('C', 0, 4, 2);
      break;
   case 136:
      play_rest(1);
      break;
   case 137:
      play_note('C', 0, 4, 2);
      break;
   case 138:
      play_rest(2);
      break;
   case 139:
      play_note('C', 0, 4, 2);
      break;
   case 140:
      play_rest(2);
      break;
   case 141:
      play_note('C', 0, 4, 2);
      break;
   case 142:
      play_note('D', 0, 4, 4);
      break;
   case 143:
      play_note('E', 0, 4, 2);
      break;
   case 144:
      play_note('C', 0, 4, 2);
      break;
   case 145:
      play_rest(2);
      break;
   case 146:
      play_note('A', 0, 3, 2);
      break;
   case 147:
      play_note('G', 0, 3, 4);
      break;
   case 148:
      play_rest(8);
      break;
   default:
      notes = -1;
   }
} //song2

void song3(uint16_t note)
{ //(Max and Kellen)
   switch (note)
   {
   case 0:
      play_note('E', 0, 4, 7);
      break;
   case 1:
      play_rest(1);
      break;
   case 2:
      play_note('E', 0, 4, 7);
      break;
   case 3:
      play_rest(1);
      break;
   case 4:
      play_note('E', 0, 4, 7);
      break;
   case 5:
      play_rest(1);
      break;
   case 6:
      play_note('E', 0, 4, 3);
      break;
   case 7:
      play_rest(1);
      break;
   case 8:
      play_note('E', 0, 4, 3);
      break;
   case 9:
      play_rest(5);
      break;
   case 10:
      play_note('E', 0, 5, 4);
      break;
   case 11:
      play_note('G', 1, 5, 4);
      break;
   case 12:
      play_note('E', 0, 5, 4);
      break;
   case 13:
      play_note('G', 0, 5, 8);
      break;
   case 14:
      play_note('E', 0, 5, 8);
      break;
      //phrase
   case 15:
      play_note('E', 1, 4, 7);
      break;
   case 16:
      play_rest(1);
      break;
   case 17:
      play_note('E', 1, 4, 7);
      break;
   case 18:
      play_rest(1);
      break;
   case 19:
      play_note('E', 1, 4, 7);
      break;
   case 20:
      play_rest(1);
      break;
   case 21:
      play_note('E', 1, 4, 3);
      break;
   case 22:
      play_rest(1);
      break;
   case 23:
      play_note('E', 1, 4, 3);
      break;
   case 24:
      play_rest(5);
      break;
   case 25:
      play_note('E', 1, 5, 4);
      break;
   case 26:
      play_note('E', 0, 5, 3);
      break;
   case 27:
      play_rest(1); //rest 1
      break;
   case 28:
      play_note('E', 0, 5, 4);
      break;
   case 29:
      play_note('G', 1, 5, 8);
      break;
   case 30:
      play_note('E', 0, 5, 8);
      break;
   default:
      notes = -1;
   }
} //song3

void play_song(uint8_t song, uint8_t note)
{
   //if you add a song, you'll have to add it to this
   //switch statement.
   switch (song)
   {
   case 0:
      song0(note); //beaver fight song
      break;
   case 1:
      song1(note); //tetris theme
      break;
   case 2:
      song2(note); //super mario bros
      break;
   case 3:
      song3(note);
      break;
   default:
      song0(note); //defaults to beaver fight song
   }
}

void play_rest(uint8_t duration)
{
   //mute for duration
   //duration is in 64th notes at 120bpm
   //PORTD |= mute;
   PORTD |= (0 << PD7);
   beat = 0;
   max_beat = duration;
   rest_flag = 1;
}

void play_rest2(uint8_t duration)
{
   //mute for duration
   //duration is in 64th notes at 120bpm
   //PORTD |= mute;
   PORTD |= (0 << PD6);
   beat2 = 0;
   max_beat2 = duration;
   rest_flag2 = 1;
}

void write_bargraph(uint8_t notes_to_play)
{
   //write the signal strength of the radio to the bargraph
   PORTD &= ~(1 << PD2); //enable bar graph display, PORTB, bit 7
   SPDR = notes_to_play; //send to display
   while (bit_is_clear(SPSR, SPIF))
   {
   }               //wait till data sent out (while loop)
   PORTB |= 0x01;  //HC595 output reg - rising edge...
   PORTB &= ~0x01; //and falling edge
   PORTD |= (1 << PD2);
}

unsigned int reverseBits(unsigned int num)
{
   unsigned int NO_OF_BITS = 8;
   unsigned int reverse_num = 0, i, temp;

   for (i = 0; i < NO_OF_BITS; i++)
   {
      temp = (num & (1 << i));
      if (temp)
         reverse_num |= (1 << ((NO_OF_BITS - 1) - i));
   }

   return reverse_num;
}

void arpeggiateDown(uint8_t note, uint8_t notes_to_play, uint8_t duration, uint8_t octave, uint8_t step)
{
   volatile char *key = mode1_d;
   uint8_t i, j;
   static uint8_t run = 2;
   static uint8_t rest_flag_arp = 0;

   //mirror the notes to play
   notes_to_play = reverseBits(notes_to_play);

   if ((type1 == 3 || type1 == 4) && (run == 1))
      chop_bot1 = 1;

   if (notes_to_play == 0)
   {
      run = step;
      notes = -1;
      chop_top1 = 0;
      chop_bot1 = 0;

      if (type1 == 3)
         p_flag1 = 1;
      else if (type1 == 4)
         p_flag1 = 0;

      play_rest(1);
      if (switch_ch == 1)
         write_bargraph(notes_to_play);
   }
   else
   {
      for (i = run; i >= 0; i--)
      {
         for (j = notes; j < 8; j++)
         {
            if (notes == 7)
            {
               if ((_BV(j) & notes_to_play) && (j == 7) && (octave + run) != 8)
               {                                                //verify that we are not trying to play on the 9th octave (Crash)
                  play_note(key[j], 0, octave + run, duration); //run has already been incremented at this point
                  if (switch_ch == 1)
                     write_bargraph(1);
               }
               notes = -1;
               run--;
               if ((run < step) && ((_BV(0) & notes_to_play) && (_BV(7) & notes_to_play)))
                  octave_flag_down1 = 1;

               if (run == 0)
               {
                  octave_flag_down1 = 0;
                  p_flag1 = 1;
                  run = step;
               }
            }
            //edge case for handling single note input on single step
            if ((_BV(j) & notes_to_play) && (notes_to_play == 1 || notes_to_play == 2 || notes_to_play == 4 || notes_to_play == 8 || notes_to_play == 16 || notes_to_play == 32 || notes_to_play == 64 || notes_to_play == 128) && (step == 1))
            {
               if (rest_flag_arp == 0)
               {
                  if ((_BV(j) & notes_to_play) && (j == 7) && (octave + run) != 8)
                  { //play thr octave at intervals
                     play_note(key[j], 0, octave + run, duration);
                  }
                  else
                  {
                     play_note(key[j], 0, octave, duration); //play all other notes at intervals
                  }
                  rest_flag_arp = 1;
                  if (switch_ch == 1)
                     write_bargraph(_BV((j * -1) + 7));
               }
               else
               {
                  rest_flag_arp = 0;
                  play_rest(duration);
                  if (switch_ch == 1)
                     write_bargraph(0);
                  break;
               }
            }
            if ((_BV(j) & notes_to_play) && (j == 7))
            {
               //play_note(key[j], 0, octave+run, duration);   //run has already been incremented at this point
               //notes = j;
               //play_rest(duration);
               break;
            }
            if ((_BV(j) & notes_to_play) && (j <= modal1))
            { //if the given note is set, edit stuffs here
                  play_note(key[j], 0, octave + run + 1, duration);
               notes = j;
               if (switch_ch == 1)
                  write_bargraph(_BV((j * -1) + 7));
               break;
            }
            if (_BV(j) & notes_to_play)
            { //if the given note is set
               if (j == 0)
                  play_note(key[j], 0, octave + run + 1, duration);
               else
                  play_note(key[j], 0, octave + run, duration);
               notes = j;
               if (switch_ch == 1)
                  write_bargraph(_BV((j * -1) + 7));
               break;
            }
         }
         break;
      }
   }
}

//edge case we want up down for single note across octaves
//notes: notes is the incremental count that holds which note we need to play
void arpeggiate(uint8_t note, uint8_t notes_to_play, uint8_t duration, uint8_t octave, uint8_t step)
{
   volatile char *key = mode1;
   uint8_t i, j;
   static uint8_t run = 0;
   static uint8_t rest_flag_arp = 0;

   if ((type1 == 3 || type1 == 4) && (run == (step - 1)))
      chop_top1 = 1;

   if (notes_to_play == 0)
   {
      run = 0;
      notes = -1;

      octave_flag_up1 = 0;

      chop_top1 = 0;
      chop_bot1 = 0;

      if (type1 == 3)
         p_flag1 = 1;
      else if (type1 == 4)
         p_flag1 = 0;

      play_rest(1);
      if (switch_ch == 1)
         write_bargraph(notes_to_play);
   }
   else
   {
      for (i = run; i < step; i++)
      {
         for (j = notes; j < 8; j++)
         {
            if (notes == 7)
            {
               if ((_BV(j) & notes_to_play) && (j == 7) && (octave + run) != 8)
               {                                                    //verify that we are not trying to play on the 9th octave (Crash)
                  play_note(key[j], 0, octave + run + 1, duration); //run has already been incremented at this point
                  if (switch_ch == 1)
                     write_bargraph(_BV(j));
               }
               notes = -1;
               run++;
               if ((run > 0) && ((_BV(0) & notes_to_play) && (_BV(7) & notes_to_play)))
                  octave_flag_up1 = 1;
               if (run == step)
               {
                  octave_flag_up1 = 0;
                  p_flag1 = 0;
                  run = 0;
               }
            }
            //edge case for handling single note input on single step
            if ((_BV(j) & notes_to_play) && (notes_to_play == 1 || notes_to_play == 2 || notes_to_play == 4 || notes_to_play == 8 || notes_to_play == 16 || notes_to_play == 32 || notes_to_play == 64 || notes_to_play == 128) && (step == 1))
            {
               if (rest_flag_arp == 0)
               {
                  if ((_BV(j) & notes_to_play) && (j == 7) && (octave + run) != 8)
                  { //play thr octave at intervals
                     play_note(key[j], 0, octave + run + 1, duration);
                  }
                  else
                  {
                     play_note(key[j], 0, octave, duration); //play all other notes at intervals
                  }
                  rest_flag_arp = 1;
                  if (switch_ch == 1)
                     write_bargraph(_BV(j));
               }
               else
               {
                  rest_flag_arp = 0;
                  play_rest(duration);
                  if (switch_ch == 1)
                     write_bargraph(0);
                  break;
               }
            }
            if ((_BV(j) & notes_to_play) && (j == 7))
            {
               //play_note(key[j], 0, octave+run, duration);	//run has already been incremented at this point
               //notes = j;
               //play_rest(duration);
               break;
            }
            if ((_BV(j) & notes_to_play) && (j >= 7 - modal1))
            { //if the given note is set, edit stuffs here
               play_note(key[j], 0, octave + run + 1, duration);
               notes = j;
               if (switch_ch == 1)
                  write_bargraph(_BV(j));
               break;
            }
            if (_BV(j) & notes_to_play)
            { //if the given note is set, edit stuffs here
               play_note(key[j], 0, octave + run, duration);
               notes = j;
               if (switch_ch == 1)
                  write_bargraph(_BV(j));
               break;
            }
         }
         break;
      }
   }
}

void arpeggiateDown2(uint8_t note, uint8_t notes_to_play, uint8_t duration, uint8_t octave, uint8_t step)
{
   volatile char *key = mode2_d;
   uint8_t i, j;
   static uint8_t run = 2;
   static uint8_t rest_flag_arp = 0;

   //mirror the notes to play
   notes_to_play = reverseBits(notes_to_play);

   if ((type2 == 3 || type2 == 4) && (run == 1))
      chop_bot2 = 1;

   if (notes_to_play == 0)
   {
      run = step;
      notes2 = -1;

      octave_flag_down1 = 0;

      chop_top2 = 0;
      chop_bot2 = 0;

      if (type2 == 3)
         p_flag2 = 1;
      else if (type2 == 4)
         p_flag2 = 0;

      play_rest2(1);
      if (switch_ch == 2)
         write_bargraph(notes_to_play);
   }
   else
   {
      for (i = run; i >= 0; i--)
      {
         for (j = notes2; j < 8; j++)
         {
            if (notes2 == 7)
            {
               if ((_BV(j) & notes_to_play) && (j == 7) && (octave + run) != 8)
               {                                                 //verify that we are not trying to play on the 9th octave (Crash)
                  play_note2(key[j], 0, octave + run, duration); //run has already been incremented at this point
                  if (switch_ch == 2)
                     write_bargraph(1);
               }
               notes2 = -1;
               run--;
               if ((run < step) && ((_BV(0) & notes_to_play) && (_BV(7) & notes_to_play)))
                  octave_flag_down2 = 1;

               if (run == 0)
               {
                  octave_flag_down2 = 0;
                  run = step;
                  p_flag2 = 1;
                  if (type2 == 2 || type2 == 3)
                     sequence_flag = 1;
               }
            }
            //edge case for handling single note input on single step
            if ((_BV(j) & notes_to_play) && (notes_to_play == 1 || notes_to_play == 2 || notes_to_play == 4 || notes_to_play == 8 || notes_to_play == 16 || notes_to_play == 32 || notes_to_play == 64 || notes_to_play == 128) && (step == 1))
            {
               if (rest_flag_arp == 0)
               {
                  if ((_BV(j) & notes_to_play) && (j == 7) && (octave + run) != 8)
                  { //play thr octave at intervals
                     play_note2(key[j], 0, octave + run, duration);
                  }
                  else
                  {
                     play_note2(key[j], 0, octave, duration); //play all other notes at intervals
                  }
                  rest_flag_arp = 1;
                  if (switch_ch == 2)
                     write_bargraph(_BV((j * -1) + 7));
               }
               else
               {
                  rest_flag_arp = 0;
                  play_rest2(duration);
                  if (switch_ch == 2)
                     write_bargraph(0);
                  break;
               }
            }
            if ((_BV(j) & notes_to_play) && (j == 7))
            {
               if (notes_to_play == 128 && steps2 == 1)
                  sequence_flag = 1;
               //play_note(key[j], 0, octave+run, duration);   //run has already been incremented at this point
               //notes = j;
               //play_rest(duration);
               break;
            }
            if ((_BV(j) & notes_to_play) && (j <= modal2))
            { //if the given note is set, edit stuffs here
               play_note2(key[j], 0, octave + run + 1, duration);
               notes2 = j;
               if (switch_ch == 2)
                  write_bargraph(_BV((j * -1) + 7));
               break;
            }
            if (_BV(j) & notes_to_play)
            { //if the given note is set
               if (j == 0)
                  play_note2(key[j], 0, octave + run + 1, duration);
               else
                  play_note2(key[j], 0, octave + run, duration);
               notes2 = j;
               if (switch_ch == 2)
                  write_bargraph(_BV((j * -1) + 7));
               break;
            }
         }
         break;
      }
   }
}

void arpeggiate2(uint8_t note, uint8_t notes_to_play, uint8_t duration, uint8_t octave, uint8_t step)
{
   volatile char *key = mode2;
   uint8_t i, j;
   static uint8_t run = 0;
   static uint8_t rest_flag_arp = 0;

   if ((type2 == 3 || type2 == 4) && (run == (step - 1)))
      chop_top2 = 1;

   if (notes_to_play == 0)
   {
      run = 0;
      notes2 = -1;

      chop_top2 = 0;
      chop_bot2 = 0;

      if (type2 == 3)
         p_flag2 = 1;
      else if (type2 == 4)
         p_flag2 = 0;

      play_rest2(1);
      if (switch_ch == 2)
         write_bargraph(notes_to_play);
   }
   else
   {
      for (i = run; i < step; i++)
      {
         for (j = notes2; j < 8; j++)
         {
            if (notes2 == 7)
            {
               if ((_BV(j) & notes_to_play) && (j == 7) && (octave + run) != 8)
               {                                                     //verify that we are not trying to play on the 9th octave (Crash)
                  play_note2(key[j], 0, octave + run + 1, duration); //run has already been incremented at this point
                  if (switch_ch == 2)
                     write_bargraph(_BV(j));
               }
               notes2 = -1;
               run++;
               if ((run > 0) && ((_BV(0) & notes_to_play) && (_BV(7) & notes_to_play)))
                  octave_flag_up2 = 1;
               if (run == step)
               { //set a flag right here for going onto the next bar in the sequence
                  octave_flag_up2 = 0;
                  run = 0;
                  p_flag2 = 0;
                  if (type2 == 1 || type2 == 4)
                     sequence_flag = 1;
               }
            }
            //edge case for handling single note input on single step
            if ((_BV(j) & notes_to_play) && (notes_to_play == 1 || notes_to_play == 2 || notes_to_play == 4 || notes_to_play == 8 || notes_to_play == 16 || notes_to_play == 32 || notes_to_play == 64 || notes_to_play == 128) && (step == 1))
            {
               if (rest_flag_arp == 0)
               {
                  if ((_BV(j) & notes_to_play) && (j == 7) && (octave + run) != 8)
                  { //play thr octave at intervals
                     play_note2(key[j], 0, octave + run + 1, duration);
                  }
                  else
                  {
                     play_note2(key[j], 0, octave, duration); //play all other notes at intervals
                  }
                  rest_flag_arp = 1;
                  if (switch_ch == 2)
                     write_bargraph(_BV(j));
               }
               else
               {
                  rest_flag_arp = 0;
                  play_rest2(duration);
                  if (switch_ch == 2)
                     write_bargraph(0);
                  break;
               }
            }
            if ((_BV(j) & notes_to_play) && (j == 7))
            {
               if (notes_to_play == 128 && steps2 == 1)
                  sequence_flag = 1;
               //play_note(key[j], 0, octave+run, duration);   //run has already been incremented at this point
               //notes = j;
               //play_rest(duration);
               break;
            }
            if ((_BV(j) & notes_to_play) && (j >= 7 - modal2))
            { //if the given note is set, edit stuffs here
               play_note2(key[j], 0, octave + run + 1, duration);
               notes2 = j;
               if (switch_ch == 2)
                  write_bargraph(_BV(j));
               break;
            }
            if (_BV(j) & notes_to_play)
            { //if the given note is set
               play_note2(key[j], 0, octave + run, duration);
               notes2 = j;
               if (switch_ch == 2)
                  write_bargraph(_BV(j));
               break;
            }
         }
         break;
      }
   }
}

void play_note(char note, uint8_t flat, uint8_t octave, uint8_t duration)
{
   //pass in the note, it's key, the octave they want, and a duration

   //function sets the value of OCR1A and the timer
   //note must be A-G
   //flat must be 1 (for flat) or 0 (for natural) (N/A on C or F)
   //octave must be 0-8 (0 is the lowest, 8 doesn't sound very good)
   //duration is in 64th notes at 120bpm
   //e.g. play_note('D', 1, 0, 16)
   //this would play a Db, octave 0 for 1 quarter note
   //120 bpm (every 32ms inc beat)
   //PORTD &= unmute;      //unmute (just in case)
   //PORTD = (1 << PC0);
   beat = 0;            //reset the beat counter
   max_beat = duration; //set the max beat
   switch (octave)
   {
   case 0:
      switch (note)
      {
      case 'A':
         if (flat)
         {
            OCR1A = Ab0;
         }
         else
         {
            OCR1A = A0;
         }
         break;
      case 'B':
         if (flat)
         {
            OCR1A = Bb0;
         }
         else
         {
            OCR1A = B0;
         }
         break;
      case 'C':
         OCR1A = C0;
         break;
      case 'D':
         if (flat)
         {
            OCR1A = Db0;
         }
         else
         {
            OCR1A = D0;
         }
         break;
      case 'E':
         if (flat)
         {
            OCR1A = Eb0;
         }
         else
         {
            OCR1A = E0;
         }
         break;
      case 'F':
         OCR1A = F0;
         break;
      case 'G':
         if (flat)
         {
            OCR1A = Gb0;
         }
         else
         {
            OCR1A = G0;
         }
         break;
      }
      break;
   case 1:
      switch (note)
      {
      case 'A':
         if (flat)
         {
            OCR1A = Ab1;
         }
         else
         {
            OCR1A = A1;
         }
         break;
      case 'B':
         if (flat)
         {
            OCR1A = Bb1;
         }
         else
         {
            OCR1A = B1;
         }
         break;
      case 'C':
         OCR1A = C1;
         break;
      case 'D':
         if (flat)
         {
            OCR1A = Db1;
         }
         else
         {
            OCR1A = D1;
         }
         break;
      case 'E':
         if (flat)
         {
            OCR1A = Eb1;
         }
         else
         {
            OCR1A = E1;
         }
         break;
      case 'F':
         OCR1A = F1;
         break;
      case 'G':
         if (flat)
         {
            OCR1A = Gb1;
         }
         else
         {
            OCR1A = G1;
         }
         break;
      }
      break;
   case 2:
      switch (note)
      {
      case 'A':
         if (flat)
         {
            OCR1A = Ab2;
         }
         else
         {
            OCR1A = A2;
         }
         break;
      case 'B':
         if (flat)
         {
            OCR1A = Bb2;
         }
         else
         {
            OCR1A = B2;
         }
         break;
      case 'C':
         OCR1A = C2;
         break;
      case 'D':
         if (flat)
         {
            OCR1A = Db2;
         }
         else
         {
            OCR1A = D2;
         }
         break;
      case 'E':
         if (flat)
         {
            OCR1A = Eb2;
         }
         else
         {
            OCR1A = E2;
         }
         break;
      case 'F':
         OCR1A = F2;
         break;
      case 'G':
         if (flat)
         {
            OCR1A = Gb2;
         }
         else
         {
            OCR1A = G2;
         }
         break;
      }
      break;
   case 3:
      switch (note)
      {
      case 'A':
         if (flat)
         {
            OCR1A = Ab3;
         }
         else
         {
            OCR1A = A3;
         }
         break;
      case 'B':
         if (flat)
         {
            OCR1A = Bb3;
         }
         else
         {
            OCR1A = B3;
         }
         break;
      case 'C':
         OCR1A = C3;
         break;
      case 'D':
         if (flat)
         {
            OCR1A = Db3;
         }
         else
         {
            OCR1A = D3;
         }
         break;
      case 'E':
         if (flat)
         {
            OCR1A = Eb3;
         }
         else
         {
            OCR1A = E3;
         }
         break;
      case 'F':
         OCR1A = F3;
         break;
      case 'G':
         if (flat)
         {
            OCR1A = Gb3;
         }
         else
         {
            OCR1A = G3;
         }
         break;
      }
      break;
   case 4:
      switch (note)
      {
      case 'A':
         if (flat)
         {
            OCR1A = Ab4;
         }
         else
         {
            OCR1A = A4;
         }
         break;
      case 'B':
         if (flat)
         {
            OCR1A = Bb4;
         }
         else
         {
            OCR1A = B4;
         }
         break;
      case 'C':
         OCR1A = C4;
         break;
      case 'D':
         if (flat)
         {
            OCR1A = Db4;
         }
         else
         {
            OCR1A = D4;
         }
         break;
      case 'E':
         if (flat)
         {
            OCR1A = Eb4;
         }
         else
         {
            OCR1A = E4;
         }
         break;
      case 'F':
         OCR1A = F4;
         break;
      case 'G':
         if (flat)
         {
            OCR1A = Gb4;
         }
         else
         {
            OCR1A = G4;
         }
         break;
      }
      break;
   case 5:
      switch (note)
      {
      case 'A':
         if (flat)
         {
            OCR1A = Ab5;
         }
         else
         {
            OCR1A = A5;
         }
         break;
      case 'B':
         if (flat)
         {
            OCR1A = Bb5;
         }
         else
         {
            OCR1A = B5;
         }
         break;
      case 'C':
         OCR1A = C5;
         break;
      case 'D':
         if (flat)
         {
            OCR1A = Db5;
         }
         else
         {
            OCR1A = D5;
         }
         break;
      case 'E':
         if (flat)
         {
            OCR1A = Eb5;
         }
         else
         {
            OCR1A = E5;
         }
         break;
      case 'F':
         OCR1A = F5;
         break;
      case 'G':
         if (flat)
         {
            OCR1A = Gb5;
         }
         else
         {
            OCR1A = G5;
         }
         break;
      }
      break;
   case 6:
      switch (note)
      {
      case 'A':
         if (flat)
         {
            OCR1A = Ab6;
         }
         else
         {
            OCR1A = A6;
         }
         break;
      case 'B':
         if (flat)
         {
            OCR1A = Bb6;
         }
         else
         {
            OCR1A = B6;
         }
         break;
      case 'C':
         OCR1A = C6;
         break;
      case 'D':
         if (flat)
         {
            OCR1A = Db6;
         }
         else
         {
            OCR1A = D6;
         }
         break;
      case 'E':
         if (flat)
         {
            OCR1A = Eb6;
         }
         else
         {
            OCR1A = E6;
         }
         break;
      case 'F':
         OCR1A = F6;
         break;
      case 'G':
         if (flat)
         {
            OCR1A = Gb6;
         }
         else
         {
            OCR1A = G6;
         }
         break;
      }
      break;
   case 7:
      switch (note)
      {
      case 'A':
         if (flat)
         {
            OCR1A = Ab7;
         }
         else
         {
            OCR1A = A7;
         }
         break;
      case 'B':
         if (flat)
         {
            OCR1A = Bb7;
         }
         else
         {
            OCR1A = B7;
         }
         break;
      case 'C':
         OCR1A = C7;
         break;
      case 'D':
         if (flat)
         {
            OCR1A = Db7;
         }
         else
         {
            OCR1A = D7;
         }
         break;
      case 'E':
         if (flat)
         {
            OCR1A = Eb7;
         }
         else
         {
            OCR1A = E7;
         }
         break;
      case 'F':
         OCR1A = F7;
         break;
      case 'G':
         if (flat)
         {
            OCR1A = Gb7;
         }
         else
         {
            OCR1A = G7;
         }
         break;
      }
      break;
   case 8:
      switch (note)
      {
      case 'A':
         if (flat)
         {
            OCR1A = Ab8;
         }
         else
         {
            OCR1A = A8;
         }
         break;
      case 'B':
         if (flat)
         {
            OCR1A = Bb8;
         }
         else
         {
            OCR1A = B8;
         }
         break;
      case 'C':
         OCR1A = C8;
         break;
      case 'D':
         if (flat)
         {
            OCR1A = Db8;
         }
         else
         {
            OCR1A = D8;
         }
         break;
      case 'E':
         if (flat)
         {
            OCR1A = Eb8;
         }
         else
         {
            OCR1A = E8;
         }
         break;
      case 'F':
         OCR1A = F8;
         break;
      case 'G':
         if (flat)
         {
            OCR1A = Gb8;
         }
         else
         {
            OCR1A = G8;
         }
         break;
      }
      break;
   default:
      OCR1A = 0x0000;
   }
}

void play_note2(char note, uint8_t flat, uint8_t octave, uint8_t duration)
{
   //flat must be 1 (for flat) or 0 (for natural) (N/A on C or F)
   //octave must be 0-8 (0 is the lowest, 8 doesn't sound very good)
   //duration is in 64th notes at 120bpm
   //e.g. play_note('D', 1, 0, 16)
   //this would play a Db, octave 0 for 1 quarter note
   //120 bpm (every 32ms inc beat)
   //PORTD &= unmute;      //unmute (just in case)
   //PORTD = (1 << PC0);
   beat2 = 0;            //reset the beat counter
   max_beat2 = duration; //set the max beat
   switch (octave)
   {
   case 0:
      switch (note)
      {
      case 'A':
         if (flat)
         {
            OCR3A = Ab0;
         }
         else
         {
            OCR3A = A0;
         }
         break;
      case 'B':
         if (flat)
         {
            OCR3A = Bb0;
         }
         else
         {
            OCR3A = B0;
         }
         break;
      case 'C':
         OCR3A = C0;
         break;
      case 'D':
         if (flat)
         {
            OCR3A = Db0;
         }
         else
         {
            OCR3A = D0;
         }
         break;
      case 'E':
         if (flat)
         {
            OCR3A = Eb0;
         }
         else
         {
            OCR3A = E0;
         }
         break;
      case 'F':
         OCR3A = F0;
         break;
      case 'G':
         if (flat)
         {
            OCR3A = Gb0;
         }
         else
         {
            OCR3A = G0;
         }
         break;
      }
      break;
   case 1:
      switch (note)
      {
      case 'A':
         if (flat)
         {
            OCR3A = Ab1;
         }
         else
         {
            OCR3A = A1;
         }
         break;
      case 'B':
         if (flat)
         {
            OCR3A = Bb1;
         }
         else
         {
            OCR3A = B1;
         }
         break;
      case 'C':
         OCR3A = C1;
         break;
      case 'D':
         if (flat)
         {
            OCR3A = Db1;
         }
         else
         {
            OCR3A = D1;
         }
         break;
      case 'E':
         if (flat)
         {
            OCR3A = Eb1;
         }
         else
         {
            OCR3A = E1;
         }
         break;
      case 'F':
         OCR3A = F1;
         break;
      case 'G':
         if (flat)
         {
            OCR3A = Gb1;
         }
         else
         {
            OCR3A = G1;
         }
         break;
      }
      break;
   case 2:
      switch (note)
      {
      case 'A':
         if (flat)
         {
            OCR3A = Ab2;
         }
         else
         {
            OCR3A = A2;
         }
         break;
      case 'B':
         if (flat)
         {
            OCR3A = Bb2;
         }
         else
         {
            OCR3A = B2;
         }
         break;
      case 'C':
         OCR3A = C2;
         break;
      case 'D':
         if (flat)
         {
            OCR3A = Db2;
         }
         else
         {
            OCR3A = D2;
         }
         break;
      case 'E':
         if (flat)
         {
            OCR3A = Eb2;
         }
         else
         {
            OCR3A = E2;
         }
         break;
      case 'F':
         OCR3A = F2;
         break;
      case 'G':
         if (flat)
         {
            OCR3A = Gb2;
         }
         else
         {
            OCR3A = G2;
         }
         break;
      }
      break;
   case 3:
      switch (note)
      {
      case 'A':
         if (flat)
         {
            OCR3A = Ab3;
         }
         else
         {
            OCR3A = A3;
         }
         break;
      case 'B':
         if (flat)
         {
            OCR3A = Bb3;
         }
         else
         {
            OCR3A = B3;
         }
         break;
      case 'C':
         OCR3A = C3;
         break;
      case 'D':
         if (flat)
         {
            OCR3A = Db3;
         }
         else
         {
            OCR3A = D3;
         }
         break;
      case 'E':
         if (flat)
         {
            OCR3A = Eb3;
         }
         else
         {
            OCR3A = E3;
         }
         break;
      case 'F':
         OCR3A = F3;
         break;
      case 'G':
         if (flat)
         {
            OCR3A = Gb3;
         }
         else
         {
            OCR3A = G3;
         }
         break;
      }
      break;
   case 4:
      switch (note)
      {
      case 'A':
         if (flat)
         {
            OCR3A = Ab4;
         }
         else
         {
            OCR3A = A4;
         }
         break;
      case 'B':
         if (flat)
         {
            OCR3A = Bb4;
         }
         else
         {
            OCR3A = B4;
         }
         break;
      case 'C':
         OCR3A = C4;
         break;
      case 'D':
         if (flat)
         {
            OCR3A = Db4;
         }
         else
         {
            OCR3A = D4;
         }
         break;
      case 'E':
         if (flat)
         {
            OCR3A = Eb4;
         }
         else
         {
            OCR3A = E4;
         }
         break;
      case 'F':
         OCR3A = F4;
         break;
      case 'G':
         if (flat)
         {
            OCR3A = Gb4;
         }
         else
         {
            OCR3A = G4;
         }
         break;
      }
      break;
   case 5:
      switch (note)
      {
      case 'A':
         if (flat)
         {
            OCR3A = Ab5;
         }
         else
         {
            OCR3A = A5;
         }
         break;
      case 'B':
         if (flat)
         {
            OCR3A = Bb5;
         }
         else
         {
            OCR3A = B5;
         }
         break;
      case 'C':
         OCR3A = C5;
         break;
      case 'D':
         if (flat)
         {
            OCR3A = Db5;
         }
         else
         {
            OCR3A = D5;
         }
         break;
      case 'E':
         if (flat)
         {
            OCR3A = Eb5;
         }
         else
         {
            OCR3A = E5;
         }
         break;
      case 'F':
         OCR3A = F5;
         break;
      case 'G':
         if (flat)
         {
            OCR3A = Gb5;
         }
         else
         {
            OCR3A = G5;
         }
         break;
      }
      break;
   case 6:
      switch (note)
      {
      case 'A':
         if (flat)
         {
            OCR3A = Ab6;
         }
         else
         {
            OCR3A = A6;
         }
         break;
      case 'B':
         if (flat)
         {
            OCR3A = Bb6;
         }
         else
         {
            OCR3A = B6;
         }
         break;
      case 'C':
         OCR3A = C6;
         break;
      case 'D':
         if (flat)
         {
            OCR3A = Db6;
         }
         else
         {
            OCR3A = D6;
         }
         break;
      case 'E':
         if (flat)
         {
            OCR3A = Eb6;
         }
         else
         {
            OCR3A = E6;
         }
         break;
      case 'F':
         OCR3A = F6;
         break;
      case 'G':
         if (flat)
         {
            OCR3A = Gb6;
         }
         else
         {
            OCR3A = G6;
         }
         break;
      }
      break;
   case 7:
      switch (note)
      {
      case 'A':
         if (flat)
         {
            OCR3A = Ab7;
         }
         else
         {
            OCR3A = A7;
         }
         break;
      case 'B':
         if (flat)
         {
            OCR3A = Bb7;
         }
         else
         {
            OCR3A = B7;
         }
         break;
      case 'C':
         OCR3A = C7;
         break;
      case 'D':
         if (flat)
         {
            OCR3A = Db7;
         }
         else
         {
            OCR3A = D7;
         }
         break;
      case 'E':
         if (flat)
         {
            OCR3A = Eb7;
         }
         else
         {
            OCR3A = E7;
         }
         break;
      case 'F':
         OCR3A = F7;
         break;
      case 'G':
         if (flat)
         {
            OCR3A = Gb7;
         }
         else
         {
            OCR3A = G7;
         }
         break;
      }
      break;
   case 8:
      switch (note)
      {
      case 'A':
         if (flat)
         {
            OCR3A = Ab8;
         }
         else
         {
            OCR3A = A8;
         }
         break;
      case 'B':
         if (flat)
         {
            OCR3A = Bb8;
         }
         else
         {
            OCR3A = B8;
         }
         break;
      case 'C':
         OCR3A = C8;
         break;
      case 'D':
         if (flat)
         {
            OCR3A = Db8;
         }
         else
         {
            OCR3A = D8;
         }
         break;
      case 'E':
         if (flat)
         {
            OCR3A = Eb8;
         }
         else
         {
            OCR3A = E8;
         }
         break;
      case 'F':
         OCR3A = F8;
         break;
      case 'G':
         if (flat)
         {
            OCR3A = Gb8;
         }
         else
         {
            OCR3A = G8;
         }
         break;
      }
      break;
   default:
      OCR3A = 0x0000;
   }
}

//consider doing an upward run, clearing the lowest notes from notes to play and the highest, then switching to a downward run... That should honestly work just fine
//THIS IS FIRE!

void music_off(void)
{
   //this turns the alarm timer off
   notes = 0;
   TCCR1B &= ~((1 << CS11) | (1 << CS10));
   //and mutes the output
   PORTD |= mute;
}

void music_on(void)
{
   //this starts the alarm timer running
   notes = 0;
   notes2 = 0;
   TCCR1B |= (1 << CS11) | (1 << CS10);
   TCCR3B |= (1 << CS31) | (1 << CS30);
   arpeggiate(notes, notes_to_play1, rate1, octave1, steps1);
   arpeggiate(notes2, notes_to_play2, rate2, octave2, steps2);
}

void music_init(void)
{
   //initially turned off (use music_on() to turn on)
   TIMSK |= (1 << OCIE1A); //enable timer interrupt 1 on compare
   TCCR1A = 0x00;          //TCNT1, normal port operation
   TCCR1B |= (1 << WGM12); //CTC, OCR1A = top, clk/64 (250kHz)
   TCCR1C = 0x00;          //no forced compare
   OCR1A = 0x0031;         //(use to vary alarm frequency)

   //initialize second timer
   ETIMSK |= (1 << OCIE3A); //enable timer interrupt 1 on compare
   TCCR3A = 0x00;           //TCNT1, normal port operation
   TCCR3B |= (1 << WGM32);  //CTC, OCR1A = top, clk/64 (250kHz)
   TCCR3C = 0x00;           //no forced compare
   OCR3A = 0x0046;          //(use to vary alarm frequency)

   music_on();

   //CH 1
   beat = 0;
   max_beat = 0;
   notes = 0;
   rest_flag = 0;

   //CH 2
   beat2 = 0;
   max_beat2 = 0;
   notes2 = 0;
   rest_flag2 = 0;
}

//this function will chop out the highest and lowest note in the sequence of notes to play
//kinda shitty way to do this
uint8_t process_notes_top1(uint8_t n)
{
   uint8_t new = n;
   int i;
   for (i = 7; i >= 0; i--)
   {
      if (n & (1 << i))
      { //if the bit is set
         new &= ~(1 << i);
         break;
      }
   }
   return new;
}

uint8_t process_notes_bot1(uint8_t n)
{
   uint8_t new = n;
   int i;
   for (i = 0; i < 8; i++)
   {
      if (n & (1 << i))
      { //if the bit is set
         new &= ~(1 << i);
         break;
      }
   }
   return new;
}

uint8_t check_notes1(uint8_t n)
{
   uint8_t count = 0;
   int i;
   for (i = 0; i < 8; i++)
   {
      if (n & (1 << i))
      {
         count++;
      }
   }
   if (count == 2)
      return 1;
   return 0;
}

//this function will chop out the highest and lowest note in the sequence of notes to play
//kinda shitty way to do this
uint8_t process_notes_top2(uint8_t n)
{
   uint8_t new = n;
   int i;
   for (i = 7; i >= 0; i--)
   {
      if (n & (1 << i))
      { //if the bit is set
         new &= ~(1 << i);
         break;
      }
   }
   return new;
}

uint8_t process_notes_bot2(uint8_t n)
{
   uint8_t new = n;
   int i;
   for (i = 0; i < 8; i++)
   {
      if (n & (1 << i))
      { //if the bit is set
         new &= ~(1 << i);
         break;
      }
   }
   return new;
}

uint8_t check_notes2(uint8_t n)
{
   uint8_t count = 0;
   int i;
   for (i = 0; i < 8; i++)
   {
      if (n & (1 << i))
      {
         count++;
      }
   }
   if (count == 2)
      return 1;
   return 0;
}

/*********************************************************************/
/*                             TIMER1_COMPA                          */
/*Oscillates pin7, PORTD for alarm tone output                       */
/*********************************************************************/

ISR(TIMER1_COMPA_vect)
{
   if (rest_flag == 0)
      PORTD ^= ALARM_PIN; //flips the bit, creating a tone
   if (beat >= max_beat)
   { //if we've played the note long enough
      rest_flag = 0;
      notes++; //move on to the next note
      //play_song(song, notes);//and play it

      if (type1 == 1)
      {
         uint8_t new = notes_to_play1;
         if (octave_flag_up1 == 1)
         {
            new = (notes_to_play1 & ~(1 << 0));
         }
         arpeggiate(notes, new, rate1, octave1, steps1);
      }

      else if (type1 == 2)
      {
         uint8_t new = notes_to_play1;
         if (octave_flag_down1 == 1)
         {
            new = (notes_to_play1 & ~(1 << 7));
         }
         arpeggiateDown(notes, new, rate1, octave1, steps1);
      }

      /*
      //Arpeggiate up down
      else if(type1 == 3){                        
         if((check_notes1(notes_to_play1) && steps1 == 1) || ((notes_to_play1 == 1 || notes_to_play1 == 2 || notes_to_play1 == 4 || notes_to_play1 == 8 || notes_to_play1 == 16 || notes_to_play1 == 32 || notes_to_play1 == 64 || notes_to_play1 == 128) && (steps1 == 1)))           //if less than three notes just play down 
               arpeggiate(notes, notes_to_play1, rate1, octave1, steps1);
         else{
            if(p_flag1 == 1)                  //after completing the run turn flag to zero, initialze the flag when changing to this mode, set it to one. 
			      arpeggiate(notes, notes_to_play1, rate1, octave1, steps1);        //issue with doing it in function is the above SHIT!!!! we cant skip notes
            else if(p_flag1 == 0){
               arpeggiateDown(notes, notes_to_play1, rate1, octave1, steps1);
            }
         }
      }

      //Arpeggiate up down
      else if(type1 == 4){                        
         if((check_notes1(notes_to_play1) && steps1 == 1) || ((notes_to_play1 == 1 || notes_to_play1 == 2 || notes_to_play1 == 4 || notes_to_play1 == 8 || notes_to_play1 == 16 || notes_to_play1 == 32 || notes_to_play1 == 64 || notes_to_play1 == 128) && (steps1 == 1)))           //if less than three notes just play down 
               arpeggiateDown(notes, notes_to_play1, rate1, octave1, steps1);
         else{
            if(p_flag1 == 0)                  //after completing the run turn flag to zero, initialze the flag when changing to this mode, set it to one. 
			      arpeggiateDown(notes, notes_to_play1, rate1, octave1, steps1);
            else if(p_flag1 == 1){
               arpeggiate(notes, notes_to_play1, rate1, octave1, steps1);
            }
         }
      }
*/

      //Arpeggiate up down, chop top and bottom
      else if (type1 == 3)
      {
         if ((check_notes1(notes_to_play1) && steps1 == 1) || ((notes_to_play1 == 1 || notes_to_play1 == 2 || notes_to_play1 == 4 || notes_to_play1 == 8 || notes_to_play1 == 16 || notes_to_play1 == 32 || notes_to_play1 == 64 || notes_to_play1 == 128) && (steps1 == 1))) //if less than three notes just play down
            arpeggiate(notes, notes_to_play1, rate1, octave1, steps1);
         else
         {
            if (p_flag1 == 1)
            { //after completing the run turn flag to zero, initialze the flag when changing to this mode, set it to one.

               //must go last
               uint8_t new = notes_to_play1;
               if (octave_flag_up1 == 1)
               {
                  new = (notes_to_play1 & ~(1 << 0));
               }

               arpeggiate(notes, new, rate1, octave1, steps1);
            }
            else if (p_flag1 == 0)
            {
               uint8_t new = notes_to_play1;

               if (chop_bot1 == 1)
               {
                  new = process_notes_bot1(notes_to_play1);
                  chop_bot1 = 0;
               }
               if (chop_top1 == 1)
               {
                  new = process_notes_top1(notes_to_play1);
                  chop_top1 = 0;
               }
               if (octave_flag_down1 == 1)
               {
                  new = (new & ~(1 << 7));
               }

               arpeggiateDown(notes, new, rate1, octave1 - 1, steps1);
            }
         }
      }

      //Arpeggiate down up, chop top and bottom
      else if (type1 == 4)
      {
         if ((check_notes1(notes_to_play1) && steps1 == 1) || ((notes_to_play1 == 1 || notes_to_play1 == 2 || notes_to_play1 == 4 || notes_to_play1 == 8 || notes_to_play1 == 16 || notes_to_play1 == 32 || notes_to_play1 == 64 || notes_to_play1 == 128) && (steps1 == 1))) //if less than three notes just play down
            arpeggiateDown(notes, notes_to_play1, rate1, octave1 - 1, steps1);
         else
         {
            if (p_flag1 == 0)
            { //after completing the run turn flag to zero, initialze the flag when changing to this mode, set it to one.
               uint8_t new = notes_to_play1;
               if (octave_flag_down1 == 1)
               {
                  new = (notes_to_play1 & ~(1 << 7));
               }
               arpeggiateDown(notes, new, rate1, octave1 - 1, steps1);
            }
            else if (p_flag1 == 1)
            {
               uint8_t new2 = notes_to_play1;

               if (chop_top1 == 1)
               {
                  new2 = process_notes_top1(notes_to_play1);
                  chop_top1 = 0;
               }

               if (chop_bot1 == 1)
               {
                  new2 = process_notes_bot1(notes_to_play1);
                  chop_bot1 = 0;
               }
               if (octave_flag_up1 == 1)
               {
                  new2 = (new2 & ~(1 << 0));
               }
               arpeggiate(notes, new2, rate1, octave1, steps1);
            }
         }
      }
   }
}

ISR(TIMER3_COMPA_vect)
{
   if (rest_flag2 == 0)
      PORTD ^= ALARM_PIN2;
   if (beat2 >= max_beat2)
   { //if we've played the note long enough
      rest_flag2 = 0;
      notes2++; //move on to the next note
      //play_song(song, notes);//and play it
      if (type2 == 1)
      {
         uint8_t new = notes_to_play2;
         if (octave_flag_up2 == 1)
         {
            new = (notes_to_play2 & ~(1 << 0));
         }
         arpeggiate2(notes2, new, rate2, octave2, steps2);
      }

      else if (type2 == 2)
      {
         uint8_t new = notes_to_play2;
         if (octave_flag_down2 == 1)
         {
            new = (notes_to_play2 & ~(1 << 7));
         }
         arpeggiateDown2(notes2, new, rate2, octave2, steps2);
      }

      /*
            //Arpeggiate up down
      else if(type2 == 3){                        
         if((check_notes2(notes_to_play2) && steps2 == 1) || ((notes_to_play2 == 1 || notes_to_play2 == 2 || notes_to_play2 == 4 || notes_to_play2 == 8 || notes_to_play2 == 16 || notes_to_play2 == 32 || notes_to_play2 == 64 || notes_to_play2 == 128) && (steps2 == 1)))           //if less than three notes just play down 
               arpeggiate2(notes2, notes_to_play2, rate2, octave2, steps2);
         else{
            if(p_flag2 == 1)                  //after completing the run turn flag to zero, initialze the flag when changing to this mode, set it to one. 
			      arpeggiate2(notes2, notes_to_play2, rate2, octave2, steps2);
            else if(p_flag2 == 0){
               arpeggiateDown2(notes2, notes_to_play2, rate2, octave2, steps2);
            }
         }
      }

      //Arpeggiate up down
      else if(type2 == 4){                        
         if((check_notes2(notes_to_play2) && steps2 == 1) || ((notes_to_play2 == 1 || notes_to_play2 == 2 || notes_to_play2 == 4 || notes_to_play2 == 8 || notes_to_play2 == 16 || notes_to_play2 == 32 || notes_to_play2 == 64 || notes_to_play2 == 128) && (steps2 == 1)))           //if less than three notes just play down 
               arpeggiateDown2(notes2, notes_to_play2, rate2, octave2, steps2);
         else{
            if(p_flag2 == 0)                  //after completing the run turn flag to zero, initialze the flag when changing to this mode, set it to one. 
               arpeggiateDown2(notes2, notes_to_play2, rate2, octave2, steps2);
            else if(p_flag2 == 1){
              arpeggiate2(notes2, notes_to_play2, rate2, octave2, steps2);
            }
         }
      }
*/

      //Arpeggiate up down, chop top and bottom
      else if (type2 == 3)
      {
         if ((check_notes2(notes_to_play2) && steps1 == 2) || ((notes_to_play2 == 1 || notes_to_play2 == 2 || notes_to_play2 == 4 || notes_to_play2 == 8 || notes_to_play2 == 16 || notes_to_play2 == 32 || notes_to_play2 == 64 || notes_to_play2 == 128) && (steps2 == 1))) //if less than three notes just play down
            arpeggiate2(notes2, notes_to_play2, rate2, octave2, steps2);
         else
         {
            if (p_flag2 == 1)
            { //after completing the run turn flag to zero, initialze the flag when changing to this mode, set it to one.
               uint8_t new = notes_to_play2;
               if (octave_flag_up2 == 1)
               {
                  new = (notes_to_play2 & ~(1 << 0));
               }
               arpeggiate2(notes2, new, rate2, octave2, steps2);
            }
            else if (p_flag2 == 0)
            {
               uint8_t new = notes_to_play2;

               if (chop_bot2 == 1)
               {
                  new = process_notes_bot2(notes_to_play2);
                  chop_bot2 = 0;
               }
               if (chop_top2 == 1)
               {
                  new = process_notes_top2(notes_to_play2);
                  chop_top2 = 0;
               }
               if (octave_flag_down2 == 1)
               {
                  new = (new & ~(1 << 7));
               }

               arpeggiateDown2(notes2, new, rate2, octave2 - 1, steps2);
            }
         }
      }

      //Arpeggiate down up, chop top and bottom
      else if (type2 == 4)
      {
         if ((check_notes2(notes_to_play2) && steps2 == 1) || ((notes_to_play2 == 1 || notes_to_play2 == 2 || notes_to_play2 == 4 || notes_to_play2 == 8 || notes_to_play2 == 16 || notes_to_play2 == 32 || notes_to_play2 == 64 || notes_to_play2 == 128) && (steps2 == 1))) //if less than three notes just play down
            arpeggiateDown2(notes2, notes_to_play2, rate2, octave2 - 1, steps2);
         else
         {
            if (p_flag2 == 0)
            { //after completing the run turn flag to zero, initialze the flag when changing to this mode, set it to one.
               uint8_t new = notes_to_play2;
               if (octave_flag_down2 == 1)
               {
                  new = (notes_to_play2 & ~(1 << 7));
               }
               arpeggiateDown2(notes2, new, rate2, octave2 - 1, steps2);
            }
            else if (p_flag2 == 1)
            {
               uint8_t new2 = notes_to_play2;

               if (chop_top2 == 1)
               {
                  new2 = process_notes_top2(notes_to_play2);
                  chop_top2 = 0;
               }

               if (chop_bot2 == 1)
               {
                  new2 = process_notes_bot2(notes_to_play2);
                  chop_bot2 = 0;
               }
               if (octave_flag_up2 == 1)
               {
                  new2 = (new2 & ~(1 << 0));
               }
               arpeggiate2(notes2, new2, rate2, octave2, steps2);
            }
         }
      }
   }
}
