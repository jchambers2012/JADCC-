#include <Servo.h>
Servo myservo;
#define readingNumbers 10
#define readingNumberSensor 9
#define MODE_AUTO     2
#define MODE_OPEN     1
#define MODE_CLOSE    0
#define DOOR_OPENED   1
#define DOOR_CLOSED   0
byte mode = MODE_AUTO;  //Current mode of the system to know how the servo should be set
byte door = DOOR_CLOSED;  //Current mode of the system to know how the servo should be set
byte door_current = DOOR_CLOSED;  //Current mode of the system to know how the servo should be set

byte readIndex = 0;
byte readingGood = 0;
int readTotal=0;
int readAvg[readingNumberSensor];
byte servo_low=0;
byte servo_high=0;
byte servo_current=0;
byte servo_to=0;
int readSensor[readingNumberSensor][readingNumbers];
#define NIC_SENSOR_I  0
#define SENSOR_LOW_I  1
#define SENSOR_HIGH_I 2
#define SERVO_LOW_I   3
#define SERVO_HIGH_I  4
#define SET_DELAY_I   5
#define SWITCH_1_I    6
#define SWITCH_2_I    7
#define SWITCH_3_I    8
#define BLOWER_VERSION "0.11 BETA"

#define  NIC_SENSOR    A0 
// non invasive current sensor 
#define  SENSOR_LOW    A1 
// SET 1
#define  SENSOR_HIGH   A2 
// SET 2
#define  SERVO_LOW     A3 
// SET 3
#define  SERVO_HIGH    A4 
// SET 4
#define  SET_DELAY     A5 
// Delay Off Timer
#define  SERVO_PIN     2 
// 
#define  DOOR_PIN      3 
// 
#define  S1            4 
// 
#define  S2            5 
// 
#define  S3            6 
// 
char buf[200];

//Function to see of the Analog servo location has chagend enought to update the servo
bool servoEqual(int a, int b)
{
  int difference;
  difference = a - b;
  if (difference > 10) return true;
  if (difference < -10 ) return true;
  return false;
}


void read_sensors() {
    readSensor[NIC_SENSOR_I ][readIndex] = analogRead(NIC_SENSOR);
    readSensor[SENSOR_LOW_I ][readIndex] = analogRead(SENSOR_LOW);
    readSensor[SENSOR_HIGH_I][readIndex] = analogRead(SENSOR_HIGH);
    readSensor[SERVO_LOW_I  ][readIndex] = analogRead(SERVO_LOW);
    readSensor[SERVO_HIGH_I ][readIndex] = analogRead(SERVO_HIGH);
    readSensor[SET_DELAY_I  ][readIndex] = analogRead(SET_DELAY);
    readSensor[SWITCH_1_I   ][readIndex] = digitalRead(S1);
    readSensor[SWITCH_2_I   ][readIndex] = digitalRead(S2);
    readSensor[SWITCH_3_I   ][readIndex] = digitalRead(S3);
    if (readIndex>=readingNumbers)
    {
      readIndex=0;
    }else{
      readIndex = readIndex + 1;
    }
    for (byte i = 0; i < readingNumberSensor ; i++) {
      readTotal = 0;
      for (byte thisReading = 0; thisReading < readingNumbers ; thisReading++) {
        readTotal = readTotal + readSensor[i][thisReading];
      }
      if(readTotal>0)
      {
        readAvg[i] = (readTotal / readingNumbers);
      }else{
        readAvg[i] = 0;
      }
    }
    servo_low =  map(readAvg[SERVO_LOW_I ],0,1024,0,180);
    servo_high = map(readAvg[SERVO_HIGH_I],0,1024,0,180);
    readAvg[SWITCH_1_I] = constrain(readAvg[SWITCH_1_I],0,1);
    readAvg[SWITCH_2_I] = constrain(readAvg[SWITCH_2_I],0,1);
    readAvg[SWITCH_3_I] = constrain(readAvg[SWITCH_3_I],0,1);
 
}
void setup() {
  Serial.begin(115200);
  pinMode(DOOR_PIN, OUTPUT);
  pinMode(S1, INPUT);
  pinMode(S2, INPUT);
  pinMode(S3, INPUT);

  Serial.print ( F("Blower Controller Version: ") );
  Serial.println ( BLOWER_VERSION );
  Serial.println ( F("Created By Jason Chambers") );
  Serial.println ( F("Warning - This device does not contain any Emergency Control or Fail-Safe Functions. This device should not be used as a life safety system.") );

  delay(100);
  //Pre fill the sensor buffer so the servos have a good setting to move too
  for (byte x = 0; x < (readingNumbers*2) ; x++) {
    read_sensors();
    for (byte i = 0; i < readingNumberSensor ; i++) {
      readTotal = readTotal + readAvg[i];
    }
  }
  //Create a random number to delay/prevent all the servos from turning on at the smae time

  Serial.print("Random delay of ");
  randomSeed(readTotal);
  readTotal = random(1, 10);
  readTotal = readTotal * random(100, 1000);
  Serial.print(readTotal);
  Serial.print(" ms before starting servo");
  delay(readTotal);
  myservo.attach(SERVO_PIN);
  myservo.write(servo_low);
  servo_current = servo_low;
  servo_to = servo_low;
  delay(50000);

}
void loop() {
  read_sensors();
  if(1)
  {
    sprintf(buf
            , " NIC: %04d SEN_LOW: %04d SEN_HIGH: %04d SER_LOW: %04d SER_HIGH: %04d DELAY: %04d SW_1: %04d SW_2: %04d SW_3: %04d servo_current: %04d  servo_to: %04d "
            , readAvg[NIC_SENSOR_I]
            , readAvg[SENSOR_LOW_I]
            , readAvg[SENSOR_HIGH_I]
            , readAvg[SERVO_LOW_I]
            , readAvg[SERVO_HIGH_I]
            , readAvg[SET_DELAY_I]
            , readAvg[SWITCH_1_I]
            , readAvg[SWITCH_2_I]
            , readAvg[SWITCH_3_I]
            , servo_current
            , servo_to
            );

  }
  Serial.print(buf);
  //Check the Switches for what mode we are in:
  if(readAvg[SWITCH_1_I] == 0)
  {
    //Force Open
    Serial.print(F(" MODE: OPEN "));
    mode = MODE_OPEN;
    servo_to = servo_high;
    door = DOOR_OPENED;
  }else if(readAvg[SWITCH_2_I] == 0)
  {
    //Auto Mode
    Serial.print(F(" MODE: AUTO "));
    if(mode != MODE_AUTO)
    {
      servo_to = servo_low;
      digitalWrite(DOOR_PIN, LOW);
    }
    mode = MODE_AUTO;  
    //Check the NIC Sensor vs the set limits:
    if (readAvg[NIC_SENSOR_I] >= readAvg[SENSOR_HIGH_I])
    {
      servo_to = servo_high;
      door = DOOR_OPENED;
    }else if (readAvg[NIC_SENSOR_I] <= readAvg[SENSOR_LOW_I])
    {
      servo_to = servo_low;
      door = DOOR_CLOSED;
    }

  }else if(readAvg[SWITCH_3_I] == 0)
  {
    //Force Closed
    Serial.print(F(" MODE: CLOSED "));
    mode = MODE_CLOSE;
    servo_to = servo_low;
    door = DOOR_CLOSED;
  }
  

  if(door !=door_current)
  {
    //To Do Add delay logic
    digitalWrite(DOOR_PIN, door);
    door = door_current;
  }
  //Check to see if the servo needs to be moved, if so move it 1deg each loop
  if(servoEqual(servo_to, servo_current)==true)
  {
    if(servo_current<servo_to)
    {
        servo_current = servo_current + 1;
        myservo.write(servo_current);
    }else{
        servo_current = servo_current - 1;
        myservo.write(servo_current);
    } 
  }
  Serial.println();
  
}
