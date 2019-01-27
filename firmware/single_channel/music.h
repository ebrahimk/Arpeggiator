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

/***********************************/

//control consts ch2 
extern volatile uint8_t save2;
extern volatile uint8_t delete2;

//musical consts ch2
extern volatile uint8_t attribute2;
extern volatile uint8_t notes_to_play2;
extern volatile uint8_t rate2;
extern volatile uint8_t steps2;
extern volatile uint8_t octave2;

void arpeggiate(uint8_t note, uint8_t notes, uint8_t duration, uint8_t octave, uint8_t step);
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
