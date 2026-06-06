#include <WiFi.h>
#include <WebServer.h>

// Configuração da rede Wi-Fi
const char* ssid = "SKYZHE1K_5G";
const char* password = "rHGMRSNqyFjY";

// LED onboard do ESP32-C3
#define LED_PIN 8

WebServer server(80);

// Página HTML
String paginaHTML() {
  return R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Controle LED ESP32-C3</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            text-align: center;
            margin-top: 50px;
            background-color: #f0f0f0;
        }

        button {
            width: 150px;
            height: 60px;
            font-size: 20px;
            margin: 10px;
            border: none;
            border-radius: 10px;
            cursor: pointer;
        }

        .on {
            background-color: green;
            color: white;
        }

        .off {
            background-color: red;
            color: white;
        }
    </style>
</head>
<body>
    <h1>Controle do LED ESP32-C3</h1>

    <button class="on" onclick="fetch('/on')">LIGAR</button>
    <button class="off" onclick="fetch('/off')">DESLIGAR</button>

</body>
</html>
)rawliteral";
}

void handleRoot() {
  server.send(200, "text/html", paginaHTML());
}

void handleLedOn() {
  digitalWrite(LED_PIN, LOW);   // LED ON (active-low)
  server.send(200, "text/plain", "LED Ligado");
}

void handleLedOff() {
  digitalWrite(LED_PIN, HIGH);  // LED OFF (active-low)
  server.send(200, "text/plain", "LED Desligado");
}

void setup() {

  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);

  // LED inicialmente apagado
  digitalWrite(LED_PIN, HIGH);

  // Nome que pode aparecer no roteador
  WiFi.setHostname("ESP32-C3-LED");

  // Conexão Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.println();
  Serial.print("Conectando ao WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("================================");
  Serial.println("WiFi conectado!");

  Serial.print("Hostname : ");
  Serial.println(WiFi.getHostname());

  Serial.print("IP       : ");
  Serial.println(WiFi.localIP());

  Serial.print("MAC      : ");
  Serial.println(WiFi.macAddress());

  Serial.print("URL      : http://");
  Serial.println(WiFi.localIP());

  Serial.println("================================");

  // Rotas HTTP
  server.on("/", handleRoot);
  server.on("/on", handleLedOn);
  server.on("/off", handleLedOff);

  server.begin();
  Serial.println("Servidor HTTP iniciado");
}

void loop() {
  server.handleClient();
}