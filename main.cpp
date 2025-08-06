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
  <script src="https://cdnjs.cloudflare.com/ajax/libs/xlsx/0.18.5/xlsx.full.min.js"></script>
  <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
  <title>Controle da Máquina</title>
  <style>
    :root {
      --azul: #00cfff;
      --azul-hover: #0099cc;
      --fundo: #181c24;
      --texto: #eaf6fb;
      --cinza-claro: #232a36;
      --borda: #00cfff;
      --card: #232a36;
      --shadow: 0 8px 24px rgba(0, 207, 255, 0.08);
      --radius: 16px;
      --gradient: linear-gradient(90deg, #00cfff 0%, #0056b3 100%);
    }

    body {
      margin: 0;
      font-family: 'Segoe UI', 'Roboto', Arial, sans-serif;
      background: var(--fundo);
      color: var(--texto);
      min-height: 100vh;
      display: flex;
      flex-direction: column;
      align-items: center;
      padding: 24px;
      padding-bottom: 100px;
    }

    h1 {
      margin-top: 24px;
      font-size: 2.5em;
      background: var(--gradient);
      -webkit-background-clip: text;
      -webkit-text-fill-color: transparent;
      text-shadow: 0 2px 8px #00cfff44;
      letter-spacing: 2px;
    }

    h2 {
      margin-top: 40px;
      color: #00cfff;
      font-size: 1.3em;
      border-bottom: 2px solid var(--azul);
      padding-bottom: 5px;
      width: 90%;
      max-width: 500px;
      text-align: left;
      letter-spacing: 1px;
    }

    .card {
      background: var(--card);
      border: 1px solid var(--borda);
      border-radius: var(--radius);
      box-shadow: var(--shadow);
      padding: 24px;
      margin: 24px 0;
      width: 100%;
      max-width: 650px;
      display: flex;
      flex-direction: column;
      align-items: center;
    }

    .button-row {
      display: flex;
      flex-wrap: wrap;
      gap: 16px;
      justify-content: center;
      margin-bottom: 12px;
    }

    button {
      background: var(--gradient);
      color: white;
      border: none;
      padding: 14px 28px;
      border-radius: 12px;
      font-size: 16px;
      font-weight: 500;
      transition: 0.2s;
      box-shadow: 0 4px 16px #00cfff22;
      cursor: pointer;
      outline: none;
      border-bottom: 2px solid #0099cc;
      letter-spacing: 1px;
      position: relative;
      overflow: hidden;
    }

    button:hover, button:focus {
      background: var(--azul-hover);
      box-shadow: 0 6px 24px #00cfff44;
      border-bottom: 2px solid #00cfff;
    }

    #status {
      margin-top: 30px;
      font-weight: bold;
      font-size: 1.1em;
      color: #00cfff;
      background: var(--cinza-claro);
      padding: 12px 24px;
      border-radius: 10px;
      border: 1px solid var(--borda);
      box-shadow: 0 2px 8px #00cfff22;
      text-align: center;
      width: 100%;
      max-width: 500px;
    }

    .info-row {
      display: flex;
      justify-content: space-between;
      width: 100%;
      max-width: 500px;
      margin: 12px 0;
      gap: 16px;
    }

    .info-block {
      background: var(--cinza-claro);
      border-radius: 10px;
      padding: 12px 18px;
      flex: 1;
      text-align: center;
      color: #00cfff;
      font-weight: 500;
      font-size: 1.1em;
      border: 1px solid #00cfff44;
      box-shadow: 0 2px 8px #00cfff22;
    }

    footer {
      width: 100%;
      background: #232a36;
      color: #00cfff;
      text-align: center;
      padding: 18px 0;
      position: fixed;
      bottom: 0;
      left: 0;
      font-size: 15px;
      font-weight: 400;
      box-shadow: 0 -2px 12px #00cfff22;
      letter-spacing: 1px;
    }

    footer p {
      margin: 0;
    }

    @media(max-width: 700px) {
      .card, #status, .info-row { max-width: 98vw; }
      h1 { font-size: 1.7em; }
      button { font-size: 15px; padding: 12px 16px; }
      .info-block { font-size: 1em; padding: 10px 8px; }
    }
    @media(max-width: 500px) {
      .button-row { gap: 8px; }
      .info-row { flex-direction: column; gap: 8px; }
      .card { padding: 12px; }
    }
  </style>
</head>
<body>
  <h1>Controle da Máquina</h1>

  <div class="card">
    <h2>Processo Automático</h2>
    <div class="button-row">
      <button onclick="enviarComando('ligarProcesso')">▶ Iniciar Processo Automático</button>
    </div>
  </div>

  <div class="card">
    <h2>Etapas Individuais</h2>
    <div class="button-row">
      <button onclick="enviarComando('ligarTrava')">Ativar Trava</button>
      <button onclick="enviarComando('desligarTrava')">Desativar Trava</button>
    </div>
    <div class="button-row">
      <button onclick="enviarComando('ligarEsteira')">Ligar Esteira</button>
      <button onclick="enviarComando('desligarEsteira')">Desligar Esteira</button>
    </div>
    <div class="button-row">
      <button onclick="enviarComando('ligarSeparador')">Ativar Separador</button>
      <button onclick="enviarComando('desligarSeparador')">Desligar Separador</button>
    </div>
    <div class="button-row">
      <button onclick="enviarComando('ligarMagazine')">Ativar Magazine</button>
      <button onclick="enviarComando('desligarMagazine')">Desativar Magazine</button>
    </div>
    <div class="button-row">
      <button onclick="enviarComando('ligarMedidor')">Ligar Medidor</button>
      <button onclick="enviarComando('desligarMedidor')">Desligar Medidor</button>
    </div>
  </div>

  <div class="card">
    <h2>Informações do Processo</h2>
    <div class="info-row">
      <div class="info-block">
        <span>Tempo decorrido</span>
        <div id="tempoDecorrido">0s</div>
      </div>
      <div class="info-block">
        <span>Peças metálicas</span>
        <div id="contadorMetal">0</div>
      </div>
      <div class="info-block">
        <span>Peças não metálicas</span>
        <div id="contadorPlastico">0</div>
      </div>
    </div>
    <div class="button-row">
            <button onclick="exportarParaExcel()">📥 Exportar Dados</button>
        </div>
        <div class="button-row">
            <button onclick="mostrarGrafico()">📊 Mostrar Gráfico</button>
        </div>
        <div style="width:100%;max-width:500px;margin:20px auto;display:none;" id="graficoContainer">
            <canvas id="graficoBarra"></canvas>
        </div>
    </div>

  <div id="status">Aguardando comandos...</div>

  <footer>
    <p>Desenvolvido por Gustavo Chimello, Lucas Miyaki & Olavo Xavier</p>
  </footer>

<script>
        function exportarParaExcel() {
            const metal = document.getElementById("contadorMetal").innerText || "0";
            const plastico = document.getElementById("contadorPlastico").innerText || "0";
            const tempo = document.getElementById("tempoDecorrido").innerText.replace(" segundos", "").replace("s", "") || "0";
            const total = parseInt(metal) + parseInt(plastico);

            const dados = [
                ["Tipo", "Quantidade"],
                ["Peças Metálicas", metal],
                ["Peças Plásticas", plastico],
                ["Tempo de Processo (s)", tempo],
                ["Total de Peças", total]
            ];

            const planilha = XLSX.utils.aoa_to_sheet(dados);

            planilha["A1"].s = { font: { bold: true }, alignment: { horizontal: "center" } };
            planilha["B1"].s = { font: { bold: true }, alignment: { horizontal: "center" } };

            for (let r = 1; r <= dados.length; r++) {
                for (let c = 0; c < 2; c++) {
                    const cell = XLSX.utils.encode_cell({ r: r, c: c });
                    if (planilha[cell]) {
                        planilha[cell].s = { alignment: { horizontal: "center" } };
                    }
                }
            }

            planilha['!cols'] = [{ wch: 22 }, { wch: 18 }];

            const wb = XLSX.utils.book_new();
            XLSX.utils.book_append_sheet(wb, planilha, "Relatório");
            XLSX.writeFile(wb, "relatorio_producao.xlsx");
        }
    </script>

    <script>
        let graficoBarra = null;

        function mostrarGrafico() {
            document.getElementById("graficoContainer").style.display = "block";

            const metal = parseInt(document.getElementById("contadorMetal").innerText) || 0;
            const plastico = parseInt(document.getElementById("contadorPlastico").innerText) || 0;

            if (graficoBarra) {
                graficoBarra.destroy();
            }

            const ctx = document.getElementById("graficoBarra").getContext("2d");
            graficoBarra = new Chart(ctx, {
                type: "bar",
                data: {
                    labels: ["Peças Metálicas", "Peças Plásticas"],
                    datasets: [{
                        label: "Quantidade",
                        data: [metal, plastico],
                        backgroundColor: ["#00cfff", "#0099cc"],
                        borderColor: ["#00cfff", "#0099cc"],
                        borderWidth: 2
                    }]
                },
                options: {
                    responsive: true,
                    plugins: {
                        legend: { display: false },
                        title: { display: true, text: "Produção Atual" }
                    },
                    scales: {
                        x: { title: { display: true, text: "Tipo de Peça" } },
                        y: { beginAtZero: true, title: { display: true, text: "Quantidade" } }
                    }
                }
            });
        }
    </script>

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
      document.getElementById("tempoDecorrido").innerText = data + "s";
    })
    .catch(() => {
      document.getElementById("tempoDecorrido").innerText = "Erro";
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