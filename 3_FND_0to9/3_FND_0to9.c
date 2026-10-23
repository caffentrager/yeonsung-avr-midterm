#include <avr/io.h>

// 버튼을 누를 때마다 FND 숫자 0 ~ 9 순환 (오른쪽 끝 1자리)
//   PF0 버튼
unsigned char ss_data[10] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x27, 0x7f, 0x6f};

void main()
{
    DDRF = 0x00;                    // PF0 ~ PF3 버튼 입력
    DDRC = 0xFF;                    // FND Segment
    DDRG = 0b00001111;              // FND Position (PG0 ~ PG3)
    DDRD = 0xFF;                    // AM/PM FND Segment
    DDRA = 0xFF;                    // LED

    PORTD = 0x00;
    PORTA = 0x00;
    PORTG = 0x07;                   // PG3만 LOW : 오른쪽 끝 자리 선택

    unsigned char Qa = 0;
    unsigned char Qb = 0;
    unsigned char FND = 0;

    PORTC = ss_data[FND];

    while(1)
    {
        Qa = PINF & 0x01;

        // 상승엣지
        if((Qa & 0x01) != 0 && (Qb & 0x01) == 0)
        {
            if(FND >= 9)
                FND = 0;
            else
                FND = FND + 1;
        }

        PORTC = ss_data[FND];

        Qb = Qa;
    }
}
