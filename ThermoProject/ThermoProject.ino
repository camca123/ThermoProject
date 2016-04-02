#include <Servo.h>
#include <PID_v1.h>

#define THERM_PIN 0
#define JAG_PIN 3
#define Kp 5
#define Ki 0
#define Kd 0

//float out_lookup[] = {150,145,140,135,130,125,120,115,110,105,100,95,90,85,80,75,70,65,60,55,50,45,40,35,30,25,20,15,10,5,0,-5,-10,-15,-20,-25,-30,-35,-40};
//float in_lookup[] = {1618,1827,2070,2350,2676,3057,3503,4026,4643,5372,6238,7269,8504,9988,11780,13951,16597,19835,23820,28749,34879,42548,52200,64422,80003,100000,125851,159522,203723,262229,340346,445602,588793,785573,1058901,1442861,1988706,2774565,3921252};

const float _100kHoneywellLookup[][2] = { // Resistance, Temperature (deg C)
  {1618, 150},
  {1827, 145},
  {2070, 140},
  {2350, 135},
  {2676, 130},
  {3057, 125},
  {3503, 120},
  {4026, 115},
  {4643, 110},
  {5372, 105},
  {6238, 100},
  {7269, 95},
  {8504, 90},
  {9988, 85},
  {11780, 80},
  {13951, 75},
  {16597, 70},
  {19835, 65},
  {23820, 60},
  {28749, 55},
  {34879, 50},
  {42548, 45},
  {52200, 40},
  {64422, 35},
  {80003, 30},
  {100000, 25},
  {125851, 20},
  {159522, 15},
  {203723, 10},
  {262229, 5},
  {340346, 0},
  {445602, -5},
  {588793, -10},
  {785573, -15},
  {1058901, -20},
  {1442861, -25},
  {1988706, -30},
  {2774565, -35},
  {3921252, -40}
};

Servo jag;

const int ARR_LEN = 39;

const double vref = 5.0; //Reference voltage
const double pullup1 = 99.7*1000; //Pullup resistor

double temp = 25; //Thermistor temp
double PIDOutput = 0;
double setpoint = 25;
PID tempController(&temp, &PIDOutput, &setpoint, Kp, Ki, Kd, DIRECT);


void setup() {
  jag.attach(JAG_PIN);  //Jag is connected to a pin
  tempController.SetMode(AUTOMATIC); //Turn on the PID
  tempController.SetOutputLimits(0, 100); //Set the output limits of the controller
  tempController.SetSampleTime(100); //100 ms update rate
  tempController.SetControllerDirection(DIRECT); //Set the direction of the controller
  Serial.begin(9600); //Initialize serial with 9600 baud
  Serial.println("Startup");
}

void loop() {
  setPower(jag, 100);
  delay(2000);
  setPower(jag, 50);
  delay(2000);
  setPower(jag, 0);
  delay(2000);
  return;
  if (Serial.available() > 0) //If data is available
    setpoint = Serial.read();
  temp = readTherm(0, 500, pullup1, vref); //Read for 1 second and return the average
  tempController.Compute(); //Do PID calculations
  setPower(jag, PIDOutput); //Set the power of the jag
  
  //temp = random(100);
  Serial.println(String(temp)); // Print temperature reading to the serial console
}

int setPower (Servo controller, double percent) {
  controller.write(map(percent, 0, 100, 0, 90));
  return controller.read(); //Return what we wrote
}

float readTherm(int port, long time, double pullup_res, double v_ref) {
  long tempTotal = 0, numPoints = 0;
  unsigned long before = millis();
  
  while(millis() - before < time){
    tempTotal += (pullup_res*(v_ref-((analogRead(port)/1023.0)*v_ref)))/((analogRead(port)/1023.0)*v_ref);
    numPoints++;
  }
  
  float avg = ((float)tempTotal)/((float)numPoints);
  return multiMap(avg, _100kHoneywellLookup, sizeof(_100kHoneywellLookup)/sizeof(_100kHoneywellLookup[0]));
}

float multiMap(float val, const float lookup[][2], uint8_t sizee) {
  // take care the value is within range
  // val = constrain(val, _in[0], _in[size-1]);
  if (val <= lookup[0][0]) { //Value is too small
    //Serial.print("VALUE TOO SMALL:  ");
    //Serial.println(String(val)+" <= "+String(lookup[0][0]));
    return lookup[0][1];
    }
  if (val >= lookup[sizee-1][0]) { //Value is too large
    //Serial.print("VALUE TOO LARGE  ");
    //Serial.println(String(val)+" >= "+String(lookup[sizee-1][0]));
    return lookup[sizee-1][1];
  }

  // search right interval
  uint8_t pos = 1;  // _in[0] allready tested
  while(val > lookup[pos][0])
    pos++;

  // this will handle all exact "points" in the _in array
  if (val == lookup[pos][0])
    return lookup[pos][1];

  // interpolate in the right segment for the rest
  return (val - lookup[pos-1][0]) * (lookup[pos][1] - lookup[pos-1][1]) / (lookup[pos][0] - lookup[pos-1][0]) + lookup[pos-1][1];
}
