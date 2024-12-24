#include <WiFi.h>
#include <ESPAsyncWebServer.h>


// Статические IP настройки
IPAddress local_IP(192, 168, 3, 213); // Задайте статический IP
IPAddress gateway(192, 168, 3, 100);    // Шлюз
IPAddress subnet(255, 255, 255, 0);  // Маска подсети

// Параметры подключения к Wi-Fi
const char* ssid = "TP-LINK_nio2";
const char* password = "k5219k5219";

// Пины
const int relayPin = 12; // Пин для реле

// Время работы
int startHour = 9;
int stopHour = 11;

// Веб-сервер
AsyncWebServer server(80);

void setup() {
  Serial.begin(115200);
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, HIGH); // Отключить реле при запуске

  // Настройка Wi-Fi с использованием статического IP
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("Ошибка настройки статического IP!");
  }

  WiFi.begin(ssid, password);
  Serial.print("Подключение к Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi подключен.");
  Serial.print("IP адрес: ");
  Serial.println(WiFi.localIP());

  // Настройка веб-сервера
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    String html = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Управление гирляндой</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      margin: 0;
      padding: 0;
      display: flex;
      justify-content: center;
      align-items: center;
      min-height: 100vh;
      background-color: #f4f4f9;
      color: #333;
    }
    .container {
      max-width: 400px;
      width: 90%;
      background: #fff;
      padding: 20px;
      box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);
      border-radius: 10px;
    }
    h1 {
      font-size: 1.5rem;
      margin-bottom: 20px;
      text-align: center;
    }
    p {
      font-size: 1rem;
      margin-bottom: 10px;
    }
    form {
      display: flex;
      flex-direction: column;
    }
    input[type="number"],
    input[type="submit"] {
      padding: 10px;
      margin-bottom: 15px;
      border: 1px solid #ccc;
      border-radius: 5px;
      font-size: 1rem;
    }
    input[type="submit"] {
      background: #007bff;
      color: #fff;
      cursor: pointer;
      border: none;
    }
    input[type="submit"]:hover {
      background: #0056b3;
    }
    .footer {
      text-align: center;
      margin-top: 15px;
      font-size: 0.9rem;
      color: #777;
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>Управление гирляндой</h1>
    <p>Статус Wi-Fi: <strong>)rawliteral" + String(WiFi.status() == WL_CONNECTED ? "Подключено" : "Отключено") + R"rawliteral(</strong></p>
    <p>IP адрес: <strong>)rawliteral" + WiFi.localIP().toString() + R"rawliteral(</strong></p>
    <p>Текущее время включения: <strong>)rawliteral" + String(startHour) + R"rawliteral(:00</strong></p>
    <p>Текущее время выключения: <strong>)rawliteral" + String(stopHour) + R"rawliteral(:00</strong></p>
    <form action="/set" method="POST">
      <label>Время включения (час):</label>
      <input type="number" name="startHour" value=")rawliteral" + String(startHour) + R"rawliteral(" min="0" max="23">
      <label>Время выключения (час):</label>
      <input type="number" name="stopHour" value=")rawliteral" + String(stopHour) + R"rawliteral(" min="0" max="23">
      <input type="submit" value="Сохранить">
    </form>
    <div class="footer">
      <p>ESP32 Controller © 2024</p>
    </div>
  </div>
</body>
</html>
    )rawliteral";
    request->send(200, "text/html", html);
  });

  server.on("/set", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("startHour", true) && request->hasParam("stopHour", true)) {
      startHour = request->getParam("startHour", true)->value().toInt();
      stopHour = request->getParam("stopHour", true)->value().toInt();
    }
    request->send(200, "text/plain", "Параметры сохранены. Перезагрузите страницу.");
  });

  server.begin();
  Serial.println("Сервер запущен.");
}

void loop() {
  // Проверка времени и управление реле
  time_t now = time(nullptr);
  struct tm *timeinfo = localtime(&now);
  int currentHour = timeinfo->tm_hour;

  if (currentHour >= startHour && currentHour < stopHour) {
    digitalWrite(relayPin, LOW); // Включить реле
  } else {
    digitalWrite(relayPin, HIGH); // Отключить реле
  }
}
