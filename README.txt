ATmega128 실습 예제 (7.3728MHz 보드)

[보드 배치]
  버튼          PF0 ~ PF3 (누르면 1)
  LED           PA0 ~ PA7
  FND 세그먼트   PORTC
  FND 자리 선택  PG0 ~ PG3 (LOW = 선택)
  AM/PM FND     PORTD
  타이머         Timer2 CTC, 분주 1024, OCR2 = 71  ->  10ms

[예제]
  1_LED_Hold     PF0~PF3 버튼을 누르는 동안 PA0~PA3 LED ON
  2_LED_Toggle   PF0 버튼을 누를 때마다 PA0 LED 토글
  3_FND_0to9     PF0 버튼을 누를 때마다 오른쪽 끝 FND가 0~9 순환
  4_CTC_Blink    Timer2 CTC 모드로 PA0 LED 0.5초마다 토글
  5_Stopwatch    PF0 시작/정지, PF1 리셋(정지 중), PF2 표시 전환 (SS.CC <-> MM.SS)
  6_Clock        12시간제 시계
                 전원 ON(전체 깜빡임) -> AM/PM -> 시 -> 분 -> 동작
                 PF0 다음 단계, PF1 값 증가
                 PA0 LED = 동작 중 0.5초 점멸, PA1 LED = AM이면 ON (AM/PM 표시)

[참고]
  각 폴더의 .c 파일을 AVR Studio 새 프로젝트(AVR GCC, atmega128)에 넣어서 빌드.
