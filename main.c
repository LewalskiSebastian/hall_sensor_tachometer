#define FOSC 16000000                       // Częstotliwość zegara
#define BAUD 9600
#define MYUBRR FOSC/16/BAUD -1
#define d_RECEIVE_DATA_COMPLETE !(UCSR0A&(1<<RXC0))
#define d_SEND_DATA_COMPLETE !(UCSR0A&(1<<UDRE0))

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <stdint.h>


int i = 0;
int send_i = 0;
bool wait = false;
bool high = false;
bool low = true;
bool add = false;
volatile int analogVal;
char buffer[20];
char newLine = '\n';

void main(void) {
	timer_init();   // Inicjalizacja timera
	usart_init();
	adc_init();
	sei();   // Włączenie przerwań
	ADCSRA |= (1 << ADSC); // Rozpoczęcie przetwarzania ADC 
	while(1) 
	{
		;
	}
}

void timer_init() {   // Inicjalizacja timera1
	TCCR1A = 0;
	TCCR1B = 0;
	TCNT1  = 0;

	OCR1A = 3907;           // Rejestr do którego porównywany jest rejestr timera 16MHz/1024/0.25Hz
	TCCR1B |= (1 << WGM12);   // Tryb CTC
	TCCR1B |= (1 << CS10) | (1 << CS12);    // 1024 prescaler
	TIMSK1 |= (1 << OCIE1A);  // Przerwanie po porównaniu rejestrów
}

void adc_init() {
	// Zeruj bit ADLAR w ADMUX (0x7C)
	// ADCL będzie zawierał 8 młodszych bitów, a ADCH 2 starsze
	ADMUX &= ~(1 << ADLAR);
	
	// Ustawia bity REFS1 i REFS0 w ADMUX (0x7C) w celu wyboru napięcia referencyjnego z nóżki AREF (5V)
	ADMUX |= (1 << REFS0);
	
	// Zeruje bity MUX3-0 w ADMUX (0x7C) w celu ustawienia wejścia A0
	ADMUX &= ~((1 << MUX3)|(1 << MUX2)|(1 << MUX1)|(1 << MUX0));
	
	// Ustawia bit ADEN w ADCSRA (0x7A) aby włączyć przetwornik ADC.
	ADCSRA |= (1 << ADEN);
	
	// Ustawia bit ADATE w ADCSRA (0x7A) aby włączyć automatyczne wyzwalanie przetwornika
	ADCSRA |= (1 << ADATE);
	
	// Zeruje bity ADTS2-0 w ADCSRB (0x7B) aby włączyć tryb ciągłego przetwarzania
	ADCSRB &= ~((1 << ADTS2)|(1 << ADTS1)|(1 << ADTS0));
	
	// Ustawia prescaler na 2 (16MHz/2 = 8MHz)
	ADCSRA |= (1 << ADPS0);
	
	// Ustawia bit ADIE w ADCSRA (0x7A) aby aktywować przerwania wewnętrzne ADC
	ADCSRA |= (1 << ADIE);
}

void usart_init() {
	UBRR0H = (MYUBRR >> 8);
	UBRR0L = MYUBRR;
	
	UCSR0B |= (1 << RXEN0) | (1 << TXEN0);      // Włącza wysyłanie i odbieranie 
	UCSR0B |= (1 << RXCIE0);                    // Włącza przerwania USART
	UCSR0C |= (1 << UCSZ01) | (1 << UCSZ00);    // Ustawia format ramki 8 bitów danych 1 bit stopu
}

void usart_putchar(char b8_data)
{
	while(d_SEND_DATA_COMPLETE) { ; }
	UDR0 = b8_data;
}


void usart_pstr(char *s) {
	// Pętla po całym stringu
	while (*s) {
		usart_putchar(*s);
		s++;
	}
}

ISR(TIMER1_COMPA_vect)    // Przerwanie timera1 (porównanie rejestrów)
{
	send_i = i;
	itoa(send_i, buffer, 10);
	usart_pstr(buffer);
	usart_putchar(newLine);
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
