#ifndef SOPORTE_PLACA_H
#define SOPORTE_PLACA_H 

#include <stdbool.h>
#include <stdint.h>


/**
 * @brief Handles correspondientes a los pines de entrada/salida, para 
 * usar en el parámetro hPin (primer parámetro) de las funciones SP_Pin_xxx
 * 
 * @note PA6 es el pin 6 del puerto A
 */
enum SP_Pines{
    SP_PB3, 
    SP_PA2,
    SP_PA3,
    SP_NUM_PINES
};



enum SP_PinesEspecial{ 
    SP_LED = SP_PB3 
    
};

//--------------------------------------------------------------------------------------
/**
 * @brief Handle que representa un objeto Pin.
 * Toma valores de las constantes SP_Pines
 * @note: Es el indice en una tabla 
 */
typedef unsigned SP_HPin; 



/**
 * @brief Definición de modo de operación de un Pin
 * 
 */
typedef enum SP_Pin_Modo{
    SP_PIN_ENTRADA,          // Entrada flotante
    SP_PIN_ENTRADA_PULLUP,   // Entrada con resistencia pull-up interna
    SP_PIN_ENTRADA_PULLDN,   // Entrada con resistencia pull-down interna
    SP_PIN_SALIDA,           // Salida push-pull
    SP_PIN_SALIDA_OPEN_DRAIN, // Salida con drenador abierto
    SP_PIN_SALIDA_VA, //Salida con velocidad alta 
    SP_PIN_ALTERNATIVO //Funcion ALternativa 
}SP_Pin_Modo;


void SP_Pin_setModo(SP_HPin hPin,SP_Pin_Modo modo);
void SP_Pin_selFnAlt(SP_HPin hPin,int nrFnAlt);
bool SP_Pin_read(SP_HPin hPin);
void SP_Pin_write(SP_HPin hPin, bool valor);
void SP_init(void);
void SP_delay(uint32_t tiempo);

#endif
