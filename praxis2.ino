#include <Servo.h>

// Definir los servomotores y sus pines correspondientes
Servo servo1;
Servo servo2;
Servo servo3;
Servo servo4;
Servo servo5;
Servo servo6;

#define sensorIni 8


const int posLimites[][7]={
  //Fun,  M1,   M2,   M3,   M4,   M5,   M6
  {1,     0,    45,   45,   0,    5,    40},      //limite inferior
  {2,     180,  155,  180,  180,  150,  120}      //limite superior
};

/*
const int posiciones[][7]={
  //Fun,  M1,   M2,   M3,   M5,   velocidade,   tiempo
  {0,     90,   110,  45,   90,   3000,         500},     //posicion inicial robot 
  {1,     25,   155,  170,  -1,   2500,         0},       // posiciona brazo atras y gira
  {1,     -1,   75,   -1,   150,  3500,         0},       //busca pieza
  {2000,  40,   0,    0,    0,    3000,         1000},    //Funcion 2000: significa cerrar mano; el segundo valor es la cantidad en % (0 - 100)
  {1,     -1,   155,  170,  90,   2000,         0},       //devuelve brazo
  {1,     140,  -1,   -1,   -1,   2000,         0},       //gira brazo
  {1,     -1,   95,   130,  100,  3500,         0},       //posiciona en deposito de pieza
  {2000,  170,  0,    0,    0,    3000,         1000},    //Funcion 2000: significa abri/cerrar mano; el segundo valor es la cantidad en % (0 - 100)
  {1,     90,   155,  170,  90,   1500,         0},       //gira brazo
};
*/

const int posiciones[][7]={
  //Fun,  M1,   M2,   M3,   M5,   velocidade,   tiempo
  {0,     90,   110,  45,   90,   3000,         500},     //posicion inicial robot 
  {1,     -1,   155,  170,  -1,   2500,         0},       // posiciona brazo atras y gira
  {1,     -1,   85,   -1,   140,  3500,         0},       //busca pieza
  {2000,  60,   0,    0,    0,    3000,         1000},    //Funcion 2000: significa cerrar mano; el segundo valor es la cantidad en % (0 - 100)
  {1,     -1,   155,  170,  90,   2500,         0},       // posiciona brazo atras y gira
  {1,     170,  -1,   -1,   -1,   2000,         0},       //gira brazo
  {1,     -1,   95,   130,  100,  3500,         0},       //posiciona en deposito de pieza
  {2000,  170,  0,    0,    0,    3000,         1000},    //Funcion 2000: significa abri/cerrar mano; el segundo valor es la cantidad en % (0 - 100)
  {1,     90,   155,  170,  90,   1500,         0},       //gira brazo
};

// Definir el tiempo total de movimiento (en milisegundos)
int move_time = 3000;

int mInicio = 0;

void setup() {

  Serial.begin(9600);                       //Start monitor
  Serial.println("Connecting...");

  pinMode(sensorIni, INPUT_PULLUP);
  
  // Inicializar los servomotores
  servo1.attach(2);
  servo2.attach(3);
  servo3.attach(4);
  servo4.attach(5);
  servo5.attach(6);
  servo6.attach(7);

  girarBrazo(6);          //gira brazo a pos Inicial
  abrircerrarMano(170);   //abre totalmente la mano
  moverBrazo(0);          //mueve el brazo a posicion inicial

  delay(2000);            //espera un tiempo para iniciar
}

void loop() {

  if(digitalRead(sensorIni)==LOW && mInicio==0){
    delay(50);
    if(digitalRead(sensorIni)==LOW){
        mInicio = 1;
    }
  }

if(mInicio==1){
  for(int i = 1; i<sizeof(posiciones)/sizeof(posiciones[0]); i++){
    if(posiciones[i][0]==2000){
      abrircerrarMano(posiciones[i][1]);
    }else if (posiciones[i][0]==3000){
      girarBrazo(posiciones[i][1]);
    }else{
      moverBrazo(i);
    }
    
    delay(posiciones[i][6]);
  }

  delay(2000);
  mInicio=0;
}
  
  
}


void moverBrazo(int index){

  move_time = posiciones[index][5];
  
  int posM1 = servo1.read();    //Posicion inicial que corresponde a la posicion actual del motor
  int posM2 = servo2.read();    //Posicion inicial que corresponde a la posicion actual del motor
  int posM3 = servo3.read();    //Posicion inicial que corresponde a la posicion actual del motor
  int posM5 = servo5.read();    //Posicion inicial que corresponde a la posicion actual del motor

  int posFinM1 = posiciones[index][1]; 
  int posFinM2 = posiciones[index][2];  
  int posFinM3 = posiciones[index][3]; 
  int posFinM5 = posiciones[index][4];  
  

// Calcular la velocidad de movimiento de cada motor
//el resultado da un factor muy bajo. Parecido con 0,02. Un para cada servo. Este valor se utiliza como valor multiplicador del incremento del angulo
  float speed1 = (posFinM1 - posM1) / (float) move_time;    
  float speed2 = (posFinM2 - posM2) / (float) move_time;
  float speed3 = (posFinM3 - posM3) / (float) move_time; 
  float speed5 = (posFinM5 - posM5) / (float) move_time;

  
  // Mover los servomotores simultáneamente en función del tiempo transcurrido
  //el start_time es para registrar el tiempo actual al inicio de la funcion
  //es un valor numerico que nunca se reinicia
  unsigned long start_time = millis();
   
  
  while (millis() - start_time <= move_time) {
    unsigned long elapsed_time = millis() - start_time;
    
    int angle1 = posM1 + speed1 * elapsed_time;
    int angle2 = posM2 + speed2 * elapsed_time;
    int angle3 = posM3 + speed3 * elapsed_time;
    int angle5 = posM5 + speed5 * elapsed_time;

    if(posFinM1>-1){servo1.write(angle1);}
    if(posFinM2>-1){servo2.write(angle2);}
    if(posFinM3>-1){servo3.write(angle3);}
    if(posFinM5>-1){servo5.write(angle5);}

    delay(20);
  }
  
  // Mover los servomotores a las posiciones finales (por si acaso se quedaron cortos)
  if(posFinM1>-1){servo1.write(posFinM1);}
  if(posFinM2>-1){servo2.write(posFinM2);}
  if(posFinM3>-1){servo3.write(posFinM3);}
  if(posFinM5>-1){servo5.write(posFinM5);}

}


void abrircerrarMano(int KPI_newPos){

  int posMINservo6  = 40;   //cerrado
  int posMAXservo6  = 170;  //abierto
  int posServoMano  = servo6.read();

  int newPos = posMINservo6+((KPI_newPos*80)/100);


  if(newPos>posServoMano){
    for (int thisPos = posServoMano; thisPos < newPos; thisPos++) {
      servo6.write(thisPos);
      delay(20);
    }
  }else{
    for (int thisPos = posServoMano; thisPos > newPos; thisPos--) {
      servo6.write(thisPos);
      delay(20);
    }
  }
}




// Función para mover un servomotor a un ángulo específico con una velocidad lenta
void girarBrazo(int newPos) {
  int posServo = servo4.read();
  Serial.println(posServo);

  if(newPos>posServo){
    for (int thisPos = posServo; thisPos < newPos; thisPos++) {
      servo4.write(thisPos);
      posServo=thisPos;
      delay(10);
    }
  }else{
    for (int thisPos = posServo; thisPos > newPos; thisPos--) {
      servo4.write(thisPos);
      posServo=thisPos;
      delay(10);
    }
  }
  
  Serial.print(" - S4: ");
  Serial.println(posServo); // the analog reading
}
