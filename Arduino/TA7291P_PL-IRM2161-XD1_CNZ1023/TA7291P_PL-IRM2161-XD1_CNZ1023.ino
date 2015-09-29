#include <IRremote.h>

//void receiveValue(String,int *);

// モータ
//右
const int motorR1 = 8;
const int motorR2 = 9;
const int PWM_motR = 10;
//左
const int motorL1 = 11;
const int motorL2 = 12;
const int PWM_motL = 13;
//スピード
int speedL = 255;//後で直進する比を実測、255:237とか
int speedR = 255;
//IRモジュール
const int IRMDL = 7;
IRrecv irrecv(IRMDL);
decode_results results;
//フォトインタラプタ、change回数
int volatile changeL = 0;
int volatile changeR = 0;
//測距モジュール
int count = 0;//pin番号ではない
//ブザー
const int buzzer = 6;
int melo = 500;
//関数のID
int method_id;

//-----初期化-----
void setup() {
  // モータ
  //右
  pinMode(motorR1, OUTPUT);
  pinMode(motorR2, OUTPUT);
  //左
  pinMode(motorL1, OUTPUT);
  pinMode(motorL2, OUTPUT);
  //IRモジュール
  irrecv.enableIRIn();
  //フォトインタラプタ
  attachInterrupt(1, photo_changingL, CHANGE);
  attachInterrupt(0, photo_changingR, CHANGE);
  //ブザー
  pinMode(buzzer, OUTPUT);
  
  Serial.begin(9600);
}
//----------------

//--割り込み関数--
void photo_changingL() {                    //フォトインタラプタ左
  changeL++;
  Serial.print(changeL);
  Serial.println("times_changed_L");
  if (changeL > changeR) {
    speedL--;
    speedR = 255;//後で直進する比を実測
  }
}

void photo_changingR() {                    //フォトインタラプタ右
  changeR++;
  Serial.print(changeR);
  Serial.println("times_changed_R");
  if (changeR > changeL) {
    speedR--;
    speedL = 255;//後で直進する比を実測
  }
}
//----------------
//-メインで使う関数-

void cw(int melo){                            //トンツートンツー・ー・ー
  int melo_speed = 1;
  digitalWrite(buzzer,HIGH);
  delay(melo * melo_speed);
  digitalWrite(buzzer,LOW);
  delay(100 * melo_speed);
}

void CBuzzer(bool b) {                       //ブザー
  Serial.print("\t\t\tenterd CBuzzer"+b);
  Serial.println(")");
  int count = 0;
//  while (count < 8) {
//    //tone(buzzer, 131, melo); //ド
//    digitalWrite(buzzer,HIGH);
//    delay(melo);
//    digitalWrite(buzzer,LOW);
//    delay(melo);
//    count++;
//  }
  if(b){
    cw(100);cw(100);cw(100);//ブザー1
    delay(200);
    cw(300);cw(300);cw(300);
    delay(200);
    cw(100);cw(100);cw(100);
    b = false;
  }else{
    cw(300);cw(300);cw(100);//ブザー2
    delay(200);
    cw(300);cw(300);cw(300);
    b = true;
  }
  Serial.print("\t\t\texit CBuzzer"+b);
  Serial.println(")");
}

void barricade_check() {                    //障害物検知
  int distance;
  distance = (6762 / (analogRead(0) - 9)) - 4;
  Serial.print("\t\t"+distance);
  Serial.println("cm");

  if (distance < 80) 
    count++;
  else
    count = 0;
      
  if(count > 10) {
    Serial.println("\t\t!!!STOP!!!");
    int tmpL = changeL;
    int tmpR = changeR;
    brake();
    Serial.println("\t\tenter CBuzzer(true)");
    CBuzzer(true);//ブザー1
    Serial.println("\t\texited CBuzzer(true)");
    while (1){
      distance = (6762 / (analogRead(0) - 9)) - 4;
      Serial.print("\t\t\t"+distance);
      Serial.println("cm");
      if ((6762 / (distance - 9)) - 4 > 80)
        count--;
      if(count < 1)
        break;
    }
    Serial.println("\t\t!!!RESTART!!!");
    Serial.println("\t\tenter CBuzzer(false)");    
    CBuzzer(false);//ブザー2
    Serial.println("\t\texited CBuzzer(false)");
    changeL = tmpL;
    changeR = tmpR;
    if (method_id == 0)
      ahead();
    else
      ahead_d();
  }
}

void motor_speed() {                        //モータ速度呼び出し
  analogWrite(PWM_motR, speedL);
  analogWrite(PWM_motL, speedR);
}

void circle(int angle) {                    //回転
  Serial.print("\t\t\tentered circle("+angle);
  Serial.println(")");
  if (angle == 180)
    while (1) {
      motor_speed();
      if (changeL > 19 || changeR > 19)
        break;
    }
  else if (angle == 135)
    while (1) {
      motor_speed();
      if (changeL > 14 || changeR > 14)
        break;
    }
  else
    while (1) {
      motor_speed();
      if (changeL > 9 || changeR > 9)
        break;
    }
  changeL = changeR = 0;
  Serial.print("\t\t\texit circle("+angle);
  Serial.println(")");
}

void ahead() {                              //前進
  Serial.println("\t\tentered ahead()");
  method_id = 0;
  digitalWrite(motorR1, HIGH);
  digitalWrite(motorR2, LOW);
  analogWrite(PWM_motR, speedL);
  digitalWrite(motorL1, LOW);
  digitalWrite(motorL2, HIGH);
  analogWrite(PWM_motL, speedR);
  Serial.println("\t\tenter while(1)");
  while (1) {
    motor_speed();
    barricade_check();
    if (changeL > 682 || changeR > 682)
      break;
  }
  Serial.println("\t\texited while(1)");
  changeL = changeR = 0;
  Serial.println("\t\texit ahead())");
}


void ahead_d() {                            //前進斜め
  Serial.println("\t\tentered ahead_d()");
  method_id = 1;
  digitalWrite(motorR1, HIGH);
  digitalWrite(motorR2, LOW);
  analogWrite(PWM_motR, speedL);
  digitalWrite(motorL1, LOW);
  digitalWrite(motorL2, HIGH);
  analogWrite(PWM_motL, speedR);
  Serial.println("\t\tenter while(1)");
  while (1) {
    motor_speed();
    barricade_check();
    if (changeL > 964 || changeR > 964)
      break;
  }
  Serial.println("\t\texited while(1)");
  changeL = changeR = 0;
  Serial.println("\t\texit ahead_d())");
}

void astern(int changechange) {             //後進
  Serial.print("\t\tentered astern("+changechange);
  Serial.println(")");
  digitalWrite(motorR1, LOW);
  digitalWrite(motorR2, HIGH);
  analogWrite(PWM_motR, speedL);
  digitalWrite(motorL1, HIGH);
  digitalWrite(motorL2, LOW);
  analogWrite(PWM_motL, speedR);
  Serial.println("\t\tenter while(1)");
  while (1) {
    motor_speed();
    if (changeL >= changechange || changeR >= changechange){
      Serial.println("\t\t\texited while(1)");
      break;
    }
  }
  Serial.println("\t\texited while(1)");
  changeL = changeR = 0;
  Serial.print("\t\texit astern("+changechange);
  Serial.println(")");
}

void brake() {                              //ブレーキ
  Serial.println("\tentered brake()");
  digitalWrite(motorR1, LOW);
  digitalWrite(motorR2, LOW);
  digitalWrite(motorL1, LOW);
  digitalWrite(motorL2, LOW);
  int changechange = changeL;
  if (changechange < changeR)
    changechange = changeR;
  changeL = changeR = 0;
  Serial.print("\tenter astern("+changechange);
  Serial.println(")");
  astern(changechange);
  Serial.print("\texited astern("+changechange);
  Serial.println(")");
  digitalWrite(motorR1, LOW);
  digitalWrite(motorR2, LOW);
  digitalWrite(motorL1, LOW);
  digitalWrite(motorL2, LOW);
  Serial.println("\texit brake()");
}

void left(int left) {                       //左旋回
  Serial.print("\t\tentered left("+left);
  Serial.println(")");
  digitalWrite(motorR1, LOW);
  digitalWrite(motorR2, HIGH);
  analogWrite(PWM_motR, speedL);
  digitalWrite(motorL1, LOW);
  digitalWrite(motorL2, HIGH);
  analogWrite(PWM_motL, speedR);
  Serial.print("\t\tenter circle("+left);
  Serial.println(")");
  circle(left);
  Serial.print("\t\texited circle("+left);
  Serial.println(")");
  Serial.print("\t\texit circle("+left);
  Serial.println(")");
}
void right(int right) {                     //右旋回
  Serial.print("\t\tentered right("+right);
  Serial.println(")");
  digitalWrite(motorR1, HIGH);
  digitalWrite(motorR2, LOW);
  analogWrite(PWM_motR, speedL);
  digitalWrite(motorL1, HIGH);
  digitalWrite(motorL2, LOW);
  analogWrite(PWM_motL, speedR);
  Serial.print("\t\tenter circle("+right);
  Serial.println(")");
  circle(right);
  Serial.print("\t\texited circle("+right);
  Serial.println(")");
  Serial.print("\t\texit circle("+right);
  Serial.println(")");
}

void receiveIR(String receiveValue,int *id){    //受信
const int angle1 = 90;
const int angle2 = 135;
const int angle3 = 180;
 if (receiveValue == "3887053538") {
      Serial.println("\tReceived_P1");
      if (id[0] == 2) {
        if (id[1] == 1)
          left(angle3);
        if (id[1] == 3)
          left(angle1);
        if (id[1] == 4)
          left(angle2);
        ahead();
        id[1] = 2;
      } else if (id[0] == 3) {
        if (id[1] == 1)
          left(angle3);
        if (id[1] == 2)
          right(angle2);
        if (id[1] == 4)
          left(angle2);
        ahead_d();
        id[1] = 3;
      } else if (id[0] == 4) {
        if (id[1] == 1)
          left(angle3);
        if (id[1] == 2)
          right(angle2);
        if (id[1] == 3)
          right(angle1);
        ahead();
        id[1] = 4;
      }
      id[0] = 1;
    } else if (receiveValue == "1513342804") {
      Serial.println("\tReceived_P2");
      if (id[0] == 1) {
        if (id[1] == 2)
          left(angle3);
        Serial.println("\tangle180");
        if (id[1] == 3)
          right(angle2);
        if (id[1] == 4)
          right(angle1);
        ahead();
        Serial.println("\tahead");
        id[1] = 2;
      } else if (id[0] == 3) {
        if (id[1] == 1)
          left(angle2);
        if (id[1] == 2)
          left(angle3);
        if (id[1] == 4)
          left(angle1);
        ahead_d();
        id[1] = 3;
      } else if (id[0] == 4) {
        if (id[1] == 1)
          left(angle2);
        if (id[1] == 2)
          left(angle3);
        if (id[1] == 3)
          right(angle2);
        ahead();
        id[1] = 4;
      }
      id[0] = 2;
    } else if (receiveValue == "4092259158") {
      Serial.println("\tReceived_P3");
      if (id[0] == 1) {
        if (id[1] == 2)
          left(angle2);
        if (id[1] == 3)
          left(angle3);
        if (id[1] == 4)
          right(angle2);
        ahead_d();
        id[1] = 1;
      } else if (id[0] == 2) {
        if (id[1] == 1)
          right(angle1);
        if (id[1] == 3)
          left(angle3);
        if (id[1] == 4)
          right(angle2);
        ahead();
        id[1] = 2;
      } else if (id[0]== 4) {
        if (id[1] == 1)
          left(angle1);
        if (id[1] == 2)
          left(angle2);
        if (id[1] == 3)
          left(angle3);
        ahead();
        id[1] = 4;
      }
      id[0]= 3;
    } else if (receiveValue == "869376052") {
      Serial.println("\tReceived_P4");
      if (id[0] == 1) {
        if (id[1] == 2)
          left(angle1);  //or  right(90);
        if (id[1] == 3)
          left(angle2);  //or  right(45);
        if (id[1] == 4)
          left(angle3); //or no_rotate
        ahead();  //or  astern();
        id[1] = 1;
      } else if (id[0] == 2) {
        if (id[1] == 1)
          right(angle2);         //or left(45);
        if (id[1] == 3)
          left(angle2);          //or right(45);
        if (id[1] == 4)
          left(angle3);          //or no_rotate
        ahead_d();               //or  astern_();
        id[1] = 2;
      } else if (id[0] == 3) {
        if (id[1] == 1)
          right(angle2);         //or left(45);
        if (id[1] == 2)
          left(angle1);          //or right(90);
        if (id[1] == 4)
          left(angle3);          //or no_rotate
        ahead();                 //or astern();
        id[1] = 3;
      }
      id[0] = 4;
    }

}

//----------------
//-----メイン-----
int id[] = {1,2};

void loop() {

  if (irrecv.decode(&results)) {
    Serial.println(results.value, HEX);
    Serial.println(results.value, DEC);
    Serial.println(results.value, BIN);

    String receiveValue = "";
    receiveValue = (String)results.value;
    //Received_P :　これから向かう点
    //id[0]          :　今いる点
    //id[1]          :　前にいた点
    
    receiveIR(receiveValue,id);

    Serial.println("brake() ni hairimasu.");
    brake();
    Serial.println("dousa syuuryou desu.\n");
    Serial.print("ima P");
    Serial.print(id[1]);
    Serial.println("ni irun.");
    Serial.print("P");
    Serial.print(id[0]);
    Serial.println("kara kitan.");
    Serial.println("----------");

    irrecv.resume();
  }
  delay(1000);
  Serial.print("speedL:");
  Serial.print(speedL);
  Serial.print(" speedR:");
  Serial.println(speedR);
}
//---------------
