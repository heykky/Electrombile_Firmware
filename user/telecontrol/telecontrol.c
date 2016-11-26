/*
 * telecontrol.c
 *
 *  Created on: 2016/11/25
 *      Author: lc
 */
#include <eat_periphery.h>

#include "telecontol.h"

void telecontrol_initial(void)
{
    eat_gpio_setup(EAT_PIN60_COL2, EAT_GPIO_DIR_INPUT, EAT_GPIO_LEVEL_LOW);
    eat_gpio_setup(EAT_PIN62_COL0, EAT_GPIO_DIR_INPUT, EAT_GPIO_LEVEL_LOW);

    eat_gpio_setup(EAT_PIN59_COL3, EAT_GPIO_DIR_OUTPUT, EAT_GPIO_LEVEL_HIGH);
    eat_gpio_setup(EAT_PIN61_COL1, EAT_GPIO_DIR_OUTPUT, EAT_GPIO_LEVEL_HIGH);
}

void telecontrol_cut(void)
{
    eat_gpio_setup(EAT_PIN59_COL3, EAT_GPIO_DIR_OUTPUT, EAT_GPIO_LEVEL_LOW);
    eat_gpio_setup(EAT_PIN61_COL1, EAT_GPIO_DIR_OUTPUT, EAT_GPIO_LEVEL_LOW);
}

void telecontrol_connect(void)
{
    eat_gpio_setup(EAT_PIN59_COL3, EAT_GPIO_DIR_OUTPUT, EAT_GPIO_LEVEL_HIGH);
    eat_gpio_setup(EAT_PIN61_COL1, EAT_GPIO_DIR_OUTPUT, EAT_GPIO_LEVEL_HIGH);
}

