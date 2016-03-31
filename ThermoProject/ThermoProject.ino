#include <Servo.h>
#include <PID_v1.h>

#define THERM_PIN 0
#define JAG_PIN 9
#define Kp 1
#define Ki 0
#define Kd 0

const float _100kHoneywellLookup[][2] = { // Resistance, Temperature (deg C)
  {1, 941},
  {19, 362},
  {37, 299}, //top rating 300C
  {55, 266},
  {73, 245},
  {91, 229},
  {109, 216},
  {127, 206},
  {145, 197},
  {163, 190},
  {181, 183},
  {199, 177},
  {217, 171},
  {235, 166},
  {253, 162},
  {271, 157},
  {289, 153},
  {307, 149},
  {325, 146},
  {343, 142},
  {361, 139},
  {379, 135},
  {397, 132},
  {415, 129},
  {433, 126},
  {451, 123},
  {469, 121},
  {487, 118},
  {505, 115},
  {523, 112},
  {541, 110},
  {559, 107},
  {577, 105},
  {595, 102},
  {613, 99},
  {631, 97},
  {649, 94},
  {667, 92},
  {685, 89},
  {703, 86},
  {721, 84},
  {739, 81},
  {757, 78},
  {775, 75},
  {793, 72},
  {811, 69},
  {829, 66},
  {847, 62},
  {865, 59},
  {883, 55},
  {901, 51},
  {919, 46},
  {937, 41},
  {955, 35},
  {973, 27},
  {991, 17},
  {1009, 1},
  {1023, 0} //to allow internal 0 degrees C
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
  if (Serial.available() > 0) //If data is available
    setpoint = Serial.read();
  temp = readTherm(0, 500, pullup1, vref); //Read for 1 second and return the average
  tempController.Compute(); //Do PID calculations
  setPower(jag, PIDOutput); //Set the power of the jag
  
  //temp = random(100);
  Serial.println(String(temp)); // Print temperature reading to the serial console
}

int setPower (Servo controller, double percent) {
  controller.write(map(percent, -180, 180, 0, 100));
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
  return multiMap(avg, (float**)_100kHoneywellLookup, sizeof(_100kHoneywellLookup)/sizeof(_100kHoneywellLookup[0]));
}

float multiMap(float val, float** lookup, uint8_t sizee) {
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
