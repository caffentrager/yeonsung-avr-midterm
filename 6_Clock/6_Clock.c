#include <avr/io.h>
#include <avr/interrupt.h>

// 12시간제 시계
//   PF0 : 다음 단계로 이동
//   PF1 : 현재 단계의 값 증가
volatile unsigned char csec = 0;
volatile unsigned char sec = 0;
volatile unsigned char min = 0;
volatile unsigned char hour = 12;
volatile unsigned char AMPM = 0;

unsigned char debounce_time = 10;       // 10 x 10ms = 100ms
volatile unsigned char debounce = 0;

// 0 = Initial (전체 FND Blink, 시계 정지)
// 1 = AMPM set
// 2 = Hour set
// 3 = Min set
// 4 = Operation
volatile unsigned char status = 0;

unsigned char position = 0;
unsigned char seg;
unsigned char ss_data[10] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x27, 0x7f, 0x6f};
unsigned char position_data[4] = {0x0e, 0x0d, 0x0b, 0x07};

volatile unsigned char Qa = 0;
volatile unsigned char Qb = 0;

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
    csec++;

    if(csec >= 100)
    {
        csec = 0;

        if(status == 4) // Operation에서만 시간 증가
        {
            sec++;

            if(sec >= 60)
            {
                sec = 0;

                min++;

                if(min >= 60)
                {
                    min = 0;

                    hour++;

                    if(hour == 12) // 11 -> 12
                    {
                        AMPM = !AMPM;
                    }

                    if(hour >= 13) // 12 -> 1
                    {
                        hour = 1;
                    }
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
        Qa = PINF & 0b00000011;

        // 버튼을 놓는 순간 : Hour / Minute 설정에서 Blink를 ON부터 다시 시작
        if(Qa == 0 && Qb != 0)
        {
            if(status == 2 ||
               status == 3)
            {
                csec = 0;
            }
        }

        // PF0 상승엣지 : 다음 설정 단계로 이동
        if(debounce == 0)
        {
            if((Qa & 0b00000001) != 0 &&
               (Qb & 0b00000001) == 0)
            {
                status++;

                // Operation 다음에는 AMPM 설정부터 다시 설정 (Initial은 전원 ON 때만)
                if(status >= 5)
                {
                    status = 1;
                }

                // Operation 진입 : 0초부터 1초 시작
                if(status == 4)
                {
                    csec = 0;
                    sec = 0;
                }

                debounce = debounce_time;
            }
        }

        // PF1 상승엣지 : 현재 설정값 증가
        if(debounce == 0)
        {
            if((Qa & 0b00000010) != 0 &&
               (Qb & 0b00000010) == 0)
            {
                // AM / PM 설정
                if(status == 1)
                {
                    AMPM = !AMPM;
                }

                // Hour 설정
                else if(status == 2)
                {
                    hour++;

                    if(hour >= 13)
                    {
                        hour = 1;
                    }
                }

                // Minute 설정
                else if(status == 3)
                {
                    min++;

                    if(min >= 60)
                    {
                        min = 0;
                    }
                }

                debounce = debounce_time;
            }
        }

        Qb = Qa;

        // LED
        // PA0 = Blink LED, PA1 = AM/PM LED
        PORTA = 0;

        // Operation 상태 : PA0 0.5초 ON / OFF
        if(status == 4)
        {
            if(csec < 50)
            {
                PORTA |= 0b00000001;
            }
        }

        // AM / PM 표시 : AMPM == 0 이면 PA1 ON (status 0 Initial에서만 Blink)
        if(AMPM == 0)
        {
            if(!(status == 0 && csec >= 50))
            {
                PORTA |= 0b00000010;
            }
        }

        // Ghosting 방지
        PORTG = 0x0F;

        if(position == 0)
        {
            if(hour < 10)
            {
                seg = 0x00;     // 십의 자리 0 숨김
            }
            else
            {
                seg = ss_data[hour/10];
            }
        }
        else if(position == 1)
        {
            seg = ss_data[hour%10];
        }
        else if(position == 2)
        {
            seg = ss_data[min/10];
        }
        else
        {
            seg = ss_data[min%10];
        }

        // status 0 : Initial, 전체 FND Blink
        if(status == 0 &&
           csec >= 50)
        {
            seg = 0x00;
        }

        // status 2 : Hour 두 자리 Blink (버튼 누르는 동안은 계속 표시)
        else if(status == 2 &&
                csec >= 50 &&
                Qa == 0 &&
                position < 2)
        {
            seg = 0x00;
        }

        // status 3 : Minute 두 자리 Blink
        else if(status == 3 &&
                csec >= 50 &&
                Qa == 0 &&
                position >= 2)
        {
            seg = 0x00;
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
