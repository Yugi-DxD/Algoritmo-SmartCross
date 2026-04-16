// C++ code
//

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

void GerenciarTravessia(int tempoBaseTravessia) // CORRIGIDO: Sem ponto e vírgula aqui
{
  float larguraAvenida = 500.0; // cm
  float velocidadeLimiar = 80.0; // cm/s - CORRIGIDO: Ponto e vírgula adicionado

  int tempoDecorrido = 0;
  int tempoAtualTravessia = tempoBaseTravessia;

  float distanciaAnterior = CalcularDistancia();

  while(tempoDecorrido < tempoAtualTravessia) {
    delay(200); // Esse será o nosso dT = 0.2s
    tempoDecorrido += 200; // CORRIGIDO: Nome da variável

    float distanciaAtual = CalcularDistancia();

    if(distanciaAtual > 2.0 && distanciaAtual < larguraAvenida){ // CORRIGIDO: Nome da variável
      float dS = abs(distanciaAnterior - distanciaAtual);
      float velocidadeMedia = dS / 0.2; // CORRIGIDO: S maiúsculo

      if(velocidadeMedia > 5.0 && velocidadeMedia < velocidadeLimiar){
        float tempoExtra = (distanciaAtual / velocidadeMedia) * 1000;

        if((tempoDecorrido + tempoExtra) > tempoAtualTravessia){
          tempoAtualTravessia = tempoDecorrido + tempoExtra;

          if(tempoAtualTravessia > 15000){
            tempoAtualTravessia = 15000;
          }
        }
      }
    }
    distanciaAnterior = distanciaAtual; // CORRIGIDO: Movido para DENTRO do laço while
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
