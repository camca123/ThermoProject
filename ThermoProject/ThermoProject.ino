#include <Servo.h>
#include <PID_v1.h>

#define THERM_PIN 0
#define JAG_PIN 9
#define Kp 1
#define Ki 0
#define Kd 0


Servo jag;

long out[] = {150,145,140,135,130,125,120,115,110,105,100,95,90,85,80,75,70,65,60,55,50,45,40,35,30,25,20,15,10,5,0,-5,-10,-15,-20,-25,-30,-35,-40};
long in[] = {1618,1827,2070,2350,2676,3057,3503,4026,4643,5372,6238,7269,8504,9988,11780,13951,16597,19835,23820,28749,34879,42548,52200,64422,80003,100000,125851,159522,203723,262229,340346,445602,588793,785573,1058901,1442861,1988706,2774565,3921252};
const int ARR_LEN = 39;

const double vref = 5.0; //Reference voltage
const double pullup1 = 99.7*1000; //Pullup resistor

double temp = 25; //Thermistor temp
double PIDOutput = 0;
double setpoint = 25;
PID tempController(&temp, &PIDOutput, &setpoint, Kp, Ki, Kd, REVERSE);


void setup()
{
  jag.attach(JAG_PIN);  //Jag is connected to a pin
  Serial.begin(19200);
  Serial.println("Startup");
}

void loop()
{
  
}

int setPower (Servo controller, double percent)
{
  controller.write(map(percent, -180, 180, 0, 100));
  return controller.read();
}

int readTherm(int port, int samples, double pullup_res, double v_ref)
{
  long tempTotal = 0;
  
  for(int C = 0;C < samples;C++)
  {
    tempTotal += (pullup_res*(v_ref-((analogRead(port)/1023.0)*v_ref)))/((analogRead(port)/1023.0)*v_ref);
    delay(1);
  }
  long avg = tempTotal/(double)(samples);
  return multiMap((long)(avg), in, out, ARR_LEN);
}

int multiMap(long val, long* _in, long* _out, uint8_t sizee)
{
  // take care the value is within range
  // val = constrain(val, _in[0], _in[size-1]);
  if (val <= _in[0]){Serial.print("VALUE TOO SMALL:  "); Serial.println(String(val)+" <= "+String(_in[0])); return _out[0];}
  if (val >= _in[sizee-1]){Serial.print("VALUE TOO LARGE  "); Serial.println(String(val)+" >= "+String(_in[0])); return _out[sizee-1];}

  // search right interval
  uint8_t pos = 1;  // _in[0] allready tested
  while(val > _in[pos]) pos++;

  // this will handle all exact "points" in the _in array
  if (val == _in[pos]) return _out[pos];

  // interpolate in the right segment for the rest
  return (val - _in[pos-1]) * (_out[pos] - _out[pos-1]) / (_in[pos] - _in[pos-1]) + _out[pos-1];
}
