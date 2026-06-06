#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

// Configurações da rede Wi-Fi que o ESP32 vai criar (Modo AP)
const char* ap_ssid = "Plug-Shelly-AP"; 
const char* ap_password = "123456789"; // Mínimo de 8 caracteres

// LED onboard do ESP32-C3 Super Mini
#define LED_PIN 8
WebServer server(80);

String paginaHTML() {
  return R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Controle LED ESP32-C3 (AP)</title>
    <style>
        body { font-family: Arial, sans-serif; text-align: center; margin-top: 50px; background-color: #f0f0f0; }
        button { width: 150px; height: 60px; font-size: 20px; margin: 10px; border: none; border-radius: 10px; cursor: pointer; }
        .on { background-color: green; color: white; }
        .off { background-color: red; color: white; }
    </style>
</head>
<body>
    <h1>Controle do LED ESP32-C3</h1>
    <p>Modo Ponto de Acesso Direto</p>
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

  Serial.println("\n\n--- INICIALIZANDO SERVIDOR HTTP (MODO AP) ---");

  // Configura o Wi-Fi para operar como Ponto de Acesso (AP)
  WiFi.mode(WIFI_AP);
  WiFi.disconnect();
  delay(200); // Tempo para o rádio limpar estados anteriores
  
  // Inicializa a rede sem fio própria do chip
  WiFi.softAP(ap_ssid, ap_password);

  // --- CORREÇÃO DO ESP32-C3: AGUARDA O IP DO AP FICAR VÁLIDO ---
  // Evita ler o IP enquanto a interface DHCP interna do chip ainda está subindo
  IPAddress apIP = WiFi.softAPIP();
  while (apIP.toString() == "0.0.0.0") {
    delay(100);
    apIP = WiFi.softAPIP();
  }

  // --- IMPRESSÃO ÚNICA DO IP ---
  Serial.println("[WIFI] Rede Wi-Fi criada com sucesso!");
  Serial.print("[WIFI] SSID (Nome da Rede): ");
  Serial.println(ap_ssid);
  Serial.print("[INFO] Online | IP: ");
  Serial.println(apIP); // Exibe 192.168.4.1 exatamente uma única vez

  // Rotas preparadas
  server.on("/", handleRoot);
  server.on("/on", handleLedOn);
  server.on("/off", handleLedOff);

  // Inicializa o servidor web definitivamente aqui
  server.begin();
  Serial.println("[SERVER] Servidor pronto! Acesse pelo navegador.");
}

void loop() {
  // Processa continuamente as requisições
  server.handleClient();
}
