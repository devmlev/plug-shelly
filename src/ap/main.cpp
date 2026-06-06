#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h> // OBRIGATÓRIO: Resolve o erro de 'HTTPClient'

// Configurações da rede Wi-Fi que o ESP32 vai criar (Modo AP)
const char* ap_ssid = "Plug-Shelly-AP"; 
const char* ap_password = "123456789"; 

// IP fixo que o Shelly recebeu na sua rede
const char* shelly_ip = "192.168.4.2"; 

#define LED_PIN 8
WebServer server(80);

void enviarComandoShelly(bool ligar) {
  HTTPClient http;
  
  // Endpoint oficial RPC para controle de relé na linha Shelly Plus (Gen2)
  String url = "http://" + String(shelly_ip) + "/rpc/Switch.Set";
  
  Serial.print("[SHELLY] Enviando comando POST para: ");
  Serial.println(url);

  http.begin(url);
  
  // Configura os cabeçalhos exigidos pelo firmware Shelly Plus
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Connection", "close");

  // Monta o corpo do JSON estruturado: {"id": 0, "on": true/false}
  String jsonPayload = "{\"id\":0,\"on\":" + String(ligar ? "true" : "false") + "}";

  // Envia a requisição usando POST passando o payload JSON
  int httpCode = http.POST(jsonPayload); 

  if (httpCode > 0) {
    Serial.printf("[SHELLY] Codigo HTTP retornado: %d\n", httpCode);
    
    // Mostra o retorno de confirmação do Shelly no terminal
    String resposta = http.getString();
    Serial.print("[SHELLY] Resposta do firmware: ");
    Serial.println(resposta);
  } else {
    Serial.printf("[ERRO] Falha de conexao: %s\n", http.errorToString(httpCode).c_str());
  }
  
  http.end(); 
}

String paginaHTML() {
  return R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Controle Plug Shelly via ESP32</title>
    <style>
        body { font-family: Arial, sans-serif; text-align: center; margin-top: 50px; background-color: #f0f0f0; }
        button { width: 150px; height: 60px; font-size: 20px; margin: 10px; border: none; border-radius: 10px; cursor: pointer; }
        .on { background-color: green; color: white; }
        .off { background-color: red; color: white; }
    </style>
</head>
<body>
    <h1>Controle do Plug Shelly via ESP32-C3</h1>
    <p>Modo Ponto de Acesso Direto</p>
    <button class="on" onclick="fetch('/on')">LIGAR TOMADA</button>
    <button class="off" onclick="fetch('/off')">DESLIGAR TOMADA</button>
</body>
</html>
)rawliteral";
}

void handleRoot() { 
  server.send(200, "text/html", paginaHTML()); 
}

void handleLedOn() { 
  digitalWrite(LED_PIN, LOW);   // Acende o LED do ESP32
  Serial.println("[AÇÃO] Comando recebido: LIGAR");
  enviarComandoShelly(true);    // Executa o POST para a tomada ligar
  server.send(200, "text/plain", "ON"); 
}

void handleLedOff() { 
  digitalWrite(LED_PIN, HIGH);  // Apaga o LED do ESP32
  Serial.println("[AÇÃO] Comando recebido: DESLIGAR");
  enviarComandoShelly(false);   // Executa o POST para a tomada desligar
  server.send(200, "text/plain", "OFF"); 
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH); 

  uint32_t startTime = millis();
  while (!Serial && (millis() - startTime < 3000)) { delay(10); }

  Serial.println("\n\n--- INICIALIZANDO CONTROLADOR SHELLY (AP) ---");

  WiFi.mode(WIFI_AP);
  WiFi.disconnect();
  delay(200); 
  
  WiFi.softAP(ap_ssid, ap_password);

  IPAddress apIP = WiFi.softAPIP();
  while (apIP.toString() == "0.0.0.0") {
    delay(100);
    apIP = WiFi.softAPIP();
  }

  Serial.println("[WIFI] Rede Wi-Fi criada com sucesso!");
  Serial.print("[INFO] Acesse o painel pelo IP: ");
  Serial.println(apIP); 

  server.on("/", handleRoot);
  server.on("/on", handleLedOn);
  server.on("/off", handleLedOff);

  server.begin();
  Serial.println("[SERVER] Pronto para comandar o Shelly.");
}

void loop() {
  server.handleClient();
}
