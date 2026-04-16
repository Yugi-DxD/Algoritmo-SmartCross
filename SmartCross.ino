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

  for(int i = 0 ; i < 3 ; i++)
  {
  pinMode(ledVeiculo[i], OUTPUT);
    if(i < 2) pinMode(ledPedestre[i], OUTPUT);
  }

}

void loop()
{
estado++;

  EstadoSemaforo(estado);
  if(estado > 4) estado = 1;

}

float CalcularDistancia()
{
  digitalWrite(pinTrigger, LOW);
  delayMicroseconds(2);
  digitalWrite(pinTrigger, HIGH);
  delayMicroseconds(10);
  digitalWrite(pinTrigger, LOW);

  // Lê o tempo de retorno do eco
  long duracao = pulseIn(pinEccho, HIGH);

  // Calcula a Distância do obstáculo até o sensor: D = v * t
  // onde "v" é a velocidade do som e "t" é o tempo
  float distancia = (0.034 * duracao) / 2;
  // Divide por 2 pois o pulso vai e volta, percorre o mesmo espaço duas vezes

  return distancia;
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
        delay(tempoPedestre);
        break;

    case 4: //Margem de Segurança
        digitalWrite(ledPedestre[1], LOW);
        digitalWrite(ledPedestre[0], HIGH);
        delay(tempoEspera);
        break;
  }

}
