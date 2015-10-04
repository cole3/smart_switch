#include "config.h"
#include "GPIO.h"
#include "delay.h"
#include "soft_uart.h"
#include "ir.h"


#define CONFIG_WATCHDOG

#define OFF               0
#define ON                1
#define REV               2


#define TURN_ON(x)        do {x = 0;} while(0)
#define TURN_OFF(x)       do {x = 1;} while(0)
#define REVERSE(x)        do {x = !x;} while(0)


code unsigned long _key_on[][4] = {
    {0x2D, 0x2D, 0x58, 0xA7},
    {0x1A, 0xA1, 0xDC, 0x73},
    {0xC0, 0x01, 0x10, 0x2E}
};
code unsigned long _key_off[][4]  = {
    {0x2D, 0x2D, 0x30, 0xCF},
    {0x1A, 0xA1, 0xDC, 0x43},
    {0xC0, 0x01, 0x10, 0x2E}
};


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

void beep_play(bit on)
{
    unsigned int i, n;

    if (on) {
        for (n=0; n<300; n++) {
            REVERSE(BEEP);
            for (i=0; i<300; i++);
        }
    }

    for (n=0; n<1000; n++) {
        REVERSE(BEEP);
        for (i=0; i<100; i++);
    }

    if (!on) {
        for (n=0; n<300; n++) {
            REVERSE(BEEP);
            for (i=0; i<300; i++);
        }
    }

    TURN_OFF(BEEP);
}

unsigned char check_key(unsigned char *key)
{
    unsigned char size = sizeof(_key_on) / sizeof(_key_on[0]);
    unsigned char i;
    unsigned char state = 0xFF;

    for (i=0; i<size; i++) {
        if (state != REV &&
            _key_on[i][0] == key[0] &&
            _key_on[i][1] == key[1] &&
            _key_on[i][2] == key[2] &&
            _key_on[i][3] == key[3]) {
            state = (state == OFF) ? REV : ON;
        }

        if (state != REV &&
            _key_off[i][0] == key[0] &&
            _key_off[i][1] == key[1] &&
            _key_off[i][2] == key[2] &&
            _key_off[i][3] == key[3]) {
            state = (state == ON) ? REV : OFF;
        }
    }

    return state;
}


unsigned char Ir_Buf[4];
bit state = OFF;
bit change = FALSE;
unsigned char cnt = 0;

main()
{
#ifdef CONFIG_WATCHDOG
    WDT_reset(D_WDT_SCALE_128);
#endif

    EA = 1;
    gpio_init();
    ir_rcv_init();

    TURN_ON(LED);
    delay_ms(500);
    TURN_OFF(LED);

    while (1) {
#ifdef CONFIG_WATCHDOG
        WDT_reset(D_WDT_SCALE_64);
#endif
        if (ir_rcv(Ir_Buf)) {

            uart_init();
            uart_send(Ir_Buf[0]);
            uart_send(Ir_Buf[1]);
            uart_send(Ir_Buf[2]);
            uart_send(Ir_Buf[3]);
            uart_send(0xFF);
            uart_deinit();

            switch (check_key(Ir_Buf)) {
            case ON:
                state = ON;
                change = TRUE;
                break;
            case OFF:
                state = OFF;
                change = TRUE;
                break;
            case REV:
                state = !state;
                change = TRUE;
                break;
            default:
                change = FALSE;
                break;
            }
        }

        if (change) {
            change = FALSE;

            if (state == ON) {
                TURN_ON(RELAY);
            } else {
                TURN_OFF(RELAY);
            }

            beep_play(state);

            TURN_ON(LED);
            delay_ms(200);
            TURN_OFF(LED);
        }
    }
}