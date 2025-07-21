#include <soporte_placa.h>
#include <stm32l0xx.h> 
#include <stdint.h>

/*------------ IMPLEMENTACIÓN -------------*/

/**
 * @brief Rutina de servicio de interrupción de timer SysTick
 * 
 */
void SysTick_Handler(void);

/* Inicialización general */

/**
 * @brief Inicializa la librería. Es necesario llamar a
 * esta función antes de usar la misma.
 * 
 */
void SP_init(void){
    
    SystemCoreClockUpdate();
    
    uint32_t const frecuencia_hertz = SystemCoreClock;
    uint32_t const cuentas_por_milisgundo = frecuencia_hertz/1000;

    
    SysTick_Config(cuentas_por_milisgundo); 
}



/*------------ TEMPORIZACIÓN -------------*/

/**
 * @brief Variable actualizada una vez por milisegundo en el handler
 * de interrupción del timer del sistema (SysTick)
 * 
 */
static uint32_t volatile ticks;

/**
 * @brief Retardo con bloqueo durante un tiempo dado en milisegundos
 * @note Llamar a SP_init antes de usar.
 * @param tiempo Tiempo en milisegundos
 */
void SP_delay(uint32_t tiempo){
    uint32_t const ticks_inicial = ticks;
    uint32_t tiempo_transcurrido = ticks - ticks_inicial;
    while(tiempo_transcurrido < tiempo){
        tiempo_transcurrido = ticks - ticks_inicial;
    }
}


void SysTick_Handler(void){
    ++ticks;
}

/*------------ GPIO -------------*/

//Definimos una estructura Pin
typedef struct Pin{
    GPIO_TypeDef * puerto;
    int nrPin;
    uint32_t mascaraIOPEN;
}Pin;

static Pin const pines[SP_NUM_PINES] = {
    [SP_PB3]={.puerto=GPIOB,.nrPin=3,.mascaraIOPEN=RCC_IOPENR_GPIOBEN}, 
    [SP_PA2]={.puerto=GPIOA,.nrPin=2,.mascaraIOPEN=RCC_IOPENR_GPIOAEN}, 
    [SP_PA3]={.puerto=GPIOA,.nrPin=3,.mascaraIOPEN=RCC_IOPENR_GPIOAEN}, 
};


/**
 * @brief Obtiene un puntero a Pin a partir de su handle
 * 
 * @param hPin Handle
 * @return Pin const* Puntero al objeto Pin (solo lectura) 
 */
static Pin const * pinDeHandle(SP_HPin hPin){
    return &pines[hPin];
}

typedef enum Velocidad{VEL_BAJA,VEL_MEDIA,VEL_ALTA,VEL_MAX} Velocidad;


/**
 * @brief Configura el modo de salida 
 * 
 * @param pin 
 * @param vel 
 * @param openDrain 
 * @param funcionAlterna 
 */
static void modo_salida(Pin const *pin, Velocidad vel,  bool openDrain, bool funcionAlterna){
    

    int const offset_1 = pin->nrPin * 2;
    uint32_t const mascara_1 = 0x3; // 2 bits 


    //Configura salida como : open drain o push pull-------------------------
    if(openDrain){
        //Configura como open drain
        pin->puerto->OTYPER |= (1 << pin->nrPin);
    }
    else {
        //configura como push-pull
        pin->puerto->OTYPER |= (0 << pin->nrPin);
                
    }


    //Configura la salida como : Funcion Alternativa o de proposito general --------------------------
    if(funcionAlterna){
        //configurado como funcion alternativa
        pin->puerto->MODER = ( pin->puerto->MODER & ~(mascara_1 << offset_1))
                            | ((0b10 & mascara_1)<<offset_1);
    }
    else{
        //Configura el pin como GPIO
        pin->puerto->MODER = ( pin->puerto->MODER & ~(mascara_1 << offset_1))
                            | ((0b01 & mascara_1) << offset_1);
    }

    //Configuracion de la velocidad
    switch (vel)
    {
        case VEL_BAJA:
            //velocidad Baja
            pin->puerto->OSPEEDR = ( pin->puerto->OSPEEDR & ~(mascara_1 << offset_1))
                | ((0b00 & mascara_1)<<offset_1);

            break;
        case VEL_MEDIA:
            //velocidad media
            pin->puerto->OSPEEDR = ( pin->puerto->OSPEEDR & ~(mascara_1 << offset_1))
                | ((0b01 & mascara_1) << offset_1);

            break;
        case VEL_ALTA:
            //Velocidad ALta
            pin->puerto->OSPEEDR = ( pin->puerto->OSPEEDR & ~(mascara_1 << offset_1))
                | ((0b10 & mascara_1) << offset_1);
            break;
        case VEL_MAX:
            //Velocidad Maxima
            pin->puerto->OSPEEDR = ( pin->puerto->OSPEEDR & ~(mascara_1 << offset_1))
                | ((0b11 & mascara_1) << offset_1);
            break;
    }
}

typedef enum ModoPull{M_FLOTANTE,M_PULL_UP,M_PULL_DOWN} ModoPull;



/**
 * @brief Configura el Modo de Entrada
 * 
 * @param pin 
 * @param modoPull 
 */
static void modo_entrada(Pin const *pin,ModoPull modoPull){

    int const offset_2 = (pin->nrPin *2);

    uint32_t const mascara_2 = 0x3 ; // 2 bits

    //ENTRADA
    pin->puerto->MODER = ( pin->puerto->MODER & ~(mascara_2 << offset_2))
                        | ((0b00 & mascara_2)<<offset_2);

    //configura entrada en modo : FLOTANTE , PULL UP , PULL DOWN

    switch (modoPull){
        case M_FLOTANTE:
            //Modo Flotante
            pin->puerto->PUPDR = ( pin->puerto->PUPDR & ~(mascara_2 << offset_2))
                                | ((0b00 & mascara_2) << offset_2);
            break;
        case M_PULL_UP:
            //Modo Pull up
            pin->puerto->PUPDR = ( pin->puerto->PUPDR & ~(mascara_2 << offset_2))
                                             | ((0b01 & mascara_2)<<offset_2);

            pin->puerto->BSRR = (1 << pin->nrPin);
            break;
        case M_PULL_DOWN:
            //Modo Pull Down
            pin->puerto->PUPDR = ( pin->puerto->PUPDR & ~(mascara_2 << offset_2))
                                         | ((0b10 & mascara_2) << offset_2);
                    
            pin->puerto->BRR = (1 << pin->nrPin);
            break;        
    }
}


/**
 * @brief Configura el modo de un pin
 * 
 * @param hPin Handle al objeto Pin
 * @param modo Modo a configurar
 */
void SP_Pin_setModo(SP_HPin hPin,SP_Pin_Modo modo){
    if(hPin >= SP_NUM_PINES) return; 
    Pin const *pin = pinDeHandle(hPin); //Recuperamos el puntero
    __disable_irq();// Deshabita las interrupciones
    RCC->IOPENR |= pin->mascaraIOPEN;

    switch (modo){
        case SP_PIN_ENTRADA:
            modo_entrada(pin,M_FLOTANTE);
            break;
        case SP_PIN_ENTRADA_PULLUP:
            modo_entrada(pin,M_PULL_UP);
            break;
        case SP_PIN_ENTRADA_PULLDN:
            modo_entrada(pin,M_PULL_DOWN);
            break;
        case SP_PIN_SALIDA:
            modo_salida(pin,VEL_BAJA,false,false);
            break;
        case SP_PIN_SALIDA_OPEN_DRAIN:
            modo_salida(pin,VEL_BAJA,true,false);
            break;
        case SP_PIN_SALIDA_VA:
            modo_salida(pin,VEL_ALTA,false,false);
            break;
        case SP_PIN_ALTERNATIVO:
            modo_salida(pin,VEL_ALTA,false,true);
            break;
    }
    __enable_irq();
}


/**
 * @brief Lee el buffer de entrada de un Pin
 * 
 * @param pin Handle al objeto Pin
 * @return true Entrada ALTA
 * @return false Entrada BAJA
 */
bool SP_Pin_read(SP_HPin hPin){
   
    Pin const *pin = pinDeHandle(hPin);// Recuperamos el puntero
    
    return  (pin->puerto->IDR & (1 << pin->nrPin)); 
}

/**
 * @brief Escribe el buffer de salida de un Pin
 * 
 * @param pin Handle al objeto Pin
 * @param valor True: Salida ALTA. False: Salida BAJA.
 */
void SP_Pin_write(SP_HPin hPin, bool valor){
   Pin const *pin = pinDeHandle(hPin);// Recuperamos el puntero

    if(valor){
        pin->puerto->BSRR =(1 << pin->nrPin); 
    }else{
        pin->puerto->BRR = (1 << pin->nrPin); 
    }
   
}

/**
 * @brief Configura el multiplexor de función alternativa
 * para el pin (ver la tabla de la hoja de datos para 
 * opciones disponibles)
 * 
 * @param hPin Pin
 * @param nrFnAlt Número de función (0 a 15)
 */
void SP_Pin_selFnAlt(SP_HPin hPin,int nrFnAlt){
    enum{ MASCARA = 0xF }; // 4 bits

    Pin const *pin = pinDeHandle(hPin);
    int const offset = (pin->nrPin % 8)*4;
    int const indice = (pin->nrPin / 8)&1;

    pin->puerto->AFR[indice] =  (pin->puerto->AFR[0] & ~(MASCARA << offset))
                              | ((nrFnAlt & MASCARA) << offset);    
}
