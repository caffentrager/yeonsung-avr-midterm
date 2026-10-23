#include <avr/io.h>
#include <avr/interrupt.h>

// Timer2 CTC 모드로 LED(PA0) 0.5초마다 토글
volatile unsigned char csec = 0;
volatile unsigned char LED = 0;

void t2OCI_init(void)
{
    // CTC Mode, 분주 1024
    // 7372800 / 1024 / (71 + 1) = 100Hz (정확히 10ms)
    TCCR2 = (1<<WGM21) |
            (0<<WGM20) |
            (1<<CS22) |
            (0<<CS21) |
            (1<<CS20);

    OCR2 = 71;

    TIMSK = (1<<OCIE2);
}

ISR(TIMER2_COMP_vect)
{
    csec++;                         // 10ms마다 호출

    if(csec >= 50)                  // 500ms
    {
        csec = 0;
        LED = !LED;
    }
}

void main()
{
    DDRA = 0xFF;                    // LED
    PORTA = 0x00;

    t2OCI_init();

    sei();

    while(1)
    {
        if(LED)
            PORTA = 0x01;
        else
            PORTA = 0x00;
    }
}
