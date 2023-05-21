#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>

// Настройки Wi-Fi сети
const char* ssid = "sloth";
const char* password = "bytnthrowdbab0";

// Настройки удаленного сервера
const char* serverHost = "http://tcpbin.com";
const int serverPort = 4242;

// Создание экземпляра веб-сервера
ESP8266WebServer server(80);

const int bufferSize = 5000; // Размер буфера для хранения данных, полученных через Serial
char serialData[bufferSize]; // Буфер для хранения данных, полученных через Serial
int serialReadIndex = 0; // Индекс для чтения данных из буфера
int serialWriteIndex = 0; // Индекс для записи данных в буфер

void handleRoot() {
    String html = "<html><head>";
    html += "<style>";
    html += "body { font-family: Arial, sans-serif; }";
    html += "#data-container { background-color: #f9f9f9; border: 1px solid #ddd; padding: 10px; }";
    html += "</style>";
    html += "</head><body>";
    html += "<h1>Данные с ESP8266:</h1>";
    html += "<div id='data-container'></div>";
    html += "<script>";
    html += "setInterval(function() {";
    html += "  getData();";
    html += "}, 5000);"; // Обновление данных каждую секунду
    html += "function getData() {";
    html += "  var xhttp = new XMLHttpRequest();";
    html += "  xhttp.onreadystatechange = function() {";
    html += "    if (this.readyState == 4 && this.status == 200) {";
    html += "      document.getElementById('data-container').innerHTML = this.responseText;";
    html += "    }";
    html += "  };";
    html += "  xhttp.open('GET', '/data', true);";
    html += "  xhttp.send();";
    html += "}";
    html += "</script>";
    html += "</body></html>";
    server.send(200, "text/html", html);
}

void handleData() {
    String data;
    int dataIndex = serialReadIndex;
    while (dataIndex != serialWriteIndex) {
        data += serialData[dataIndex];
        dataIndex = (dataIndex + 1) % bufferSize; // Получение следующего индекса в кольцевом буфере
    }

    String html = "<style>";
    html += "#data-container { background-color: #f9f9f9; border: 1px solid #ddd; padding: 10px; }";
    html += "</style>";
    html += "<div id='data-container'>" + data + "</div>";
    server.send(200, "text/html", html);
}

void sendDataToServer(const String& data) {
    WiFiClient client;
    if (client.connect(serverHost, serverPort)) {
        client.print("POST /endpoint HTTP/1.1\r\n");
        client.print("Host: ");
        client.print(serverHost);
        client.print("\r\n");
        client.print("Content-Type: application/x-www-form-urlencoded\r\n");
        client.print("Content-Length: ");
        client.print(data.length());
        client.print("\r\n\r\n");
        client.print(data);
        client.print("\r\n");
        client.flush();
        client.stop();
    }
}

void setup() {
    // Настройка последовательного порта для отладки
    Serial.begin(115200);

    // Подключение к Wi-Fi сети
    WiFi.begin(ssid, password);

    // Ожидание подключения к Wi-Fi сети
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Подключение к Wi-Fi...");
    }

    // Вывод информации о подключении
    Serial.println("");
    Serial.println("Wi-Fi подключено");
    Serial.print("IP адрес: ");
    Serial.println(WiFi.localIP());

    // Обработчик корневого URL-адреса
    server.on("/", handleRoot);

    // Обработчик URL-адреса для получения данных
    server.on("/data", handleData);

    // Запуск веб-сервера
    server.begin();
    Serial.println("Веб-сервер запущен");
}

void loop() {
    // Обработка клиентских запросов
    server.handleClient();

    // Проверка доступности данных через Serial
    while (Serial.available()) {
        char c = Serial.read();
        int nextWriteIndex = (serialWriteIndex + 1) % bufferSize; // Получение следующего индекса для записи в кольцевом буфере

        if (nextWriteIndex != serialReadIndex) {
            serialData[serialWriteIndex] = c;
            serialWriteIndex = nextWriteIndex;
        }
    }

    // Отправка данных на удаленный сервер
    String dataToSend;
    int dataIndex = serialReadIndex;
    while (dataIndex != serialWriteIndex) {
        dataToSend += serialData[dataIndex];
        dataIndex = (dataIndex + 1) % bufferSize; // Получение следующего индекса в кольцевом буфере
    }

   /* if (!dataToSend.isEmpty()) {
        sendDataToServer(dataToSend);
        serialReadIndex = serialWriteIndex;
    }*/
}
