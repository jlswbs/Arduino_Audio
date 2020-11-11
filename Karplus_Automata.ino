// 2x Karplus-Strong and Cellular Automata //

#define SAMPLE_RATE 22050
#define SIZE        256
#define OFFSET      32
#define LED_PIN     13
#define LED_PORT    PORTB
#define LED_BIT     5
#define SPEAKER_PIN 11

  int out1;
  int last1 = 0;
  int curr1 = 0;
  uint8_t delaymem1[SIZE];
  uint8_t locat1 = 0;
  uint8_t bound1 = SIZE;
  int accum1 = 0;
  int lowpass1 = 2;
  bool trig1 = false;

  int out2;
  int last2 = 0;
  int curr2 = 0;
  uint8_t delaymem2[SIZE];
  uint8_t locat2 = 0;
  uint8_t bound2 = SIZE;
  int accum2 = 0;
  int lowpass2 = 1;
  bool trig2 = false;

  bool rules[8] = {0,1,1,1,1,0,0,0};
  
  bool state[SIZE];
  bool newstate[SIZE];
  int i,j,k,temp;
    

ISR(TIMER1_COMPA_vect) {
   
  OCR2A = 0xff & ((out1 + out2)>>1);

  if (trig1) {
    
    for (int m = 0; m < SIZE; m++) delaymem1[m] = random();
    
    trig1 = false;
    
  } else {
    
    delaymem1[locat1++] = out1;

    if (locat1 >= bound1) locat1 = 0;
    
    curr1 = delaymem1[locat1];
    
    out1 = accum1 >> lowpass1;
    
    accum1 = accum1 - out1 + ((last1>>1) + (curr1>>1));

    last1 = curr1;
    
  }

  if (trig2) {
    
    for (int n = 0; n < SIZE; n++) delaymem2[n] = random();
    
    trig2 = false;
 
 } else {
    
    delaymem2[locat2++] = out2;

    if (locat2 >= bound2) locat2 = 0;
    
    curr2 = delaymem2[locat2];
    
    out2 = accum2 >> lowpass2;
    
    accum2 = accum2 - out2 + ((last2>>1) + (curr2>>1));

    last2 = curr2;
    
  }
   
}

void startPlayback(){

    pinMode(SPEAKER_PIN, OUTPUT);
    pinMode(LED_PIN, OUTPUT);

    ASSR &= ~(_BV(EXCLK) | _BV(AS2));
 
    TCCR2A |= _BV(WGM21) | _BV(WGM20);
    TCCR2B &= ~_BV(WGM22);

    TCCR2A = (TCCR2A | _BV(COM2A1)) & ~_BV(COM2A0);
    TCCR2A &= ~(_BV(COM2B1) | _BV(COM2B0));

    TCCR2B = (TCCR2B & ~(_BV(CS12) | _BV(CS11))) | _BV(CS10);

    OCR2A = 0;

    cli();

    TCCR1B = (TCCR1B & ~_BV(WGM13)) | _BV(WGM12);
    TCCR1A = TCCR1A & ~(_BV(WGM11) | _BV(WGM10));

    TCCR1B = (TCCR1B & ~(_BV(CS12) | _BV(CS11))) | _BV(CS10);

    OCR1A = F_CPU / SAMPLE_RATE;

    TIMSK1 |= _BV(OCIE1A);

    sei();
    
}


void stopPlayback()
{

    TIMSK1 &= ~_BV(OCIE1A);

    TCCR1B &= ~_BV(CS10);
    TCCR2B &= ~_BV(CS10);

    digitalWrite(SPEAKER_PIN, LOW);
}


void setup() {

  startPlayback();
  
  for (i=0;i<SIZE;i++) state[i]= random(2);

}


void loop() {
   
  for (j=0;j<SIZE;j++) {
         
    k = 4*state[(j-1+SIZE)%SIZE] + 2*state[j] + state[(j+1)%SIZE];
    
    newstate[j] = rules[k];

  }

  for (j=0;j<SIZE;j++) state[j] = newstate[j];
  
  
  LED_PORT ^= 1 << LED_BIT;

  temp = 0;

  for (i = 0; i < SIZE; i++) temp = temp + state[i];

  if (state[0]) { bound1 = random(temp, SIZE); trig1 = true; }
  if (state[1]) { bound2 = random(OFFSET,   temp); trig2 = true; }
 
  delay (160);

}
