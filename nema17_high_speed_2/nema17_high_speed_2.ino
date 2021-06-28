const int STEP = 9;//for step signal
const int DIR = 8;//for direction
const int M1 = 10;
const int M2 = 11;
const int M3 = 12;
volatile unsigned long time;
int Ts = 5; //等速時間

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(STEP, OUTPUT);
  pinMode(DIR, OUTPUT);
  pinMode(M1,OUTPUT);
  pinMode(M2,OUTPUT);
  pinMode(M3,OUTPUT);
  digitalWrite(DIR,HIGH);
  digitalWrite(M1,LOW);
  digitalWrite(M2,LOW);
  digitalWrite(M3,LOW);
  
}  

int x = 3000;
void loop(){
  while(1){ 
    digitalWrite(STEP, HIGH);
    delayMicroseconds(x*9/10);
    digitalWrite(STEP, LOW);
    delayMicroseconds(x*1/10);
    x -= 1; // accel
    if(x < 110){
      time = millis();
      while(1){
        digitalWrite(STEP, HIGH);
        delayMicroseconds(x*9/10);
        digitalWrite(STEP, LOW);
        delayMicroseconds(x*1/10);
        if((millis()-time)>= Ts * 1000){
          x += 1; // deaccel
        }
        if(x > 3000){
          Serial.println("OUT");
          break;
        }
      }
    }
    }
  }
 
