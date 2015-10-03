#include "config.h"
#include "GPIO.h"
#include "delay.h"
#include "soft_uart.h"
#include "ir.h"


#define TURN_ON(x)        do {x = 0;} while(0)
#define TURN_OFF(x)       do {x = 1;} while(0)
#define REVERSE(x)        do {x ^= 1;} while(0)


void gpio_init(void)
{
    GPIO_InitTypeDef t_gpio_type;

    t_gpio_type.Mode = GPIO_HighZ;
    t_gpio_type.Pin = GPIO_Pin_0;	// UART RXD
    GPIO_Inilize(GPIO_P3, &t_gpio_type);

    t_gpio_type.Mode = GPIO_PullUp;
    t_gpio_type.Pin = GPIO_Pin_1;	// UART TXD
    GPIO_Inilize(GPIO_P3, &t_gpio_type);

    t_gpio_type.Mode = GPIO_PullUp;
    t_gpio_type.Pin = GPIO_Pin_2;	// Relay
    GPIO_Inilize(GPIO_P3, &t_gpio_type);
    TURN_OFF(RELAY);

    t_gpio_type.Mode = GPIO_PullUp;
    t_gpio_type.Pin = GPIO_Pin_3;	// LED
    GPIO_Inilize(GPIO_P3, &t_gpio_type);
    TURN_OFF(LED);

    t_gpio_type.Mode = GPIO_PullUp;
    t_gpio_type.Pin = GPIO_Pin_4;	// Beep
    GPIO_Inilize(GPIO_P3, &t_gpio_type);
    TURN_OFF(BEEP);

    t_gpio_type.Mode = GPIO_HighZ;
    t_gpio_type.Pin = GPIO_Pin_5;	// IR rev
    GPIO_Inilize(GPIO_P3, &t_gpio_type);
}

unsigned char Ir_Buf[4];
//char c=0;
//unsigned int i=0;
//unsigned int ir_loop_cnt = 0;


main()
{
    EA = 1;
    gpio_init();
    ir_rcv_init();

    while (1) {
        //uart_send(0xA3);
        //BEEP = !BEEP;
        //for (i=0; i<200; i++);
        if (ir_rcv(Ir_Buf)) {
            REVERSE(RELAY);

            TURN_ON(LED);

            uart_init();

            uart_send(Ir_Buf[0]);
            uart_send(Ir_Buf[1]);
            uart_send(Ir_Buf[2]);
            uart_send(Ir_Buf[3]);
            uart_send(0xFF);

            uart_deinit();

            TURN_OFF(LED);
        }
    }
}