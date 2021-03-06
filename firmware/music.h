//function prototypes defined here
extern volatile uint16_t beat;
extern volatile uint16_t max_beat;
extern volatile uint8_t  notes;

//global control
extern volatile uint8_t switch_ch;

//control consts ch1 
extern volatile uint8_t save1;
extern volatile uint8_t delete1;

//musical consts ch1
extern volatile uint8_t attribute1;
extern volatile uint8_t notes_to_play1; 
extern volatile uint8_t rate1;
extern volatile uint8_t steps1; 
extern volatile uint8_t octave1;
extern volatile uint8_t type1;

//for signaling when to change the up down behavior
extern volatile uint8_t p_flag1;
extern volatile uint8_t p_flag2;

/***********************************/

extern volatile uint16_t beat2;
extern volatile uint16_t max_beat2;
extern volatile uint8_t  notes2;

//musical consts ch2
extern volatile uint8_t attribute2;
extern volatile uint8_t notes_to_play2;
extern volatile uint8_t rate2;
extern volatile uint8_t steps2;
extern volatile uint8_t octave2;
extern volatile uint8_t repeat2;
extern volatile uint8_t type2;

//sequence constants
extern volatile uint8_t play;         
extern volatile uint8_t stop;        
extern volatile uint8_t sequence_flag;
extern volatile uint8_t sequence_to_play[4];

//mode variables
extern volatile char* mode1;
extern volatile char* mode1_d;
extern volatile char* mode2;
extern volatile char* mode2_d;
extern volatile uint8_t modal1; 
extern volatile uint8_t modal2;

extern char C[8]; 
extern char dorian[8]; 
extern char phrygian[8]; 
extern char lydian[8]; 
extern char mixolydian[8];
extern char aeolian[8];
extern char locrian[8];

extern char C_d[8]; 
extern char dorian_d[8]; 
extern char phrygian_d[8]; 
extern char lydian_d[8]; 
extern char mixolydian_d[8];
extern char aeolian_d[8];
extern char locrian_d[8];

void arpeggiate2(uint8_t note, uint8_t notes_to_play, uint8_t duration, uint8_t octave, uint8_t step);
void arpeggiate(uint8_t note, uint8_t notes, uint8_t duration, uint8_t octave, uint8_t step);
void song0(uint16_t note); //Beaver Fight Song
void song1(uint16_t note); //Tetris Theme (A)
void song2(uint16_t note); //Mario Bros Theme
void song3(uint16_t note);
void play_song(uint8_t song, uint8_t note);
void play_rest(uint8_t duration);
void play_rest2(uint8_t duration);
void play_note(char note, uint8_t flat, uint8_t octave, uint8_t duration);
void play_note2(char note, uint8_t flat, uint8_t octave, uint8_t duration);
void music_off(void);
void music_on(void);
void music_init(void);
