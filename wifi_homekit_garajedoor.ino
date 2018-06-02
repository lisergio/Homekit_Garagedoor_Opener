#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <WiFiManager.h> 

IPAddress server(192, 168, 0, 56); // IP de la raspberry Pi
const char* host = "garajedooropener"; // nombre del entorno


int final_carrera_abierta = 13;
boolean estado_final_carrera_abierta;
int final_carrera_cerrada = 14;
boolean estado_final_carrera_cerrada;
int pulsador_maniobra = 0;
boolean estado_pulsador_maniobra;
int seguridad = 2;
boolean estado_seguridad;
int subir = 16;
int bajar = 12;
boolean puerta_abierta;
boolean puerta_cerrada;
boolean averia=LOW;


#define BUFFER_SIZE 100

WiFiClient wclient;
PubSubClient client(wclient, server);

void callback(const MQTT::Publish& pub) {
  Serial.println (pub.payload_string());
    if(pub.payload_string() == "abrir")
    {
       abrir_puerta(); 
    }
    else if(pub.payload_string() == "cerrar")
    {
       cerrar_puerta(); 
    }    
}


void setup() {
pinMode (final_carrera_abierta, INPUT);
pinMode (final_carrera_cerrada, INPUT);
pinMode (pulsador_maniobra, INPUT);
pinMode (seguridad, INPUT);
pinMode (subir, OUTPUT);
pinMode (bajar, OUTPUT);

Serial.begin(115200);
digitalWrite (subir, LOW);
digitalWrite (bajar, LOW);
EEPROM.begin(512);
delay(10);
client.set_callback(callback);

if (WiFi.status() != WL_CONNECTED) {
Serial.print("Connecting to ");
Serial.print(WiFi.SSID());
Serial.println("...");
WiFi.mode(WIFI_STA);
WiFi.begin();
if (WiFi.waitForConnectResult() != WL_CONNECTED){
  Serial.println("WiFi not connected");
  WiFiManager wifiManager;  
  if (!wifiManager.startConfigPortal("GarajeDoorSetup")) { // SSID 
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    ESP.reset();
    delay(5000);
    }
  Serial.print("connected to ");
  Serial.println(WiFi.SSID());
  EEPROM.commit();
  delay(2000);
}
else
{
 Serial.println("WiFi connected");                       
}

}
delay(200);
estado_final_carrera_abierta = digitalRead(final_carrera_abierta);
estado_final_carrera_cerrada = digitalRead(final_carrera_cerrada);

if ( estado_final_carrera_abierta == HIGH )
{
  puerta_abierta =HIGH;
  Serial.println("La puerta esta abierta");
  client.publish("garajedooropener","abierta");
}

if ( estado_final_carrera_cerrada == HIGH )
{
  puerta_cerrada =HIGH;
  Serial.println("La puerta esta cerrada");
  client.publish("garajedooropener","cerrada");
}

if (puerta_abierta==LOW && puerta_cerrada==LOW)
{
  Serial.println("PUERTA EN MALA SITUACION");
  Serial.println("SISTEMA DETENIDO.....");
  Serial.println("COMPRUEBE LA SITUACION DE LA PUERTA");
  averia=HIGH;
}
}

void loop() {
if (averia == LOW)
{
estado_final_carrera_abierta = digitalRead(final_carrera_abierta);
estado_final_carrera_cerrada = digitalRead(final_carrera_cerrada);
estado_seguridad = digitalRead(seguridad);
estado_pulsador_maniobra = digitalRead(pulsador_maniobra);

if (estado_pulsador_maniobra == LOW && estado_seguridad==HIGH)
{
  if (puerta_cerrada==HIGH)
  {
    Serial.println("Abrir puerta");
    abrir_puerta();
  }
  else if (puerta_abierta==HIGH)
  {
    Serial.println("Cerrar puerta");
    cerrar_puerta();
  }
  
}
delay(10);

}
delay(10);

if (WiFi.status() == WL_CONNECTED) {
if (!client.connected()) {
if (client.connect("ESP8266: garajedooropener")) {
  client.publish("outTopic",(String)"hello world, I'm "+host);
  client.subscribe(host+(String)"/#");
}
}
if (client.connected())

client.loop();
}
}

// *********************************
// CODIGO PARA SISTEMA DE DOS RELES
// *********************************

void abrir_puerta()
{
  client.publish("garajedooropener","abriendo");
  estado_final_carrera_abierta = digitalRead(final_carrera_abierta);
  while ( estado_final_carrera_abierta == LOW )
  {
    estado_final_carrera_abierta = digitalRead(final_carrera_abierta);
    digitalWrite (subir, HIGH);
    delay(10);
  }
  digitalWrite (subir,LOW);
  Serial.println ("PUERTA ABIERTA");
  client.publish("garajedooropener","abierta");
  puerta_abierta=HIGH;
  puerta_cerrada=LOW;
}

void cerrar_puerta()
{
  client.publish("garajedooropener","cerrando");
  boolean salir=LOW ;
  while (estado_final_carrera_cerrada== LOW and estado_seguridad==HIGH)
  {
    estado_final_carrera_cerrada = digitalRead(final_carrera_cerrada);
    estado_seguridad= digitalRead(seguridad);
    if (estado_seguridad==LOW)
    {
      salir=HIGH;
    }
    digitalWrite (bajar, HIGH);
    delay(10);
  }
  digitalWrite (bajar, LOW);
  
  if (salir == HIGH )
  {
    Serial.println("SEGURIDAD ACTIVADA");
    Serial.println("ABRIENDO PUERTA");
    abrir_puerta(); 
  }
  else
  {
  Serial.println("PUERTA CERRADA");
  client.publish("garajedooropener","cerrada");
  puerta_cerrada=HIGH;
  puerta_abierta=LOW;
  }
}

// *********************************
// CODIGO PARA SISTEMA DE UN RELE
// *********************************
/*
void abrir_puerta()
{
  client.publish("garajedooropener","abriendo");
  estado_final_carrera_abierta = digitalRead(final_carrera_abierta);
  digitalWrite (subir, HIGH);
  delay(500);
  digitalWrite (subir, LOW);
  while ( estado_final_carrera_abierta == LOW )
  {
    estado_final_carrera_abierta = digitalRead(final_carrera_abierta);
    delay(10);
  }
  Serial.println ("PUERTA ABIERTA");
  client.publish("garajedooropener","abierta");
  puerta_abierta=HIGH;
  puerta_cerrada=LOW;
}

void cerrar_puerta()
{
  client.publish("garajedooropener","cerrando");
  boolean salir=LOW ;
  digitalWrite (subir, HIGH);
  delay(500);
  digitalWrite (subir, LOW);
  while (estado_final_carrera_cerrada== LOW and estado_seguridad==HIGH)
  {
    estado_final_carrera_cerrada = digitalRead(final_carrera_cerrada);
    estado_seguridad= digitalRead(seguridad);
    if (estado_seguridad==LOW)
    {
      salir=HIGH;
    }
    delay(10);
  }
  
  if (salir == HIGH )
  {
    Serial.println("SEGURIDAD ACTIVADA");
    Serial.println("ABRIENDO PUERTA");
    digitalWrite (subir, HIGH);
    delay(500);
    digitalWrite (subir, LOW);
    abrir_puerta(); 
  }
  else
  {
  Serial.println("PUERTA CERRADA");
  client.publish("garajedooropener","cerrada");
  puerta_cerrada=HIGH;
  puerta_abierta=LOW;
  }
}
*/
