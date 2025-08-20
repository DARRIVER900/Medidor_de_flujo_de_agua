// Pines del ultrasónico
const int trigPin = 9;
const int echoPin = 10;

// Micrófono
const int sensorSonido = A1;

// Umbrales
int umbralDistanciaBajo = 17;   // cm, distancia donde se considera nivel bajo
int umbralSonido = 200;         // sonido del solenoide

// Tiempos
unsigned long tiempoNivelBajoInicio = 0;
const unsigned long tiempoConfirmacion = 5000;  // 5 segundos para confirmar fuga
unsigned long tiempoSinCambioInicio = 0;
const unsigned long tiempoSinCambioMax = 30000; // 30 segundos para estado "sin sonido o cambio"

enum EstadoSistema {
  NORMAL,
  NIVEL_BAJO_MOMENTANEO,
  POSIBLE_FUGA,
  SIN_SONIDO_O_CAMBIO
};

EstadoSistema estadoActual = NORMAL;

float distanciaAnterior = 0;

void setup() {
  Serial.begin(9600);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  distanciaAnterior = medirDistancia();
  tiempoSinCambioInicio = millis();
}

void loop() {
  float distancia = medirDistancia();           // en cm
  int sonido = analogRead(sensorSonido);
  unsigned long tiempoActual = millis();

  // Detectar si hubo cambio
  bool huboCambio = (abs(distancia - distanciaAnterior) > 1) || (sonido > umbralSonido);

  switch (estadoActual) {
    case NORMAL:
      if (distancia > umbralDistanciaBajo && sonido < umbralSonido) {
        tiempoNivelBajoInicio = tiempoActual;
        estadoActual = NIVEL_BAJO_MOMENTANEO;
        Serial.println("Nivel bajo momentáneo detectado.");
      } else {
        Serial.println("Estado Normal.");
      }
      break;

    case NIVEL_BAJO_MOMENTANEO:
      if (distancia <= umbralDistanciaBajo || sonido > umbralSonido) {
        estadoActual = NORMAL;
        Serial.println("Nivel recuperado o sonido detectado. Volviendo a normal.");
      } else if (tiempoActual - tiempoNivelBajoInicio > tiempoConfirmacion) {
        estadoActual = POSIBLE_FUGA;
        Serial.println("⚠ Posible fuga confirmada.");
      } else {
        Serial.println("Esperando confirmación de fuga...");
      }
      break;

    case POSIBLE_FUGA:
      if (distancia <= umbralDistanciaBajo || sonido > umbralSonido) {
        estadoActual = NORMAL;
        Serial.println("Nivel recuperado o sonido detectado. Volviendo a normal.");
      } else {
        Serial.println("⚠ Estado: Posible fuga activa.");
      }
      break;

    case SIN_SONIDO_O_CAMBIO:
      if (huboCambio) {
        estadoActual = NORMAL;
        tiempoSinCambioInicio = tiempoActual;
        Serial.println("Cambio detectado, volviendo a normal.");
      } else {
        if (tiempoActual - tiempoSinCambioInicio > tiempoSinCambioMax) {
          Serial.println("⚠ Sin sonido ni cambio por mucho tiempo. Revisar sistema.");
        } else {
          Serial.println("Sin sonido ni cambio, esperando.");
        }
      }
      break;
  }

  // Actualizar estado "sin sonido o cambio"
  if (!huboCambio && estadoActual == NORMAL) {
    if (tiempoActual - tiempoSinCambioInicio > tiempoSinCambioMax) {
      estadoActual = SIN_SONIDO_O_CAMBIO;
      Serial.println("Estado cambiado a sin sonido o cambio.");
    }
  } else if (huboCambio && estadoActual == SIN_SONIDO_O_CAMBIO) {
    estadoActual = NORMAL;
    tiempoSinCambioInicio = tiempoActual;
    Serial.println("Cambio detectado, volviendo a normal.");
  }

  distanciaAnterior = distancia;
  delay(1000);
}

// Función para medir distancia con ultrasónico
float medirDistancia() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duracion = pulseIn(echoPin, HIGH);
  float distancia = duracion * 0.0343 / 2; // en cm

  Serial.print("Distancia: ");
  Serial.print(distancia);
  Serial.print(" cm | Sonido: ");
  Serial.println(analogRead(sensorSonido));

  return distancia;
}

