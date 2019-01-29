/************************************
 *Author: 	Kamron Ebrahimi
 *Description: 
 *************************************/
#define TRUE 0x01
#define FALSE 0x00
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include "music.h"

//Count stores the value displayed to the seven seg 
uint16_t count;

//The previous state of the encoders
uint8_t prev;

//holds data to be sent to the segments. logic zero turns a digit on
uint8_t segment_data[5];

//The the current digit 
uint8_t digit_to_display;

//The on each loop iteration we simply write the value at a particular index out to PORTA
uint8_t segment_codes[10] = {0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8, 0x80, 0x90};

//These are the encodings for the value to be written to PORTB, index 0 is digit zero etc etc
uint8_t digit_select[5] = {0x00, 0x10, 0x20, 0x30, 0x40};

/***********************************************************************/
//                              tcnt0_init                             
//Initalizes timer/counter0 (TCNT0). TCNT0 is running in async mode
//with external 32khz crystal.  Runs in normal mode with no prescaling.
//Interrupt occurs at overflow 0xFF.
/***********************************************************************/
void tcnt0_init(void){
	ASSR   |=  (1<<AS0); //ext osc TOSC
	TIMSK  |=   (1<<TOIE0); //enable TCNT0 overflow interrupt
	TCCR0  |=  (1<<CS00); //normal mode, no prescale
}


/***********************************************************************
 *Function:		tcnt0_init
 *Description:		Initializes tcnt2 for use as a PWM output. 
 *			The OC2 register is PORTB bit 7. This output
 *			will feed the PWM input of the LED display for 
 *			calibrating brightness using the CdS cell. 
***********************************************************************/
void tcnt2_init(void){
	TCCR2 =  (1<<WGM21) | (1<<WGM20) | (1<<COM21) | (1<<COM20) | (1<<CS20);// | (1<<CS21);
	//set OCR2 to 0 (bottom) for 100% duty cycle, Fast-PWM, inverting mode, Clk/32 prescale	
	//recall that the PWM input of the LED display is tied to a PN transistor, the longer the PWM output for uc 
	//is low the brighter the display 
}


/***********************************************************************
 *Function:		tcnt1_init()
 *Description:		This function initializes timer 1
 *            		Timer 1 serves as the source of the alarm noise
 *           		This timer is used in combination with Timer0 
 * 			This timer is configured to interrupt upon Overflow, in the ISR         		
 *			we will configure the PORTC bit 0 to go high, then low on another 
 *			compare match (software PWM rerouting), change value in OCR1A to get different frequencies
 *			Normal mode, OCIE1A interrupt ON, 
 ***********************************************************************/
void tcnt1_init(void){
	TCCR1A = 0x00;			//disconnect (COM) OC1A and CTC mode 
	TCCR1B = (1 << CS10) | (1 << WGM12); 		//prescale = 1 
	TCCR1C =   0x00;                //no forced compare 
	TIMSK |= (1 << OCIE1A); 	//enable interrupt when TCNT1 = OCR1A 
	OCR1A = 0x3CCC;
}

/***********************************************************************
 *Function:             tcnt3_init()
 *Description:          This function initializes timer 3
 *			Primary function of this timer is to provide volume control                       
 *                     	Thus the left encoder will control volume, we will increment some global
 *                    	volume variable which is then written to OCR3A (PORT E bit 3, PG 81 ATmega128 datasheet)
 ***********************************************************************/
void tcnt3_init(void){
	TCCR3A = (1 << WGM30) | (0 << COM3A0) | (1 << COM3A1);// | (1 << WGM31);
	TCCR3B = (1 << WGM32) | (1 << CS30); /* | (1 << CS31);*/
}

/***********************************************************************/
//                            spi_init                               
//Initalizes the SPI port on the mega128. Does not do any further   
//external device specific initalizations.  Sets up SPI to be:                        
//master mode, clock=clk/2, cycle half phase, low polarity, MSB first
//interrupts disabled, poll SPIF bit in SPSR to check xmit completion
/***********************************************************************/
void spi_init(void){
	SPCR  |=   (1<<SPE) | (1<<MSTR); //enable SPI, master mode 
	SPSR  |=   (1<<SPI2X); // double speed operation
}

/*********************************************************************/
// spi_read
//Reads the SPI port.
/*********************************************************************/
uint8_t spi_read(void){
	SPDR = 0x00; //"dummy" write to SPDR
	while (bit_is_clear(SPSR,SPIF)){} //wait till 8 clock cycles are done
	return(SPDR); //return incoming data from SPDR
}

/****************************************************************************/
//                            chk_buttons                                      
//Checks the state of the button number passed to it. It shifts in ones till   
//the button is pushed. Function returns a 1 only once per debounced button    
//push so a debounce and toggle function can be implemented at the same time.  
//Adapted to check all buttons from Ganssel's "Guide to Debouncing"            
//Expects active low pushbuttons on PINA port.  Debounce time is determined by 
//external loop delay times 12. 
/*****************************************************************************/
int8_t chk_buttons(uint8_t buttons){
	static uint16_t state[8] = {0, 0, 0, 0, 0, 0, 0, 0}; //holds present state
	state[buttons] = (state[buttons] << 1) | (! bit_is_clear(PINA, buttons)) | 0xE000;
	if (state[buttons] == 0xF000){ 
		state[buttons] |= 0x0800; 
		return 1;
	}
	return 0;
}

/*****************************************
 *
 *
 *
 *
 *****************************************/
int8_t chk_buttonsC(uint8_t buttons){
        static uint16_t state[8] = {0, 0, 0, 0, 0, 0, 0, 0}; //holds present state
        state[buttons] = (state[buttons] << 1) | (! bit_is_clear(PINC, buttons)) | 0xE000;
        if (state[buttons] == 0xF000) return 1;
        return 0;
}

/*****************************************
 *
 *
 *
 *
 *****************************************/
int8_t chk_buttonsF(uint8_t buttons){
        static uint16_t state[8] = {0, 0, 0, 0, 0, 0, 0, 0}; //holds present state
        state[buttons] = (state[buttons] << 1) | (! bit_is_clear(PINF, buttons)) | 0xE000;
        if (state[buttons] == 0xF000) return 1;
        return 0;
}

/*****************************************
 *
 *
 *
 *
 *****************************************/
uint8_t display_pattern(uint8_t display_seq3[], uint8_t display_seq4[], uint8_t length, uint8_t speed){
	static uint8_t dis_timer = 0;
	static uint8_t count = 0; 

	segment_data[3] = display_seq3[count];
	segment_data[4] = display_seq4[count];
	dis_timer++;
	if(dis_timer == speed){         //7 looks pretty nice 
		count++;
		count %= length;
		if(count == 0)
			return 1; 
	}
	dis_timer %= speed;
	return 0; 
}

//***********************************************************************************
//                                   segment_sum                                    
//takes a 16-bit binary input value and places the appropriate equivalent 4 digit 
//BCD segment code in the array segment_data for display.                       
//array is loaded at exit as:  |digit3|digit2|colon|digit1|digit0|
//***********************************************************************************

//uint8_t segment_codes[10] = {0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8, 0x80, 0x90};
//HERE 
void segsum(uint16_t sum, uint8_t colon, uint8_t attribute, uint8_t channel, uint8_t notes_to_play){
	
	//break up decimal sum into 4 digit-segments
	static uint8_t dis_index = 0;
	static uint8_t speed = 7;
	static uint8_t num_dis = 9;

	//display 1	
	static uint8_t dis1_3[8] = {0xFE, 0xDF, 0xBF, 0xFB, 0xF7, 0xEF, 0xBF, 0xFD};	
	static uint8_t dis1_4[8] = {0xFE, 0xFD, 0xBF, 0xEF, 0xF7, 0xFB, 0xBF, 0xDF};

	//display 2
	static uint8_t dis2_3[14] = {0xFE, 0xFF, 0xFF, 0xFF, 0xBF, 0xFB, 0xF7, 0xFF, 0xFF, 0xFF, 0xBF, 0xFD, 0xFE, 0xFF};
	static uint8_t dis2_4[14] = {0xFF, 0xFE, 0xDF, 0xBF, 0xFF, 0xFF, 0xFF, 0xF7, 0xEF, 0xBF, 0xFF, 0xFF, 0xFF, 0xFE};

	//display a
	static uint8_t dis3_3[12] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xFD, 0xFB, 0xF7, 0xEF, 0xDF};
	static uint8_t dis3_4[12] = {0xFE, 0xDF, 0xEF, 0xF7, 0xFB, 0xFD, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

	static uint8_t dis4_3[10] = {0xFE, 0xDF, 0xef, 0xf7, 0xfb, 0xbf, 0xef, 0xf7, 0xfb, 0xfd}; 
        static uint8_t dis4_4[10] = {0xff, 0xfe, 0xdf, 0xbf, 0xfd, 0xfe, 0xdf, 0xbf, 0xfd, 0xff};

	static uint8_t dis5_3[8] = {0xFE, 0xFF, 0xFF, 0xFF, 0xff, 0xf7,0xfb, 0xfd};
        static uint8_t dis5_4[8] = {0xff, 0xfe,0xdf, 0xef, 0xf7, 0xff, 0xff, 0xff};

	static uint8_t dis6_3[8] = {0xEE, 0xDF, 0xEF, 0xDF, 0xEF, 0xD7, 0xEB, 0xDD};
	static uint8_t dis6_4[8] = {0xFD, 0xFA, 0xDD, 0xEB, 0xF5, 0xFB, 0xFD, 0xFB};

	static uint8_t* displays3[9] = {dis1_3, dis2_3, dis3_3, dis4_3, dis4_3, dis5_3, dis5_3, dis6_3, dis6_3}; 
	static uint8_t* displays4[9] = {dis1_4, dis2_4, dis3_4, dis4_4, dis4_4, dis5_4, dis5_4, dis6_4, dis6_4};
	static uint8_t dis_lengths[9] = {8, 14, 12, 10, 10, 8, 8, 8, 8};  

	segment_data[0] = segment_codes[sum % 10]; 

	//set colon
	segment_data[2] = colon; 

	//set attribute indicator
	switch(attribute){
		case 1:
			segment_data[1] = 0x12; //0x92;
			break;
		case 2:
			segment_data[1] = 0x4C;//0xCC;
			break;
		case 3:
			segment_data[1] = 0x40;//0xC0;
			break;
	}

	if(notes_to_play != 0){
		if(display_pattern(displays3[dis_index], displays4[dis_index], dis_lengths[dis_index], speed)){
			dis_index++; 
			dis_index %= num_dis;
		}
	}
	else{
		display_pattern(displays3[0], displays4[0], dis_lengths[0], 20);
	}
}

/***********************************************
 *
 *
 *CONTROL MECHANISMS FOR EXCEEDING OCTAVE/STEP BOUNDRIES with the 8th note
 *
 *
 **********************************************/
void set_control(uint8_t channel, uint8_t attribute, uint8_t inc){
	if(channel == 1){
		switch(attribute){
			case 1:							//steps
				if(inc){			//octaves can be 0-8, 0 being the lowest!
					switch(octave1){	//if we want to increment the number of steps to be taken we need to check the octave
						case 0:
							if(steps1 > 9){			//we can go a maximum of 9 steps at the 0th octave
								steps1 = 9;
							}
							if(steps1 < 9)
								steps1++;
							break;
						case 1:
							if(steps1 > 8){     
								steps1 = 8;
							}
							if(steps1 < 8)
								steps1++;
							break;
						case 2:
							if(steps1 > 7){                 
								steps1 = 7;
							}
							if(steps1 < 7)
								steps1++;
							break;
						case 3:
							if(steps1 > 6){                 
								steps1 = 6;
							}
							if(steps1 < 6)
								steps1++;
							break;
						case 4:
							if(steps1 > 5){                 
								steps1 = 5;
							}
							if(steps1 < 5)
								steps1++;
							break;
						case 5:
							if(steps1 > 4){                 
								steps1 = 4;
							}
							if(steps1 < 4)
								steps1++;
							break;
						case 6:
							if(steps1 > 3){                 
								steps1 = 3;
							}
							if(steps1 < 3)
								steps1++;
							break;
						case 7:
							if(steps1 > 2){                 
								steps1 = 2;
							}
							if(steps1 < 2)
								steps1++;
							break;
						case 8:
							if(steps1 > 1){                 
								steps1 = 1;
							}
							if(steps1 < 1)
								steps1++;
							break;
					}
				}
				else{		//decrement the number of steps
					if(steps1 > 1) 
						steps1--;
				}
				break;
			case 2:									//rate
				if(inc){
					if(rate1 < 9)
						rate1++;
				}
				else{
					if(rate1 > 1)
						rate1--;
				}
				break;
			case 3:							 		//octave
				if(inc){
					switch(steps1){        //if we want to increment the number of steps to be taken we need to check the octave
						case 1:
							if(octave1 > 8){
								octave1 = 8;
							}
							if(octave1 < 8)
								octave1++;
							break;
						case 2:
							if(octave1 > 7){
								octave1 = 7;
							}
							if(octave1 < 7)
								octave1++;
							break;					
						case 3:
							if(octave1 > 6){
								octave1 = 6;
							}
							if(octave1 < 6)
								octave1++;
							break;		
						case 4:
							if(octave1 > 5){
								octave1 = 5;
							}
							if(octave1 < 5)
								octave1++;
							break;	
						case 5:
							if(octave1 > 4){
								octave1 = 4;
							}
							if(octave1 < 4)
								octave1++;
							break;	
						case 6:
							if(octave1 > 3){
								octave1 = 3;
							}
							if(octave1 < 3)
								octave1++;
							break;	
						case 7:
							if(octave1 > 2){
								octave1 = 2;
							}
							if(octave1 < 2)
								octave1++;
							break;	
						case 8:
							if(octave1 > 1){
								octave1 = 1;
							}
							if(octave1 < 1)
								octave1++;
							break;	
						case 9:
							if(octave1 > 0){
								octave1 = 0;
							}
							if(octave1 < 0)
								octave1++;
							break;
					}
				}
				else{
					if(octave1 > 0)
						octave1--;
				}
				break;
		}
	}
	else if(channel == 2){
		switch(attribute){
			case 1:                                                 //steps
				if(inc){                        //octaves can be 0-8, 0 being the lowest!
					switch(octave2){        //if we want to increment the number of steps to be taken we need to check the octave
						case 0:
							if(steps2 > 9){                 //we can go a maximum of 9 steps at the 0th octave
								steps2 = 9;
							}
							if(steps2 < 9)
								steps2++;
							break;
						case 1:
							if(steps2 > 8){
								steps2 = 8;
							}
							if(steps2 < 8)
								steps2++;
							break;
						case 2:
							if(steps2 > 7){
								steps2 = 7;
							}
							if(steps2 < 7)
								steps2++;
							break;
						case 3:
							if(steps2 > 6){
								steps2 = 6;
							}
							if(steps2 < 6)
								steps2++;
							break;
						case 4:
							if(steps2 > 5){
								steps2 = 5;
							}
							if(steps2 < 5)
								steps2++;
							break;
						case 5:
							if(steps2 > 4){
								steps2 = 4;
							}
							if(steps2 < 4)
								steps2++;
							break;
						case 6:
							if(steps2 > 3){
								steps2 = 3;
							}
							if(steps2 < 3)
								steps2++;
							break;
						case 7:
							if(steps2 > 2){
								steps2 = 2;
							}
							if(steps2 < 2)
								steps2++;
							break;
						case 8:
							if(steps2 > 1){
								steps2 = 1;
							}
							if(steps2 < 1)
								steps2++;
							break;
					}
				}
				else{           //decrement the number of steps
					if(steps2 > 1)
						steps2--;
				}
				break;
			case 2:                                                                 //rate
				if(inc){
					if(rate2 < 9)
						rate2++;
				}
				else{
					if(rate2 > 1)
						rate2--;
				}
				break;
			case 3:                                                                 //octave
				if(inc){
					switch(steps2){        //if we want to increment the number of steps to be taken we need to check the octave
						case 1:
							if(octave2 > 8){
								octave2 = 8;
							}
							if(octave2 < 8)
								octave2++;
							break;
						case 2:
							if(octave2 > 7){
								octave2 = 7;
							}
							if(octave2 < 7)
								octave2++;
							break;
						case 3:
							if(octave2 > 6){
								octave2 = 6;
							}
							if(octave2 < 6)
								octave2++;
							break;
						case 4:
							if(octave2 > 5){
								octave2 = 5;
							}
							if(octave2 < 5)
								octave2++;
							break;
						case 5:
							if(octave2 > 4){
								octave2 = 4;
							}
							if(octave2 < 4)
								octave2++;
							break;
						case 6:
							if(octave2 > 3){
								octave2 = 3;
							}
							if(octave2 < 3)
								octave2++;
							break;
						case 7:
							if(octave2 > 2){
								octave2 = 2;
							}
							if(octave2 < 2)
								octave2++;
							break;
						case 8:
							if(octave2 > 1){
								octave2 = 1;
							}
							if(octave2 < 1)
								octave2++;
							break;
						case 9:
							if(octave2 > 0){
								octave2 = 0;
							}
							if(octave2 < 0)
								octave2++;
							break;
					}
				}
				else{
					if(octave2 > 0)
						octave2--;
				}
				break;
		}
	}
}

void blink_LED(uint8_t counter){
	switch(counter){
		case 0:
			PORTE |= (1<<PE0);
			PORTE &= ~(1<<PE3);
			break;
                case 1:
                        PORTE |= (1<<PE1);
                        PORTE &= ~(1<<PE0);
                        break;
                case 2:
                        PORTE |= (1<<PE2);
                        PORTE &= ~(1<<PE1);
                        break;
                case 3:
                        PORTE |= (1<<PE3);
                        PORTE &= ~(1<<PE2);
                        break;
		case 5:
			PORTE &= ~((1<<PE0) | (1<<PE1) | (1<<PE2) | (1<<PE3));
	}
}

//!!!!1
/***********************************************************************
 *Function: 		timer0 ISR          
 *Description:        	This functions polls the push buttons,
 *                  	polls the encoders and writes the mode to the bar graph display
 *                 	The function increases the values of the clock or alarm based of the encoder input 
 *                	and also increment ths number of minutes every 60 seconds.
 ***********************************************************************/
ISR(TIMER0_OVF_vect){
	static uint8_t encoder_val;	//value read in from the encoder SPI 
	uint8_t i; 
	static uint8_t tempo = 8; 
	static uint16_t ms;
	//static uint8_t play_count = 0; 

	ms++;
	if(ms % tempo == 0) {
		//for note duration (64th notes) 
		beat++;
		beat2++; 
	}

	//make PORTA an input port with pullups, write all 0's to DDRA and all 1's to PORTA 
	DDRA = 0x00;
	PORTA = 0xFF; 

	//enable tristate buffer for pushbutton switches, write PORTC bits 4-6 all HIGH
	PORTB = (1 << PB4) | (1 << PB5) | (1 << PB6);

	//now check each button and increment the count as needed
	for(i = 0; i < 8 ; i++){
		if(chk_buttons(i)){
			if(switch_ch == 1) 
				notes_to_play1 |= (1 << i);
			else
				notes_to_play2 |= (1 << i);
		}
		else
			if(switch_ch == 1)
				notes_to_play1 &= ~(1 << i);	//!!!1 
			else
				notes_to_play2 &= ~(1 << i);
	}


	//check for state change input, save notes, delete notes, switch channel
	for(i = 0; i < 3 ; i++){
		if(chk_buttonsC(i)){
			if(i == 0 && switch_ch == 1)	//save1				
				save1 = 1; 
			if(i == 1)
				delete1 = 1; 
			if(i == 2){		//far left button
				if(switch_ch == 1){
					switch_ch = 2;
					PORTC &= ~((1 << PC7) | (1 << PC6) | (1 << PC5)); 
					PORTC |= (1 << PC6) | (1 << PC5);
				}
				else if(switch_ch == 2){
					switch_ch = 1;
					PORTC &= ~((1 << PC7) | (1 << PC6) | (1 << PC5));
					PORTC |= (1 << PC7) | (1 << PC5);	
				}
			}	
		}
	}

	//check for channel 2 configurations 
	for(i = 0; i < 6 ; i++){
		if(chk_buttonsF(i)){
			if(i == 0 && switch_ch == 2)
				sequence_to_play[0] = notes_to_play2;
			if(i == 1 && switch_ch == 2)
				sequence_to_play[1] = notes_to_play2;
			if(i == 2 && switch_ch == 2)
				sequence_to_play[2] = notes_to_play2;
			if(i == 3 && switch_ch == 2)
				sequence_to_play[3] = notes_to_play2;
			if(i == 4){
				play = 1;
				PORTE |= (1<<PE4); 		//sequence playing LED
				sequence_flag = 0; 
			}
			if(i == 5){
				stop = 1;
				PORTE &= ~(1<<PE4);
			}
		}
	}

	//now check the encoders for input
	//Connections:
	//	CLK_INH:	PORTE, bit 6
	//	SHIFT_LD_N: 	PORTE, bit 5 
	PORTE |= (1 << PE6);	//parallel load the 74HC165, write a 1 to CLK_INH and a 0 to SHIFT_LD_N
	PORTE &= ~(1<<PE5); 
	PORTE &= ~(1 << PE6); 
	PORTE |= (1 << PE5); 	//enable shift mode reactivate CLK, CLK_ING = 0 and SHIFT_LD_N = 1
	encoder_val = spi_read();	//read the data
	PORTE |= (1 << PE6);		//disable this slave device so we can write to bar graph


	//check the left encoder CONTROL ATTRIBUTE: 1-steps, 2-rate, 3-octave
	if(((prev & 0b11) == 0b11) && ((encoder_val & 0b11) == 0b10)){	//we have clockwise rotation of the encoders, we see a shift from 0b11 to 0b10
		if(switch_ch == 1){
			attribute1++;
			if(attribute1 > 3)
				attribute1 = 1;
		}
		if(switch_ch == 2){
			attribute2++; 
			if(attribute2 > 3) 
				attribute2 = 1;
		}
	}
	else if(((prev & 0b11) == 0b11) && ((encoder_val & 0b11) == 0b01)){ //we have counter clockwise rotation
		if(switch_ch == 1){
			attribute1--; 
			if(attribute1 < 1)
				attribute1 = 3;
		}
		if(switch_ch == 2){
			attribute2--; 
			if(attribute2 < 1) 
				attribute2 = 3;
		}
	}

	//check the right encoder
	if(((prev & 0b1100) == 0b1100) && ((encoder_val & 0b1100) == 0b1000)){   
		if(switch_ch == 1)
			set_control(1, attribute1, 1);
		if(switch_ch == 2)
			set_control(2, attribute2, 1);
	}
	else if(((prev & 0b1100) == 0b1100) && ((encoder_val & 0b1100) == 0b0100)){ 
		if(switch_ch == 1)
			set_control(1, attribute1, 0);
		if(switch_ch == 2)
			set_control(2, attribute2, 0);
	}

	//disable tristate buffer for pushbutton switches, toogle the Y5 output for safety
	PORTB = (1 << PB4) | (0 << PB5) | (1 << PB6);

	//re-enable the bar graph for brightness
	PORTD &= ~(1 << PD2);

	//set prev equal to the current state
	prev = encoder_val; 

	//save notes
	static uint8_t saved_notes1 = 0; 
	static uint8_t saved1_flag = 0; 
	//check for channel
	if(save1){      
		saved_notes1 = notes_to_play1;            //save the notes
		saved1_flag = 1;
		save1 = 0;  
	}
	if(saved1_flag == 1)
		notes_to_play1 = saved_notes1; 
	if(delete1 == 1){
		saved1_flag = 0;
		delete1 = 0;
		notes_to_play1 = 0; 
	}	




/*
	static uint8_t saved_notes2 = 0;
	static uint8_t saved2_flag = 0;

	//check for channel 2
	if(save2){
		saved_notes2 = notes_to_play2;            //save the notes
		saved2_flag = 1;
		save2 = 0;
	}
	if(saved2_flag == 1)
		notes_to_play2 = saved_notes2;
	if(delete2 == 1){
		saved2_flag = 0;
		delete2 = 0;
		notes_to_play2 = 0; 
	}
*/
	static uint8_t counter = 0; 	

	if(play){
		if(sequence_flag){
			counter++; 
			counter %= 4; 
			sequence_flag = 0; 
		        blink_LED(counter);
		}
		notes_to_play2 = sequence_to_play[counter]; 
	}
	if(stop){
		counter = 0; 
		blink_LED(5);	//turn all LEDS off 
		play = 0; 
		stop = 0; 
		notes_to_play2 = 0; 	
	}




	//set value to be displayed to the LED 
	if(switch_ch == 1){
		switch(attribute1){
			case 1:
				count = steps1;
				break;
			case 2:
				count = rate1;
				break;
			case 3:
				count = octave1;
				break;
		}	
	}
	else{
		switch(attribute2){
			case 1:
				count = steps2;
				break;
			case 2:
				count = rate2;
				break;
			case 3:
				count = octave2;
				break;
		}
	}


	//call segsum
	if(switch_ch == 1)
		segsum(count, 0xff, attribute1, 1, notes_to_play1); 		//value, colon, attribute, channel, 0xfc to turn on colon 
	else
		segsum(count, 0xff, attribute2, 1, notes_to_play2); 
}

/***********************************************************************
 *Function:       	set_volume()      
 *Description:         	This function is called within main. Based off of the 
 *                    	the value in the passed paramter, the value of OCR3A 
 *                   	will change accordingly to detemine the volume of the 
 *                  	audio amplifier output.
 *			NOTE: timer 3 is configured in inverting mode, 
 *			the smaller the value of OCR3A, the louder the amplifier!
 ***********************************************************************/
int main(){
	//set port bits 4-7 B as outputs, a 1 in DDRB.n indicates that pin n of the given port is an output 
	DDRB = 0xFF;

	//set outputs for LEDS, bits 6, 5 for SHFTLD and CLK INHIBIT
	DDRE = 0xFF;	
	//	PORTE = 0x4f;

	//set PORTD bit 2 as output for OE_N input of Bar graph display (in lab3 this pin was tied to PB7, we need this in lab 4 for Timer2 PWM output)
	DDRD = 0xC4;	//added PORTD pin 6 as output

	//set PORTC is used as a Software PWM PORT C bit 0
	DDRC = 0xE0;		//all input except pins 6 and 7 (CHANNEL LEDS)
	PORTC = 0x1F;		//attach pullups	
	PORTC |= (1 << PC7) | (1 << PC5);	//initialize colors

	//set PRTF, used for channel 2 configuration
	DDRF = 0x00;	//use all pins as inputs 
	PORTF = 0xFF; 	//configure pull ups on all pins

	//Disable the tristate buffer, write unconnected pin Y5 LOW
	PORTB =  (1 << PB4) | (0 << PB5) | (1 << PB6);	

	//initialize SPI, and timers
	tcnt0_init();
	tcnt2_init(); 
	spi_init();
	music_init(); 

	//enable interrupts
	sei(); 

	//read the initial state of encoders
	PORTE |= (1 << PE6);     
	PORTE |= (1 << PE5);
	prev = spi_read();

	//initialize global variables
	digit_to_display = 0;

	//music initializers
	rate1 = 1;
	steps1 = 2;
	octave1 = 3;
	attribute1 = 1;
	count = rate1;

	//channel 2 initializers
	rate2 = 1;
	steps2 = 2;
	octave2 = 5;
	attribute2 = 1;
	repeat2 = 1; 

	//sequence constants
	sequence_flag = 0; 
	play = 0;
	stop = 0; 

	//start on channel 1
	switch_ch = 1; 	

	//set the 7 segment brightness 
	OCR2 = 255;

	while(1){
		//bound a counter (0-4) to keep track of the digit to display (so on each iteration of the llop we only target on digit)        
		digit_to_display = digit_to_display % 5;

		//make PORTA an output
		DDRA = 0xFF; 

		//send 7 segment code to LED segments on each iteration we only update a single digit
		PORTA = segment_data[digit_to_display]; 

		//send PORTB the digit to display
		PORTB = digit_select[digit_to_display]; 	

		//delay to ensure the segment reachs full brightness
		_delay_ms(1); 		

		//update digit to display
		digit_to_display++;

	}//while
}//main

