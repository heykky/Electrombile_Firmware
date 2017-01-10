/*
 * telecontrol.c
 *
 *  Created on: 2016/11/29
 *      Author: lc
 */
#include <eat_interface.h>

#include "telecontrol.h"

static void telecontrol_switchflag_initail(void)
{
    eat_gpio_setup(EAT_PIN60_COL2, EAT_GPIO_DIR_INPUT, EAT_GPIO_LEVEL_LOW); // electric switch flag default low
}

static void telecontrol_break_initial(void)
{
    eat_gpio_setup(EAT_PIN59_COL3, EAT_GPIO_DIR_OUTPUT, EAT_GPIO_LEVEL_LOW);// break state default off
}

static void telecontrol_switch_initial(void)
{
    eat_gpio_setup(EAT_PIN61_COL1, EAT_GPIO_DIR_OUTPUT, EAT_GPIO_LEVEL_LOW);// switch state default off
}

static void telecontrol_break_on(void)
{
    eat_gpio_setup(EAT_PIN59_COL3, EAT_GPIO_DIR_OUTPUT, EAT_GPIO_LEVEL_HIGH);// break on
}

static void telecontrol_break_off(void)
{
    eat_gpio_setup(EAT_PIN59_COL3, EAT_GPIO_DIR_OUTPUT, EAT_GPIO_LEVEL_LOW);// break off
}

void telecontrol_switch_on(void)
{
    eat_gpio_setup(EAT_PIN61_COL1, EAT_GPIO_DIR_OUTPUT, EAT_GPIO_LEVEL_HIGH);// switch on
}

void telecontrol_switch_off(void)
{
    eat_gpio_setup(EAT_PIN61_COL1, EAT_GPIO_DIR_OUTPUT, EAT_GPIO_LEVEL_LOW);// switch off
}

void telecontrol_initail(void)
{
    telecontrol_switchflag_initail();
    telecontrol_break_initial();
    telecontrol_switch_initial();
}

void telecontrol_lock(void)
{
    telecontrol_break_on();
    telecontrol_switch_on();
}

void telecontrol_unlock(void)
{
    telecontrol_switch_on();
    telecontrol_switch_off();
}

