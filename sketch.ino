#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <LiquidCrystal_I2C.h>

// ----- Definições de pinos -----
#define Vled 16
#define Aled 17
#define Cled 18
#define Lled 19

#define DHTPIN 23
#define DHTTYPE DHT22

#define DHTPIN 23
#define DHTTYPE DHT22


#define BTN_MODE 35

#define LED_TEST_BTN 14




// ----- Wi-Fi -----
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// ----- OpenWeather -----
const char* apiKey = "a8f51c73830ef7f951c060014fb0837b"; 
const char* city = "Sao%20Paulo"; 
const char* countryCode = "BR"; 
const char* units = "metric";

// ----- MQTT -----
const char* mqtt_server = "test.mosquitto.org";
const int mqtt_port = 1883;
const char* mqtt_user = "";
const char* mqtt_password = "";

const char* mqtt_topic_temp = "flood/sensors/data/device001/temp";
const char* mqtt_topic_hum = "flood/sensors/data/device001/humidity";
const char* mqtt_topic_alert = "flood/alerts/status/device001";

// ----- Objetos -----
DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);
WiFiClient espClient;
PubSubClient client(espClient);

unsigned long previousMillis = 0;
const long interval = 60000;

unsigned long lastMsg = 0;
const long intervalClima = 60000;  // Intervalo para chamada da API
const long intervalDHT = 2000;

float temperature = 0.0;
float humidity = 0.0;
int rain_intensity = 0;
bool emergency = false;
const int RAIN_INTENSITY_ALERT = 500;

// ----- Funções -----
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensagem recebida em ");
  Serial.print(topic);
  Serial.print(": ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Conectando a ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi conectado");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Conectando ao MQTT...");
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_password)) {
      Serial.println("conectado");
    } else {
      Serial.print("falhou, rc=");
      Serial.print(client.state());
      Serial.println(" tentando novamente em 5 segundos");
      delay(5000);
    }
  }
}

void verificarClima() {
  if (WiFi.status() == WL_CONNECTED) {
    String url = "http://api.openweathermap.org/data/2.5/weather?q=" + String(city) + "," + String(countryCode) + "&appid=" + String(apiKey) + "&units=" + String(units);
    Serial.println("Requisitando: " + url);
    
    HTTPClient http;
    http.begin(url);
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
      String payload = http.getString();
      Serial.println("Resposta: ");
      Serial.println(payload);

      DynamicJsonDocument doc(2048);
      DeserializationError error = deserializeJson(doc, payload);

      if (!error) {
        float temp = doc["main"]["temp"];
        int humidity = doc["main"]["humidity"];
        const char* mainWeather = doc["weather"][0]["main"];
        float windSpeed = doc["wind"]["speed"];
        float rain1h = doc["rain"]["1h"] | 0; 

        // MQTT
        char tempStr[8];
        dtostrf(temp, 1, 2, tempStr);
        client.publish(mqtt_topic_temp, tempStr);

        char humStr[8];
        dtostrf(humidity, 1, 0, humStr);
        client.publish(mqtt_topic_hum, humStr);

        // LCD
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Temp: ");
        lcd.print(temp);
        lcd.print("C");
        lcd.setCursor(0, 1);
        lcd.print("Umid: ");
        lcd.print(humidity);
        lcd.print("%");

        // Alertas e LEDs
        bool alerta = false;

        if (temp >= 35) {
          digitalWrite(Aled, HIGH);
          alerta = true;
          client.publish(mqtt_topic_alert, "Alerta: Onda de calor!");
        } else {
          digitalWrite(Aled, LOW);
        }

        if (String(mainWeather).indexOf("Thunderstorm") >= 0) {
          digitalWrite(Cled, HIGH);
          alerta = true;
          client.publish(mqtt_topic_alert, "Alerta: Tempestade!");
        } else {
          digitalWrite(Cled, LOW);
        }

        if (windSpeed >= 10.0) {
          digitalWrite(Lled, HIGH);
          alerta = true;
          client.publish(mqtt_topic_alert, "Alerta: Ventos fortes!");
        } else {
          digitalWrite(Lled, LOW);
        }

        if (rain1h >= 10) {
          digitalWrite(Vled, HIGH);
          alerta = true;
          client.publish(mqtt_topic_alert, "Alerta: Chuva intensa!");
        } else {
          digitalWrite(Vled, LOW);
        }

        

      } else {
        Serial.print("Erro ao parsear JSON: ");
        Serial.println(error.c_str());
      }
    } else {
      Serial.print("Erro na requisição: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  }
}

void readSensors() {
  // Lê temperatura e umidade local via DHT22
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();


  // Define estado de emergência local com base apenas na chuva intensa
  emergency = (rain_intensity > RAIN_INTENSITY_ALERT);

  // Mostra dados locais no monitor serial
  Serial.println("Sensores locais:");
  Serial.print("Temperatura: ");
  Serial.print(temperature);
  Serial.println(" C");

  Serial.print("Umidade: ");
  Serial.print(humidity);
  Serial.println(" %");

  Serial.print("Intensidade de chuva (analógico): ");
  Serial.println(rain_intensity);
}

void testarLEDs() {
  digitalWrite(Vled, HIGH);
  digitalWrite(Aled, HIGH);
  digitalWrite(Cled, HIGH);
  digitalWrite(Lled, HIGH);
  delay(1000);
  digitalWrite(Vled, LOW);
  digitalWrite(Aled, LOW);
  digitalWrite(Cled, LOW);
  digitalWrite(Lled, LOW);
}





void setup() {
  Serial.begin(115200);

  pinMode(LED_TEST_BTN, INPUT_PULLUP);

  // Pinos
  pinMode(Vled, OUTPUT);
  pinMode(Aled, OUTPUT);
  pinMode(Cled, OUTPUT);
  pinMode(Lled, OUTPUT);

  pinMode(LED_TEST_BTN, INPUT_PULLUP);

  pinMode(BTN_MODE, INPUT_PULLUP);

  // Sensores
  dht.begin();
  delay(2000);
  
  // Inicialização do LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.print("Iniciando...");



  // Wi-Fi e MQTT
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if (digitalRead(LED_TEST_BTN) == LOW) {
      Serial.println("Botão de teste de LEDs pressionado.");
      testarLEDs();
      delay(500);
    }

  unsigned long now = millis();

  // Envio dos dados dos sensores locais
  if (now - lastMsg > intervalDHT) {
    lastMsg = now;

    readSensors();  // Leitura de todos os sensores

      float t = dht.readTemperature();
      float h = dht.readHumidity();

      if (!isnan(t) && !isnan(h)) {
        Serial.print("Temp: ");
        Serial.println(t);
        Serial.print("Hum: ");
        Serial.println(h);
      } else {
        Serial.println("Erro ao ler o DHT22");
      }

  }

  // Verificação com a API do OpenWeather a cada 1 minuto
  if (now - previousMillis >= intervalClima) {
    previousMillis = now;
    verificarClima();
  }
}

