/*
 * telecontrol.c
 *
 *  Created on: 2016/11/29
 *      Author: lc
 */
#include <eat_interface.h>

#include "telecontrol.h"
void telecontrol_initail(void)
{

    eat_gpio_setup(EAT_PIN60_COL2, EAT_GPIO_DIR_INPUT, EAT_GPIO_LEVEL_LOW);// used for cut off alarm
    eat_gpio_setup(EAT_PIN62_COL0, EAT_GPIO_DIR_INPUT, EAT_GPIO_LEVEL_LOW);// used for cut off alarm

    eat_gpio_setup(EAT_PIN59_COL3, EAT_GPIO_DIR_OUTPUT, EAT_GPIO_LEVEL_LOW);// break off
    eat_gpio_setup(EAT_PIN61_COL1, EAT_GPIO_DIR_OUTPUT, EAT_GPIO_LEVEL_LOW);// battery off
}

void telecontrol_cutoff(void)
{
    eat_gpio_setup(EAT_PIN59_COL3, EAT_GPIO_DIR_OUTPUT, EAT_GPIO_LEVEL_HIGH);//break on
    eat_gpio_setup(EAT_PIN61_COL1, EAT_GPIO_DIR_OUTPUT, EAT_GPIO_LEVEL_LOW);// battery off
}

void telecontrol_connect(void)
{
    eat_gpio_setup(EAT_PIN59_COL3, EAT_GPIO_DIR_OUTPUT, EAT_GPIO_LEVEL_LOW);//break off
    eat_gpio_setup(EAT_PIN61_COL1, EAT_GPIO_DIR_OUTPUT, EAT_GPIO_LEVEL_HIGH);// battery on
}

void telecontrol_lock(void)
{
    eat_gpio_setup(EAT_PIN59_COL3, EAT_GPIO_DIR_OUTPUT, EAT_GPIO_LEVEL_LOW);//break on
    eat_gpio_setup(EAT_PIN61_COL1, EAT_GPIO_DIR_OUTPUT, EAT_GPIO_LEVEL_HIGH);// battery on
}

