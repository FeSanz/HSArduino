#ifdef ESP32
  #include <WiFi.h>
  #include <HTTPClient.h>
#else
  #include <ESP8266WiFi.h>
  #include <ESP8266HTTPClient.h>
  #include <WiFiClient.h>
#endif

#include <EMailSender.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,16,2);

const char* ssid = "TP-Link_E23C";
const char* password =  "59582717";

String host = "http://192.168.0.126/humedad/api.php?api_humedity=";
HTTPClient http;

int led = 23;//23
const int sensorHumedityPin =34;//34

bool water = false;


int8_t connection_state = 0;
uint16_t reconnect_interval = 10000;

EMailSender emailSend("raumentada112@gmail.com", "CondorXR112");

void connectToNetwork() 
{
  WiFi.scanNetworks();
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    lcd.print("Conectando...");
    lcd.clear();
  }
  lcd.clear();
  lcd.print("Conectado!!!");
}

void postHumedityData(int rangeHumedity, int percentageHumedity)
{
  if(WiFi.status()== WL_CONNECTED)
    {
      http.begin(host + "register_humedity");
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");

      String postData = "plant=acua01&rango=" + String(rangeHumedity) + "&percentage=" + String(percentageHumedity); 
      int httpCode = http.POST(postData);
      String payload = http.getString();
   
      Serial.println(httpCode);   //Print HTTP return code
      Serial.println(payload);    //Print request response payload
   
      http.end();
    }
    else 
    {
      Serial.println("WiFi Desconectado");
    }
}

void senEmail(String text)
{
    EMailSender::EMailMessage message;
    message.subject = "Acuaponía. Sensor de humedad del suelo";
    message.message = text;

   // EMailSender::Response resp = emailSend.send("felipe.antonio.condor@gmail.com", message);
    const char* emails[] = {"felipe.antonio.condor@gmail.com", "felipemusic7777@gmail.com", "felipemusic777@hotmail.com"};
    EMailSender::Response resp = emailSend.send(emails, 3, message);

    Serial.println("Enviando...");

    Serial.println(resp.status);
    Serial.println(resp.code);
    Serial.println(resp.desc);
}

void setup() 
{
  Serial.begin(115200);
   
  lcd.init();
  lcd.backlight();

  connectToNetwork();

  pinMode(sensorHumedityPin, INPUT);
  pinMode(led, OUTPUT);

  lcd.setCursor(0,1); 
  lcd.print(WiFi.localIP());
  delay(3000);
  lcd.clear();

  senEmail("Se detectó poca humedad en el suelo<br>Se ha activado el riego automático.<br>Verifique en: http://groundhumedity.epizy.com");
  
  lcd.print("Humedad suelo: ");
}
 
void loop() 
{ 
  int rango_humedity = (int) (analogRead(sensorHumedityPin) / 4.002);
  int percentage_humedity = (int) (100 -(((rango_humedity*100)/512)-100));

  lcd.setCursor(0,1); 
  lcd.print("                "); 
  lcd.setCursor(0,1); 

  if(percentage_humedity > 100)
  {
    lcd.print("100 %  Rango:");
    percentage_humedity = 100;
  }
  else
  {
    lcd.print(percentage_humedity);
    lcd.print(" %  Rango:");
  }
  
  lcd.print(rango_humedity);

  if(rango_humedity >= 800 && !water)//768
  {
    water = true;
    Serial.println("Regar plantas, falta humedad");
  }

  if(water)
  {
      digitalWrite(led, HIGH);
  }

  if(rango_humedity <= 600 && water)//512
  {
    water = false;
    Serial.println("Humedad adecuada!!!");
    digitalWrite(led, LOW);
  }

   postHumedityData(rango_humedity, percentage_humedity);
  delay(3000);
}
