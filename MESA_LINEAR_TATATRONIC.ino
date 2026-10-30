#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <FastAccelStepper.h>
#include <Keypad.h>

// ================= MAPEAMENTO DE PINOS (UNO) =================

// Driver Leadshine DM555 (Negativo Comum / Chaveando Positivo)
#define STEP_PIN    9   // Ligado no PUL+ (Obrigatório do FastAccelStepper)
#define DIR_PIN    10   // Ligado no DIR+ 

// Teclado 4x3 (7 Fios em linha)
byte rowPins[4] = {2, 3, 4, 5}; 
byte colPins[3] = {6, 7, 8}; 

// LCD 16x2 I2C (Pinos nativos no Arduino Uno)
LiquidCrystal_I2C lcd(0x27, 16, 2); 

// =============================================================

FastAccelStepperEngine engine = FastAccelStepperEngine();
FastAccelStepper *stepper = NULL;

// Passo ajustado: (685.714 * 20mm) / 12mm = 1142.857 passos por mm
const float stepsPerMm = 1142.857;

char keys[4][3] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, 4, 3);

float targetDistance = 0;
uint32_t currentSpeedHz = 2500; 

// Variáveis de Controle de Estado
bool isJogging = false; 
bool isAutoCycling = false;
bool autoMovingForward = true;
int32_t autoStartPos = 0;
int32_t autoEndPos = 0;

// Variáveis para controle de clique curto vs segurar por 2s (HOLD)
char heldKey = NO_KEY;
bool holdTriggered = false;
unsigned long lastSpeedChange = 0;

void setup() {
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("   TATATRONIC   ");
  delay(1500);

  engine.init();
  stepper = engine.stepperConnectToPin(STEP_PIN);
  
  if (stepper) {
    // 'false' inverte o sentido de giro tanto no Jog quanto no Vai-e-Vem
    stepper->setDirectionPin(DIR_PIN, false);
    stepper->setSpeedInHz(currentSpeedHz);
    stepper->setAcceleration(2500); 
  }

  // Tempo de segurar a tecla: 2000 ms (2 segundos)
  keypad.setHoldTime(2000); 

  showUI();
}

void loop() {
  char key = keypad.getKey();
  KeyState kState = keypad.getState();

  // --- 1. LÓGICA DO CICLO INFINITO (VAI E VEM) ---
  if (isAutoCycling && stepper) {
    if (!stepper->isRunning()) {
      // Chegou no destino, inverte a direção
      autoMovingForward = !autoMovingForward;
      if (autoMovingForward) {
        stepper->moveTo(autoEndPos);
      } else {
        stepper->moveTo(autoStartPos);
      }
    }
  }

  // --- 2. AO APERTAR UMA TECLA ---
  if (key != NO_KEY) {
    heldKey = key;
    holdTriggered = false;

    // Ações imediatas (* e #)
    if (key == '*') {
      // CANCELA TUDO E PARA IMEDIATAMENTE (SEM DESACELERAR)
      isAutoCycling = false;
      isJogging = false;
      targetDistance = 0;
      if (stepper) {
        stepper->forceStopAndNewPosition(stepper->getCurrentPosition());
      }
      showUI();
      holdTriggered = true; 
    } 
    else if (key == '#') {
      // INICIA O CICLO DE VAI E VEM
      if (stepper && targetDistance > 0 && !isAutoCycling) {
        isAutoCycling = true;
        autoStartPos = stepper->getCurrentPosition();
        autoEndPos = autoStartPos + (int32_t)(targetDistance * stepsPerMm);
        autoMovingForward = true;
        
        stepper->moveTo(autoEndPos);
        showJogUI("AUTO VAI-E-VEM");
      }
      holdTriggered = true;
    }
  }

  // --- 3. LÓGICA DE SEGURAR POR 2 SEGUNDOS (HOLD) ---
  if (kState == HOLD && stepper && !isAutoCycling) {
    if (heldKey == '4') { // 4 = Esquerda (Positivo)
      holdTriggered = true;
      stepper->runForward();
      if (!isJogging) { 
        showJogUI("<< ESQUERDA (4)"); 
        isJogging = true; 
      }
    } 
    else if (heldKey == '6') { // 6 = Direita (Negativo)
      holdTriggered = true;
      stepper->runBackward();
      if (!isJogging) { 
        showJogUI("DIREITA (6) >>"); 
        isJogging = true; 
      }
    }
    else if (heldKey == '0') { // Segurar '0' altera a velocidade em Hz
      holdTriggered = true;
      if (millis() - lastSpeedChange >= 500) {
        lastSpeedChange = millis();
        currentSpeedHz += 500;
        if (currentSpeedHz > 6000) currentSpeedHz = 500;
        stepper->setSpeedInHz(currentSpeedHz);
        showUI();
      }
    }
  }

  // --- 4. AO SOLTAR A TECLA (RELEASED) ---
  if (kState == RELEASED && heldKey != NO_KEY) {
    // Se estava em Jog manual e soltou o 4 ou 6, para a mesa
    if (isJogging && (heldKey == '4' || heldKey == '6')) {
      stepper->stopMove(); 
      isJogging = false;
      showUI();
    }
    // Se foi um clique rápido num número (0 a 9) sem ter ativado o HOLD
    else if (!holdTriggered && !isAutoCycling && (heldKey >= '0' && heldKey <= '9')) {
      targetDistance = (targetDistance * 10) + (heldKey - '0');
      showUI();
    }

    heldKey = NO_KEY;
  }
}

void showUI() {
  lcd.clear();
  // Linha 1: Distância
  lcd.setCursor(0, 0);
  lcd.print("Dist: ");
  lcd.print(targetDistance, 1); 
  lcd.print(" mm");
  
  // Linha 2: Velocidade
  lcd.setCursor(0, 1);
  lcd.print("Vel : ");
  lcd.print(currentSpeedHz);
  lcd.print(" Hz");
}

void showJogUI(const char* msg) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("   MOVIMENTO   ");
  lcd.setCursor(0, 1);
  lcd.print(msg);
}
