/*
  stepper motor reference
  http://howtomechatronics.com/tutorials/arduino/how-to-control-stepper-motor-with-a4988-driver-and-arduino/

  receiver reference
  http://www.instructables.com/id/RF-315433-MHz-Transmitter-receiver-Module-and-Ardu/

  force sensor (scale) reference
  https://github.com/bogde/HX711
*/

#include "HX711.h"        // for force sensor
#include <SPI.h>
#include "RF24.h"         // for radio controls

// LEDS
#define READY_LED_PIN 8
#define MOTOR_TURNING_LED_PIN 9

// Stepper motor config
#define STEPS_PER_ROTATION 200
#define STEP 3
#define DIR 4
#define STEPPER_ENABLE_PIN 10

// scale is not quite linear :( so this constant
// was used to get us correct values at
// beaker's weight
#define SCALE_CALIBRATION -680.f

// Beaker weight in grams
#define BEAKER_WEIGHT 1973

// Init global scale object
// HX711.DOUT - pin #A1
// HX711.PD_SCK - pin #A0
HX711 scale(A1, A0);

// PINS 6, 7 for radio ce / csn
RF24 radio(6,7);

void set_rotation_to_pull_up(){
  digitalWrite(DIR,HIGH);
}

void set_rotation_to_give_slack(){
  digitalWrite(DIR,LOW);
}

void rotate(int totalSteps) {
  digitalWrite(MOTOR_TURNING_LED_PIN, HIGH);
  for(int i=0;i<totalSteps;++i) {
    digitalWrite(STEP,HIGH);
    delayMicroseconds(1000);
    digitalWrite(STEP,LOW);
    delayMicroseconds(1000);
  }
  digitalWrite(MOTOR_TURNING_LED_PIN, LOW);
}

void do_full_rotation() {
  Serial.print("[full rotation]");
  rotate(STEPS_PER_ROTATION);
}

void do_quarter_rotation() {
  Serial.print("[quarter rotation]");
  rotate(STEPS_PER_ROTATION/4);
}

void do_eigth_rotation() {
  Serial.print("[eigth rotation]");
  rotate(STEPS_PER_ROTATION/8);
}

void lift_till_hanging() {
  Serial.println("\nTime to lift till hanging...");

  float lifting = 0;

  set_rotation_to_pull_up();
  lifting = scale.get_units(10);
  while(lifting < 0.95 * BEAKER_WEIGHT){
    do_quarter_rotation();
    Serial.print(lifting);
    lifting = scale.get_units(10);
  }
}

void lower_till_touching() {
  Serial.println("\nTime to lower till touching...");

  float lifting = 0;

  set_rotation_to_give_slack();
  lifting = scale.get_units(10);
  while(lifting > 0.25 * BEAKER_WEIGHT){
    Serial.println(lifting);
    do_eigth_rotation();
    delay(250);
    lifting = scale.get_units(10);
  }
}

void enable_motor() {
  digitalWrite(STEPPER_ENABLE_PIN,LOW);
}

void disable_motor() {
  digitalWrite(STEPPER_ENABLE_PIN,HIGH);
}

void give_slack() {
  Serial.println("\nTime to give slack...");
  set_rotation_to_give_slack();
  enable_motor();
  do_full_rotation();
  do_full_rotation();
  disable_motor();
}

void reset_robot() {
  enable_motor();
  lift_till_hanging();
  lower_till_touching();
  disable_motor();
}

void sendRadioResponse() {
  radio.stopListening();
  Serial.println("Send to robot: K");
  for(int i=0;i<10;i++){
    if(radio.write( "K", 1)){
      Serial.println("Sent success response!");
      radio.startListening();
      return;
    }
    Serial.println("failed, trying again...");
    delay(200);
  }
  radio.startListening();
  Serial.println("Failed 10 times. Giving up :(");
}

void setup() {
  // LED setup
  pinMode(READY_LED_PIN, OUTPUT);
  pinMode(MOTOR_TURNING_LED_PIN, OUTPUT);
  digitalWrite(READY_LED_PIN, LOW);
  digitalWrite(MOTOR_TURNING_LED_PIN, LOW);

  Serial.begin(115200);

  // radio setup
  radio.begin();
  radio.setRetries(15,15);
  radio.setPayloadSize(4);
  radio.setPALevel(RF24_PA_LOW);

  byte addresses[][6] = {"1Node","2Node"};
  radio.openWritingPipe(addresses[1]);
  radio.openReadingPipe(1,addresses[0]);

  radio.startListening();

  // stepper motor setup
  Serial.println("\n\nSetting up stepper...");
  pinMode(STEP,OUTPUT);
  pinMode(DIR,OUTPUT);
  pinMode(STEPPER_ENABLE_PIN,OUTPUT);
  disable_motor();

  // scale setup
  Serial.println("Setting up scale...");
  scale.set_scale(SCALE_CALIBRATION);
  // MAKE SURE nothing pulling on winch during setup!!
  scale.tare(); // reset the scale to 0.

  Serial.println("Done with setup. Ready to serve...");
  digitalWrite(READY_LED_PIN, HIGH);
}

void loop() {
  int msg[1];

  if( radio.available() ){
    radio.read(msg, 1);
    char command = msg[0];

    if(command =='L') // Lift request
    {
      Serial.print("Received command: ");
      Serial.println(command);
      reset_robot(); // blocking
      sendRadioResponse();
    } else if(command =='S') // Slack request
    {
      Serial.print("Received command: ");
      Serial.println(command);
      give_slack(); // non-blocking
    } else {
      digitalWrite(LED_BUILTIN,0);
    }
  }
}
