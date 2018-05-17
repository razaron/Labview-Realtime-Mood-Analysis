

/*********************************************************************************
 **
 **  LVFA_Firmware - Provides Basic Arduino Sketch For Interfacing With LabVIEW.
 **
 **  Written By:    Sam Kristoff - National Instruments
 **  Written On:    November 2010
 **  Last Updated:  Dec 2011 - Kevin Fort - National Instruments
 **
 **  This File May Be Modified And Re-Distributed Freely. Original File Content
 **  Written By Sam Kristoff And Available At www.ni.com/arduino.
 **
 *********************************************************************************/


/*********************************************************************************
 **
 ** Includes.
 **
 ********************************************************************************/ 
// Standard includes.  These should always be included.
#include <Wire.h>
#include <SPI.h>
#include <Servo.h>
#include "LabVIEWInterface.h" 
#include <math.h>  //include Math library

const int pin[] = {3,5,6};  //pin for red LED driver
const int FIELD_MAX = 2; // max number of field inputs
const int LITE_SCALE = 1;

const int hotPin = 7;
const int coldPin = 8;
const int ESI[] = {10,11,12};

unsigned long time[] = {0,0}; //Holds time values for calculating hold time

int field = 0;  //index for current field

float input[FIELD_MAX]; //Array to store values;

int red[] = {0,0};  //red colour value: min = 0, max = 255
int grn[] = {0,0};  //green colour value: min = 0, max = 255
int blu[] = {0,0};  //blue colour value: min = 0, max = 255

float hue[] = {0,0};  //hue value, range of 0deg to 360deg. First value holds "previous" value for comparison
float sat = 1;  //Saturation value,ranges between 0 and 1
float light = 0;  //Lightness value, ranges betwen 0 and 1

int offset = 0;

float x_val;  //x-axis "mood value" read from serial input
float y_val;  //y-axis "mood value" read from serial input

/*********************************************************************************
 **  setup()
 **
 **  Initialize the Arduino and setup serial communication.
 **
 **  Input:  None
 **  Output: None
 *********************************************************************************/
void setup()
{  
  // Initialize Serial Port With The Default Baud Rate
  syncLV();

  // Place your custom setup code here
  Serial.setTimeout(2000);
  pinMode(hotPin, INPUT);
  pinMode(coldPin, INPUT);
  pinMode(ESI[0], INPUT);
  pinMode(ESI[1], INPUT);
  pinMode(ESI[2], INPUT);
}


/*********************************************************************************
 **  loop()
 **
 **  The main loop.  This loop runs continuously on the Arduino.  It 
 **  receives and processes serial commands from LabVIEW.
 **
 **  Input:  None
 **  Output: None
 *********************************************************************************/
void loop()
{   
  // Check for commands from LabVIEW and process them.   
 
  checkForCommand();
  // Place your custom loop code here (this may slow down communication with LabVIEW)
  while(digitalRead(coldPin)==HIGH && digitalRead(hotPin)==LOW){
    offset += 1;
  }
  while(digitalRead(coldPin)==HIGH && digitalRead(hotPin)==HIGH){
    offset -= 1;  
  }
  
  time[0] = millis();
  if(Serial.available()){

    for(field  = 0; field < 3; field++){
      input[field] = Serial.parseFloat();
    }
    Serial.print(field);
    Serial.println(" values received:");
    for(int i=0; i<(field); i++){
      Serial.println(input[i]);
    }
    field = 0;
  }
  
  /*
  x_val = input[0];
  y_val = input[1];
  light = input[1];
  */
  x_val = digitalRead(ESI[0]);
  y_val = digitalRead(ESI[1]);
  light = digitalRead(ESI[2]);
  
  if(y_val>1) y_val = 1;
  
  hue[1] = getHue(x_val, y_val);
  hue[1] += offset;
  if(hue[1]>360) hue[1] -= 360;
  Serial.print("The hue value is:");
  Serial.println(hue[1]);
  Serial.print("The saturation value is:");
  Serial.println(sat);
  light = getLite(light);
  Serial.print("The lightness value is:");
  Serial.println(light);
  
  if(hue[1] == hue[0]){
	  	time[1] = millis() - time[0];
		delay(1000 - time[1]);
	}
  else{
	  	/*HSL_RGB(hue[0], sat, light, red[0], grn[0], blu[0]);*/
	  	HSL_RGB(hue[1], sat, light, red[1], grn[1], blu[1]);
	  	time[1] = millis() - time[0];
	  	fade(red[0], grn[0], blu[0], red[1], grn[1], blu[1], (1000-time[1]));
	}
	
  /*Serial.println("The RGB values are: ");
  Serial.print("Red: ");
  Serial.println(red[1]);
  Serial.print("Green: ");
  Serial.println(grn[1]);
  Serial.print("Blue: ");
  Serial.println(blu[1]);*/
  
    hue[0] = hue[1];
    red[0] = red[1];
    grn[0] = grn[1];
    blu[0] = blu[1];
  
  if(acqMode==1)
  {
    sampleContinously();
  }

}


float getHue(float x, float y){
  float angle;
  if(x==0 && y>0){
    angle = 0;
  }
  else if(x>0 && y>0){
    angle = atan2(x, y);
  }
  else if(x>0 && y==0){
    angle = PI/2;
  }
  else if(x>0 && y<0){
    angle = (PI/2) + atan2(y, x);
  }
  else if(x==0 && y<0){
    angle  = PI;
  }
  else if(x<0 && y<0){
    angle = (3* PI/2) + atan2(y, x);
  }
  else if(x<0 && y==0){
    angle = 3 * PI/2;
  }
  else if(x<0 && y>0){
    angle = (2*PI) + atan2(x, y);
  }
  else{
    angle = 0;
  }
  angle = angle * 180/PI;
  
  return angle;  
}

float getLite(int val){
  float lite = val/1024;
  
  return val;
}
  
void HSL_RGB(float hue, float sat, float lite, int &red, int &grn, int &blu){
  int var_i;
  float var_1, var_2, var_3, var_h, var_r, var_g, var_b;

  if ( sat == 0 )                       //S values = 0 - 1
  {
    red = lite * 255;
    grn = lite * 255;
    blu = lite * 255;
  }
  else
  {
    var_h = hue/60;
    if ( var_h == 6 ) var_h = 0;      //hue must be < 1
    var_i = int( var_h ) ;            //Or ... var_i = floor( var_h )
    var_1 = lite * ( 1 - sat );
    var_2 = lite * ( 1 - sat * ( var_h - var_i ) );
    var_3 = lite * ( 1 - sat * ( 1 - ( var_h - var_i ) ) );

    if      ( var_i == 0 ) { 
      var_r = lite     ; 
      var_g = var_3 ; 
      var_b = var_1 ;
    }
    else if ( var_i == 1 ) { 
      var_r = var_2 ; 
      var_g = lite     ; 
      var_b = var_1 ;
    }
    else if ( var_i == 2 ) { 
      var_r = var_1 ; 
      var_g = lite     ; 
      var_b = var_3 ;
    }
    else if ( var_i == 3 ) { 
      var_r = var_1 ; 
      var_g = var_2 ; 
      var_b = lite     ;
    }
    else if ( var_i == 4 ) { 
      var_r = var_3 ; 
      var_g = var_1 ; 
      var_b = lite     ;
    }
    else                   { 
      var_r = lite     ; 
      var_g = var_1 ; 
      var_b = var_2 ;
    }

    red = (1-var_r) * 255;                  //RGB results = 0 - 255
    grn = (1-var_g) * 255;
    blu = (1-var_b) * 255;
  }
  
}

//function fades between two RGB values over fade time period t
//maximum value of fade time = 30 seconds before gradient values
//get too small for floating point math to work.

void fade(int r1, int g1, int b1, int r2, int g2, int b2, int t)
{

  float r_float1, g_float1, b_float1;
  float r_float2, g_float2, b_float2;
  float grad_r, grad_g, grad_b;
  float output_r, output_g, output_b;
  
  //declare integer RGB values as float values
  r_float1 = (float) r1;
  g_float1 = (float) g1;
  b_float1 = (float) b1;
  r_float2 = (float) r2;
  g_float2 = (float) g2;
  b_float2 = (float) b2;
  
  //calculate rates of change of R, G, and B values
  grad_r = (r_float2-r_float1)/t;
  grad_g = (g_float2-g_float1)/t;
  grad_b = (b_float2-b_float1)/t;
  
  //loop round, incrementing time value "i"
  for ( float i=0; i<=t; i++ )
  {
    
    output_r = r_float1 + grad_r*i;
    output_g = g_float1 + grad_g*i;
    output_b = b_float1 + grad_b*i;
    
    //map values - arduino is sinking current, not sourcing it
    output_r = map (output_r,0,255,255,0);
    output_g = map (output_g,0,255,255,0);
    output_b = map (output_b,0,255,255,0);
    
    //output
    analogWrite(pin[0], (int)output_r);
    analogWrite(pin[1], (int)output_g);
    analogWrite(pin[2], (int)output_b);
    
    //hold at this colour set for 1ms
    delay(1);
    
  }
}











