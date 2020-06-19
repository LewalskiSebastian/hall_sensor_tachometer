unsigned long i = 0;
unsigned long send_i = 0;
bool wait = false;
bool high = false;
bool low = true;
bool add = false;
volatile int analogVal;

void setup() {
  noInterrupts();   // Wyłączeniue przerwań cli()
  timer_init();   // Inicjalizacja timera
  Serial.begin(9600);   // Inicjalizacja USART
  adc_init(); // Inicjalizacja ADC
  interrupts();   // Włączenie przerwań sei();
  // Ustawia bit ADSC w ADCSRA (0x7A) aby zacząć konwersję ADC
  ADCSRA |=B01000000;
}

void loop() {
  ;
}

void timer_init() {   // Inicjalizacja timera1
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;

  OCR1A = 3907;            // Rejestr do którego porównywany jest rejestr timera 16MHz/1024/0.25Hz
  TCCR1B |= (1 << WGM12);   // Tryb CTC
  TCCR1B |= (1 << CS10) | (1 << CS12);    // 1024 prescaler 
  TIMSK1 |= (1 << OCIE1A);  // Przerwanie po porównaniu rejestrów
}

void adc_init() {
  // Zeruj bit ADLAR w ADMUX (0x7C)
  // ADCL będzie zawierał 8 młodszych bitów, a ADCH 2 starsze
  ADMUX &= B11011111;
 
  // Ustawia bity REFS1 i REFS0 w ADMUX (0x7C) w celu wyboru napięcia referencyjnego z nóżki AREF (5V)
  ADMUX |= B01000000;
 
  // Zeruje bity MUX3-0 w ADMUX (0x7C) w celu ustawienia wejścia A0
  ADMUX &= B11110000;
  
  // Ustawia bit ADEN w ADCSRA (0x7A) aby włączyć przetwornik ADC.
  ADCSRA |= B10000000;
 
  // Ustawia bit ADATE w ADCSRA (0x7A) aby włączyć automatyczne wyzwalanie przetwornika
  ADCSRA |= B00100000;
 
  // Zeruje bity ADTS2-0 w ADCSRB (0x7B) aby włączyć tryb ciągłego przetwarzania
  ADCSRB &= B11111000;
 
  // Ustawia prescaler na 2 (16MHz/2 = 8MHz)
  ADCSRA |= B00000001;
 
  // Ustawia bit ADIE w ADCSRA (0x7A) aby aktywować przerwania wewnętrzne ADC
  ADCSRA |= B00001000;
}

ISR(TIMER1_COMPA_vect)    // Przerwanie timera1 (porównanie rejestrów)
{
  send_i = i;
  Serial.println(send_i); // Wyślij zmierzoną ilość impulsów na USART
  i -= send_i;
}

// Przerwanie po odczycie ADC
ISR(ADC_vect){
  analogVal = ADCL | (ADCH << 8); // Odczyt zmierzonej wartości
  if (analogVal == 1023) {  // Jeżeli minął jeden "szczyt"
    high = true;
    low = false;
  } else if (analogVal < 31) {  // To zwiększ "i" o jeden
    if (high) i++;
    low = true;
    high = false;
  }
}
