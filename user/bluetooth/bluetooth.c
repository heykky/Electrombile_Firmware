/*
 * bluetooth.h
 *
 *  Created on: 2017/01/15
 *      Author: kky
 */
#include <stdio.h>
#include <string.h>
#include <eat_interface.h>

#include "fs.h"
#include "log.h"
#include "timer.h"
#include "modem.h"
#include "thread.h"
#include "setting.h"
#include "bluetooth.h"
#include "thread_msg.h"
#include "audio_source.h"

#define BLUETOOTH_TIMER_PERIOD (1 * 1000) // 1s for once

static eat_bool isBluetoothInRange_now = EAT_FALSE;
static eat_bool isBluetoothInRange_pre = EAT_FALSE;


/*
*fun:reset the bluetooth state
*/
static void bluetooth_resetState(void)
{
    isBluetoothInRange_now = EAT_FALSE;
    isBluetoothInRange_pre = EAT_FALSE;
}

/*
*fun:check the bluetooth id
*/
static int bluetooth_checkId(u8* buf)
{
    if(strstr((const char *)buf, "+BTSCAN: 0") && strstr((const char *)buf,setting.BluetoothId))
    {
        LOG_DEBUG("PASS");
        isBluetoothInRange_now = EAT_TRUE;
    }
    else
    {
        LOG_DEBUG("NOPASS");
    }

    return 0;
}

static void bluetooth_scanResultProc(void)
{
    if(isBluetoothInRange_now && !isBluetoothInRange_pre)//event when bluetooth is found
    {
        audio_bluetoothFoundSound();
    }

    if(!isBluetoothInRange_now && isBluetoothInRange_pre)//event when bluetooth is lost
    {
        audio_bluetoothLostSound();
    }

    isBluetoothInRange_pre = isBluetoothInRange_now;
    isBluetoothInRange_now = EAT_FALSE;
}

static void bluetooth_scanProc(void)
{
    static int time = 0;
    static eat_bool isFirst = EAT_TRUE;

    time++;

    if(time == 10 && !isFirst)
    {
        modem_AT("AT+BTSCAN=0" CR);//stop scan
    }
    if(time == 12)
    {
        isFirst = EAT_FALSE;
        bluetooth_scanResultProc();
        modem_AT("AT+BTSCAN=1,10" CR);//start scan
        time = 0;
    }
}

static void bluetooth_mod_ready_rd(void)
{
    u8 buf[256] = {0};

    if (eat_modem_read(buf, 256))
    {
        LOG_DEBUG("modem recv: %s", buf);

        bluetooth_checkId(buf);
    }
}

static void bluetooth_onesecondLoop(void)
{
    if(is_bluetoothOn())
    {
        bluetooth_scanProc();
    }
}

void app_bluetooth_thread(void *data)
{
    EatEvent_st event;
    MSG_THREAD* msg = 0;

	LOG_DEBUG("bluetooth thread start.");

    modem_AT("AT+BTPOWER=1" CR);

    LOG_INFO("TIMER_BLUETOOTH start.");
    eat_timer_start(TIMER_BLUETOOTH, BLUETOOTH_TIMER_PERIOD);

    while(EAT_TRUE)
	{
        eat_get_event_for_user(THREAD_BLUETOOTH, &event);
        switch(event.event)
        {
            case EAT_EVENT_TIMER:
                switch (event.data.timer.timer_id)
                {
                    case TIMER_BLUETOOTH:
                        bluetooth_onesecondLoop();
                        eat_timer_start(event.data.timer.timer_id, BLUETOOTH_TIMER_PERIOD);
                        break;

                    default:
                        LOG_ERROR("timer(%d) expire!", event.data.timer.timer_id);
                        break;
                }
                break;

            case EAT_EVENT_MDM_READY_RD:
                bluetooth_mod_ready_rd();
                break;

            case EAT_EVENT_USER_MSG:
                msg = (MSG_THREAD*) event.data.user_msg.data_p;
                switch (msg->cmd)
                {
                    case CMD_THREAD_BLUETOOTHRESET:
                        bluetooth_resetState();
                        break;

                    default:
                        LOG_ERROR("cmd(%d) not processed!", msg->cmd);
                        break;
                }
                freeMsg(msg);
                break;

            case EAT_EVENT_AUD_PLAY_FINISH_IND:
                {
                   LOG_DEBUG("EAT_EVENT_AUD_PLAY_FINISH_IND happen");
                   audio_stopSound();
                }
                break;

            default:
                LOG_ERROR("event(%d) not processed!", event.event);
                break;
        }
    }
}








