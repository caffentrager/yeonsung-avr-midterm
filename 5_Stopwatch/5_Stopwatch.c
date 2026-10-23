#include <avr/io.h>
#include <avr/interrupt.h>

// 스톱워치
//   PF0 : 시작 / 정지
//   PF1 : 리셋 (정지 중일 때만)
//   PF2 : 표시 전환  SS.CC <-> MM.SS
volatile unsigned char sw_csec = 0;     // 1/100초
volatile unsigned char sw_sec = 0;      // 초 (0 ~ 59)
volatile unsigned char sw_min = 0;      // 분 (0 ~ 59)
volatile unsigned char running = 0;     // 0 = 정지, 1 = 동작
volatile unsigned char show_min = 0;    // 0 = SS.CC, 1 = MM.SS

unsigned char debounce_time = 10;       // 10 x 10ms = 100ms
volatile unsigned char debounce = 0;

unsigned char position = 0;
unsigned char seg;
unsigned char d0;
unsigned char d1;
unsigned char ss_data[10] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x27, 0x7f, 0x6f};
unsigned char position_data[4] = {0x0e, 0x0d, 0x0b, 0x07};

unsigned char Qa = 0;
unsigned char Qb = 0;

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
    if(debounce > 0)
    {
        debounce--;
    }

    // 10ms마다 호출
    if(running == 1)
    {
        sw_csec++;

        if(sw_csec >= 100)
        {
            sw_csec = 0;

            sw_sec++;

            if(sw_sec >= 60)
            {
                sw_sec = 0;

                sw_min++;

                if(sw_min >= 60)
                {
                    sw_min = 0;
                }
            }
        }
    }
}

void main()
{
    DDRC = 0xFF;                // FND Segment
    DDRG = 0b00001111;          // FND Position (PG0 ~ PG3)
    DDRD = 0xFF;                // AM/PM FND Segment
    DDRA = 0xFF;                // LED
    DDRF = 0x00;                // PF0 ~ PF3 버튼 입력

    PORTC = 0x00;
    PORTG = 0x0F;
    PORTD = 0x00;
    PORTA = 0x00;

    t2OCI_init();

    sei();

    while(1)
    {
        Qa = PINF & 0b00000111;

        if(debounce == 0)
        {
            // PF0 상승엣지 : 시작 / 정지
            if((Qa & 0b00000001) != 0 &&
               (Qb & 0b00000001) == 0)
            {
                running = !running;
                debounce = debounce_time;
            }

            // PF1 상승엣지 : 리셋 (정지 중일 때만)
            else if((Qa & 0b00000010) != 0 &&
                    (Qb & 0b00000010) == 0)
            {
                if(running == 0)
                {
                    sw_csec = 0;
                    sw_sec = 0;
                    sw_min = 0;
                }

                debounce = debounce_time;
            }

            // PF2 상승엣지 : 표시 전환
            else if((Qa & 0b00000100) != 0 &&
                    (Qb & 0b00000100) == 0)
            {
                show_min = !show_min;
                debounce = debounce_time;
            }
        }

        Qb = Qa;

        // 표시할 값 : 앞 두 자리(d0), 뒤 두 자리(d1)
        if(show_min == 1)
        {
            d0 = sw_min;
            d1 = sw_sec;
        }
        else
        {
            d0 = sw_sec;
            d1 = sw_csec;
        }

        // Ghosting 방지
        PORTG = 0x0F;

        if(position == 0)
        {
            seg = ss_data[d0/10];
        }
        else if(position == 1)
        {
            seg = ss_data[d0%10];
            seg |= 0x80;            // 구분 소수점
        }
        else if(position == 2)
        {
            seg = ss_data[d1/10];
        }
        else
        {
            seg = ss_data[d1%10];
        }

        PORTC = seg;

        // 현재 자리 ON
        PORTG = position_data[position];

        // 다음 자리
        position++;

        if(position >= 4)
        {
            position = 0;
        }
    }
}
