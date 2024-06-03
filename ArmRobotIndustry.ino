#include <Servo.h>
#include <DFPlayer_Mini_Mp3.h>

SoftwareSerial mySerial(18, 19); //pin RX dan TX

Servo servo1;
Servo servo2;
Servo servo3;
Servo servo4;

int pinServo1 = 24;
int pinServo2 = 28;
int pinServo3 = 32;
int pinServo4 = 36;

int potentiometer1    = A0;
int potentiometer2    = A1;
int potentiometer3    = A2;
int pinIR             = A3;
int buttonStopRecord  = 4;
int buttonReset       = 5;
int pinCakram         = 6;
int buttonPlayRecord  = 7;
int ledStopRecord     = 38;
int ledStartRecord    = 40;
int pinrelay          = 44;

bool cakram_status = false;
bool isRecord = false;
bool isPlay = false;
bool w_reset = false;
int indexRecord = 0;
int sensorIR;
int servo4_close = 86;
int servo4_open  = 48;

const int moveCount = 10;
const int servoNumber = 4;
int movesServos[moveCount][servoNumber];
unsigned long startTime_servo = 0;
unsigned long interval_servo = 300;
unsigned long currentTime=0;

Servo servos[servoNumber] = {servo1, servo2, servo3, servo4};

void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);
  mp3_set_serial (mySerial); 
  delay(5); 
  mp3_set_volume (30);
  mp3_play(1);
  delay(800);
  
  pinMode(pinIR, INPUT);
  servo1.attach(pinServo1);
  servo2.attach(pinServo2);
  servo3.attach(pinServo3);
  servo4.attach(pinServo4);
  servo4.write(servo4_open);

  pinMode(pinCakram, INPUT_PULLUP);
  pinMode(buttonReset, INPUT_PULLUP);
  pinMode(buttonStopRecord, INPUT_PULLUP);
  pinMode(buttonPlayRecord, INPUT_PULLUP);

  pinMode(ledStartRecord, OUTPUT);
  pinMode(ledStopRecord, OUTPUT);
  pinMode(pinrelay, OUTPUT);
}

void loop() {
  currentTime = millis();
  on_off_motor();
  if (!isPlay) {
    int valPotentiometer1 = analogRead(potentiometer1);
    int valPotentiometer2 = analogRead(potentiometer2);
    int valPotentiometer3 = analogRead(potentiometer3);

    int valServo1 = map(valPotentiometer1, 0, 1023, 10, 145);
    int valServo2 = map(valPotentiometer2, 0, 1023, 22, 100);
    int valServo3 = map(valPotentiometer3, 0, 1023, 10, 100);

    servo1.write(valServo1);
    servo2.write(valServo2);
    servo3.write(valServo3);
   
    if (digitalRead(pinCakram) == LOW ) {
      if (cakram_status) {
        cakram_status = false;
        clawOpen(); 
      } else {
        cakram_status = true;
        clawClose();
      }
      delay(400);
    }
  }
  record();
  delay(5);
}

void record() {
  int buttonRecordMoveState =  digitalRead(buttonStopRecord);
  int buttonPlayRecordState =  digitalRead(buttonPlayRecord);
  
  if (digitalRead(buttonReset)==LOW) reset_program();
  
  if (buttonRecordMoveState == LOW && isRecord) {
    digitalWrite(ledStopRecord, LOW);
    digitalWrite(ledStartRecord, LOW);
    recordMove();
    delay(400);
    
    if (indexRecord==10) digitalWrite(ledStopRecord, LOW);else digitalWrite(ledStopRecord, HIGH);
    digitalWrite(ledStartRecord, LOW);
  } else if (buttonRecordMoveState == LOW && isPlay) {
    isPlay = false;
    digitalWrite(ledStopRecord, LOW);
    digitalWrite(ledStartRecord, LOW);
  }

  if (buttonPlayRecordState == LOW) {
    digitalWrite(ledStopRecord, LOW);
    digitalWrite(ledStartRecord, HIGH);
    isPlay = true;
    isRecord = false;
  }

  if (isPlay) {
    if (w_reset) {
      reset_program();
    } else {
      playRecord();
      if (w_reset) reset_program();
      delay(300);
    }
  }
}

void recordMove() {
  if (indexRecord < moveCount) {
    int servoRead1 =  servo1.read();
    int servoRead2 =  servo2.read();
    int servoRead3 =  servo3.read();
    int servoRead4 =  servo4.read();

    movesServos[indexRecord][0] = servoRead1;
    movesServos[indexRecord][1] = servoRead2;
    movesServos[indexRecord][2] = servoRead3;
    movesServos[indexRecord][3] = servoRead4;
    
    indexRecord++;
  }
}

void playRecord() {
  moves();
  goToStartPosition();
  delay(200);
}

void moves() {
  for ( int i = 0; i < indexRecord - 1; i++) {
    for ( int j = 0; j < servoNumber; ++j ) {
      if (j >= 3) {
        if (movesServos[i + 1][j] == servo4_close ) clawClose(); else clawOpen();
      } else {
        setServoPosition(servos[j], movesServos[i][j], movesServos[i + 1][j],15);
      }
    }
  }
}  
  
void goToStartPosition() {
  for ( int j = 0; j < servoNumber; ++j ) {
      if (j >= 3) {
        if (movesServos[0][j] == servo4_close) clawClose();  else clawOpen();
      } else {
        setServoPosition(servos[j], servos[j].read(), movesServos[0][j],15);
    }
  }
}

void setServoPosition(Servo servo, int startPosition, int endPosition,int Delay) {
 if (startPosition > endPosition) {
    for (int i = startPosition; i >= endPosition; i--) {
      on_off_motor();
      servo.write(i);
      delay(Delay);
      if (digitalRead(buttonReset)==LOW) w_reset = true;
    }
  } else {
    for (int i = startPosition; i <= endPosition; i++) {
      on_off_motor();
      servo.write(i);
      delay(Delay);
      if (digitalRead(buttonReset)==LOW) w_reset = true;
    }
  }
}

void clawOpen() {
  servo4.write(servo4_open);
}

void clawClose() {
  servo4.write(servo4_close);
}

void reset_program() {
  digitalWrite(ledStopRecord, HIGH);
  digitalWrite(ledStartRecord, HIGH);
  servo4.write(servo4_open);
  cakram_status = false;
  indexRecord = 0;
  memset(movesServos, 0, sizeof(movesServos));
  isRecord = true;
  isPlay = false;
  w_reset = false;
  mp3_play(2); 
  delay(1000);
}

void on_off_motor() {
  if (digitalRead(pinIR)==LOW) digitalWrite(pinrelay, HIGH); else digitalWrite(pinrelay, LOW);
}
