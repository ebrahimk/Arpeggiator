/************************************
 *Author: 	Kamron Ebrahimi
 *Lab:		Lab 6 code 
 *Description: 	This labs utilizes the ATmega128's SPI and timer/counters to periodically
 *		interrupt the processor and poll rotary encoders and pushbuttons for input.
 *		Rotary encoders increment a count displayed on a seven segment display and
 *		the pushbuttons set a mode, which is displayed on a bar graph display, 
 *		which determines how much the count is incremented by per rotary encoder turn.	
 *Date:		12/6/2018
 *************************************/
//#define F_CPU 16000000 // cpu speed in hertz 
#define TRUE 0x01
#define FALSE 0x00
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stdlib.h>
#include "hd44780.h"
#include "lm73_functions.h"
#include "twi_master.h"
#include "uart_functions.h"
#include "si4734.h"
#include <avr/eeprom.h>
#include "music.h"

#define RX_VOLUME 0x4000

void toggle_radio(uint8_t flag);
uint8_t map_signal(uint8_t data);

//globals for music
//uint8_t channel1_display[8] = {0xFE, 0xDF, 0xBF, 0xFB, 0xF7, 0xEF, 0xBF, 0xFD};
//uint8_t channel1_count; 
//uint8_t display_num = 0;


//Count stores the value displayed to the seven seg 
uint16_t count;

//clock stores the current time
uint16_t clock; 
uint8_t hour; 
uint8_t min; 

//alarm stores the time when the alarm is triggered, same configuration 
uint16_t alarm;
uint8_t al_hour;
uint8_t al_min; 

//alarm toggling 
uint8_t is_armed; 
uint8_t prev_state;
uint8_t alarm_on; 
uint8_t arm_leds; 

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

//timing variable for incrementing the minute 
uint16_t time;

//the current mode the alarm clock operates in, set by the pushbuttons
uint8_t mode;		 

//timing variable used for blinking the semicolon at one secong intervals
uint16_t sec; 

//The volume of the clocks alarm, used in a function in main to set OCR3A
//uint8_t volume; 

//globbal variables for driving the TWI temperature sensor breakout
char    lcd_string_array[16];  //holds a string to refresh the LCD
char 	local_temp[8] = {'L'};		//temp will be the final string written to the second line of the LCD
char	remote_temp[8] = {'R'}; 
char 	remote_raw[16];
char 	temp_final[16];
int	local_bin; 
int 	remote_bin; 
extern  uint8_t lm73_wr_buf[2];
extern uint8_t lm73_rd_buf[2];
uint8_t is_metric; 

//globals for uart
volatile uint8_t  rcv_rdy;
char              rx_char;

//globals for the Radio 
uint8_t radio_change;
uint8_t band_change;
uint8_t radio_alarm; 

uint8_t radio_on; 

enum radio_band{FM, AM, SW};
volatile enum radio_band current_radio_band;

volatile uint8_t STC_interrupt;  //flag bit to indicate tune or seek is done

uint8_t si4734_tune_status_buf[8]; //buffer for holding tune_status data  
uint16_t eeprom_fm_freq;
uint16_t eeprom_am_freq;
uint16_t eeprom_sw_freq;
uint8_t  eeprom_volume;
uint16_t current_fm_freq;
uint16_t current_am_freq;
uint16_t current_sw_freq;
uint8_t  current_volume;


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

/********************************************************************
 *Name:			init_radio()
 *Description:		initializes all port i/o operations for setting up the si4734
 *Return:		N/A
 ******************************************************************/
void init_radio(){
        DDRE |= (1 << PE7); 			//configure PORTE bits 7 as an output 
        PORTE |= (1<< PE2);     		//Write a HIGH to the RESET pin of the radio, the radio initiliazes on a falling edge of PE2
	PORTE &= ~(1 << PE7);			//write a LOW to GPIO2, now on the falling edge the radio will be in TWI mode
	
	_delay_ms(100);						
	PORTE &= ~(1 << PE2);			//send the falling edge to RESET configuring the radio in TWI mode
	_delay_ms(100); 
	
        EIMSK |= (1 << INT7);                   //initialize interrupt 7
        EICRB |= (1<< ISC71) | (1 << ISC70);    //configure interrupt to occur on rising edge of PORTE7  
	DDRE &= ~(1 << PE7);  			//set PE7 as an interrupr input	
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

	//display 1	
	static uint8_t dis1_3[8] = {0xFE, 0xDF, 0xBF, 0xFB, 0xF7, 0xEF, 0xBF, 0xFD};	
	static uint8_t dis1_4[8] = {0xFE, 0xFD, 0xBF, 0xEF, 0xF7, 0xFB, 0xBF, 0xDF};

	//display 2
	static uint8_t dis2_3[14] = {0xFE, 0xFF, 0xFF, 0xFF, 0xBF, 0xFB, 0xF7, 0xFF, 0xFF, 0xFF, 0xBF, 0xFD, 0xFE, 0xFF};
	static uint8_t dis2_4[14] = {0xFF, 0xFE, 0xDF, 0xBF, 0xFF, 0xFF, 0xFF, 0xF7, 0xEF, 0xBF, 0xFF, 0xFF, 0xFF, 0xFE};

	//display a
	static uint8_t dis3_3[12] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xFD, 0xFB, 0xF7, 0xEF, 0xDF};
	static uint8_t dis3_4[12] = {0xFE, 0xDF, 0xEF, 0xF7, 0xFB, 0xFD, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

	static uint8_t* displays3[3] = {dis1_3, dis2_3, dis3_3}; 
	static uint8_t* displays4[3] = {dis1_4, dis2_4, dis3_4};
	static uint8_t dis_lengths[3] = {8, 14, 12};  

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
			dis_index %= 3;
		}
	}
	else{
		display_pattern(displays3[0], displays4[0], dis_lengths[0], speed);
	}
}

/*
   void snooze_mode(){
   clear_display();

   line1_col1();
   if(radio_alarm)
   string2lcd("ALARM:R  SNOOZE");
   else
   string2lcd("ALARM:A  SNOOZE");
   al_min += 10; //add ten minutes to the alarm	

//correct for rollover
if(al_min >= 60){
al_min = al_min % 60;
al_hour++; 
}
//turn the alarm of
if(radio_alarm)
toggle_radio(FALSE); 
alarm_on = FALSE; 
is_armed = TRUE;	//10 cycles of the indicator
// on the next iteration of timer 0 ISR, alarms value will be changed
}
 */


/************************************
 *Name: 		convert()		
 *Description:		This function takes an integer value read in from the LM73 sensors, and performs
 *			a conversion process changing the raw binary number input from the LM73 to a human understandable
 * 			temperature. Furthermore this button checks what state the alarm clock is in, if it is metric
 *			the conversion changes the value to a Celius temperature, otherwise the readout is in standard. 
 ***********************************/
/*
   void convert(int temp_bin, char temp_arr[]){
   float temp;
   uint8_t decimal_comp; 
   temp = (-40) + 0.00782*((float)temp_bin-(-5120));  
   if(!is_metric)
   temp = (temp *1.8) + 32;


   itoa(temp, temp_arr + 1, 10);
   temp_arr[strlen(temp_arr)] = '.';

   decimal_comp = ((uint8_t)(temp * 10) % 10);
   itoa(decimal_comp, temp_arr+4, 10);

   decimal_comp = ((uint8_t)(temp * 100) % 10);
   itoa(decimal_comp, temp_arr+5, 10);

   if(!is_metric)
   temp_arr[strlen(temp_arr)] = 'F';
   else
   temp_arr[strlen(temp_arr)] = 'C';	
   }
 */

ISR(USART0_RX_vect){			//we need this data as an integer in order to process it
	static  uint8_t  i;
	rx_char = UDR0;              //get character
	remote_raw[i++]=rx_char;  //store in array 
	//if entire string has arrived, set flag, reset index
	if(rx_char == '\0'){
		rcv_rdy=1;
		remote_raw[--i]  = (' ');     //clear the count field
		remote_raw[i+1]  = (' ');
		remote_raw[i+2]  = (' ');
		i=0;
	}
}

/****************************************
 *Name: 		ISR(INT7_vect)
 *Description:		This interrupt is triggered whenever the GPIO2 pin on the si4734 goes high.
 *			This interrupt scheme is used by the radio header library to signal that an operation is completed by the radio	
 *Return:		N/A 
 **************************************/
ISR(INT7_vect){
	STC_interrupt = TRUE;
}

/****************************************
 *Name:			toggle_radio()
 *Description:		turns the radio either on or off and sets some global signalling flags
 *Return:		N/A 
 **************************************/
/*void toggle_radio(uint8_t flag){						//handles powering the radio down if switching between am and fm 
  if(flag){
  set_property(RX_VOLUME, 0x003F);
  radio_on = TRUE;
  return; 
  }
  if(radio_on)
  radio_on = FALSE;
  else
  radio_on = TRUE;

  if(radio_on)
  set_property(RX_VOLUME, 0x003F);		//sets the output volume of the radio, set to default value (100%) 63	
  else
  set_property(RX_VOLUME, 0x0000);  
  }
 */

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
	//CHANNEL TWO 
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
	}

	//make PORTA an input port with pullups, write all 0's to DDRA and all 1's to PORTA 
	DDRA = 0x00;
	PORTA = 0xFF; 
	time++;
	sec++; 

	//enable tristate buffer for pushbutton switches, write PORTC bits 4-6 all HIGH
	PORTB = (1 << PB4) | (1 << PB5) | (1 << PB6);

	//now check each button and increment the count as needed
	for(i = 0; i < 8 ; i++){
		if(chk_buttons(i)){
			if(switch_ch == 1) 
				notes_to_play1 |= (1 << i);
			//CHANNEL TWO
		}
		else
			if(switch_ch == 1)
				notes_to_play1 &= ~(1 << i); 
		//CHANNEL TWO
	}


	//check for state change input, save notes, delete notes, switch channel
	for(i = 0; i < 3 ; i++){
		if(chk_buttonsC(i)){
			if(switch_ch == 1){
				if(i == 0)	//save1				
					save1 = 1; 
				if(i == 1)
					delete1 = 1; 
			}
			//CHANNEL TWO
			if(i == 2){
				if(switch_ch == 1)
					switch_ch = 2;
				else if(switch_ch == 2)
					switch_ch = 1;
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
	PORTE = (1 << PE5); 	//enable shift mode reactivate CLK, CLK_ING = 0 and SHIFT_LD_N = 1
	encoder_val = spi_read();	//read the data
	PORTE |= (1 << PE6);		//disable this slave device so we can write to bar graph


	//check the left encoder CONTROL ATTRIBUTE: 1-steps, 2-rate, 3-octave
	if(((prev & 0b11) == 0b11) && ((encoder_val & 0b11) == 0b10)){	//we have clockwise rotation of the encoders, we see a shift from 0b11 to 0b10
		if(switch_ch == 1){
			attribute1++;
			if(attribute1 > 3)
				attribute1 = 1;
		}
		//CHANNEL TWO
	}
	else if(((prev & 0b11) == 0b11) && ((encoder_val & 0b11) == 0b01)){ //we have counter clockwise rotation
		if(switch_ch == 1){
			attribute1--; 
			if(attribute1 < 1)
				attribute1 = 3;
		}
		//CHANNEL TWO
	}

	//check the right encoder
	if(((prev & 0b1100) == 0b1100) && ((encoder_val & 0b1100) == 0b1000)){   
		if(switch_ch == 1)
			set_control(1, attribute1, 1);
	}
	else if(((prev & 0b1100) == 0b1100) && ((encoder_val & 0b1100) == 0b0100)){ 
		if(switch_ch == 1)
			set_control(1, attribute1, 0);
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
	else
		count = switch_ch;
	//CHANNEL TWO


	//call segsum
	segsum(count, 0xff, attribute1, 1, notes_to_play1); 		//value, colon, attribute, channel, 0xfc to turn on colon 
}

/***********************************************************************
 *Function:        	map_to_brightness()      
 *Description:         	Takes the result of the ADC calculation. Uses the  
 *                    	result to load Timer 2's OCR2 register witha value that adjusts 
 *                   	the brightness accordingly. 
 *			Configured for smooth transitions with a 10k ohm resistor as R1
 *			Based of experimentation, OCR2 can go as low as 10 before even in a pitch dark room, the LEDs are too dim. 
 ***********************************************************************/
void map_to_brightness(uint16_t adc_result){
	if(adc_result <= 200)			//in decreasing brightness	
		OCR2 = 255; 
	else if(adc_result <= 300)	
		OCR2 = 200;
	else if(adc_result <= 400)
		OCR2 = 150; 
	else if(adc_result <= 500)	
		OCR2 = 100;
	else if(adc_result <= 600)
		OCR2 = 80; 
	else if(adc_result <= 700)
		OCR2 = 40; 
	else 
		OCR2 = 25;
}

/*******************************************************************
 *Name:			map_signal()
 *Description:		Maps the received signal strength grabbed from the si4734 to a value easily read 
 *			for the bar graph display.  
 *Return:		N/A 
 ******************************************************************/
uint8_t map_signal(uint8_t data){ 
	if(data >= 80)
		return 255;
	else if(data >= 70)
		return 127; 
	else if(data >= 60)
		return 63;
	else if(data >= 50)
		return 31; 
	else if(data >= 40)
		return 15;		
	else if(data >= 30)
		return 7;
	else if(data >= 20)
		return 3;
	else if(data >= 10)
		return 1;
	else
		return 0; 
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

	//set bits 6, 5  and 3(volume control) and 2 (active high reset radio) of PORTE as outputs, for SHIFT_LD_N and CLK_INH and Radio RESET
	DDRE = 0x6C;	

	//set PORTD bit 2 as output for OE_N input of Bar graph display (in lab3 this pin was tied to PB7, we need this in lab 4 for Timer2 PWM output)
	DDRD = 0x84;

	//set PORTC is used as a Software PWM PORT C bit 0
	DDRC = 0x00;
	PORTC = 0xFF;		//attach pullups	

	//Disable the tristate buffer, write unconnected pin Y5 LOW
	PORTB =  (1 << PB4) | (0 << PB5) | (1 << PB6);	

	//initialize SPI, and timers
	tcnt0_init();
	tcnt2_init(); 
	//	tcnt1_init();
	//tcnt3_init();
	spi_init();
	lcd_init();
	init_twi();
	uart_init();
	init_radio();
	music_init(); 
	//enable interrupts
	sei(); 

	//read the initial state of encoders
	PORTE |= (1 << PE6);     
	PORTE |= (1 << PE5);
	prev = spi_read();

	//initialize global variables
	digit_to_display = 0;

	hour = 24; 

	min = 0; 
	time = 0;
	sec = 0;	

	mode = 1; 

	al_hour = 24; 
	al_min = 0;

	prev_state = 0;
	alarm_on = FALSE;
	is_armed = 0; 
	arm_leds = 0; 

	is_metric = FALSE; 

	//initialize alarm and clock so we dont triggerthe alarm if uninit
	alarm = 1;
	clock = 5;

	//remote sensor and temp globals
	rcv_rdy = 0;
	local_bin = 0;
	remote_bin = 0;

	//music initializers
	rate1 = 1;
	steps1 = 2;
	octave1 = 3;
	attribute1 = 1;
	count = rate1;

	//start on channel 1
	switch_ch = 1; 	

	//Initalize ADC, its ports, and configuration
	DDRF  &= ~(_BV(DDF7)); //make port F bit 7 is ADC input  
	PORTF &= ~(_BV(PF7));  //port F bit 7 pullups must be off
	ADMUX = (1 << REFS0) | (1<< MUX2) | (1<<MUX1) | (1<<MUX0);      //single-ended, input PORTF bit 7, right adjusted, 10 bits
	ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1<< ADPS1) | (1 << ADPS0);       //ADC enabled, don't start yet, single shot mode 
	uint16_t adc_result; 

	//LM73 TWI interface
	lm73_wr_buf[0] = LM73_PTR_TEMP; //load lm73_wr_buf[0] with temperature pointer address
	twi_start_wr(LM73_ADDRESS, lm73_wr_buf, 1);  //start the TWI write process      
	_delay_ms(2);    //wait for the xfer to finish
	uint16_t temp_counter = 0; 

	//init radio globals
	radio_change = FALSE; 
	current_radio_band = FM; 
	band_change = FALSE; 
	radio_alarm = FALSE; 
	radio_on = FALSE; 

	fm_pwr_up();				//send power up command to radio, enable GPIO2 pin to go HIGH when a seek/tune is complete 
	current_fm_freq = 10630; 		//units of 10kHz
	fm_tune_freq();		 		//send the frequency to tune to, halts execution until EXT7 interrupt is triggered. 
	set_property(RX_VOLUME, 0x0000);	//radio starts OFF
	current_am_freq = 1240;		 	//local beaver channel

	line1_col1();
	string2lcd("ALARM:A NOT SET");         
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

		//read analog voltage for dimming functionality
		ADCSRA |= (1 << ADSC);        //poke ADSC and start conversion
		while((ADCSRA & (1 << ADIF)) != (1<< ADIF)){}//spin while interrupt flag not set
		ADCSRA |= (1 << ADIF); //its done, clear flag by writing a one 
		adc_result = ADC; 
		map_to_brightness(adc_result);                      //read the ADC output as 16 bits

		//set the volume, simply set OCR3A to the volume global variable (uint8_t)
		//	OCR3A = volume1; 

	}//while
}//main

