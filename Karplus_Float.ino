// Simple Karplus-Strong - floating point //

#define SAMPLE_RATE 22050
#define SIZE        256
#define OFFSET      32
#define LED_PIN     13
#define LED_PORT    PORTB
#define LED_BIT     5
#define SPEAKER_PIN 11

  float out;
  float last = 0;
  float curr = 0;
  float delaymem[SIZE];
  uint8_t locat = 0;
  uint8_t bound = SIZE;
  float accum = 0;
  float lowpass = 0.99; // 0 ... 1.0
  bool trig = false;
 
  const float noise[128] = {
    0.646,0.833,0.398,0.750,0.835,0.322,0.552,0.979,0.549,0.330,0.619,0.361,0.757,0.414,0.492,0.695,
    0.973,0.328,0.838,0.739,0.954,0.032,0.357,0.663,0.282,0.230,0.711,0.625,0.591,0.660,0.048,0.349,
    0.162,0.794,0.311,0.529,0.166,0.602,0.263,0.654,0.689,0.748,0.451,0.084,0.229,0.913,0.152,0.826,
    0.538,0.996,0.078,0.443,0.107,0.962,0.005,0.775,0.817,0.869,0.084,0.400,0.260,0.800,0.431,0.911,
    0.182,0.264,0.146,0.136,0.869,0.580,0.550,0.145,0.853,0.622,0.351,0.513,0.402,0.076,0.240,0.123,
    0.184,0.240,0.417,0.050,0.903,0.945,0.491,0.489,0.338,0.900,0.369,0.111,0.780,0.390,0.242,0.404,
    0.096,0.132,0.942,0.956,0.575,0.060,0.235,0.353,0.821,0.015,0.043,0.169,0.649,0.732,0.648,0.451,
    0.547,0.296,0.745,0.189,0.687,0.184,0.368,0.626,0.780,0.081,0.929,0.776,0.487,0.436,0.447,0.306}; 


ISR(TIMER1_COMPA_vect) {
   
  OCR2A = 0xff & uint8_t(255 * out);
  
  if (trig){
  
    for (int i = 0; i < SIZE; i++) {
    
      accum = accum - (lowpass * (accum - noise[i>>1]));
     
      delaymem[i] = accum;
      
    }
  
    trig = false;

  }else{
    
    delaymem[locat++] = out;

    if (locat >= bound) locat = 0;
    
    curr = delaymem[locat];

    out = 0.5 * (last + curr);
    
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
  
  bound = random(OFFSET, SIZE);

  delay (160);
  
}
