/*   ArduinoNetCar v2    
*   
*   GNU General Public License 
*   (c)2012  Ouarrak Ayoub
*
*   http://arduinonetcar.blogspot.it/
*/

#include <SPI.h>          
#include <Servo.h>        
#include <avr/wdt.h>      
#include <Ethernet.h>     

byte mac[] = {0x90, 0xA2, 0xDA, 0x00, 0x62, 0x8F};       

IPAddress ip(192, 168, 1, 251);    
IPAddress gateway(192, 168, 1, 1);   
IPAddress subnet(255, 255, 255, 0);   

char RX;                                                   
String command = "";            
int forward  = 0;                                           
int back     = 0;                                              
int left     = 0;                                              
int right    = 0;                                             
int light    = 0;                                             
int lightCam = 0;                                            
int sound    = 0;                                             
int alt      = 0;  
int up       = 0;
int down     = 0;   
int device   = 0;
int camU     = 0; 
int camD     = 0; 
int camL     = 0; 
int camR     = 0; 
int camC     = 0; 
int camS     = 0; 
int DxSx     = 90;  
int UpDown   = 90;
           
int MotorForward_VCC = 5;    // pin motor                           
int MotorForward_GND = 3;                                 
                
int MotorDXSX_VCC = 9;   // pin motor                                
int MotorDXSX_GND = 6;

int velocity = 250;   

int frontLedRX = 4;                                     
int frontLedLX = 7;                                     

boolean motorForwardOn = false;
boolean right1     = false;
boolean left1      = false; 
boolean lightOn    = false;
boolean camSleep   = true;
boolean lightCamOn = false;  

EthernetServer ArduinoServer(80);  // port 80
Servo servoDS, servoUD; 


void setup() {                                                                                                         
   Ethernet.begin(mac, ip, gateway, subnet);                                                       
   ArduinoServer.begin();  

   pinMode(MotorForward_VCC, OUTPUT);                    
   pinMode(MotorForward_GND, OUTPUT);                      
   pinMode(MotorDXSX_VCC, OUTPUT);                         
   pinMode(MotorDXSX_GND, OUTPUT);                                               
   pinMode(frontLedRX, OUTPUT);                           
   pinMode(frontLedLX, OUTPUT);                             
   pinMode(A0, OUTPUT);            //pin servo1
   pinMode(A1, OUTPUT);            //pin servo2    
   pinMode(A2, OUTPUT);            //pin led camera

   servoDS.attach(A0);
   servoUD.attach(A1);

   Serial.begin(9600); 
   // move up the cam
   for(int i = 0; i < 90; ++i){
     servoDS.write(i);
     delay(400);
   }
} 


void loop() {
   command = "";  
   EthernetClient client = ArduinoServer.available();

   if(client != false) {
      while(client.connected()) {               
         if(client.available()) {               
            RX = client.read();                  
            command += RX;                       

            if(RX == '\n') {
               forward = command.indexOf("forward");
               back = command.indexOf("back");
               left = command.indexOf("left");
               right = command.indexOf("right");
               light = command.indexOf("light");
               lightCam = command.indexOf("LightCam");
               sound = command.indexOf("sound");
               device = command.indexOf("PC");
               alt = command.indexOf("alt");
               up = command.indexOf("up");
               down = command.indexOf("down");
               camU = command.indexOf("camUp");
               camD = command.indexOf("camDown");
               camL = command.indexOf("camLeft");
               camR = command.indexOf("camRight");
               camS = command.indexOf("camSleep");

               client.println("HTTP/1.1 200 OK");                    
               client.println("Content-Type: text/html");              
               client.println();
               client.print("<html><head>");                                  
               if(device > 0) {
                  drive();
                  // 192.168.1.4 := ip server web
                  client.print("<link href = 'http://192.168.1.4:8080/controlPC.htm'</head></html>");  
               } 
               else {
                  drive();
                  client.print("<link href = 'http://192.168.1.4:8080/control.htm'</head></html>");  
               }
               delay(1);
               wdt_reset();        
               break;
            }
         }
      }
      client.stop();         
   }
}

void drive() {
   if (forward > 0)       
      Forward(velocity);
 
   if(back > 0) 
      Back(velocity);

   if(alt > 0) 
      Stop();

   if(camU > 0) {
      if(UpDown < 180) {
         UpDown += 10;
         servoUD.write(UpDown);
         camSleep = true;
      }
   }

   if(camD > 0) {
      if(UpDown > 0) {
         UpDown -= 10;
         servoUD.write(UpDown);
         camSleep = true;
      }
   }

   if(camL > 0) {
      if(DxSx < 180) {
         DxSx += 10;
         servoDS.write(DxSx);
         camSleep = true;
      }  
   } 

   if(camR > 0) {
      if(DxSx > 0) {
         DxSx -= 10;
         servoDS.write(DxSx);
         camSleep = true;
      }
   }

   if(camS > 0) {
      if(camSleep == true) {
         servoDS.write(87);
         delay(300);
         servoUD.write(90);
         DxSx = 87;
         UpDown = 90;
         camSleep = false;
      }
      else {  
         servoDS.write(87);
         delay(300);
         servoUD.write(1);
         camSleep = true;
      }
  }

   if(left > 0)
      Left();

   if(right > 0)
      Right();

   if(up > 0) {
      if(motorForwardOn == true) {
         if(velocity < 255) {
            velocity += 5;
            Forward(velocity);
         }
      } 
      else {
         if(velocity < 255)
         velocity += 5;
      }
   }

   if(down > 0) {
      if(motorForwardOn == true) {
         if(velocity > 0) {
            velocity -= 5;  
            Forward(velocity);
         }
      } 
      else {
         if(velocity > 0)
         velocity -= 5;
      }
   }

   if(light > 0) {
      if(lightOn == false) {
         digitalWrite(frontLedRX, HIGH);
         digitalWrite(frontLedLX, HIGH);
         lightOn = true;
      }
      else {
         digitalWrite(frontLedRX, LOW);
         digitalWrite(frontLedLX, LOW);
         lightOn = false;
      }  
   }

   if(lightCam > 0) {
      if(lightCamOn == false) {
         digitalWrite(A2, HIGH);
         lightCamOn = true;
      }
      else {
         digitalWrite(A2, LOW);
         lightCamOn = false;
      }
   }
}

void Stop() {
   digitalWrite(MotorForward_VCC, LOW);
   digitalWrite(MotorForward_GND, LOW);
   motorForwardOn = false;
}

void StopDXSX() {
   digitalWrite(MotorDXSX_VCC, LOW);
   digitalWrite(MotorDXSX_GND, LOW);
}

void Forward(int velocity) {
   analogWrite(MotorForward_VCC, velocity);
   analogWrite(MotorForward_GND, 0);
   motorForwardOn = true;
}

void Back(int velocity) {
   digitalWrite(MotorForward_VCC, LOW);
   analogWrite(MotorForward_GND, velocity);
   motorForwardOn = false;
}

void Right() {
   if(right1 == true) {
      digitalWrite(MotorDXSX_VCC, HIGH);
      digitalWrite(MotorDXSX_GND, LOW);
      left1 = false;
   }
   else {
      StopDXSX();
      right1 = true;
   }
}

void Left() {
   if(left1 == true) {
      digitalWrite(MotorDXSX_VCC, LOW);
      digitalWrite(MotorDXSX_GND, HIGH);
      right1 = false;
   }
   else {
      StopDXSX();
      left1 = true;
   }
}
