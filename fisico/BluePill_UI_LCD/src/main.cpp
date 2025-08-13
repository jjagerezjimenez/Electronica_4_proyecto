#include <Arduino.h>
#include <LiquidCrystal.h>

// LCD 16x2 en modo 4 bits (alimentado a 3.3 V)
// RS=PB10, E=PB11, D4=PB12, D5=PB13, D6=PB14, D7=PB15
LiquidCrystal lcd(PB10, PB11, PB12, PB13, PB14, PB15);

// Botones con pull-up interno (activos en LOW)
const uint8_t PIN_BTN  = PA0;  // MENU
const uint8_t PIN_UP   = PA1;  // UP
const uint8_t PIN_DOWN = PA2;  // DOWN

// LED integrado de la BluePill (PC13, activo en LOW)
const uint8_t PIN_LED  = PC13;

/* ----- Tiempos de manejo de botón ---------------------------------------- */
const uint16_t ANTI_REBOTE_MS = 30;
const uint16_t DOBLE_CLIC_MS  = 350;
const uint16_t PULSO_LARGO_MS = 800;

/* ----- Estados del menú ------------------------------------ */
enum Menu { MENU_INICIO, MENU_INFO, MENU_CONF };
Menu menuActual = MENU_INICIO;

/* ----- Variables globales ---------------------------------- */
unsigned long tBajada = 0, tSubida = 0;     // marcas de tiempo MENU
unsigned long tPrimeraSubida = 0;           // para ventana de doble clic
bool clicPendiente    = false;               // posible clic simple
bool estadoAnterior   = HIGH;                // último estado leído de MENU

int  valorConfig = 0;                        // variable a ajustar
const int VAL_MIN = 0, VAL_MAX = 99;

/* ----------------------------------------------------------- */
void mostrarMenu() {
  lcd.clear();
  switch (menuActual) {
    case MENU_INICIO: lcd.print("Menu principal"); break;
    case MENU_INFO:   lcd.print("Info del sistema"); break;
    case MENU_CONF:   lcd.print("Config: "), lcd.print(valorConfig); break;
  }

  // Mensajes de depuración por Serial1 (opcional)
  Serial1.print("Estado: ");
  switch (menuActual) {
    case MENU_INICIO: Serial1.println("INICIO"); break;
    case MENU_INFO:   Serial1.println("INFO");   break;
    case MENU_CONF:   Serial1.print("CONFIG valor="); Serial1.println(valorConfig); break;
  }
}

/* ===================== SETUP =============================== */
void setup() {
  // Serie para depuración por USB-TTL en PA9/PA10
  Serial1.begin(115200);
  Serial1.println("Inicio OK");

  pinMode(PIN_LED,  OUTPUT);
  pinMode(PIN_BTN,  INPUT_PULLUP);
  pinMode(PIN_UP,   INPUT_PULLUP);
  pinMode(PIN_DOWN, INPUT_PULLUP);

  // Pequeña espera para estabilización del LCD a 3.3 V
  delay(50);
  lcd.begin(16, 2);
  mostrarMenu();
}

/* ===================== LOOP ================================ */
void loop() {
  // LED de vida (~1 Hz). PC13 es activo en LOW, por eso se invierte.
  digitalWrite(PIN_LED, (millis() & 512) ? LOW : HIGH);

  /* --------- Gestión del botón principal (MENU) ----------- */
  bool btn = digitalRead(PIN_BTN);

  // Flanco de bajada (inicio de pulsación)
  if (estadoAnterior == HIGH && btn == LOW) {
    delay(ANTI_REBOTE_MS);
    if (!digitalRead(PIN_BTN)) {
      tBajada = millis();
    }
  }

  // Flanco de subida (decisión de corto/doble/largo)
  if (estadoAnterior == LOW && btn == HIGH) {
    delay(ANTI_REBOTE_MS);
    if (digitalRead(PIN_BTN)) {
      tSubida = millis();
      const unsigned long dur = tSubida - tBajada;

      if (dur >= PULSO_LARGO_MS) {
        // Pulsación larga: ir a Configuración
        menuActual = MENU_CONF;
        clicPendiente = false;
        mostrarMenu();
      } else {
        // Posible corto / doble
        // Criterio exacto: intervalo entre la primera subida y la segunda bajada
        if (clicPendiente && (tBajada - tPrimeraSubida) < DOBLE_CLIC_MS) {
          // Doble clic: ir a Información
          menuActual = MENU_INFO;
          clicPendiente = false;
          mostrarMenu();
        } else {
          // Primer clic corto: queda pendiente hasta que venza la ventana
          clicPendiente  = true;
          tPrimeraSubida = tSubida;
        }
      }
    }
  }

  // Vencimiento de la ventana de doble clic -> clic simple
  if (clicPendiente && (millis() - tPrimeraSubida) > DOBLE_CLIC_MS) {
    menuActual = MENU_INICIO; // clic simple confirmado
    clicPendiente = false;
    mostrarMenu();
  }

  estadoAnterior = btn;

  /* --------- Botones UP / DOWN en modo configuración ------- */
  if (menuActual == MENU_CONF) {
    static bool prevUp = HIGH, prevDn = HIGH;
    bool upNow = digitalRead(PIN_UP);
    bool dnNow = digitalRead(PIN_DOWN);

    // SUBIR (flanco de bajada + antirrebote)
    if (prevUp == HIGH && upNow == LOW) {
      delay(ANTI_REBOTE_MS);
      if (!digitalRead(PIN_UP) && valorConfig < VAL_MAX) {
        ++valorConfig;
        mostrarMenu();
      }
    }

    // BAJAR (flanco de bajada + antirrebote)
    if (prevDn == HIGH && dnNow == LOW) {
      delay(ANTI_REBOTE_MS);
      if (!digitalRead(PIN_DOWN) && valorConfig > VAL_MIN) {
        --valorConfig;
        mostrarMenu();
      }
    }

    prevUp = upNow;
    prevDn = dnNow;
  }
}
