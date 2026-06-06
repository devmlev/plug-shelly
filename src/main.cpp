#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

// Configuração da rede Wi-Fi (Sua rede 2.4GHz)
const char* ssid = "SKYZHE1K"; 
const char* password = "rHGMRSNqyFjY";

// LED onboard do ESP32-C3 Super Mini
#define LED_PIN 8
WebServer server(80);

// COMENTADO: Variável de tempo da serial removida para liberar o loop
// unsigned long tempoSerial = 0;

String paginaHTML() {
  return R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Controle LED ESP32-C3</title>
    <style>
        body { font-family: Arial, sans-serif; text-align: center; margin-top: 50px; background-color: #f0f0f0; }
        button { width: 150px; height: 60px; font-size: 20px; margin: 10px; border: none; border-radius: 10px; cursor: pointer; }
        .on { background-color: green; color: white; }
        .off { background-color: red; color: white; }
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
  digitalWrite(LED_PIN, LOW);   // Lógica invertida: LOW acende o LED físico
  Serial.println("[AÇÃO] Comando recebido: LED LIGADO");
  server.send(200, "text/plain", "ON"); 
}

void handleLedOff() { 
  digitalWrite(LED_PIN, HIGH);  // Lógica invertida: HIGH apaga o LED físico
  Serial.println("[AÇÃO] Comando recebido: LED DESLIGADO");
  server.send(200, "text/plain", "OFF"); 
}

void setup() {
  // Inicializa a Serial
  Serial.begin(115200);

  // Configura o LED (Começa desligado)
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH); 

  // Aguarda até o monitor serial ser aberto
  uint32_t startTime = millis();
  while (!Serial && (millis() - startTime < 3000)) { 
    delay(10); 
  }

  Serial.println("\n\n--- INICIALIZANDO SERVIDOR HTTP ---");

  // Configura e dispara o Wi-Fi em segundo plano
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  WiFi.begin(ssid, password);

  // Rotas preparadas
  server.on("/", handleRoot);
  server.on("/on", handleLedOn);
  server.on("/off", handleLedOff);
}

void loop() {
  // COMENTADO: Envio periódico de status removido para deixar a serial 100% livre
  /*
  unsigned long tempoAtual = millis();
  if (tempoAtual - tempoSerial > 4000) {
    tempoSerial = tempoAtual;
    if (WiFi.status() == WL_CONNECTED) {
      Serial.print("[INFO] Online | IP: ");
      Serial.println(WiFi.localIP());
    } else {
      Serial.println("[INFO] Tentando reconectar ao WiFi...");
    }
  }
  */

  // Se o Wi-Fi estiver conectado, processa os comandos HTTP do navegador
  if (WiFi.status() == WL_CONNECTED) {
    static bool servidorIniciado = false;
    if (!servidorIniciado) {
      server.begin();
      servidorIniciado = true;
      Serial.println("[SERVER] Servidor pronto! Acesse pelo navegador.");
    }
    server.handleClient();
  }
}
