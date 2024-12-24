#include <WiFi.h>
#include <EEPROM.h>
#include <WebServer.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// Настройки Wi-Fi
const char* ssid = "TuRbiNa5";
const char* password = "cfvfhwtd";
bool gir_on=false;
// Статические IP настройки
IPAddress local_IP(192, 168, 0, 113); // Задайте статический IP
IPAddress gateway(192, 168, 0, 1);    // Шлюз
IPAddress subnet(255, 255, 255, 0);  // Маска подсети
IPAddress primaryDNS(192, 168, 0, 1);    // DNS-сервер (по желанию)
IPAddress secondaryDNS(8, 8, 4, 4);  // Резервный DNS (по желанию)

// Пины
#define RELAY_PIN_1 12  // Пин реле
#define RELAY_PIN_2 33  // Пин реле
// Глобальные переменные
int startHour = 18; // Час включения по умолчанию
int startMinute = 0; // Минута включения по умолчанию
int stopHour = 23; // Час выключения по умолчанию
int stopMinute = 0; // Минута выключения по умолчанию

// Для работы с временем
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 3 * 3600, 3600000); // Часовой пояс +3 и обновление раз в час

// Веб-сервер
WebServer server(80);

// Сохранение параметров в EEPROM
void saveSettings() {
  EEPROM.write(0, startHour);
  EEPROM.write(1, startMinute);
  EEPROM.write(2, stopHour);
  EEPROM.write(3, stopMinute);
  EEPROM.commit();
}

// Загрузка параметров из EEPROM
void loadSettings() {
  startHour = EEPROM.read(0);
  startMinute = EEPROM.read(1);
  stopHour = EEPROM.read(2);
  stopMinute = EEPROM.read(3);
}

// Функция проверки времени для включения/выключения гирлянды
void controlRelay() {
  int currentHour = timeClient.getHours();
  int currentMinute = timeClient.getMinutes();

  // Проверка времени
  if ((currentHour > startHour || (currentHour == startHour && currentMinute >= startMinute)) &&
      (currentHour < stopHour || (currentHour == stopHour && currentMinute < stopMinute))) {
    digitalWrite(RELAY_PIN_1, LOW); // Включить реле
     digitalWrite(RELAY_PIN_2, LOW); // Включить реле
    gir_on=true;
  } else {
    digitalWrite(RELAY_PIN_1, HIGH); // Выключить реле
     digitalWrite(RELAY_PIN_2, HIGH); // Включить реле
    gir_on=false;
  }
}

// Веб-страница для управления
String generateHTMLPage() {
  String page = "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Управление гирляндой</title>";
  page += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  page += "<style>";
  page += "body { font-family: Arial, sans-serif; text-align: center; padding: 20px; background: #f3f4f6; color: #333; }";
  page += "h1 { color: #007BFF; }";
  page += "form { margin: 20px auto; width: 90%; max-width: 400px; padding: 20px; background: #fff; border-radius: 10px; box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1); }";
  page += "label { display: block; margin-bottom: 10px; font-weight: bold; }";
  page += "input { width: 100%; padding: 10px; margin: 10px 0; border: 1px solid #ccc; border-radius: 5px; }";
  page += "input[type='submit'] { background: #007BFF; color: white; border: none; cursor: pointer; }";
  page += "input[type='submit']:hover { background: #0056b3; }";
  page += ".info { margin-top: 20px; padding: 10px; background: #e9ecef; border-radius: 5px; }";
  page += "</style></head><body>";
  page += "<h1>Настройки гирлянды</h1>";
  page += "<form action='/set' method='POST'>";
  page += "<label>Время включения:</label>";
  page += "<input type='time' name='startTime' value='" + String(startHour < 10 ? "0" : "") + String(startHour) + ":" + String(startMinute < 10 ? "0" : "") + String(startMinute) + "'>";
  page += "<label>Время выключения:</label>";
  page += "<input type='time' name='stopTime' value='" + String(stopHour < 10 ? "0" : "") + String(stopHour) + ":" + String(stopMinute < 10 ? "0" : "") + String(stopMinute) + "'>";
  page += "<input type='submit' value='Сохранить'>";
  page += "</form>";
  page += "<div class='info'>";
  page += "<p><b>Статус Wi-Fi:</b> " + String(WiFi.isConnected() ? "Подключено" : "Не подключено") + "</p>";
  page += "<p><b>Статус Гирлянды:</b> " + String(gir_on ? "Включена" : "Выключена") + "</p>";
  page += "<p><b>IP адрес:</b> " + WiFi.localIP().toString() + "</p>";
  page += "<p><b>Текущее время:</b> " + timeClient.getFormattedTime() + "</p>";
  page += "</div></body></html>";
  return page;
}

// Обработчики запросов
void handleRoot() {
  server.send(200, "text/html", generateHTMLPage());
}

void handleSet() {
  if (server.hasArg("startTime") && server.hasArg("stopTime")) {
    String startTime = server.arg("startTime");
    String stopTime = server.arg("stopTime");

    // Разбиваем строку времени
    startHour = startTime.substring(0, 2).toInt();
    startMinute = startTime.substring(3, 5).toInt();
    stopHour = stopTime.substring(0, 2).toInt();
    stopMinute = stopTime.substring(3, 5).toInt();

    // Сохраняем настройки
    saveSettings();
  }
  server.sendHeader("Location", "/");
  server.send(303);
}

void setup() {
  Serial.begin(115200);

  // Настройка EEPROM
  EEPROM.begin(4);
  loadSettings();

  // Настройка пина реле
  pinMode(RELAY_PIN_1, OUTPUT);
  digitalWrite(RELAY_PIN_1, HIGH); // Реле выключено по умолчанию
 pinMode(RELAY_PIN_2, OUTPUT);
  digitalWrite(RELAY_PIN_2, HIGH); // Реле выключено по умолчанию



if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("Ошибка настройки статического IP!");
  }

  // Подключение к Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Подключение к Wi-Fi...");
  }
  Serial.println("Wi-Fi подключен");
  Serial.println("IP адрес: " + WiFi.localIP().toString());

  // Синхронизация времени через NTP
  timeClient.begin();
  timeClient.update();

  // Настройка веб-сервера
  server.on("/", handleRoot);
  server.on("/set", handleSet);
  server.begin();
}

void loop() {
  timeClient.update(); // Обновляем текущее время
  controlRelay(); // Проверяем, нужно ли включить/выключить гирлянду
  server.handleClient(); // Обрабатываем запросы
  delay(1000);
}
