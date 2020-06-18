#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>


#define FOSC 16000000                       // Cz�stotliwo�� zegara
#define BAUD 9600
#define MYUBRR FOSC/16/BAUD -1
#define d_RECEIVE_DATA_COMPLETE !(UCSR0A&(1<<RXC0))
#define d_SEND_DATA_COMPLETE !(UCSR0A&(1<<UDRE0))


volatile char ReceivedChar;
char hello[] = "Hello World!";
char newLine[] = "\n";
char buffer[20];
float f = 3.14;


void usart_putchar(char bit8_data)
{
	while(d_SEND_DATA_COMPLETE) { ; }
	UDR0 = bit8_data;
}

void usart_pstr(char *s) {
	// Zap�tla si� na ca�ym stringu
	while (*s) {
		usart_putchar(*s);
		s++;
	}
}

int main(void)
{
	/* Inicjalizacja diody na pin 13 */
	
	DDRB |= (1 << DDB5);		// Ustaw PB5 (pin 13) jako wyj�cie
	PORTB |= (1 << DDB5);		// Stan wysoki na PB5 (pin 13)
	
	/* Inicjalizacja przerwa� na pin2 */
	
	DDRD &= ~(1 << DDD2);       // Ustaw PB0 (pin8) jako wej�cie

	PORTD &= ~(1 << PORTD2);		// Wy��cz pullUp na PB0 (pin 8)

	PCICR |= (1 << PCIE2);     // set PCIE2 to enable PCMSK2 scan
	PCMSK2 |= (1 << PCINT18);   // set PCINT18 to trigger an interrupt on state change
	
	/* Inicjalizacja USART */
	
	UBRR0H = (MYUBRR >> 8);
	UBRR0L = MYUBRR;
	
	UCSR0B |= (1 << RXEN0) | (1 << TXEN0);      // Aktywuje wysy�anie i odbieranie
	UCSR0B |= (1 << RXCIE0);                    // Aktywuje przerwanie podczas odbierania
	UCSR0C |= (1 << UCSZ01) | (1 << UCSZ00);    // Ustawia format ramki: 8 bit danych, 1 bit stop
	
	/* Inicjalizacja timera */
	
	TCCR1A = 0;
	TCCR1B = 0;
	TCNT1  = 0;

	OCR1A = 15625;//31250;            // Rejestr do kt�rego por�wnywany jest rejestr timera 16MHz/1024/1Hz
	TCCR1B |= (1 << WGM12);   // Tryb CTC
	TCCR1B |= (1 << CS10) | (1 << CS12);    // 1024 prescaler
	TIMSK1 |= (1 << OCIE1A);  // Przerwanie po por�wnaniu rejestr�w

	sei();		// W��czenie przerwa�
	
	usart_pstr(hello);		// Wita si� przez USART

	while(1)
	{
		;                                      // G��wna p�tla
	}
}

// Przerwanie USART
ISR (USART_RX_vect)
{
	ReceivedChar = UDR0;					// Czytaj z bufora RX
	ReceivedChar--;							// Odejmuje on odebranego znaku
	UDR0 = ReceivedChar;					// Pisz do bufora TX
}

// Przerwanie zewn�trzne INT0 (PD2 - pin 2)
ISR (PCINT2_vect)
{
	if( (PIND & (1 << PIND2)) == 1 )
	{
		/* Przej�cie ze stanu niskiego na wysoki */
		PORTB |= (1 << DDB5);	// Zapal LED
	}
	else
	{
		/* Przej�cie ze stanu wysokiego na niski */
		PORTB &= ~(1 << DDB5);	// Zga� LED
	}
}

// Przerwanie Timera1
ISR(TIMER1_COMPA_vect)    // Przerwanie timera1 (por�wnanie rejestr�w)
{
	dtostrf(f, 6, 4, buffer);	// Formatuje liczb� zmiennoprzecinkow� do stringa
	
	usart_pstr(newLine);			// Wypisuje string z bufora do USART
	usart_pstr(buffer);			// Wypisuje string z bufora do USART
}