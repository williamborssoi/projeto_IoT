#include "BluetoothSerial.h"
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it 
#endif
BluetoothSerial SerialBT;
// Se tiver um sensor DHT ou nenhum sensor descomente a linha 9 
// Se tiver um sensor LM-35 comente a linha 9
#define __SENSOR_DHT__ // Configura sensor a ser utilizado no experimento 
#include <DHT.h>
#define DHTPIN 13        // Pino no qual o sensor está conectado 
#define DHTTYPE DHT11     // Define tipo do sensor
DHT dht(DHTPIN, DHTTYPE); // Instancia objeto DHT 
float temperatura = 0;
int umidade = 0;
const int analogIn = A3;  // Pino para leitura do sensor LM-35 
#define PINO_LED 2
#define PINO_RELE 12 
unsigned long lastMsg = 0; 
void setup() {
pinMode(PINO_LED, OUTPUT); 
pinMode(PINO_RELE, OUTPUT);
Serial.begin(115200);
dht.begin();   // Inicializa sensor de temperatura/umidade 
SerialBT.begin("ESP32-ADS"); // Nome do dispositivo Bluetooth
Serial.println("Dispositivo iniciou, prossiga com o pareamento bluetooth!"); 
}
void mostraDados(float temperatura, int umidade, boolean mostraUmidade=true) { 
Serial.printf("Temperatura: %.2f °C\n", temperatura);
// Envia dados via Bluetooth 
SerialBT.print(temperatura); 
SerialBT.print("|");
if (mostraUmidade) {
Serial.printf("Umidade: %d%%\n", umidade);
// Envia dados via Bluetooth 
SerialBT.print(umidade); 
SerialBT.print("|");
}
Serial.println(); 
}
void medirTemperaturaUmidade() {
#ifdef __SENSOR_DHT__  // Compilação condicional 
temperatura = dht.readTemperature(false); 
umidade = dht.readHumidity();
if (isnan(temperatura) || isnan(umidade)) { 
Serial.println("Erro de leitura"); 
temperatura = random(0, 45);
umidade = random(0, 100); 
}
#else
//  Sensor LM-35int RawValue = 0; 
double Voltage = 0;
RawValue = analogRead(analogIn); 
// 3300 se ligado em 3,3V
Voltage = (RawValue / 4096.0) * 5000; 
temperatura = Voltage * 0.1;
#endif
mostraDados(temperatura, umidade, umidade); 
}
void loop() {
// Faz 'pausa' não bloqueante para ler o sensor 
// somente a cada 2 segundos
unsigned long now = millis(); 
if (now - lastMsg > 2000) {
lastMsg = now;
medirTemperaturaUmidade(); 
}
if (Serial.available()) { 
SerialBT.write(Serial.read());
}
if (SerialBT.available()) {
char caractereBT = SerialBT.read(); 
if (caractereBT == 'D') {
digitalWrite(PINO_LED, LOW); 
digitalWrite(PINO_RELE, HIGH);
Serial.print("Comando <DESLIGAR> recebido: "); 
Serial.println(caractereBT);
} else if (caractereBT == 'L') { 
digitalWrite(PINO_LED, HIGH); 
digitalWrite(PINO_RELE, LOW);
Serial.print("Comando <LIGAR> recebido: "); 
Serial.println(caractereBT);
}
}
delay(20); 
}
