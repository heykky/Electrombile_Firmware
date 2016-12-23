
#include <eat_periphery.h>
#include <eat_timer.h>
#include <eat_interface.h>


#include <math.h>

#include "thread_msg.h"
#include "thread.h"
#include "battery.h"
#include "setting.h"
#include "data.h"
#include "log.h"
#include "adc.h"
#include "mem.h"
#include "timer.h"

enum
{
    BATTERY_TYPENULL = 0,
    BATTERY_TYPE36 = 36,
    BATTERY_TYPE48 = 48,
    BATTERY_TYPE60 = 60,
    BATTERY_TYPE72 = 72,
};

enum
{
    BATTERY_ALARM_NULL  = 0,
    BATTERY_ALARM_50    = 4,
    BATTERY_ALARM_30    = 5,
};

#define MAX_VLOTAGE_NUM 10

#define MAX_PERCENT_NUM 100
#define BATTERY_TIMER_PEROID (5*60*1000)   // once for 5 mins ,check and store battery voltage and percent

#define ADvalue_2_Realvalue(x) (x*103/3/1000.f) //unit mV, 3K & 100k divider
#define Voltage2Percent(x) exp((x-37.873)/2.7927)

static u32 BatteryVoltage[MAX_VLOTAGE_NUM] = {0};

/*
*set the battery's voltage
*/
static void battery_store_voltage(u32 voltage)
{
    static int count = 0;

    if(count >= MAX_VLOTAGE_NUM)
    {
        count = 0;
    }

    BatteryVoltage[count++] = voltage;
}

/*
* get the battery's voltage
*/
static u32 battery_get_Voltage(void)
{
    u32 voltage = 0;
    int count;

    for(count = 0;count < MAX_VLOTAGE_NUM;count++)
    {
        voltage += BatteryVoltage[count];
    }

    voltage /= MAX_VLOTAGE_NUM;

    return voltage;
}

/*
*fun:judge if there is new battery type and return battery percent
*/
static u8 battery_Judge_type(u32 voltage)
{
    float realVoltage = ADvalue_2_Realvalue(voltage);
    u8 battery_type = BATTERY_TYPENULL;
    int percent = 0;//the result of the fumula may be larger than 256

    if(realVoltage > 64 )
    {
        voltage = voltage*48/72;    //normalizing to 48V
        battery_type = BATTERY_TYPE72;
    }
    else if(realVoltage > 52)
    {
        voltage = voltage*48/60;    //normalizing to 48V
        battery_type = BATTERY_TYPE60;
    }
    else if(realVoltage > 40)
    {
        voltage = voltage;          //normalizing to 48V
        battery_type = BATTERY_TYPE48;
    }
    else if(realVoltage > 28)
    {
        voltage = voltage*48/36;    //normalizing to 48V
        battery_type = BATTERY_TYPE36;
    }

    percent = (int)Voltage2Percent(ADvalue_2_Realvalue(voltage));
    percent = percent>MAX_PERCENT_NUM?MAX_PERCENT_NUM:percent;

    // if battery type has been typed by user or battery type is ok, do not judge battery type
    if(isUserBatteryTpye() || battery_type == get_battery_type())
    {
        return (u8)percent;
    }


    //judge battery type: when percent > 60, start judging, when percent < 40, stop judging
    if(60 < percent)
    {
        if(battery_type != get_batterytype_Judging())// if has not start judging, start it
        {
            LOG_DEBUG("start to judge battery type: %d",battery_type);
            set_battery_isJudging(EAT_TRUE, battery_type);
        }
    }
    else if(40 > percent )
    {
        if(get_battery_isJudging() && battery_type == get_batterytype_Judging())// confirm the batterytype_judging and finish judging
        {
            LOG_DEBUG("set battery type: %d",battery_type);
            set_battery_type(battery_type);
        }
        set_battery_isJudging(EAT_FALSE, BATTERY_TYPENULL);// stop this judgement
    }

    return (u8)percent;
}

/*
*fun:reference battery-type, return battery percent
*/
static u8 battery_getType_percent(u32 voltage)
{
    int percent = 0;
    u8 battery_type = get_battery_type();

    if(battery_type == BATTERY_TYPE72)
    {
        voltage = voltage*48/72;    //normalizing to 48V
    }
    else if(battery_type == BATTERY_TYPE60)
    {
        voltage = voltage*48/60;    //normalizing to 48V
    }
    else if(battery_type == BATTERY_TYPE48)
    {
        voltage = voltage;          //normalizing to 48V
    }
    else if(battery_type == BATTERY_TYPE36)
    {
        voltage = voltage*48/36;    //normalizing to 48V
    }
    else
    {
        return (MAX_PERCENT_NUM + 1); //BATTERY_TYPENULL as MAX_PERCENT_NUM + 1 > MAX_PERCENT_NUM
    }

    percent = (int)Voltage2Percent(ADvalue_2_Realvalue(voltage));
    percent = percent>MAX_PERCENT_NUM?MAX_PERCENT_NUM:percent;

    return (u8)percent;
}

/*
*func:check battery 10mins once while not moved,if battery low,alarm:BATTERY_ALARM_50, BATTERY_ALARM_30
*/
static u8 battery_isAlarm(void)
{
    static char batteryState = BATTERY_ALARM_NULL;

    u8 percent = 0;
    u32 voltage = 0;
    u8 percent_untype = BATTERY_TYPENULL;

    voltage = battery_get_Voltage();
    if(0 == voltage)//if battery has not been detected
    {
        return BATTERY_ALARM_NULL;
    }

    percent_untype = battery_Judge_type(voltage);//judge new battery type
    percent = battery_getType_percent(voltage);
    if(percent > MAX_PERCENT_NUM)//if battery type has not judged , get battery as no type
    {
        percent = percent_untype;
    }

    /*      refresh the Voltage and Percent of data module   */
    battery_setVoltage((u8)(ADvalue_2_Realvalue(voltage) + 0.5));
    battery_setPercent(percent);

    if(70 < percent)//battery > 70, assume as charge, reset and wait for reducing to 30
    {
        batteryState = BATTERY_ALARM_NULL;
    }
    else if(30 > percent &&batteryState != BATTERY_ALARM_30)// battery < 30 and has not alarmed, alarm it
    {
        return batteryState = BATTERY_ALARM_30;
    }

    return BATTERY_ALARM_NULL;
}

static int battery_alarm_handler(void)
{
    u8 msgLen = sizeof(MSG_THREAD) + sizeof(ALARM_INFO);
    MSG_THREAD *msg = NULL;
    ALARM_INFO *alarmType = NULL;
    char alarm_type = battery_isAlarm();

    if(alarm_type == BATTERY_ALARM_NULL || Vibration_isMoved())
    {
        return 0;
    }

    msg = allocMsg(msgLen);
    alarmType = (ALARM_INFO*)msg->data;

    msg->cmd = CMD_THREAD_ALARM;
    msg->length = sizeof(ALARM_INFO);
    alarmType->alarm_type = alarm_type;

    LOG_DEBUG("battery alarm:cmd(%d),length(%d),data(%d)", msg->cmd, msg->length, alarm_type);

    return sendMsg(THREAD_MAIN, msg, msgLen);

}

/*
*fun:event adc proc
*/
static void battery_event_adc(EatEvent_st *event)
{
    static int time_count = 0;

    if(event->data.adc.pin == ADC_VOLTAGE)
    {
        if(!Vibration_isMoved())
        {
            if(++time_count > 2 * 60 * (1000 / ADC_VOLTAGE_PERIOD))   //when stay for 2 mins, start to detect battery
            {
                battery_store_voltage(event->data.adc.v);
            }
        }
        else
        {
            time_count = 0;
        }
    }
    else
    {
        LOG_ERROR("ADC_433 = %d",event->data.adc.v);
    }
}

void app_battery_thread(void *data)
{
	EatEvent_st event;
    MSG_THREAD* msg = 0;

	LOG_INFO("battery thread start.");

    while(!eat_adc_get(ADC_VOLTAGE, ADC_VOLTAGE_PERIOD, NULL))
    {
        eat_sleep(100);//if failed ,try again after 100ms
    }

	LOG_INFO("TIMER_BATTERY_CHECK start.");
    eat_timer_start(TIMER_BATTERY_CHECK,BATTERY_TIMER_PEROID);

	while(EAT_TRUE)
	{
        eat_get_event_for_user(THREAD_BATTERY, &event);
        switch(event.event)
        {
            case EAT_EVENT_TIMER:
                switch (event.data.timer.timer_id)
                {
                    case TIMER_BATTERY_CHECK:
                        battery_alarm_handler();
                        eat_timer_start(event.data.timer.timer_id,BATTERY_TIMER_PEROID);
                        break;

                    default:
                        LOG_ERROR("timer(%d) expire!", event.data.timer.timer_id);
                        break;
                }
                break;

            case EAT_EVENT_ADC:

                battery_event_adc(&event);

                break;

            case EAT_EVENT_USER_MSG:
                msg = (MSG_THREAD*) event.data.user_msg.data_p;
                switch (msg->cmd)
                {
                    default:
                        LOG_ERROR("cmd(%d) not processed!", msg->cmd);
                        break;
                }
                freeMsg(msg);
                break;

            default:
            	LOG_ERROR("event(%d) not processed!", event.event);
                break;
        }
    }
}


