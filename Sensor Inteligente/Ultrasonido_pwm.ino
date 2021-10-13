/*UNIVERSIDAD DISTRITAL FRANCISCO JOSE DE CALDAS
Codigo Para el acondicionamiento MQTT y la lectura del sensor inteligentes
Autores:
 David Alexander Saldarriaga Horta
 Cristian David Pinzón Malaver
*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Update these with values suitable for your network.

const char* ssid = "TIGO-EE1F";
const char* password = "2NB112103510";
const char* mqtt_server = "DESKTOP-08FMA90.local"; //broker
WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];

//IPAddress wifiIp(192, 168, 1, 2);    //Ip deñ nodemcu
//IPAddress wifiNet(255, 255, 255, 0);  //Mascara
//IPAddress wifiGW(192, 168, 1, 254);  //Router

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  //DHCP
 WiFi.mode(WIFI_STA);
  //WiFi.config(wifiIp,wifiGW, wifiNet);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("data", "Distancia_Botella");
      // ... and resubscribe
      client.subscribe("data");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(500);
    }
  }
}
//...........................................................
// Zonar
const byte ledPin = 13;
const int AnalogInPin = A0;

int pw_pin=14;
int arraysize = 9;
int array[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0};
long inch;
int exact_cm_value;
int Memoria_D,Distancia_Botella,a;
int value = 0;
float Distancia = 0;

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(9600);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
// pines
  pinMode(pw_pin, INPUT);
  pinMode(AnalogInPin, INPUT);
  pinMode(ledPin, OUTPUT);
}
void sensorRead(){
for(int i = 0; i < arraysize; i++)
{
inch = pulseIn(pw_pin, HIGH);
array[i] = inch/58;
delay(10);
}
}
void array_arrangment(int *a,int n){
// Author: Bill Gentles, Nov. 12, 2010)
  for (int i = 1; i < n; ++i){
    int j = a[i];
    int k;
    for (k = i - 1; (k >= 0) && (j < a[k]); k--){
      a[k + 1] = a[k];
    }
    a[k + 1] = j;
  }
}

int filter(int *a,int n){
int i = 0;
int count = 0;
int maxCount = 0;
int filter = 0;
int median;
int prevCount = 0;

while(i<(n-1)){
  prevCount=count;
  count=0;
while(a[i]==a[i+1]){
  count++;
  i++;
}
if(count>prevCount && count>maxCount){
  filter=a[i];
  maxCount=count;
  median=0;
}
if(count==0){
  i++;
}
if(count==maxCount){//If the dataset has 2 or more modes.
  median=1;
}
if(filter==0||median==1){//Return the median if there is no mode.
  filter=a[(n/2)];
}

return filter;

}
}

void loop() {
//Lecutra analoga
  value = analogRead(AnalogInPin);
  Serial.println("Distancia Zonar: ");
  Serial.println(value);
  delay(100);
//Lectura sensor
  sensorRead();
  array_arrangment(array,arraysize);
  exact_cm_value= filter(array,arraysize);
  Serial.print("La distancia = ");
  Serial.print(exact_cm_value);
  Serial.println(" cm ");
  delay(100);
  if(exact_cm_value >0 and exact_cm_value < 120){
    Memoria_D=exact_cm_value;
    Distancia_Botella=exact_cm_value;
    a=0;
  }
  else{
    if(a>=11){
      Distancia_Botella=exact_cm_value;
    }
    if(a<=10){
      Distancia_Botella=Memoria_D;
      a=a+1;

    }
    
  }
//Envio de datos por mqtt
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 200) {
    lastMsg = now;
    ++value;
    snprintf (msg, MSG_BUFFER_SIZE, "%ld", Distancia_Botella);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("data", msg);
  }
}
