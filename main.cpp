#include <WiFi.h>
#include <WebServer.h>

// Definição de Pinos
//  Saídas
#define PINO_ESTEIRA 22
#define PINO_SEPARADOR 21
#define PINO_TRAVA 23
#define PINO_MAGAZINE 19
#define PINO_MEDIDOR 18

// Entradas (sensores)
#define ENTRADA_VP 36  // Sensor de início
#define ENTRADA_VN 39  // Sensor metálico
#define ENTRADA_D34 34 // Sensor final
#define ENTRADA_D32 32 // Sensor óptico

// Estado do Processo
enum Estado
{
  MANUAL,
  AUTOMATICO_EM_EXECUCAO,
  ESTADO_1,
  ESTADO_2,
  ESTADO_3,
  ESTADO_4,
  ESTADO_5,
  ESTADO_6,
  ESTADO_7,
  ESTADO_8
};

Estado estadoAtual = MANUAL;

// Wi-Fi e Servidor
const char *ssid = "R2D2";
const char *password = "Procyon Lotor";
WebServer server(80);

// Página HTML
const char *htmlPage = R"rawliteral(
<!DOCTYPE html>
<html lang="pt-BR">
<head>
  <meta charset="UTF-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1.0"/>
  <title>Controle da Máquina</title>
  <style>
    :root {
      --azul: #007bff;
      --azul-hover: #0056b3;
      --fundo: #f2f4f8;
      --texto: #2c3e50;
      --cinza-claro: #dfe6e9;
      --borda: #bdc3c7;
    }

    body {
      margin: 0;
      font-family: 'Segoe UI', sans-serif;
      background-color: var(--fundo);
      color: var(--texto);
      display: flex;
      flex-direction: column;
      align-items: center;
      padding: 20px;
      padding-bottom: 80px;
    }

    h1 {
      margin-top: 20px;
      color: var(--azul);
      font-size: 2.2em;
    }

    h2 {
      margin-top: 40px;
      color: #444;
      font-size: 1.4em;
      border-bottom: 2px solid var(--azul);
      padding-bottom: 5px;
      width: 90%;
      max-width: 500px;
      text-align: left;
    }

    .grid {
      display: grid;
      grid-template-columns: repeat(auto-fill, minmax(180px, 1fr));
      gap: 15px;
      width: 100%;
      max-width: 600px;
      margin-top: 20px;
    }

    button {
      background-color: var(--azul);
      color: white;
      border: none;
      padding: 14px;
      border-radius: 10px;
      font-size: 16px;
      transition: 0.3s ease;
      box-shadow: 0 4px 6px rgba(0,0,0,0.1);
      cursor: pointer;
    }

    button:hover {
      background-color: var(--azul-hover);
    }

    #status {
      margin-top: 30px;
      font-weight: bold;
      font-size: 1.1em;
      color: #555;
      background-color: var(--cinza-claro);
      padding: 10px 20px;
      border-radius: 8px;
      border: 1px solid var(--borda);
    }

    footer {
  width: 100%;
  background-color: #333;
  color: #fff;
  text-align: center;
  padding: 15px 0;
  position: fixed;
  bottom: 0;
  left: 0;
  font-size: 14px;
  font-weight: 400;
  box-shadow: 0 -2px 6px rgba(0,0,0,0.2);
}

footer p {
  margin: 0;
}

    @media(max-width: 500px) {
      h1 { font-size: 1.7em; }
      button { font-size: 15px; padding: 12px; }
    }
  </style>
</head>
<body>
  <h1>Controle da Máquina</h1>

  <h2>Processo Automático</h2>
  <div class="grid">
    <button onclick="enviarComando('ligarProcesso')">▶ Iniciar Processo Automático</button>
  </div>

  <h2>Etapas Individuais</h2>
  <div class="grid">
    <button onclick="enviarComando('ligarTrava')">1️⃣ Ativar Pino/Trava</button>
    <button onclick="enviarComando('ligarEsteira')">2️⃣ Ligar Esteira</button>
    <button onclick="enviarComando('ligarSeparador')">3️⃣ Ativar Separador</button>
    <button onclick="enviarComando('ligarMagazine')">4️⃣ Ativar Magazine</button>
    <button onclick="enviarComando('ligarMedidor')">5️⃣ Ligar Medidor</button>
  </div>

  <h2>Desligar Dispositivos</h2>
  <div class="grid">
    <button onclick="enviarComando('desligarTrava')">Desativar Trava</button>
    <button onclick="enviarComando('desligarEsteira')">Desligar Esteira</button>
    <button onclick="enviarComando('desligarSeparador')">Desligar Separador</button>
    <button onclick="enviarComando('desligarMagazine')">Desativar Magazine</button>
    <button onclick="enviarComando('desligarMedidor')">Desligar Medidor</button>
  </div>

  <h2>Tempo decorrido</h2>
<p id="tempoDecorrido">0s</p>

<h2>Contador de Peça</h2>
<p>Peças metálicas</p> <p id="contadorMetal"></p>
<p>Peças não metálicas</p> <p id="contadorPlastico"></p>


  <p id="status">Aguardando comandos...</p>

<footer>
  <p>Desenvolvido por Gustavo Chimello, Lucas Miyaki & Olavo Xavier</p>
</footer>

  <script>
    function enviarComando(cmd) {
      fetch("/comando?acao=" + cmd)
        .then(response => response.text())
        .then(data => document.getElementById("status").innerText = data)
        .catch(() => document.getElementById("status").innerText = "Erro ao enviar comando.");
    }

    function atualizarTempo() {
  fetch("/tempo")
    .then(response => response.text())
    .then(data => {
      document.getElementById("tempoDecorrido").innerText = data + " segundos";
    })
    .catch(() => {
      document.getElementById("tempoDecorrido").innerText = "Erro ao obter tempo";
    });
}

function atualizarContadores() {
    fetch("/contador")
      .then(response => response.json())
      .then(data => {
        document.getElementById("contadorMetal").innerText = data.metal;
        document.getElementById("contadorPlastico").innerText = data.plastico;
      })
      .catch(() => {
        document.getElementById("contadorMetal").innerText = "Erro";
        document.getElementById("contadorPlastico").innerText = "Erro";
      });
}

// Atualiza o tempo a cada 1 segundo
setInterval(() => {
    atualizarTempo();
    atualizarContadores();
}, 1000);

  </script>
</body>
</html>
)rawliteral";

unsigned long startTime = 0;
unsigned long elapsedTime = 0;

unsigned long contadorMetal = 0;
unsigned long contadorPlastico = 0;

// Setup
void setup()
{
  Serial.begin(115200);
  startTime = millis(); // marca o início do processo

  // Configurar saídas
  pinMode(PINO_ESTEIRA, OUTPUT);
  pinMode(PINO_SEPARADOR, OUTPUT);
  pinMode(PINO_TRAVA, OUTPUT);
  pinMode(PINO_MAGAZINE, OUTPUT);
  pinMode(PINO_MEDIDOR, OUTPUT);

  // Desligar atuadores inicialmente
  digitalWrite(PINO_ESTEIRA, LOW);
  digitalWrite(PINO_SEPARADOR, LOW);
  digitalWrite(PINO_TRAVA, HIGH);
  digitalWrite(PINO_MAGAZINE, LOW);
  digitalWrite(PINO_MEDIDOR, LOW);

  // Configurar sensores (entradas)
  pinMode(ENTRADA_VP, INPUT);
  pinMode(ENTRADA_VN, INPUT);
  pinMode(ENTRADA_D34, INPUT);
  pinMode(ENTRADA_D32, INPUT);

  // Criar rede Wi-Fi
  WiFi.softAP(ssid, password);
  Serial.println("Rede Wi-Fi criada");
  Serial.print("Acesse: http://");
  Serial.println(WiFi.softAPIP());

  // Página principal
  server.on("/", HTTP_GET, []()
            { server.send(200, "text/html", htmlPage); });

  server.on("/tempo", HTTP_GET, []()
            {
  unsigned long elapsed = 0;
  if (estadoAtual == AUTOMATICO_EM_EXECUCAO || estadoAtual >= ESTADO_1) {
    elapsed = millis() - startTime;
  }
  
  // Retorna o tempo em segundos
  String tempoStr = String(elapsed / 1000);
  server.send(200, "text/plain", tempoStr); });
  server.on("/contador", HTTP_GET, []()
            {
  String json = "{";
  json += "\"metal\":" + String(contadorMetal) + ",";
  json += "\"plastico\":" + String(contadorPlastico);
  json += "}";
  server.send(200, "application/json", json); });

  // Comandos manuais
  server.on("/comando", HTTP_GET, []()
            {
    String acao = server.arg("acao");

    if (acao == "ligarProcesso") {
  startTime = millis(); 
  estadoAtual = AUTOMATICO_EM_EXECUCAO;
  server.send(200, "text/plain", "Processo automático iniciado");
  return;
}


    if (acao == "ligarEsteira") {
      digitalWrite(PINO_ESTEIRA, HIGH);
      server.send(200, "text/plain", "Esteira ligada");
    } else if (acao == "desligarEsteira") {
      digitalWrite(PINO_ESTEIRA, LOW);
      server.send(200, "text/plain", "Esteira desligada");
    }

    if (acao == "ligarSeparador") {
      digitalWrite(PINO_SEPARADOR, HIGH);
      server.send(200, "text/plain", "Separador ligado");
    } else if (acao == "desligarSeparador") {
      digitalWrite(PINO_SEPARADOR, LOW);
      server.send(200, "text/plain", "Separador desligado");
    }

    if (acao == "ligarTrava") {
      digitalWrite(PINO_TRAVA, HIGH);
      server.send(200, "text/plain", "Trava ligada");
    } else if (acao == "desligarTrava") {
      digitalWrite(PINO_TRAVA, LOW);
      server.send(200, "text/plain", "Trava desligada");
    }

    if (acao == "ligarMedidor") {
      digitalWrite(PINO_MEDIDOR, HIGH);
      server.send(200, "text/plain", "Medidor ligado");
    } else if (acao == "desligarMedidor") {
      digitalWrite(PINO_MEDIDOR, LOW);
      server.send(200, "text/plain", "Medidor desligado");
    }

    if (acao == "ligarMagazine") {
      digitalWrite(PINO_MAGAZINE, HIGH);
      server.send(200, "text/plain", "Magazine ligado");
    } else if (acao == "desligarMagazine") {
      digitalWrite(PINO_MAGAZINE, LOW);
      server.send(200, "text/plain", "Magazine desligado");
    } });

  server.begin();
  Serial.println("Servidor HTTP iniciado");
}

// Loop com FSM
void loop()
{
  elapsedTime = millis() - startTime;
  Serial.print("Tempo decorrido (ms): ");
  Serial.println(elapsedTime);

  server.handleClient();

  if (estadoAtual == MANUAL)
    return;

  delay(300);

  switch (estadoAtual)
  {
  case AUTOMATICO_EM_EXECUCAO:
    Serial.println("Iniciando processo automático...");
    estadoAtual = ESTADO_1;
    break;

  case ESTADO_1:
    if (digitalRead(ENTRADA_VP) == HIGH)
    {
      digitalWrite(PINO_MEDIDOR, LOW);
      digitalWrite(PINO_ESTEIRA, HIGH); // Liga esteira.
      digitalWrite(PINO_TRAVA, LOW);    // Ativa pino/trava.
      Serial.println("Peça detectada no início. Esteira ligada.");
      estadoAtual = ESTADO_2;
    }
    break;

  case ESTADO_2:
    if (digitalRead(ENTRADA_D32) == HIGH)
    {
      bool ehMetal = digitalRead(ENTRADA_VN) == LOW;
      digitalWrite(PINO_SEPARADOR, ehMetal ? LOW : HIGH); // Ativa separador se for metal.
      Serial.println(ehMetal ? "Peça não metálica detectada." : "Peça metálica. Separador ativado.");
      estadoAtual = ESTADO_3;
    }
    break;

  case ESTADO_3:
    digitalWrite(PINO_MAGAZINE, HIGH); // Ativa magazine.
    Serial.println("Magazine avançando.");
    delay(2000);
    estadoAtual = ESTADO_4;
    break;

  case ESTADO_4:
    digitalWrite(PINO_MAGAZINE, LOW); // Para magazine.
    Serial.println("Pino/trava ativada./Para magazine.");
    estadoAtual = ESTADO_5;
    break;

  case ESTADO_5:
    delay(2000);
    digitalWrite(PINO_TRAVA, HIGH); // Desativa pino/trava.
    delay(2000);
    digitalWrite(PINO_MAGAZINE, HIGH); // Ativa magazine.
    Serial.println("Magazine ativado./Pino desativado.");
    estadoAtual = ESTADO_6;
    break;

  case ESTADO_6:
    if (digitalRead(ENTRADA_D34) == HIGH)
    { // Sensor final detecta peça.
      Serial.println("Peça chegou ao final da esteira.");
      if (digitalRead(PINO_SEPARADOR) == HIGH)
      {
        contadorMetal++;
      }
      else if (digitalRead(PINO_SEPARADOR) == LOW)
      {
        contadorPlastico++;
      }
      estadoAtual = ESTADO_7;
    }
    break;

  case ESTADO_7:
    delay(2000);
    digitalWrite(PINO_MAGAZINE, LOW); // Para magazine.
    delay(2000);
    digitalWrite(PINO_ESTEIRA, LOW); // Desliga esteira.
    delay(2000);
    digitalWrite(PINO_SEPARADOR, LOW); // Desativa separador.
    delay(2000);
    Serial.println("Magazine desligado. Esteira desligada. Separador desativado.");
    estadoAtual = ESTADO_8;
    break;

  case ESTADO_8:
    Serial.println("Retornando ao modo manual.");
    estadoAtual = MANUAL;
    break;

  default:
    break;

    delay(1000);
  }
}