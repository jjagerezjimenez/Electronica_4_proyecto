#include <soporte_placa.h>
#include <usart_config.h>

#define LUZ_ON 1
#define LUZ_OFF 0

int main(void){
    SP_init();  //Inicializador
    USART2_Init(115200);
    SP_Pin_setModo(SP_LED, SP_PIN_SALIDA_VA);

    USART2_Write_String("Hola mundo\n");
    while(1){
        SP_Pin_write(SP_LED,LUZ_OFF);
        SP_delay(500);
        SP_Pin_write(SP_LED,LUZ_ON);
        SP_delay(500);
    }
    return 0;
}