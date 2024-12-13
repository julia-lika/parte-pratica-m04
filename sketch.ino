#include <WiFi.h>
#include <HTTPClient.h>

// Definição dos pinos utilizados para controle dos LEDs
#define led_azul 9 // Pino do LED azul
int led_verde = 39; // Pino do LED verde
int led_vermelho = 47; // Pino do LED vermelho
int led_amarelo = 9; // Pino do LED amarelo

// Definição dos pinos para o botão e o sensor LDR
const int pinoBotao = 15;  // Pino do botão
const int pinoLdr = 4;  // Pino do sensor LDR

// Variáveis para o estado do botão e o limiar de luminosidade
int estadoBotao = 0;  // Estado atual do botão
int threshold = 600; // Limiar para baixa luminosidade

// Credenciais da rede WiFi
const char* ssid = "Wokwi-GUEST"; // Nome da rede WiFi
const char* senha = ""; // Senha da rede WiFi

// Variáveis de controle de tempo
long tempoAnterior = 0; 
long tempo = 1000;      

// Cliente WiFi
WiFiClient espClient;

// Variáveis para o LDR
unsigned long ultimaLeituraLdr = 0;
const unsigned long intervaloLdr = 1000; // Intervalo de leitura do LDR (1 segundo)
int ldrStatus = 0; // Valor atual lido pelo LDR


int contadorPressionamentos = 0;
unsigned long ultimoPressionamento = 0;
const unsigned long debounceDelay = 50;

// Classe Semáforo para controle dos LEDs
class Semaforo {
private:
  int *led_vermelho, *led_amarelo, *led_verde; // Ponteiros para os pinos dos LEDs
public:
  // Construtor que inicializa os ponteiros com os pinos dos LEDs
  Semaforo(int *vermelho, int *amarelo, int *verde) 
    : led_vermelho(vermelho), led_amarelo(amarelo), led_verde(verde) {}

  // Método para configurar os pinos dos LEDs como saída
  void iniciar() {
    pinMode(*led_vermelho, OUTPUT);
    pinMode(*led_amarelo, OUTPUT);
    pinMode(*led_verde, OUTPUT);
  }

  // Ativa a fase do semáforo com luz vermelha
  void faseVermelho(int tempo) {
    liga(*led_vermelho);
    desliga(*led_amarelo);
    desliga(*led_verde);
    delay(tempo);
  }

  // Ativa a fase do semáforo com luz amarela
  void faseAmarelo(int tempo) {
    desliga(*led_vermelho);
    liga(*led_amarelo);
    desliga(*led_verde);
    delay(tempo);
  }

  // Ativa a fase do semáforo com luz verde
  void faseVerde(int tempo) {
    desliga(*led_vermelho);
    desliga(*led_amarelo);
    liga(*led_verde);
    delay(tempo);
  }

private:
  // Liga o LED especificado
  void liga(int pino) {
    digitalWrite(pino, HIGH);
  }

  // Desliga o LED especificado
  void desliga(int pino) {
    digitalWrite(pino, LOW);
  }
};

// Instanciação da classe Semaforo
Semaforo semaforo(&led_vermelho, &led_amarelo, &led_verde);

void setup() {

  if (estadoBotao == HIGH && digitalRead(led_vermelho) == HIGH) {
    delay(1000); // Espera de 1 segundo
    semaforo.faseVerde(3000); // Abre o semáforo
  }
  // LEDs desligados
  digitalWrite(led_vermelho, LOW);
  digitalWrite(led_amarelo, LOW);
  digitalWrite(led_verde, LOW);

  // Configura o sensor LDR como entrada
  pinMode(pinoLdr, INPUT);

  // Configura o botão como entrada com resistor pull-up
  pinMode(pinoBotao, INPUT_PULLUP);

  // Inicializa a comunicação serial para debug
  Serial.begin(9600);

  // Conecta à rede WiFi
  WiFi.begin(ssid, senha);

  // Aguarda a conexão com a rede WiFi
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Conectado ao WiFi com sucesso!");

  // Verifica o estado do botão
  estadoBotao = digitalRead(pinoBotao);

  // Se conectado ao WiFi, realiza uma requisição HTTP
  if(WiFi.status() == WL_CONNECTED){
    HTTPClient http;

    String serverPath = "http://www.google.com.br"; // URL do servidor

    http.begin(serverPath.c_str());

    int httpResponseCode = http.GET(); // Executa a requisição HTTP GET

    if (httpResponseCode > 0) {
      // Exibe o código de resposta HTTP
      Serial.print("Código de resposta HTTP: ");
      Serial.println(httpResponseCode);
      String payload = http.getString();
      Serial.println(payload);
    } else {
      // Exibe o código de erro
      Serial.print("Erro: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  } else {
    Serial.println("WiFi desconectado");
  }

  // Inicializa o semáforo
  semaforo.iniciar();
}

void loop() {
  unsigned long currentMillis = millis();
  
  // Leitura periódica do LDR
  if (currentMillis - ultimaLeituraLdr >= intervaloLdr) {
    ultimaLeituraLdr = currentMillis;
    ldrStatus = analogRead(pinoLdr); // Lê o valor do LDR
    Serial.print("Valor do LDR: ");
    Serial.println(ldrStatus);

    // Controle dos LEDs com base na luminosidade detectada
    if (ldrStatus <= threshold) {
    digitalWrite(led_amarelo, HIGH);
    delay(500);
    digitalWrite(led_amarelo, LOW);
    delay(500);
    } else {
      if (estadoBotao == HIGH && digitalRead(led_vermelho) == HIGH) {
      delay(1000); // Espera de 1 segundo
      semaforo.faseVerde(3000); // Abre o semáforo
    }
    
      Serial.println("Está claro, desligando a luz");
      semaforo.faseVermelho(5000); // 5 segundos no vermelho
      semaforo.faseAmarelo(2000); // 2 segundos no amarelo
      semaforo.faseVerde(3000);   // 3 segundos no verde
    }
  }
}