#include <SoftwareSerial.h>

#include <Servo.h>
#include <Stepper.h>
#include "chars.h"


//Пины для для подключения управляемой механики
#define SERVO_PIN 2
#define DOT_PIN 8
#define STEPPER_PIN 3

//Задержки для электромагнита, в миллисекундах
#define DOT_UP_DELAY 65
#define DOT_DOWN_DELAY 25

//Крайние положения области рисования сервопривода
#define SERVO_MIN 130
#define SERVO_MAX 180

//Количество точек в высоту. Чем больше, тем меньше получаются буквы
//Не должно превышать SERVO_MAX-SERVO_MIN
#define SERVO_STEPS 25


#define SERVO_STEP (SERVO_MAX - SERVO_MIN) / SERVO_STEPS

//Время, за которое сервопривод перемещается от точки к точке в миллисекундах
#define SERVO_DELAY SERVO_STEP*20


#define STEPPER_STEP 600/SERVO_STEPS

//Отступ от нижнего края в точках до текстовой строки
#define LINE_TAB 15

Servo servo;                        //Сервопривод
Stepper stepper(4096, 3, 4, 5, 6);  //Шаговый двигатель
SoftwareSerial Bluetooth(A3, A2);   // [RX, TX] Последовательный порт для bluetooth-модуля

void setup() {
  Bluetooth.begin(9600);
  
  servo.attach(SERVO_PIN);
  pinMode(DOT_PIN, OUTPUT);
  stepper.setSpeed(3);

  
  printString("HELLO");
}

//Функция для рисования точки на бумаге
void ping() {
  digitalWrite(DOT_PIN, 1);
  delay(DOT_UP_DELAY);
  digitalWrite(DOT_PIN, 0);
  delay(DOT_DOWN_DELAY);
}

//функция для рисования столбца из 8 точек по заданному байту, 
//в котором каждый бит понимается как точка
void printLine(int b) {
  //отъезжаем в начало столбца
  servo.write(SERVO_MAX - (LINE_TAB)*SERVO_STEP);

  //Если столбец не пустой, то мы его рисуем,
  //В противном случае, просто протягиваем ленту
  if(b != 0) {
    servo.write(SERVO_MAX - (LINE_TAB-1)*SERVO_STEP);
    delay(SERVO_DELAY*12);

    //Последовательно печатаем точки считывая информацию бит за битом
    for (int j = 0; b != 0; j++) {
      servo.write(SERVO_MAX - (LINE_TAB+j)*SERVO_STEP);
      delay(SERVO_DELAY);
  
      if(b & 1) ping();
      b >>= 1;
    }
  }

  //Протягиваем ленту, этот параметр также можно менять
  stepper.step(24);
}

//Функция печатает символ в соответствии с установленным шрифтом,
//Вызывая последовательно функции printLine()
void printChar(char c) {
int n = 0;
  
  for (int i = 0; i < 8; i++) {
    if(chars[c][i] != 0) {
      printLine(chars[c][i]);
      n++;
    }
    else stepper.step(15);
  }
}

//Фукция печатает поступающую ей на вход строку,
//Вызывая последовательно функции printChar()
void printString(char* str) {
  while(*str != '\0') {
    printChar(*str);
    str+=1;
  }
}

int n;

void loop() {
  //В случае, если что-то пришло по bluetooth, мы печатем все, что сможем
  if(Bluetooth.available() > 0) {
    while(Bluetooth.available() > 0) {
      n =  Bluetooth.read();
      if(n >= 0) printChar((char)n);;
    }
  }
  //Как только мы освобождаемся мы отправляем в ответ единичку,
  //показывающую, что мы готовы принимать ещё данные
  Bluetooth.write(1);
  delay(1500);
}
