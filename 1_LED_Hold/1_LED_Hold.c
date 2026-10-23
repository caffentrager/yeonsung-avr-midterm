#include <avr/io.h>

// 버튼을 누르고 있는 동안만 LED ON
//   PF0 ~ PF3 버튼 -> PA0 ~ PA3 LED
void main()
{
    DDRF = 0x00;                    // PF0 ~ PF3 버튼 입력
    DDRA = 0xFF;                    // LED 출력

    while(1)
    {
        PORTA = PINF & 0x0F;        // 버튼 상태를 그대로 LED에 출력
    }
}
