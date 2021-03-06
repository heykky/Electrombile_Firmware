/*
 * adc.h
 *
 *  Created on: 2016年2月1日
 *      Author: jk
 */

#ifndef USER_ELECTROMBILE_ADC_H_
#define USER_ELECTROMBILE_ADC_H_

#define EAT_ADC0 EAT_PIN23_ADC1
#define EAT_ADC1 EAT_PIN24_ADC2

#define ADC_PERIOD_READ_ONCE 0

#define ADC_433 EAT_ADC0
#define ADC_433_PERIOD (2000)  //unit: ms

#define ADC_VOLTAGE EAT_ADC1
#define ADC_VOLTAGE_PERIOD  (20) //ms

#endif /* USER_ELECTROMBILE_ADC_H_ */
