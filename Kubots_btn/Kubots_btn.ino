#include <WiFi.h>
#include <WebSocketsServer_Generic.h>  //Se debe descargar esta librería
#include <WebServer.h>
#define IZQ_PWM     22 // D0 (PWMB)           
#define IZQ_AVZ     32 // D1 (BIN2)         
#define IZQ_RET     33 // D2 (BIN1)
#define STBY        02 // D4 (STBY)
#define DER_RET     16 // D5 (AIN1)
#define DER_AVZ     17 // D6 (AIN2)
#define DER_PWM     21 // D7 (PWMA)
#define PI 3.141593
const char* ssid = "Dragon_23";  //Cambiar por el nombre de sus robots ej Messi, Mbappe, etc. 
const char* password = "mante2024"; //cambiar la clave para que solo los integrandes del equipo puedan accesar a sus robots.

const int freq = 2000;
const int channel_0 = 1;
const int channel_1 = 2;
  // 8 Bit resolution for duty cycle so value is between 0 - 255
const int resolution = 8;

static const char PROGMEM INDEX_HTML[] = R"( 
<!doctype html>
<html>
<head>
  <meta charset=utf-8>
  <meta name='viewport' content='width=device-width, height=device-height, initial-scale=1.0, maximum-scale=1.0'/>
  <title>Club de Robotica Dragones UAM Mante</title>
  <style> 
    * {
      -webkit-touch-callout: none;
      -webkit-text-size-adjust: none;
      -webkit-tap-highlight-color: rgba(255, 255, 255, 0);
      -webkit-user-select: none;
      -moz-touch-callout: none;
      -moz-text-size-adjust: none;
      -ms-text-size-adjust: none;
      -ms-tap-highlight-color: rgba(255, 255, 255, 0);
      -ms-user-select: none;
      touch-callout: none;
      text-size-adjust: none;
      tap-highlight-color: rgba(255, 255, 255, 0);
      user-select: none;
    }

    body {
      margin: 0px;
    }

    .containerButtons {
      width: auto;
      display: flex;
      justify-content: space-around;
      flex-wrap: wrap;
    }

    .buttons {
      border: none;
      color: white;
      padding: 20px;
      text-align: center;
      text-decoration: none;
      display: inline-block;
      font-size: 16px;
      font-weight: bold;
      margin: 4px 2px;
      cursor: pointer;
      width: 45%;
      border-radius: 8px;
    }

    .btn-forward { background-color: #4CAF50; }
    .btn-backward { background-color: #f44336; }
    .btn-left { background-color: #008CBA; }
    .btn-right { background-color: #ff9800; }

    .slider-container {
      width: 100%;
      text-align: center;
      margin-top: 20px;
    }

    .slider {
      width: 80%;
    }
  </style>
</head>
<body id='body'>
  <div class='containerButtons'>
    <button class='buttons btn-forward' onmousedown=\"sendCommand('forward')\" onmouseup=\"sendCommand('stop')\">Adelante</button>
    <button class='buttons btn-backward' onmousedown=\"sendCommand('backward')\" onmouseup=\"sendCommand('stop')\">Atrás</button>
    <button class='buttons btn-left' onmousedown=\"sendCommand('left')\" onmouseup=\"sendCommand('stop')\">Izquierda</button>
    <button class='buttons btn-right' onmousedown=\"sendCommand('right')\" onmouseup=\"sendCommand('stop')\">Derecha</button>
  </div>
  <div class="slider-container">
    <p>Velocidad: <span id="speedValue">128</span></p>
    <input type="range" min="0" max="255" value="128" class="slider" id="speedSlider">
  </div>
  <script>
    var connection = new WebSocket('ws://'+location.hostname+':81/', ['arduino']);
    var speed = 128; // Valor inicial de velocidad

    connection.onopen = function () {
      connection.send('Connect ' + new Date());
    };
    connection.onerror = function (error) {
      console.log('WebSocket Error ', error);
    };
    connection.onmessage = function (event) {
      console.log('Server: ', event.data);
    };

    // Actualizar el valor de velocidad cuando el slider cambia
    document.getElementById('speedSlider').oninput = function() {
      speed = this.value;
      document.getElementById('speedValue').innerHTML = this.value;
    };

    // Función para enviar comandos al servidor
    function sendCommand(command) {
      var message = command + ':' + speed; // Enviar comando y velocidad
      connection.send(message);
    }
  </script>
</body>
</html>
)";

WebServer server (80);
WebSocketsServer webSocket = WebSocketsServer(81);
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", num);
      ledcWrite(channel_0, 0);
      ledcWrite(channel_1, 0);
      break;
    case WStype_CONNECTED: {
      IPAddress ip = webSocket.remoteIP(num);
      Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
      webSocket.sendTXT(num, "Connected");
    }
    break;
    case WStype_TEXT:
      Serial.printf("Número de conexión: %u - Texto recibido: %s\n", num, payload);
      if(num == 0) {
        String message = (char*)payload;
        int separatorIndex = message.indexOf(':');
        String command = message.substring(0, separatorIndex);
        int speed = message.substring(separatorIndex + 1).toInt();

        if (command == "forward") {
          digitalWrite(STBY, 1);
          digitalWrite(IZQ_AVZ, 1);
          digitalWrite(IZQ_RET, 0);
          digitalWrite(DER_AVZ, 1);
          digitalWrite(DER_RET, 0);
          ledcWrite(IZQ_PWM, speed);
          ledcWrite(DER_PWM, speed);
        } else if (command == "backward") {
          digitalWrite(STBY, 1);
          digitalWrite(IZQ_AVZ, 0);
          digitalWrite(IZQ_RET, 1);
          digitalWrite(DER_AVZ, 0);
          digitalWrite(DER_RET, 1);
          ledcWrite(IZQ_PWM, speed);
          ledcWrite(DER_PWM, speed);
        } else if (command == "left") {
          digitalWrite(STBY, 1);
          digitalWrite(IZQ_AVZ, 0);
          digitalWrite(IZQ_RET, 1);
          digitalWrite(DER_AVZ, 1);
          digitalWrite(DER_RET, 0);
          ledcWrite(IZQ_PWM, speed);
          ledcWrite(DER_PWM, speed);
        } else if (command == "right") {
          digitalWrite(STBY, 1);
          digitalWrite(IZQ_AVZ, 1);
          digitalWrite(IZQ_RET, 0);
          digitalWrite(DER_AVZ, 0);
          digitalWrite(DER_RET, 1);
          ledcWrite(IZQ_PWM, speed);
          ledcWrite(DER_PWM, speed);
        } else if (command == "stop") {
          digitalWrite(STBY, 0);
          digitalWrite(IZQ_AVZ, 0);
          digitalWrite(IZQ_RET, 0);
          digitalWrite(DER_AVZ, 0);
          digitalWrite(DER_RET, 0);
          ledcWrite(IZQ_PWM, 0);
          ledcWrite(DER_PWM, 0);
        }
      }
      break;
  }
}
void setup() {
  Serial.begin(115200);
  Serial.println();
  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP(); 
  Serial.print("IP del access point: ");
  Serial.println(myIP);
  Serial.println("WebServer iniciado...");
   // start webSocket server
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  // handle index
  server.on("/", []() {
      server.send_P(200, "text/html", INDEX_HTML);
  });
  server.begin();
  pinMode(IZQ_PWM, OUTPUT); pinMode(DER_PWM, OUTPUT); 
  pinMode(IZQ_AVZ, OUTPUT); pinMode(DER_AVZ, OUTPUT);
  pinMode(IZQ_RET, OUTPUT); pinMode(DER_RET, OUTPUT);
  pinMode(STBY, OUTPUT);
  digitalWrite(IZQ_PWM, 0); digitalWrite(DER_PWM, 0);
  digitalWrite(IZQ_AVZ, 0); digitalWrite(DER_AVZ, 0);
  digitalWrite(IZQ_RET, 0); digitalWrite(DER_RET, 0);
  digitalWrite(STBY, 1);

      //Set the PWM Settings
  //ledcSetup(channel_0, freq, resolution);
  //ledcSetup(channel_1, freq, resolution);
ledcAttach(IZQ_PWM,freq, resolution);
ledcAttach(DER_PWM,freq, resolution);
    //Attach Pin to Channel
 // ledcAttachPin(IZQ_PWM, channel_0);
 // ledcAttachPin(DER_PWM, channel_1);
  //analogWriteRange(255);
}
void loop() {
    webSocket.loop();
    server.handleClient();
}