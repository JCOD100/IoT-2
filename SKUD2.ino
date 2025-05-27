// 0 - за заводом
// 1 - внутри завода
// SDA - 8
// SCL - 9
// используемые библиотеки
#include <WiFi.h>
// для rfid 
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>
#include <string.h>
// для mq3
#include <Adafruit_ADS1X15.h>

// для сервопривода
#include <ESP32Servo.h>
Servo sg90;
#define PIN_SG90 5 // Output pin used
int timer = millis(); // счетчик
WiFiClient client;

// метка RFID
// пин прерывания
#define PN532_IRQ 35
#define I2C_SDA 8
#define I2C_SCL 9
// TwoWire I2CBME = TwoWire(0);
// создаём объект для работы со сканером
Adafruit_PN532 nfc(PN532_IRQ, 3);

bool Serverconnect() {
  // Настройки сервера
  const char* host = "192.168.43.23";        // Адрес сервера (или IP)
  const uint16_t port = 1234;             // Порт сервера
 // Подключаемся к серверу
  Serial.print("Connecting to ");
  Serial.println(host);
  while (!client.connect(host, port)) {
    Serial.println("Connection failed");
    Serial.println("Waiting 5 seconds...");
    delay(5000);
    // return;
  }
  return 1;
}

bool check_card(String rfid) {
    Serverconnect();
    Serial.println("Check Card");
    String request = "SELECT * FROM people WHERE rfid = '" + rfid + "';";
    client.print(request);
    unsigned long timeout = millis();
    while (client.available() == 0) {
      if (millis() - timeout > 5000) {
        Serial.println(">>> Client Timeout !");
        timeout = millis();
      }
    }
    String response = "";
    while (client.available() != 0) {
      char c = client.read();
      response += String(c);
      if (c == '\0') {
        break;
      }
    }
    Serial.println(response);
    client.stop();
    if (response == "NO RESULTS FIND"){
      return 0;
    }
    else{
      return 1;
    }
}

bool edit_state(String status, String rfid) {
 Serverconnect();
    Serial.println("alc");
    String request = "UPDATE people SET status = '" + status + "' WHERE rfid = '" + rfid + "';";
    client.print(request);

    unsigned long timeout = millis();
    while (client.available() == 0) {
      if (millis() - timeout > 5000) {
        Serial.println(">>> Client Timeout !");
        timeout = millis();
      }
    }
    String response = "";
    while (client.available() != 0) {
      char c = client.read();
      response += String(c);
      if (c == '\0') {
        break;
      }
    }
    Serial.println(response);
    client.stop();
    if (response == "UPDATE SUCC"){
      Serial.println("Данные обновлены успешно");
      return 1;
    }
    else{
      Serial.println("Ошибка обновления данных на сервере");
      return 0;
    }

}

int get_in_out(String data) {
  int k = 0;
  for (char x: data) {
    if (k == 4) {
      return  x - '0';
    }
    if (x =='|') {
      k += 1;
    }
  }
}

bool check_in_out(String rfid) {
  Serverconnect();
    Serial.println("Check in/out");
    String request = "SELECT * FROM people WHERE rfid = '" + rfid + "';";
    client.print(request);
    unsigned long timeout = millis();
    while (client.available() == 0) {
      if (millis() - timeout > 5000) {
        Serial.println(">>> Client Timeout !");
        timeout = millis();
      }
    }
    String response = "";
    while (client.available() != 0) {
      char c = client.read();
      response += String(c);
      if (c == '\0') {
        break;
      }
    }
    Serial.println(response);
    client.stop();
    int in_out = get_in_out(response);
    if (in_out == 0){
      return 0;
    }
    else {
      return 1;
    }
}

bool edit_in_out(int in_out, String rfid) {
 Serverconnect();
    String request = "UPDATE people SET in_out = " + String(in_out) + " WHERE rfid = '" + rfid + "';";
    client.print(request);
    unsigned long timeout = millis();
    while (client.available() == 0) {
      if (millis() - timeout > 5000) {
        Serial.println(">>> Client Timeout !");
        timeout = millis();
      }
    }
    String response = "";
    while (client.available() != 0) {
      char c = client.read();
      response += String(c);
      if (c == '\0') {
        break;
      }
    }
    Serial.println(response);
    client.stop();
    if (response == "UPDATE SUCC"){
      Serial.println("Данные обновлены успешно");
      return 1;
    }
    else{
      Serial.println("Ошибка обновления данных на сервере");
      return 0;
    }
}

String intToHex(int num) {
    if (num == 0) return "0";
    String hexChars = "0123456789ABCDEF";
    String result;
    while (num > 0) {
        result = hexChars[num % 16] + result;
        num /= 16;
    }
    return result;
}

// функция для считывания метки
bool get_RFID(String &rfid) {
    // переменные для RFID-метки
    uint8_t success;
    // буфер для хранения ID карты
    uint8_t uid[8] = {0};
    // размер буфера карты
    uint8_t uidLength;
    // слушаем новые метки
    nfc.begin();
    nfc.SAMConfig();
    success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
    // если найдена карта
    if (success) {
        // выводим в консоль полученные данные
        Serial.println("Found a card");
        Serial.print("ID Length: ");
        Serial.print(uidLength, DEC);
        Serial.println(" bytes");
        Serial.print("ID Value: ");
        nfc.PrintHex(uid, uidLength);
        Serial.println("");
        String str_uid;
        for (int i = 0; i < uidLength; i++) {
          str_uid = str_uid +  intToHex(uid[i]);
        }
        if (check_card(str_uid)) {
          rfid = str_uid;
          return true;
        } else {
          return false;
        }
    } 
    else {
        return false;
    }
}

// mq3
Adafruit_ADS1115 ads;
const float V_REF = 5.0;  // Напряжение питания датчика
const float R_LOAD = 10.0; // Сопротивление нагрузки (кОм)
// Калибровочные параметры для MQ-3
const float RO_CLEAN_AIR = 6.0;  // Типичное значение для MQ-3 в чистом воздухе
// const float ALCOHOL_CURVE[2] = {0.4, -1.5};  // Оптимизированные параметры для MQ-3
const float ALCOHOL_CURVE[2] = {-0.66, -0.62};
float readMQ3Resistance() {
    int16_t adcValue = ads.readADC_SingleEnded(0);
    if (adcValue <= 0) adcValue = 1;  // Защита от деления на 0
    float voltage = (adcValue * V_REF) / 32767.0;
    float rs = (V_REF - voltage) / voltage * R_LOAD;
    return rs;
}

float calculateAlcoholConcentration(float rs_ro_ratio) {
    // Логарифмическая модель для MQ-3
    float log_ratio = log10(rs_ro_ratio);
    float log_ppm = ALCOHOL_CURVE[0] + ALCOHOL_CURVE[1] * log_ratio;
    float ppm = pow(10, log_ppm);
    
    // Конвертация ppm в мг/л (для этанола)
    float mg_l = ppm * 0.00183 * 1.25;  // 1.25 - поправочный коэффициент
    return mg_l > 0 ? mg_l : 0;  // Не возвращаем отрицательные значения
}

void onRed() {
    digitalWrite(38, 1); // выключение красного светодиода
    digitalWrite(37, 0);
    digitalWrite(36, 0);
}

void onYellow() {
    digitalWrite(38, 0); 
    digitalWrite(37, 0);
    digitalWrite(36, 1);
}

void onGreen() {
    digitalWrite(38, 0); 
    digitalWrite(37, 1);
    digitalWrite(36, 0); 
}

void openDoor(){
  for (int pos = 0; pos <= 90; pos += 1) {
      sg90.write(pos);
  }
  delay(5000);
  for (int pos = 90; pos >= 0; pos -= 1) {
      sg90.write(pos);
      delay(5);
  }
}

// функция, получающая значение с датчиков
int isDrunk() {
    // MQ135
    int c = analogRead(A6); // хранение изначального значения MQ135
    int b = 0;
    int a = 0;
    int i = 0;
    while (i != 20) {
        if (millis() - timer > 500) {
            // MQ3
            float rs = readMQ3Resistance();
            float ro_ratio = rs / RO_CLEAN_AIR;
            float alcohol_mg_l = calculateAlcoholConcentration(ro_ratio);
            Serial.println(alcohol_mg_l,5);
            // MQ135
            b = analogRead(A6);
            if (c - b >= 50){
              // 1 случай - если дунул и не пьян
              if (alcohol_mg_l < 0.02){ 
                return 1;
              }
              // 2 случай - если дунул и пьян
              else {                        
                return 2;
              }
            }
            timer = millis();
            i++;
        }
    }
    return 3; 
}
bool WiFiconnect() {
// Настройки WiFi
// const char* ssid = "Redmi";          // Имя вашей WiFi сети
// const char* password = "12345678";  // Пароль от WiFi
const char* ssid = "HONOR";          // Имя вашей WiFi сети
const char* password = "ytrewq321";  // Пароль от WiFi
// Подключаемся к WiFi
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  return 1;
}

void setup() {
  pinMode(38, OUTPUT); // красный светодиод 
  pinMode(36, OUTPUT); // желтый светодиод
  pinMode(37, OUTPUT); // зеленый светодиод
  Serial.begin(115200);
esp_log_level_set("*", ESP_LOG_VERBOSE);  // Максимальный уровень логов
  // подключение i2c
  Wire.begin(I2C_SDA, I2C_SCL);
  // RFID
  // инициализация RFID/NFC сканера
  int versiondata = nfc.getFirmwareVersion();
  if (!versiondata) {
      Serial.print("Didn't find RFID/NFC reader");
  }
  else {
    Serial.println("Found RFID/NFC reader");
  }
  // настраиваем модуль
  nfc.SAMConfig();
  // MQ3
  if (!ads.begin()) {
      Serial.println("Failed to initialize ADS1115!");
      while(1);
  }
  ads.setGain(GAIN_ONE);  // Усиление 1x (±4.096V)
  sg90.setPeriodHertz(50); // PWM frequency for SG90
  sg90.attach(PIN_SG90, 500, 2400); // Minimum and maximum pulse width (in µs) to go from 0° to 180
  sg90.write(0);
  WiFiconnect();
}

void loop() {
    if (millis() - timer > 1000) {
        timer = millis();
        onRed();
        Serial.println("Waiting for a card ...");
        String current_rfid = "";
        if (get_RFID(current_rfid)) {
          if (check_in_out(current_rfid) == 0) {
               onYellow();
            int a = isDrunk();
            // Не пьян
            if (a == 1) {
                Serial.println("Проход открыт");
                edit_state("SOBER", current_rfid);
                edit_in_out(1, current_rfid);
                onGreen();
                openDoor();
                onRed();
            }
            // Пьян
            else if (a == 2) {
                Serial.println("Проход закрыт, пьян");
                edit_state("DRUNK", current_rfid);
                onRed();
            } 
            // Время вышло
            else {
                onRed();
            }
          } else {
              Serial.println("Проход открыт");
                onGreen();
                openDoor();
                edit_in_out(0, current_rfid);
                onRed();
          }
        }
    }
}