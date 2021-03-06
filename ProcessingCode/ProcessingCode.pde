import processing.serial.*;

Serial myPort;        // The serial port

String textInput = "";
boolean editing = false; //If the user is editing the setpoint

float currTemp = -1;
float inByte = 0;
int setpoint = 25;
float tempRange = 150;
float timeRange = 60; //Seconds
float updateTime = 200; //Milliseconds

long beforeTime = 0;
PImage img;
int CONTRAST = 2;
float microstates = 50;

Line[] lines;

class Line {
  float x_1, y_1, x_2, y_2;
  int R, G, B;
  
  Line(float x1, float y1, float x2, float y2, int r, int g, int b){
    x_1 = x1;
    y_1 = y1;
    x_2 = x2;
    y_2 = y2;
    R = r;
    G = g;
    B = b;
  }
}

void setup () {
  size(1200, 900); //Set the window size
  frame.setResizable(true); //Window is resizable
  
  println(Serial.list()); // List all the available serial ports

  try{
    myPort = new Serial(this, Serial.list()[0], 9600);
    myPort.bufferUntil('\n');
  } catch (Exception e){
    println("No serial device conneted!");
  }

  background(255, 255, 255); // Set inital background
  img = loadImage("data", "gif");
  tint(255, CONTRAST);
  image(img, 0, 0, width, height);
  
  lines = new Line[3];
  lines[0] = new Line(0, 0, 0, 0, 255, 0, 0); //Temperature line
  lines[1] = new Line(0, 0, 0, 0, 0, 255, 0); //Setpoint line
  lines[2] = new Line(0, 0, 0, 0, 0, 255, 255); //Setpoint line
}
void draw () {
  if (millis() - beforeTime > updateTime) { //Wait 1 second before updating
    for (int C = 0;C < lines.length;C++) { //Draw all of the lines
      if(C == 2 && CONTRAST < 10)
        continue;
      strokeWeight(6);
      stroke(lines[C].R, lines[C].G, lines[C].B);
      line(lines[C].x_1, height - lines[C].y_1, lines[C].x_2, height - lines[C].y_2);
    }
  
    if (lines[0].x_2 >= width) { // at the edge of the screen, go back to the beginning
      for (int C = 0;C < lines.length;C++){ //Update all of the lines
        lines[C].x_2 = 0;
        lines[C].x_1 = 0;
      }
      background(255, 255, 255); //Redraw the background
      tint(255, CONTRAST);
      image(img, 0, 0, width, height);
    } else {
      for (int C = 0;C < lines.length;C++) { //Update all of the lines
          lines[C].x_1 = lines[C].x_2;
          lines[C].y_1 = lines[C].y_2;
          lines[C].x_2 += (width/timeRange)*(updateTime/1000);
          microstates += random(5.0*sin(lines[C].x_1)/(C+1));
      }
      lines[0].y_2 = map(currTemp, 0, tempRange, 0, height);
      lines[1].y_2 = map(setpoint, 0, tempRange, 0, height);
      lines[2].y_2 = map(microstates, 0, tempRange, 0, height);
    }
    beforeTime = millis();
  }
  
  drawLabels();
}

void drawLabels() {
  for (int C = 0;C < height;C++) { //Horizontal tick marks
    if (C % (5*height/tempRange) == 0){
      strokeWeight(1);
      stroke(200, 200, 200);
      line(0, C, width, C);
      strokeWeight(4);
      stroke(0, 0, 0);
      line(0, C, 30, C);
    }
  }
  stroke(0, 0, 0);
  strokeWeight(4);
  for (int C = 0;C < width;C++) { //Vertical tick marks
    if (C % (width/timeRange) == 0)
      line(C, height, C, height-30);
  }
  
  
  fill(0, 0, 255);
  textSize(25);
  text("Temperature (deg C)", 40, height/2);
  text("Time (s)", width/2, height - 40);
  
  fill(255, 255, 255);    // White rectangle for the text update
  noStroke();
  rect(width-400, 0, width, 100);
  fill(0, 0, 255);
  text("Current Temp: " + str(currTemp) + " C", width-400, 50);
  if (editing)
    text("Set Temp: " + textInput + " C", width-400, 90);
  else
    text("Set Temp: " + str(setpoint) + " C", width-400, 90);
}


void serialEvent (Serial myPort) {
  String inString = myPort.readStringUntil('\n'); // Get the ASCII string

  if (inString != null) {
    inString = trim(inString); // Trim off any whitespace
    inByte = float(inString);
    currTemp = inByte;
    println("Temp: " + currTemp); // Print input to console
  }
}

void keyPressed() {
  textInput += key;
  if (key == ENTER || key == RETURN){
    try {
      setpoint = (int)Float.parseFloat(textInput);
    } catch (Exception e) {
      println("Error parsing user input");
      setpoint = 25;
    }
    try {
      myPort.write((int)setpoint);
    } catch (Exception e) {
      println("Error writing to serial port");
    }
    editing = false;
    textInput = "";
  } else if (key == BACKSPACE) {
    if (textInput.length() > 0) {
        textInput = textInput.substring(0, textInput.length() - 2);
      }
  } else {
    editing = true;
  }
}

void mousePressed(){
  CONTRAST = 20;
}

void mouseReleased(){
  CONTRAST = 2;
}