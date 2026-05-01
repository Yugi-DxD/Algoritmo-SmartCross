// C++ code
//
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

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

void setup()
{
  pinMode(pinTrigger, OUTPUT); // ADICIONADO: Configuração do sensor
  pinMode(pinEccho, INPUT);    // ADICIONADO: Configuração do sensor

  for(int i = 0 ; i < 3 ; i++)
  {
    pinMode(ledVeiculo[i], OUTPUT);
    if(i < 2) pinMode(ledPedestre[i], OUTPUT);
  }
}

void loop()
{
  estado++;
  if(estado > 4) estado = 1; // CORRIGIDO: Impede o "ciclo fantasma"
  EstadoSemaforo(estado);
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
    
    delay(deltaT * 1000); // O delay exige milissegundos, então multiplicamos na chamada
    tempoDecorrido += deltaT; 

    float distanciaAtual = CalcularDistancia(); // m

    // Verifica se a leitura é válida (Ignora o ponto cego e o que está fora da rua)
    if(distanciaAtual > limiteMinimoSensor && distanciaAtual < larguraAvenida){ 
      
      float dS = abs(distanciaAnterior - distanciaAtual); // m
      float velocidadeMedia = dS / deltaT;                // m/s

      // Filtra o ruído e verifica se a pessoa está lenta
      if(velocidadeMedia > limiteRuidoVelocidade && velocidadeMedia < velocidadeLimiar){
        
        // t = S / v  -> O tempo extra é a distância que falta dividida pela velocidade
        float tempoExtra = distanciaAtual / velocidadeMedia; // s

        // Se o tempo previsto for maior que o tempo que já temos, injeta mais tempo
        if((tempoDecorrido + tempoExtra) > tempoAtualTravessia){
          tempoAtualTravessia = tempoDecorrido + tempoExtra;

          // Trava de segurança limite absoluto (15 segundos)
          if(tempoAtualTravessia > 15.0){
            tempoAtualTravessia = 15.0;
          }
        }
      }
    }
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
        delay(tempoVerde);
    break;

    case 2: //Sinal ficando Amarelo
        digitalWrite(ledVeiculo[2], LOW);
        digitalWrite(ledVeiculo[1], HIGH);
        digitalWrite(ledVeiculo[0], LOW);
        digitalWrite(ledPedestre[1], LOW);
        digitalWrite(ledPedestre[0], HIGH);
        delay(tempoAmarelo);
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
        delay(tempoEspera);
        break;
  }
}
