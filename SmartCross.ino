// C++ code
//
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Configurações do OLED
#define SCREEN_WIDTH 128 // Largura do display em pixels
#define SCREEN_HEIGHT 64 // Altura do display em pixels
#define OLED_RESET    -1 // Reset compartilhado com o Arduino
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

int ledVeiculo[3] = {2, 3, 4}; // 2 = Vermelho ; 3 = Amarelo ; 4 = Verde
int ledPedestre[2] = {11, 12}; // 11 = Vermelho ; 12 = Verde
int estado = 0;

int tempoAmarelo = 2000;
int tempoVerde = 5000;
int tempoPedestre = tempoVerde;
int tempoEspera = 1000;

const int pinTrigger = 6;
const int pinEccho = 7;

float CalcularDistancia();
void EstadoSemaforo(int estado);
void GerenciarTravessia(int tempoBaseTravessia);
void AtualizarDisplayOLED(int estadoAtual, float tempoTotal, float tempoDecorrido, bool temPedestre = false, float velocidade = 0.0, float tempoExtra = 0.0);
void delayComDisplay(int estadoAtual, int tempoMS);

void setup()
{
  pinMode(pinTrigger, OUTPUT); // ADICIONADO: Configuração do sensor
  pinMode(pinEccho, INPUT);    // ADICIONADO: Configuração do sensor

  for(int i = 0 ; i < 3 ; i++)
  {
    pinMode(ledVeiculo[i], OUTPUT);
    if(i < 2) pinMode(ledPedestre[i], OUTPUT);
  }

  // Inicializa o Display OLED (Endereço I2C comum é 0x3C)
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    // Se o display falhar, o Arduino trava num loop infinito (pisque um led de erro se quiser)
    for(;;); 
  }
  
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
}

void loop()
{
  estado++;
  if(estado > 4) estado = 1; // CORRIGIDO: Impede o "ciclo fantasma"
  EstadoSemaforo(estado);
}

void AtualizarDisplayOLED(int estadoAtual, float tempoTotal, float tempoDecorrido, bool temPedestre, float velocidade, float tempoExtra) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0,0);

  // 1. Informações do Estado
  display.print(F("ESTADO: ")); display.println(estadoAtual);
  display.print(F("LEDs Ativos: "));
  
  if(estadoAtual == 1) display.println(F("Vd(Car) Vm(Ped)"));
  else if(estadoAtual == 2) display.println(F("Am(Car) Vm(Ped)"));
  else if(estadoAtual == 3) display.println(F("Vm(Car) Vd(Ped)"));
  else if(estadoAtual == 4) display.println(F("Vm(Car) Vm(Ped)"));

  // 2. Cronômetro do Semáforo
  display.print(F("Tempo: ")); 
  display.print(tempoDecorrido, 1); display.print(F("/")); 
  display.print(tempoTotal, 1); display.println(F(" s"));
  
  display.drawLine(0, 32, 128, 32, SSD1306_WHITE); // Linha divisória

  // 3. Informações do Sensor (Somente no Estado 3)
  if(estadoAtual == 3) {
    if(temPedestre) {
      display.println(F("-> PEDESTRE NA FAIXA"));
      display.print(F("Vel: ")); display.print(velocidade, 2); display.println(F(" m/s"));
      display.print(F("Acc. Tempo: +")); display.print(tempoExtra, 1); display.println(F(" s"));
    } else {
      display.println(F("-> FAIXA LIVRE"));
      display.println(F("Vel: -- m/s"));
      display.println(F("Acc. Tempo: 0.0 s"));
    }
  } else {
    display.println(F("Monitoramento Offline"));
  }

  display.display(); // Envia os dados para a tela
}

// Função para substituir os delays normais e manter a tela atualizando
void delayComDisplay(int estadoAtual, int tempoMS) {
  float tempoTotalSec = tempoMS / 1000.0;
  float tempoDecorridoSec = 0.0;
  float deltaT = 0.2; // Atualiza a tela a cada 200ms

  while(tempoDecorridoSec < tempoTotalSec) {
    AtualizarDisplayOLED(estadoAtual, tempoTotalSec, tempoDecorridoSec);
    delay(deltaT * 1000);
    tempoDecorridoSec += deltaT;
  }
}

float CalcularDistancia()
{
  digitalWrite(pinTrigger, LOW);
  delayMicroseconds(2);
  digitalWrite(pinTrigger, HIGH);
  delayMicroseconds(10);
  digitalWrite(pinTrigger, LOW);

  // Lê o tempo de retorno do eco em MICROSSEGUNDOS
  long duracao = pulseIn(pinEccho, HIGH);

  // Conversão direta para METROS (SI)
  // Velocidade do som = 343 m/s. 
  // Distância = (343 * duracao * 10^-6) / 2
  float distanciaMetros = duracao * 0.0001715; 

  return distanciaMetros;
}

void GerenciarTravessia(int tempoBaseTravessiaMS) 
{
  // --- VARIÁVEIS NO SISTEMA INTERNACIONAL (SI) ---
  float larguraAvenida = 5.0;           // Metros (m)
  float velocidadeLimiar = 0.8;         // Metros por segundo (m/s)
  float limiteMinimoSensor = 0.02;      // Metros (m) - Ponto cego do HC-SR04
  float limiteRuidoVelocidade = 0.05;   // Metros por segundo (m/s) - Filtro de objetos parados
  
  float deltaT = 0.2;                   // Segundos (s) - Intervalo de amostragem
  
  // Conversão do tempo que vem do Loop (ms) para Segundos (s)
  float tempoDecorrido = 0.0; 
  float tempoAtualTravessia = tempoBaseTravessiaMS / 1000.0; 

  float distanciaAnterior = CalcularDistancia(); // m

  while(tempoDecorrido < tempoAtualTravessia) {
    // Variáveis para o Display
    bool pedestreNaFaixa = false;
    float velExibicao = 0.0;
    float tExtraExibicao = 0.0;
    
    delay(deltaT * 1000); // O delay exige milissegundos, então multiplicamos na chamada
    tempoDecorrido += deltaT; 

    float distanciaAtual = CalcularDistancia(); // m

    // Verifica se a leitura é válida (Ignora o ponto cego e o que está fora da rua)
    if(distanciaAtual > limiteMinimoSensor && distanciaAtual < larguraAvenida){ 
      pedestreNaFaixa = true; // Gatilho visual para o OLED
      
    float dS = abs(distanciaAnterior - distanciaAtual); 
    float velocidadeMedia = dS / deltaT; // Mantém o módulo da velocidade
    velExibicao = velocidadeMedia; 
    
    if(velocidadeMedia > limiteRuidoVelocidade && velocidadeMedia < velocidadeLimiar){
      
      float distanciaRestante;
    
      // Descobre a direção do movimento
      if (distanciaAtual < distanciaAnterior) {
        // Pedestre se aproximando do sensor. O que falta é a distância atual.
        distanciaRestante = distanciaAtual;
      } else {
        // Pedestre se afastando do sensor. O que falta é a via menos onde ele está.
        distanciaRestante = larguraAvenida - distanciaAtual;
      }
    
      // Previne divisão por zero ou números negativos absurdos por ruído do sensor
      if (distanciaRestante > 0.0) {
        float tempoExtra = distanciaRestante / velocidadeMedia; 
        tExtraExibicao = tempoExtra; 
    
        if((tempoDecorrido + tempoExtra) > tempoAtualTravessia){
          tempoAtualTravessia = tempoDecorrido + tempoExtra;
    
          if(tempoAtualTravessia > 15.0){
            tempoAtualTravessia = 15.0;
          }
        }
      }
    }
    // Atualiza a tela a cada ciclo do loop
    AtualizarDisplayOLED(3, tempoAtualTravessia, tempoDecorrido, pedestreNaFaixa, velExibicao, tExtraExibicao);
    distanciaAnterior = distanciaAtual; 
  }
}

void EstadoSemaforo(int estado)
{
  switch(estado)
  {
    case 1: //Sinal Aberto, Pedestres Fechado
        digitalWrite(ledVeiculo[2], HIGH);
        digitalWrite(ledVeiculo[1], LOW);
        digitalWrite(ledVeiculo[0], LOW);
        digitalWrite(ledPedestre[1], LOW);
        digitalWrite(ledPedestre[0], HIGH);
        delayComDisplay(1, tempoVerde); // Novo delay inteligente
    break;

    case 2: //Sinal ficando Amarelo
        digitalWrite(ledVeiculo[2], LOW);
        digitalWrite(ledVeiculo[1], HIGH);
        digitalWrite(ledVeiculo[0], LOW);
        digitalWrite(ledPedestre[1], LOW);
        digitalWrite(ledPedestre[0], HIGH);
        delayComDisplay(2, tempoAmarelo);
        break;

    case 3: //Sinal Fechado, Pedestres Aberto
        digitalWrite(ledVeiculo[2], LOW);
        digitalWrite(ledVeiculo[1], LOW);
        digitalWrite(ledVeiculo[0], HIGH);
        digitalWrite(ledPedestre[1], HIGH);
        digitalWrite(ledPedestre[0], LOW);
        GerenciarTravessia(tempoPedestre);
        break;

    case 4: //Margem de Segurança
        digitalWrite(ledPedestre[1], LOW);
        digitalWrite(ledPedestre[0], HIGH);
        delayComDisplay(4, tempoEspera);
        break;
  }
}
