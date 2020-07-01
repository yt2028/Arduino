#include <PID_v1.h>
#include <LMotorController.h>
#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"

#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
 #include "Wire.h"
#endif

#define MIN_ABS_SPEED 30

MPU6050 mpu;

// MPU control/status vars
bool dmpReady = false; // DMPの初期化に成功したらtrueに設定
uint8_t mpuIntStatus; // MPUからの割り込みステータス
uint8_t devStatus; // デバイス処理時の戻り値(0：成功、0以外：失敗)
uint16_t packetSize; // expected DMPパケットのサイズ(デフォルトは42バイト)
uint16_t fifoCount; // FIFO内に現在ある全バイト数
uint8_t fifoBuffer[64]; // FIFOストレージバッファ

// orientation/motion vars
Quaternion q; // [w, x, y, z] クォータニオン
VectorFloat gravity; // [x, y, z] gravity vector
float ypr[3]; // [yaw, pitch, roll] yaw/pitch/roll container and gravity vector

//PID
double originalSetpoint = 172.5; //目標値(維持してほしい値)
;
double setpoint = originalSetpoint;
double movingAngleOffset = 0.1;
double input, output;

//set gain
double Kp = 80;   
double Kd = 2.0;
double Ki = 100;
PID pid(&input, &output, &setpoint, Kp, Ki, Kd, DIRECT);

double motorSpeedFactorLeft = 0.6;
double motorSpeedFactorRight = 0.5;

//MOTOR CONTROLLER(L298N)
int ENA = 5;
int IN1 = 6;
int IN2 = 7;
int IN3 = 9;
int IN4 = 8;
int ENB = 10;
LMotorController motorController(ENA, IN1, IN2, ENB, IN3, IN4, motorSpeedFactorLeft, motorSpeedFactorRight);

volatile bool mpuInterrupt = false; // attachInterruptで呼び出される関数dmpDataReady()で用いる値の変わる変数はvolatileをつける．
void dmpDataReady()
{
 mpuInterrupt = true;
}


void setup()
{
 // join I2C bus (I2Cdev library doesn't do this automatically)
 #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
 Wire.begin();
 TWBR = 24; // 400kHz I2C clock (200kHz if CPU is 8MHz)
 #elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
 //optionでI2Cの通信規格のほかにFastwireが用意されている．
 Fastwire::setup(400, true);
 #endif

 mpu.initialize();

 devStatus = mpu.dmpInitialize(); //MPU-6050のDMPを初期化．0:success,1:main binary block loading failed, 2: configuration bleck loading failed

 // 加速度と角速度のオフセットを設定して、値のズレを修正できます。
    mpu.setXGyroOffset(142);
    mpu.setYGyroOffset(61);
    mpu.setZGyroOffset(28);
    
    mpu.setXAccelOffset(-1600);
    mpu.setYAccelOffset(-360);
    mpu.setZAccelOffset(760);

 // make sure it worked (returns 0 if so)
 
 if (devStatus == 0)
 {
 // MPU-6050のDMPの有効/無効を切り替えるメソッド,trueを渡せば有効
 mpu.setDMPEnabled(true);

 // MPU-6050からの割り込みを有効にする
 attachInterrupt(0, dmpDataReady, RISING); // RISINGはLOWかたらHIGHに変化したときにdmpDataReadyを呼びだす．
 mpuIntStatus = mpu.getIntStatus();//現在の割り込み状態を取得

 // set our DMP Ready flag so the main loop() function knows it's okay to use it
 dmpReady = true; //初期化完了

 // MPU-6050のDMPのFIFOパケットのサイズを取得,
 //DMPの演算結果は、このパケットサイズ単位でFIFOに格納される
 packetSize = mpu.dmpGetFIFOPacketSize();
 
 //PIDの設定．
 pid.SetMode(AUTOMATIC);
 pid.SetSampleTime(10);//sampling time [ms]
 pid.SetOutputLimits(-255, 255); //Arduinoの出力範囲 (min,max)
 }
 else
 {
 // ERROR!
 // 1 = initial memory load failed
 // 2 = DMP configuration updates failed
 // (if it's going to break, usually the code will be 1)
 Serial.print(F("DMP Initialization failed (code "));
 Serial.print(devStatus);
 Serial.println(F(")"));
 }
}


void loop()
{
 // if programming failed, don't try to do anything
 
 if (!dmpReady) return;

 // wait for MPU interrupt or extra packet(s) available
 // DMPからのデータを待つ
 while (!mpuInterrupt && fifoCount < packetSize) //MPU-6050から割り込み処理があるか、FIFOに格納されているデータのサイズがパケットサイズを上回るのを待つ．
 {
  // 処理するデータを待ちながら、他の処理をする
 // no mpu data - performing PID calculations and output to motors 
 // void loop()の{}の中に書くことで、ループが回るたびに一回だけフィードバックする
 pid.Compute();
 motorController.move(output, MIN_ABS_SPEED);
 
 }

 // 割り込みフラグをリセットして、現在の割り込み状態を取得
 mpuInterrupt = false;
 mpuIntStatus = mpu.getIntStatus();

 // 現在のFIFOバッファに格納されているデータ数を取得
 fifoCount = mpu.getFIFOCount();

 // FIFOバッファがオーバーフローしていたら、FIFOバッファをリセットして、処理をやり直す．
 if ((mpuIntStatus & 0x10) || fifoCount == 1024)
 {
 // FIFOバッファをリセット
 mpu.resetFIFO();
 Serial.println(F("FIFO overflow!"));

 // otherwise, check for DMP data ready interrupt (this should happen frequently)
 }
 else if (mpuIntStatus & 0x02)
 {
 // wait for correct available data length, should be a VERY short wait
 while (fifoCount < packetSize) fifoCount = mpu.getFIFOCount();
//FIFOに格納されているデータのサイズがパケットサイズを上回るのを待つ．
//その後、FIFOからパケットサイズ分のデータを取得する．
 // read a packet from FIFO
 mpu.getFIFOBytes(fifoBuffer, packetSize);
 
 // track FIFO count here in case there is > 1 packet available
 // (this lets us immediately read more without waiting for an interrupt)
 fifoCount -= packetSize;

 mpu.dmpGetQuaternion(&q, fifoBuffer);
 mpu.dmpGetGravity(&gravity, &q);
 mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);
 input = ypr[1] * 180/M_PI + 180; //ypr[1] means that Pitch(y-axis)
 Serial.print(ypr[1]);Serial.println(',');
 }
}
