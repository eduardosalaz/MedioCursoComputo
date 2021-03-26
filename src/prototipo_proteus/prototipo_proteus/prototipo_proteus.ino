#define buzPin A3
#define enPin 8 //pin de enable
#define cDirPin 5 //pin de direccion de CYAN
#define cStepPin 2 //pin de paso de CYAN
#define mDirPin 6 //pin de direccion de MAGENTA 
#define mStepPin 3 //pin de paso de MAGENTA 
#define yDirPin 7 //pin de dirección de YELLOW
#define yStepPin 4 //pin de paso de YELLOW
#define kDirPin 13 //pin de dirección de KEY
#define kStepPin 12 //pin de paso de KEY

unsigned long rpm = 100; //el NEMA17 va de 100 a 400 rpm, checar si el usuario puede decidirlo
const float anguloMotor = 1.8; //sale de la datasheet
const float tamanioPaso = 1;

const float CSpU = 100; //CYAN STEPS per UNIT
//basicamente, cuantos pasos se requieren para bombear un mililitro
//este valor es placeholder y se tiene que calibrar cuando se haga en físico
const float MSpU = 100; //MAGENTA STEPS per UNIT
const float YSpU = 100; //YELLOW STEPS per UNIT
const float KSpU = 100; //KEY STEPS per UNIT
//por qué no se usa B de Black? porque se confunde con B de Blue en RGB

bool limpiado = true, bombeado = false;
int Red=-1, Green=-1, Blue=-1, Volumen=-1;
unsigned long Cyan, Magenta, Yellow, Key;
float CSteps, MSteps, YSteps, KSteps;
String userInput = "a";

void setup() {
  pinMode(enPin, OUTPUT);
  pinMode(cDirPin, OUTPUT);
  pinMode(cStepPin, OUTPUT);
  pinMode(mDirPin, OUTPUT);
  pinMode(mStepPin, OUTPUT);
  pinMode(yDirPin, OUTPUT);
  pinMode(yStepPin, OUTPUT);
  pinMode(kDirPin, OUTPUT);
  pinMode(kStepPin, OUTPUT);
  pinMode(buzPin, OUTPUT);
  digitalWrite(enPin, HIGH);
  
  Serial.begin(9600);
  Serial.println("Empezando Mixer: ");
  Serial.println("Comandos: Cargar, Mezclar, Limpiar");
}

void loop() {
  if(Serial.available() > 0){
    userInput = Serial.readString();
    userInput.trim();

    Serial.print("Recibi: ");
    Serial.print(userInput);
    Serial.println();

    switch(tolower(userInput[0])){
      case 'm':
        Serial.println();
        Serial.println("Mezclar pintura");
        if(!limpiado && bombeado){
          Serial.println("Ingresar cantidad de color rojo a bombear(0-255): ");
          Red = Serial.parseInt();
          delay(1000);
          //Los whiles los introduci por la simulacion de PROTEUS
          //en el prototipado los removere
          while(Red==-1){
            delay(1000); 
          }

          Serial.println("\nIngresar cantidad de color verde a bombear(0-255): ");
          Green = Serial.parseInt();
          delay(1000);
          while(Green == -1){
            delay(1000);
          }

          Serial.println("\nIngresar cantidad de color azul a bombear(0-255): ");
          Blue = Serial.parseInt();
          delay(1000);
          while(Blue == -1){
            delay(1000);
          }

          Serial.println("\nIngresar volumen en mililitros: ");
          Volumen = Serial.parseInt();
          delay(1000);
          while(Volumen == -1){
            delay(1000);
          }


          Serial.println("Colores: ");
          Serial.print("Rojo: ");
          Serial.print(Red);
          Serial.println();
          Serial.print("Verde: ");
          Serial.print(Green);
          Serial.println();
          Serial.print("Azul: ");
          Serial.print(Blue);
          Serial.println();
          Serial.print("Volumen: ");
          Serial.print(Volumen);
          Serial.println();

          pasarCMYK(Cyan, Magenta, Yellow, Key);
          obtenerPasos(CSteps, MSteps, YSteps, KSteps);
          mezclarColores();
        }else{
          Serial.println("Carga las bombas e intentalo de nuevo");
        }
        break;
      
      case 'c':
        if(limpiado){
          cargarBombas();
          bombeado = true;
          limpiado = false;
          Serial.println("Bombas cargadas");
        }else{
          Serial.println("Bombas ya cargadas");
        }
      break;

      case 'l':
        if(!limpiado) {
            limpiar();
            limpiado = true;
            bombeado = false;
            Serial.println(" ");
          }
          else {
            Serial.println("Bombas Limpias!");
            Serial.println(" ");
          }
      break;


      default:
        Serial.println("Ingresa un comando válido: Mezclar, Cargar, Limpiar");
        break;

    }

  }
  // put your main code here, to run repeatedly:
}

void pasarCMYK(unsigned long &Cyan, unsigned long &Magenta, unsigned long &Yellow, unsigned long &Key) {
  Serial.println("Convertiendo de RGB a CMYK (0-255 => 0-100)");
  float R = (float)Red/255.0;
  float G = (float)Green/255.0;
  float B = (float)Blue/255.0;
  float K = (1 - max(max(R, G),B));
  float C = (1 - R - K)/(1 - K);
  float M = (1 - G - K)/(1 - K);
  float Y = (1 - B - K)/(1 - K);
  Cyan = (float)C*100;
  Magenta = (float)M*100;
  Yellow = (float)Y*100;
  Key = (float)K*100;
  Serial.print("CMYK : ");Serial.print(Cyan);Serial.print(" ");Serial.print(Magenta);Serial.print(" ");Serial.print(Yellow);Serial.print(" ");Serial.println(Key);
}


void obtenerPasos(float &CSteps, float &MSteps, float &YSteps, float &KSteps) {
  float Cvol = ((float)(Cyan*Volumen))/100;
  float Mvol = ((float)(Magenta*Volumen))/100;
  float Yvol = ((float)(Yellow*Volumen))/100;
  float Kvol = ((float)(Key*Volumen))/100;

  CSteps = Cvol * CSpU;
  MSteps = Mvol * MSpU;
  YSteps = Yvol * YSpU;
  KSteps = Kvol * KSpU;

  Serial.println(" ");
  Serial.print("Volumen: ");Serial.print(Cvol);Serial.print(" ");Serial.print(Mvol);Serial.print(" ");Serial.print(Yvol);Serial.print(" ");Serial.println(Kvol);
  Serial.println(" ");
}

void cargarBombas() {
  digitalWrite(enPin, LOW);
  Serial.print("Cargando pintura en las bombas...");
  rotarCyan(3000, rpm);
  Serial.print("...");
  rotarMagenta(3000, rpm);
  Serial.print("...");
  rotarYellow(3000, rpm);
  Serial.print("...");
  rotarKey(3000, rpm);
  Serial.println("...Listo");
}

void limpiar() {
  digitalWrite(enPin, LOW);
  Serial.print("Limpiando bombas...");
  rotarCyan(-3000, rpm);
  Serial.print("...");
  rotarMagenta(-3000, rpm);
  Serial.print("...");
  rotarYellow(-3000, rpm);
  Serial.print("...");
  rotarKey(-3000, rpm);
  Serial.println("...Listo");
  digitalWrite(enPin, HIGH);
}

void mezclarColores() {
  digitalWrite(enPin, LOW);
  if (CSteps != 0) {
    Serial.print("Bombeando Cyan con ");
    Serial.print(CSteps);
    Serial.print(" Pasos");
    rotarCyan(CSteps, rpm);
    Serial.println(" Listo");
    setBuzz();
  }
  if (MSteps != 0) {
    Serial.print("Bombeando Magenta con ");
    Serial.print(MSteps);
    Serial.print(" Pasos");
    rotarMagenta(MSteps, rpm);
    Serial.println(" Listo");
    setBuzz();
  }
  if (YSteps != 0) {
    Serial.print("Bombeando Yellow con ");
    Serial.print(YSteps);
    Serial.print(" Pasos");
    rotarYellow(YSteps, rpm);
    Serial.println(" Listo");
    setBuzz();
  }
  if (KSteps != 0) {
    Serial.print("Bombeando Key con ");
    Serial.print(KSteps);
    Serial.print(" Pasos");
    rotarKey(KSteps, rpm);
    Serial.println(" Listo");
    setBuzz();
  }
    
  Serial.println(" ");
  Serial.println("Todos los colores bombeados");
  Serial.println(" ");
}

void setBuzz() {
  analogWrite(buzPin, 255);
  delay(1000);
  analogWrite(buzPin, 0);
  delay(50);
}

void rotarCyan(float pasos, float rpm) {
  float pasosPorRotacion = (360.00 / anguloMotor
) / tamanioPaso;
  unsigned long periodoPasoMs = ((60.0000 / (rpm * pasosPorRotacion)) * 1E6 / 2.0000) - 5;
  //milisegundos
  
  if (pasos > 0) { //si el signo de los pasos es negativo, manda el pin de direccion un low
    digitalWrite(cDirPin, HIGH);
  }
  else {
    digitalWrite(cDirPin, LOW);
  }
  
  if (pasos > 0) {
    float pasosTotales = pasos;
    for (unsigned long i = 0; i < pasosTotales; i++) {
      digitalWrite(cStepPin, HIGH);
      delayMicroseconds(periodoPasoMs);
      digitalWrite(cStepPin, LOW);
      delayMicroseconds(periodoPasoMs);
    }
  }
  else {
    float pasosTotales = pasos * -1;
    for (unsigned long i = 0; i < pasosTotales; i++) {
      digitalWrite(cStepPin, HIGH);
      delayMicroseconds(periodoPasoMs);
      digitalWrite(cStepPin, LOW);
      delayMicroseconds(periodoPasoMs);
    }
  }
}

void rotarMagenta(float pasos, float rpm) {
  float pasosPorRotacion = (360.00 / anguloMotor
) / tamanioPaso;
  unsigned long periodoPasoMs = ((60.0000 / (rpm * pasosPorRotacion)) * 1E6 / 2.0000) - 5;
  
  if (pasos > 0) {
    digitalWrite(mDirPin, HIGH);
  }
  else {
    digitalWrite(mDirPin, LOW);
  }
  
  if (pasos > 0) {
    float pasosTotales = pasos;
    for (unsigned long i = 0; i < pasosTotales; i++) {
      digitalWrite(mStepPin, HIGH);
      delayMicroseconds(periodoPasoMs);
      digitalWrite(mStepPin, LOW);
      delayMicroseconds(periodoPasoMs);
    }
  }
  else {
    float pasosTotales = pasos * -1;
    for (unsigned long i = 0; i < pasosTotales; i++) {
      digitalWrite(mStepPin, HIGH);
      delayMicroseconds(periodoPasoMs);
      digitalWrite(mStepPin, LOW);
      delayMicroseconds(periodoPasoMs);
    }
  }
}

void rotarYellow(float pasos, float rpm) {
  float pasosPorRotacion = (360.00 / anguloMotor
) / tamanioPaso;
  unsigned long periodoPasoMs = ((60.0000 / (rpm * pasosPorRotacion)) * 1E6 / 2.0000) - 5;
  
  if (pasos > 0) {
    digitalWrite(yDirPin, HIGH);
  }
  else {
    digitalWrite(yDirPin, LOW);
  }
  
  if (pasos > 0) {
    float pasosTotales = pasos;
    for (unsigned long i = 0; i < pasosTotales; i++) {
      digitalWrite(yStepPin, HIGH);
      delayMicroseconds(periodoPasoMs);
      digitalWrite(yStepPin, LOW);
      delayMicroseconds(periodoPasoMs);
    }
  }
  else {
    float pasosTotales = pasos * -1;
    for (unsigned long i = 0; i < pasosTotales; i++) {
      digitalWrite(yStepPin, HIGH);
      delayMicroseconds(periodoPasoMs);
      digitalWrite(yStepPin, LOW);
      delayMicroseconds(periodoPasoMs);
    }
  }
}

void rotarKey(float pasos, float rpm) {
  float pasosPorRotacion = (360.00 / anguloMotor
) / tamanioPaso;
  unsigned long periodoPasoMs = ((60.0000 / (rpm * pasosPorRotacion)) * 1E6 / 2.0000) - 5;
  
  if (pasos > 0) {
    digitalWrite(kDirPin, HIGH);
  }
  else {
    digitalWrite(kDirPin, LOW);
  }
  
  if (pasos > 0) {
    float pasosTotales = pasos;
    for (unsigned long i = 0; i < pasosTotales; i++) {
      digitalWrite(kStepPin, HIGH);
      delayMicroseconds(periodoPasoMs);
      digitalWrite(kStepPin, LOW);
      delayMicroseconds(periodoPasoMs);
    }
  }
  else {
    float pasosTotales = pasos * -1;
    for (unsigned long i = 0; i < pasosTotales; i++) {
      digitalWrite(kStepPin, HIGH);
      delayMicroseconds(periodoPasoMs);
      digitalWrite(kStepPin, LOW);
      delayMicroseconds(periodoPasoMs);
    }
  }
}
