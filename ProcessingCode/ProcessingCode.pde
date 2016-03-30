import processing.serial.*;

Serial myPort;        // The serial port
int xPos = 1;         // horizontal position of the graph
float currTemp = 0;
float inByte = 0;
float x1 = 0;
float y1 = 0;
float x2 = 0;
float y2 = 0;

void setup () {
  // set the window size:
  size(1200, 900);

  // List all the available serial ports
  // if using Processing 2.1 or later, use Serial.printArray()
  println(Serial.list());

  try{
  myPort = new Serial(this, Serial.list()[0], 9600);
  myPort.bufferUntil('\n');
  } catch (Exception e){
    println("No serial device conneted!");
  }

  // set inital background:
  background(255, 255, 255);
}
void draw () {  
  stroke(255, 0, 0);
  line(x1, height - y1, x2, height - y2);
  drawLabels();

  // at the edge of the screen, go back to the beginning:
  if (x2 >= width) {
    x2 = 0;
    x1 = 0;
    background(255, 255, 255);
  } else {
    // increment the horizontal position:
    x1 = x2;
    y1 = y2;
    y2 = inByte;
    x2++;
  }
}

void drawLabels() {
  stroke(0, 0, 0);
  for (int C = 0;C < height;C++){
    if (C % (height/100) == 0)
      line(0, C, 30, C);
  }
  for (int C = 0;C < width;C++){
    if (C % (width/100) == 0)
      line(C, height, C, height-30);
  }
  
  
  fill(0, 0, 255);
  textSize(25);
  text("Temperature (deg C)", 40, height/2);
  text("Time", width/2, height - 30);
  
  fill(255, 255, 255);    // black rectangle = background for information
  noStroke();
  rect(width-400, 0, width, 80);
  fill(0, 0, 255);
  text("Current Temp: " + str(currTemp) + " C", width-400, 50);
}


void serialEvent (Serial myPort) {
  // get the ASCII string:
  String inString = myPort.readStringUntil('\n');

  if (inString != null) {
    // trim off any whitespace:
    inString = trim(inString);
    // convert to an int and map to the screen height:
    inByte = float(inString);
    println(inByte);
    currTemp = inByte;
    inByte = map(inByte, 0, 100, 0, height);
  }
}