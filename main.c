#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>


#define FOSC 16000000							// Częstotliwość zegara
#define BAUD 9600
#define MYUBRR FOSC/16/BAUD -1
#define d_RECEIVE_DATA_COMPLETE !(UCSR0A&(1<<RXC0))
#define d_SEND_DATA_COMPLETE !(UCSR0A&(1<<UDRE0))


volatile char ReceivedChar;
char hello[] = "Hello World!";
char newLine[] = "\n";
char buffer[20];
float f = 3.14;


/* Pisanie zmiennej ośmiobitowej do UART */
void usart_putchar(char bit8_data)
{
	while(d_SEND_DATA_COMPLETE) { ; }
	UDR0 = bit8_data;
}

/* Pisanie string do UART */
void usart_pstr(char *s) {
	while (*s) {							// Zapętla się na całym stringu
		usart_putchar(*s);
		s++;
	}
}

int main(void)
{
	/* Inicjalizacja diody na pin 13 */
	
	DDRB |= (1 << DDB5);					// Ustaw PB5 (pin 13) jako wyjście
	PORTB |= (1 << DDB5);					// Stan wysoki na PB5 (pin 13)
	
	/* Inicjalizacja przerwań na pin2 */
	
	DDRD &= ~(1 << DDD2);					// Ustaw PB0 (pin8) jako wejście

	PORTD &= ~(1 << PORTD2);				// Wyłącz pullUp na PB0 (pin 8)

	PCICR |= (1 << PCIE2);					// set PCIE2 to enable PCMSK2 scan
	PCMSK2 |= (1 << PCINT18);				// set PCINT18 to trigger an interrupt on state change
	
	/* Inicjalizacja USART */
	
	UBRR0H = (MYUBRR >> 8);
	UBRR0L = MYUBRR;
	
	UCSR0B |= (1 << RXEN0) | (1 << TXEN0);      // Aktywuje wysyłanie i odbieranie
	UCSR0B |= (1 << RXCIE0);                    // Aktywuje przerwanie podczas odbierania
	UCSR0C |= (1 << UCSZ01) | (1 << UCSZ00);    // Ustawia format ramki: 8 bit danych, 1 bit stop
	
	/* Inicjalizacja timera */
	
	TCCR1A = 0;
	TCCR1B = 0;
	TCNT1  = 0;

	OCR1A = 15625;//31250; = 0.5Hz			// Rejestr do którego porównywany jest rejestr timera 16MHz/1024/1Hz
	TCCR1B |= (1 << WGM12);					// Tryb CTC
	TCCR1B |= (1 << CS10) | (1 << CS12);    // 1024 prescaler
	TIMSK1 |= (1 << OCIE1A);				// Przerwanie po porównaniu rejestrów

	sei();									// Włączenie przerwań
	
	usart_pstr(hello);						// Wita się przez USART

	while(1)
	{
		;									// Główna pętla
	}
}

// Przerwanie USART
ISR (USART_RX_vect)
{
	ReceivedChar = UDR0;					// Czytaj z bufora RX
	ReceivedChar--;							// Odejmuje on odebranego znaku
	UDR0 = ReceivedChar;					// Pisz do bufora TX
}

// Przerwanie zewnętrzne INT0 (PD2 - pin 2)
ISR (PCINT2_vect)
{
	if( (PIND & (1 << PIND2)) == 1 )
	{
		/* Przejście ze stanu niskiego na wysoki */
		PORTB |= (1 << DDB5);				// Zapal LED
	}
	else
	{
		/* Przejście ze stanu wysokiego na niski */
		PORTB &= ~(1 << DDB5);				// Zgaś LED
	}
}

// Przerwanie Timera1
ISR(TIMER1_COMPA_vect)						// Przerwanie timera1 (porównanie rejestrów)
{
	dtostrf(f, 6, 4, buffer);				// Formatuje liczbę zmiennoprzecinkową do stringa
	
	usart_pstr(newLine);					// Wypisuje string z bufora do USART
	usart_pstr(buffer);						// Wypisuje string z bufora do USART
}
