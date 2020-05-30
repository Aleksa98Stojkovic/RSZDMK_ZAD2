#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#define USART_RX_BUFFER_SIZE 64

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define BR_KORISNIKA 10
 char korisnici[BR_KORISNIKA][32] =

{

    "Sundjer Bob Kockalone",

    "Dijego Armando Maradona",

    "Bond. Dzejms bond.",

    "Zoran Kostic Cane",

    "Kim Dzong Un",
	
	"Aleksa Stojkovic",
	
	"Dejvid Hilbert",
	
	"Nikolo Paganini",
	
	"Jan Lekun",
	
	"Mika Antic"

};

char PIN[BR_KORISNIKA][5] = {"5346", "2133", "7445", "8756", "7435", "1221", "1111", "1029", "1209", "0666"}; // mozda treba 5 zbog NULL


char Rx_Buffer[USART_RX_BUFFER_SIZE];			//prijemni FIFO bafer
volatile unsigned char Rx_Buffer_Size = 0;	//broj karaktera u prijemnom baferu
volatile unsigned char Rx_Buffer_First = 0;
volatile unsigned char Rx_Buffer_Last = 0;

ISR(USART_RX_vect)
{
	Rx_Buffer[Rx_Buffer_Last++] = UDR0;		//ucitavanje primljenog karaktera
	Rx_Buffer_Last &= USART_RX_BUFFER_SIZE - 1;	//povratak na pocetak u slucaju prekoracenja
	if (Rx_Buffer_Size < USART_RX_BUFFER_SIZE)
		Rx_Buffer_Size++;					//inkrement brojaca primljenih karaktera
}

void usartInit(unsigned long baud)
{
	UCSR0A = 0x00;	//inicijalizacija indikatora
					//U2Xn = 0: onemogucena dvostruka brzina
					//MPCMn = 0: onemogucen multiprocesorski rezim

	UCSR0B = 0x98;	//RXCIEn = 1: dozvola prekida izavanog okoncanjem prijema
					//RXENn = 1: dozvola prijema
					//TXENn = 1: dozvola slanja

	UCSR0C = 0x06;	//UMSELn[1:0] = 00: asinroni rezim
					//UPMn[1:0] = 00: bit pariteta se ne koristi
					//USBSn = 0: koristi se jedan stop bit
					//UCSzn[2:0] = 011: 8bitni prenos

	UBRR0 = F_CPU / (16 * baud) - 1;

	sei();	//I = 1 (dozvola prekida)
}

unsigned char usartAvailable()
{
	return Rx_Buffer_Size;		//ocitavanje broja karaktera u prijemnom baferu
}

void usartPutChar(char c)
{
	while(!(UCSR0A & 0x20));	//ceka da se setuje UDREn (indikacija da je predajni bafer prazan)
	UDR0 = c;					//upis karaktera u predajni bafer
}

void usartPutString(char *s)
{
	while(*s != 0)				//petlja se izvrsava do nailaska na nul-terminator
	{
		usartPutChar(*s);		//slanje tekuceg karaktera
		s++;					//azuriranje pokazivaca na tekuci karakter
	}
}

void usartPutString_P(const char *s)
{
	while (1)
	{
		char c = pgm_read_byte(s++);	//citanje sledeceg karaktera iz programske memorije
		if (c == '\0')					//izlazak iz petlje u slucaju
			return;						//nailaska na terminator
		usartPutChar(c);				//slanje karaktera
	}
}

char usartGetChar()
{
	char c;

	if (!Rx_Buffer_Size)						//bafer je prazan?
		return -1;
	c = Rx_Buffer[Rx_Buffer_First++];			//citanje karaktera iz prijemnog bafera
	Rx_Buffer_First &= USART_RX_BUFFER_SIZE - 1;	//povratak na pocetak u slucaju prekoracenja
	Rx_Buffer_Size--;							//dekrement brojaca karaktera u prijemnom baferu

	return c;
}

unsigned char usartGetString(char *s)
{
	unsigned char len = 0;

	while(Rx_Buffer_Size) 			//ima karaktera u faferu?
		s[len++] = usartGetChar();	//ucitavanje novog karaktera

	s[len] = 0;						//terminacija stringa
	return len;						//vraca broj ocitanih karaktera
}

bool proveri_korisnika(char ime[])
{
	
	for(int i = 0; i < BR_KORISNIKA; i++)
	{
		
		if(strcmp(ime, korisnici[i]) == 0)
		{
			return true;
		}
	}
	
	return false;
}

bool proveri_pin(char pin[])
{
	
	for(int i = 0; i < BR_KORISNIKA; i++)
	{
		
		if(strcmp(pin, PIN[i]) == 0)
		{
			return true;
		}
	}
	
	return false;
}

void isprazni_bafer()
{
	char temp;
	
	while(usartAvailable())
		temp = usartGetChar();
}

int main()
{
	
	usartInit(9600);
	char ime[32], pin[5];
	
	while(1)
	{
		
		usartPutString("Unesite ime i prezime:\n");
		while(!usartAvailable());
		_delay_ms(100);
		
		usartGetString(ime);
		
		if(proveri_korisnika(ime) == false)
		{
			usartPutString("Ime i prezime se ne nalaze u bazi\n");
		}
		else
		{
			int i = 0;
			
			usartPutString("Unesite PIN:\n");
			
			while(!usartAvailable());
			_delay_ms(100);
			pin[i++] = usartGetChar();
			usartPutChar('*');
			
			if(usartAvailable() == 0)
			{			
				while(!usartAvailable());
				_delay_ms(100);
				pin[i++] = usartGetChar();
				usartPutChar('*');
				
				if(usartAvailable() == 0)
				{
					while(!usartAvailable());
					_delay_ms(100);
					pin[i++] = usartGetChar();
					usartPutChar('*');
			
					if(usartAvailable() == 0)
					{
						while(!usartAvailable());
						_delay_ms(100);
						pin[i++] = usartGetChar();
						usartPutChar('*');
						
						if(proveri_pin(pin))
						{
							usartPutString("\nIsparvno je unet pin!\n");
						}
						else
						{
							usartPutString("\nPin je neispravan!\n");
						}
					}
					else
					{
						usartPutString("\nNeispravan format pina\n");
						isprazni_bafer();
					}
				}
				else
				{
					usartPutString("\nNeispravan format pina\n");
					isprazni_bafer();
				}
			}
			else
			{
				usartPutString("\nNeispravan format pina\n");
				isprazni_bafer();
			}
		}
		
	};
	return 0;
}