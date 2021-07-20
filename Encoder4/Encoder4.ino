/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
|     Encoder4.ino                                                     |
|     ロータリーエンコーダ4逓倍                                            |
|      PIN2-A相（デジタル入力割り込み）                                    |
|      PIN3-B相（デジタル入力割り込み）                                    |                                              |
|      使用するエンコーダ分解能 500 ppr to 2000 cpr                        |
\~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#define PIN_ENCA 2
#define PIN_ENCB 3
#define PIN_IN1 7
#define PIN_IN2 8
#define PIN_VREF 9
//#define PIN_STB 10

const float REF = 10800;                 // 目標角度[deg.]
const float Ts = 0.01;                  // サンプリング時間 [s]
const float VREF = 6;                  // 電圧の最大値（電源電圧の値）
const int PWM_MAX = 255;
const int PWM_MIN = 0;
volatile float u = 0.0;                 // 制御入力 [V]
volatile float t = 0.0;                 // 計測用時間変数 [s]
volatile int  encoder_cnt = 0;          // エンコーダカウント用変数 
volatile float n0 = 0.0;                // 回転数(前)
volatile float n = 0.0;                 // 回転数(後)
volatile float dn;                      // 回転数の差 n - n0
volatile float ang = 0;                 // 角度 [deg.]
volatile float ang_vel = 0;             // 角速度 [rps]
volatile unsigned long time;            // [msec.]
volatile uint8_t prev = 0;

void setup() {
  pinMode(PIN_IN1,OUTPUT);
  pinMode(PIN_IN2,OUTPUT);      
  pinMode(PIN_VREF,OUTPUT); 
  pinMode(PIN_ENCA,INPUT);      // A相用信号入力　
  pinMode(PIN_ENCB,INPUT);      // B相用信号入力　
  digitalWrite(PIN_ENCA,HIGH);
  digitalWrite(PIN_ENCB,HIGH);
  /* A,B相の立ち上がり/下がりエッジでupdateEncoder関数を呼び出し  */
  attachInterrupt(0, updateEncoder, CHANGE); // 0 ==> pin 2
  attachInterrupt(1, updateEncoder, CHANGE); // 1 ==> pin 3
  Serial.begin(115200);
}

void loop() {
  time = millis();

  // Pythonからのデータ取得
  if(Serial.available() > 0){
    String buf = Serial.readStringUntil('e');
    if(buf.length() > 0) {
      u = atof(strtok(buf.c_str(),","));
    }
  }

  u = u * PWM_MAX / VREF;
  int mag = constrain(abs(u),PWM_MIN,PWM_MAX);
  //digitalWrite(PIN_STB,HIGH);
  if(u >= 0){
    analogWrite(PIN_VREF,mag);
    digitalWrite(PIN_IN1,HIGH);
    digitalWrite(PIN_IN2,LOW);
  }else{
    analogWrite(PIN_VREF,mag);
    digitalWrite(PIN_IN1,LOW);
    digitalWrite(PIN_IN2,HIGH);
  }
  //n = float(encoder_cnt) / 2000; 
  //dn = n - n0; // サンプリング中の回転数の差 
  ang = float(encoder_cnt) * 0.18; // 0.18 = 360/2000 [deg] 1パルスあたりの回転角
  //ang_vel = dn / Ts;
  //n0 = n;
  //Serial.println(ang_vel);       // 角速度
  while(1){
    if((millis()- time) > Ts * 1000){
      break;
    }
  }
  t = t + Ts;
  // Pythonへデータを送る
  String str = String(t) + " " + String(ang,3)+ " " + String(u*100/PWM_MAX,3) + " ";
  Serial.println(str);
}

/* 割り込み関数 */
void updateEncoder(){
  uint8_t a = digitalRead(PIN_ENCA);
  uint8_t b = digitalRead(PIN_ENCB);
  uint8_t ab = (a << 1)| b;
  uint8_t encoded = (prev << 2) | ab;

  if(encoded == 0b1101 || encoded == 0b0100 || encoded == 0b0010 || encoded == 0b1011){
    encoder_cnt ++;
  } else if(encoded == 0b1110 || encoded == 0b0111 || encoded == 0b0001 || encoded == 0b1000){
    encoder_cnt --;
  }
  prev = ab;
}
               
  
