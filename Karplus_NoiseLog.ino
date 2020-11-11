// Simple Karplus-Strong and Noise(1/f) seed is Logistic equation //

#define SAMPLE_RATE 22050
#define SIZE        256
#define OFFSET      32
#define LED_PIN     13
#define LED_PORT    PORTB
#define LED_BIT     5
#define SPEAKER_PIN 11

  int out;
  int last = 0;
  int curr = 0;
  uint8_t delaymem[SIZE];
  uint8_t locat = 0;
  uint8_t bound = SIZE;
  int accum = 0;
  int lowpass = 1;
  bool trig = false;

  float r = 4.0;
  float m = 0.7;
  float x = 0.1;
  float y = 0.1;
  

ISR(TIMER1_COMPA_vect) {
   
  OCR2A = 0xff & out;

  if (trig) {
    
    for (int i = 0; i < SIZE; i++) delaymem[i] = random();
    
    trig = false;
    
  } else { 
    
    delaymem[locat++] = out;

    if (locat >= bound) locat = 0;
    
    curr = delaymem[locat];
    
    out = accum >> lowpass;
    
    accum = accum - out + ((last>>1) + (curr>>1));

    last = curr;
    
  }
   
}

void startPlayback()
{
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

}


void loop() {
  
  LED_PORT ^= 1 << LED_BIT;
  
  trig = true;

  float nx = x;
  float ny = y;
  
  x = r * nx * (1 - nx); // Logistic equation

  y = m * ny + x * sqrt(1 - pow(m,2)); // Noise 1/f equation
  
  bound = OFFSET + ((SIZE-OFFSET) * (y/1.8));

  delay (160);
  
}
