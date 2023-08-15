//BLYNK
#define BLYNK_TEMPLATE_ID           "TMPL3MaXILfMC"
#define BLYNK_TEMPLATE_NAME         "Quickstart Template"
#define BLYNK_AUTH_TOKEN            "IFadZgFV0w7ctemOu-xhw076aPmSXNQ9"
#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// Your WiFi credentials.
char ssid[] = "smartdevice";
char pass[] = "smartdevice";

//MAX30010
#include <DFRobot_MAX30102.h>
DFRobot_MAX30102 particleSensor;
float temp = 0;

//oled
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1 //Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C //See datasheet for Address
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//gps
#include <TinyGPS++.h>
#define RXD2 10
#define TXD2 9
HardwareSerial neogps(1);
TinyGPSPlus gps;
String g = "https://maps.google.com/maps/place/";
String LA ;
String LON ;

//VIB
#define vib 2 

void setup()
{
//Init serial
Serial.begin(115200);
//serial end


//gps
neogps.begin(9600, SERIAL_8N1, RXD2, TXD2);
//gps end

//oled
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
    display.clearDisplay();
  display.display();
  delay(2000);
//oled end



//blynk init
Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
while (!particleSensor.begin()) {
Serial.println("MAX30102 was not found");
delay(1000);
}
//blynk end
  

//max30010 init

particleSensor.sensorConfiguration(/*ledBrightness=*/50, /*sampleAverage=*/SAMPLEAVG_4, \
                        /*ledMode=*/MODE_MULTILED, /*sampleRate=*/SAMPLERATE_100, \
                        /*pulseWidth=*/PULSEWIDTH_411, /*adcRange=*/ADCRANGE_16384);
digitalWrite(vib,HIGH);
delay(1000);
digitalWrite(vib,LOW);
}

//calculations and var for spo2
int32_t SPO2; //SPO2
int8_t SPO2Valid; //Flag to display if SPO2 calculation is valid
int32_t heartRate; //Heart-rate
int8_t heartRateValid; //Flag to display if heart-rate calculation is valid 
//max30010 init end




void loop()
{



  // Build the message to send
      char text[ 64 ];

      strcpy_P( text, (const char *) F("https://maps.google.com/maps/place/") );
      char *ptr = &text[ strlen(text) ];

      dtostrf( gps.location.lat(), 4, 5, ptr ); // append the latitude
      ptr = &ptr[ strlen(ptr) ];

      *ptr++ = ',';  // append one character
      *ptr++ = ' ';

      dtostrf( gps.location.lng(), 4, 5, ptr ); // append the longitude

      
  Serial.println(F("Wait about four seconds"));
  particleSensor.heartrateAndOxygenSaturation(/**SPO2=*/&SPO2, /**SPO2Valid=*/&SPO2Valid, /**heartRate=*/&heartRate, /**heartRateValid=*/&heartRateValid);
  temp = particleSensor.readTemperatureC();
  //Print result 

  //BPM AND SPO2 value
  Serial.print(F("heartRate="));
  Serial.print(heartRate, DEC);
  
  Serial.print(F(", heartRateValid="));
  Serial.println(heartRateValid, DEC);
  
  Serial.print(F("; SPO2="));
  Serial.println(SPO2, DEC);
  
  Serial.print(F(", SPO2Valid="));
  Serial.println(SPO2Valid, DEC);

  Serial.print(F(", Temprature="));
  Serial.println(temp);


  //loc
  Serial.println(text);

  

  
  
//call oled function
  boolean newData = false;
  for (unsigned long start = millis(); millis() - start < 1000;)
  {
    while (neogps.available())
    {
      if (gps.encode(neogps.read()))
      {
        newData = true;
      }
    }
  }

  //If newData is true
  if(newData == true)
  {
    newData = false;
    Serial.println(gps.satellites.value());
    print_speed();
  }
  else
  {
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.setTextSize(3);
    display.print("No Data");
    display.display();
  } 
  delay(2000);
oled();

if(heartRate >= 80)
{
  analogWrite(2,255);
  delay(1000);
  analogWrite(2,0);
  delay(2000);
}
if(heartRate >= 120)
{
  analogWrite(2,255);
  delay(2000);
  analogWrite(2,0);
  delay(2000);
}
if(heartRate >= 160)
{
  analogWrite(2,255);
  delay(3000);
  analogWrite(2,0);
    delay(2000);
}


//blynk write values
Blynk.virtualWrite(V1,SPO2);
Blynk.virtualWrite(V0,heartRate);
Blynk.virtualWrite(V2,temp);
Blynk.virtualWrite(V4,text);
Blynk.virtualWrite(V5,gps.speed.kmph());

//Blynk call function
Blynk.run();

}


void print_speed()
{
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
       
  if (gps.location.isValid() == 1)
  {
   //String gps_speed = String(gps.speed.kmph());
    display.setTextSize(1);

    //lattitude
    display.setCursor(25, 5);
    display.print("Lat: ");
    display.setCursor(50, 5);
    display.print(gps.location.lat(),6);
    LA = (gps.location.lat());


    display.setCursor(25, 20);
    display.print("Lng: ");
    display.setCursor(50, 20);
    display.print(gps.location.lng(),6);
    LON = (gps.location.lng());
    
    //speed
    display.setCursor(25, 35);
    display.print("Speed: ");
    display.setCursor(65, 35);
    display.print(gps.speed.kmph());
    
    //Satellit connection
    display.setTextSize(1);
    display.setCursor(0, 50);
    display.print("SAT:");
    display.setCursor(25, 50);
    display.print(gps.satellites.value());
    

    //altitude
    display.setTextSize(1);
    display.setCursor(70, 50);
    display.print("ALT:");
    display.setCursor(95, 50);
    display.print(gps.altitude.meters(), 0);
    display.display();
    
  }
  else
  {
    //if no values then display no data
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.setTextSize(3);
    display.print("No Data");
    display.display();
  }  

}


void oled()
{
    display.setTextSize(1);
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);

    //hedder
    display.setCursor(25, 5);
    display.print("Body_Vitals");

    //Temprature
    display.setCursor(10, 20);
    display.print("Temprature:");
    display.setCursor(80, 20);
    display.print(temp);
    display.setCursor(115, 20);
    display.print("C");


    //bpm
    display.setCursor(10, 30);
    display.print("Beat/min:");
    display.setCursor(80, 30);
    display.print(heartRate);
    display.display();

    //spp02 level
    display.setCursor(10, 40);
    display.print("Sp02_level");
    display.setCursor(80, 40);
    display.print(SPO2);
    display.setCursor(115, 40);
    display.print("%");
    display.display();
}
