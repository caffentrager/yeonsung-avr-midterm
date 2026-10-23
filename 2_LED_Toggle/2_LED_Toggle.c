#include <avr/io.h>

// 버튼을 누를 때마다 LED 토글
//   PF0 버튼 -> PA0 LED
void main()
{
    DDRF = 0x00;                    // PF0 ~ PF3 버튼 입력
    DDRA = 0xFF;                    // LED 출력

    unsigned char Qa = 0;
    unsigned char Qb = 0;
    unsigned char LED = 0;

    while(1)
    {
        Qa = PINF & 0x01;

        // 상승엣지 : 이전에는 안 눌림, 지금은 눌림
        if((Qa & 0x01) != 0 && (Qb & 0x01) == 0)
        {
            LED = !LED;
        }

        PORTA = LED;

        Qb = Qa;
    }
}
