#include <soporte_placa.h>
#include "usart_config.h"

/*Habilitar el reloj para USART*/
//PAGINA 201 DEL MANUAL DE REFERENCIA DE LA FAMILIA
static void habilitar_reloj_usart (void){
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
}

//PAGINA 196 DEL MANUAL 
//habilita el reloj del puerto A
static void habilitar_reloj_GPIOA (void){
    RCC->IOPENR |= RCC_IOPENR_IOPAEN;
}


//PAGINA 194 DEL MANUAL
static void usart_reset(void){
    RCC->APB1RSTR |= RCC_APB1RSTR_USART2RST;
    RCC->APB1RSTR &= ~RCC_APB1RSTR_USART2RST;
}

static void configuracion_usart(uint32_t baudrate){
    usart_reset();
    USART2->BRR = SystemCoreClock/baudrate; 
    USART2->CR1 |= (USART_CR1_UE | USART_CR1_TE | USART_CR1_RE) ;    
   
}

void USART2_Init(uint32_t baudrate) {
    // Habilitar el reloj para USART2 y el GPIOA
    SystemCoreClockUpdate();
    habilitar_reloj_usart();
    habilitar_reloj_GPIOA();

    // Configurar pines para USART2 (PA2 como TX y PA3 como RX)
    SP_Pin_setModo(SP_PA2, SP_PIN_ALTERNATIVO);
    SP_Pin_selFnAlt(SP_PA2,4);

    // Configurar PA9 como salida funcion alternativa PUSH/PULL de velocidad media
    SP_Pin_setModo(SP_PA3, SP_PIN_ENTRADA_PULLUP);
    SP_Pin_selFnAlt(SP_PA3,4);

    // Configurar los parámetros del USART2
    configuracion_usart(baudrate);
}

void USART2_write(uint8_t c) {
    while(!(USART2->ISR & USART_ISR_TXE));
    USART2->TDR = c;
}

bool USART2_read(char *caracter) {
    bool bandera=false;
    if(USART2->ISR & USART_ISR_RXNE){
        *caracter= (char) USART2->RDR;
        bandera=true;
    }
    return bandera;
}

/**
 * @brief Recorre la cadena de caracteres y envia caracter por caracter
 * a traves del USART2 utilizando la funcion USART_write()
 * 
 * @param string 
 */
void USART2_Write_String(const char* string){
    while(*string != '\0'){
        USART2_write(*string);
        string++;
    }
}