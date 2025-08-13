#include <LiquidCrystal.h>

/* ----- LCD 16×2 en modo 4 bits (RS,E,D4,D5,D6,D7) ---------- */
LiquidCrystal lcd(11, 12, 3, 4, 5, 6);

/* ----- Pines ------------------------------------------------*/
const uint8_t PIN_LED   = 13;   // LED de usuario
const uint8_t PIN_BTN   = 2;    // Botón principal (menú)
const uint8_t PIN_UP    = A0;   // SUBIR  (btn2)
const uint8_t PIN_DOWN  = A1;   // BAJAR  (btn3)

/* ----- Tiempos de manejo de botón ---------------------------------------- */
const uint16_t ANTI_REBOTE_MS = 30;
const uint16_t DOBLE_CLIC_MS  = 350;
const uint16_t PULSO_LARGO_MS = 800;

/* ----- Estados del menú ------------------------------------ */
enum Menu { MENU_INICIO, MENU_INFO, MENU_CONF };
Menu menuActual = MENU_INICIO;

/* ----- Variables globales ---------------------------------- */
unsigned long tBajada = 0, tSubida = 0;   // marcas de tiempo
bool clicPendiente    = false;            // posible clic simple
bool estadoAnterior   = HIGH;

int  valorConfig = 0;              // variable a ajustar
const int VAL_MIN = 0, VAL_MAX = 99;

/* ----------------------------------------------------------- */
void mostrarMenu() {
  lcd.clear();
  switch (menuActual) {
    case MENU_INICIO: lcd.print("Menu principal"); break;
    case MENU_INFO:   lcd.print("Info del sistema"); break;
    case MENU_CONF:   lcd.print("Config: "), lcd.print(valorConfig); break;
  }
}

/* ===================== SETUP =============================== */
void setup() {
  pinMode(PIN_LED,  OUTPUT);
  pinMode(PIN_BTN,  INPUT_PULLUP);
  pinMode(PIN_UP,   INPUT_PULLUP);
  pinMode(PIN_DOWN, INPUT_PULLUP);

  lcd.begin(16, 2);   // inicializa el LCD
  mostrarMenu();      // muestra la primera pantalla
}

/* ===================== LOOP ================================ */
void loop() {
  /* LED parpadea ~1 Hz para indicar que corre */
  digitalWrite(PIN_LED, millis() & 512);

  /* --------- Gestión del botón principal (menús) ---------- */
  bool btn = digitalRead(PIN_BTN);

  /* Flanco de bajada */
  if (estadoAnterior == HIGH && btn == LOW) {
    delay(ANTI_REBOTE_MS);
    if (!digitalRead(PIN_BTN)) tBajada = millis();
  }

  /* Flanco de subida */
  if (estadoAnterior == LOW && btn == HIGH) {
    delay(ANTI_REBOTE_MS);
    if (digitalRead(PIN_BTN)) {
      tSubida = millis();
      unsigned long dur = tSubida - tBajada;

      if (dur >= PULSO_LARGO_MS) {            // pulsación larga
        menuActual = MENU_CONF;
        clicPendiente = false;
        mostrarMenu();
      } else {                                // posible clic corto
        if (clicPendiente && (tSubida - tBajada) < DOBLE_CLIC_MS) {
          menuActual = MENU_INFO;             // doble clic
          clicPendiente = false;
          mostrarMenu();
        } else {
          clicPendiente = true;               // primer clic
        }
      }
    }
  }

  /* Ventana de doble clic expirada -> clic simple */
  if (clicPendiente && (millis() - tSubida) > DOBLE_CLIC_MS) {
    menuActual = MENU_INICIO;                 // clic simple confirmado
    clicPendiente = false;
    mostrarMenu();
  }

  estadoAnterior = btn;

  /* --------- Botones UP / DOWN en modo configuración -------- */
  if (menuActual == MENU_CONF) {
    static bool prevUp = HIGH, prevDn = HIGH;
    bool upNow = digitalRead(PIN_UP);
    bool dnNow = digitalRead(PIN_DOWN);

    if (prevUp == HIGH && upNow == LOW) {         // SUBIR
      delay(ANTI_REBOTE_MS);
      if (!digitalRead(PIN_UP) && valorConfig < VAL_MAX) {
        ++valorConfig;
        mostrarMenu();
      }
    }

    if (prevDn == HIGH && dnNow == LOW) {         // BAJAR
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
