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

#define BLUETOOTH_SCAN_PERIOD (20 * 1000) // 20s for once

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
    if(strstr((const char *) buf, "+BTSCAN: 0") && strstr(buf,setting.BluetoothId))
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

static void bluetooth_scanHandler(void)
{
    //event when bluetooth is found
    if(isBluetoothInRange_now && !isBluetoothInRange_pre)
    {
        if(MED_AUDIO_SUCCESS != eat_audio_play_file(AUDIO_FILE_NAME_FOUND, EAT_FALSE, NULL, 15, EAT_AUDIO_PATH_SPK1))
        {
            eat_audio_play_data(audio_defaultAudioSource_found(), audio_sizeofDefaultAudioSource_found(), EAT_AUDIO_FORMAT_AMR, EAT_AUDIO_PLAY_ONCE, 15, EAT_AUDIO_PATH_SPK1);
        }
    }

    //event when bluetooth is lost
    if(!isBluetoothInRange_now && isBluetoothInRange_pre)
    {
        if(MED_AUDIO_SUCCESS != eat_audio_play_file(AUDIO_FILE_NAME_LOST, EAT_FALSE, NULL, 15, EAT_AUDIO_PATH_SPK1))
        {
            eat_audio_play_data(audio_defaultAudioSource_lost(), audio_sizeofDefaultAudioSource_lost(), EAT_AUDIO_FORMAT_AMR, EAT_AUDIO_PLAY_ONCE, 15, EAT_AUDIO_PATH_SPK1);
        }
    }

    isBluetoothInRange_pre = isBluetoothInRange_now;
    isBluetoothInRange_now = EAT_FALSE;

    modem_AT("AT+BTSCAN=1,10" CR);
}

static void bluetooth_stopSound(void)
{
    LOG_DEBUG("EAT_EVENT_AUD_PLAY_FINISH_IND happen");

    if(EAT_TRUE != eat_audio_stop_data())
    {
        eat_audio_stop_file();
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

void app_bluetooth_thread(void *data)
{
    EatEvent_st event;
    MSG_THREAD* msg = 0;

	LOG_DEBUG("bluetooth thread start.");

    modem_AT("AT+BTPOWER=1" CR);

    LOG_INFO("TIMER_BLUETOOTH_SCAN start.");
    eat_timer_start(TIMER_BLUETOOTH_SCAN,BLUETOOTH_SCAN_PERIOD);

    while(EAT_TRUE)
	{
        eat_get_event_for_user(THREAD_BLUETOOTH, &event);
        switch(event.event)
        {
            case EAT_EVENT_TIMER:
                switch (event.data.timer.timer_id)
                {
                    case TIMER_BLUETOOTH_SCAN:
                        bluetooth_scanHandler();
                        eat_timer_start(event.data.timer.timer_id, BLUETOOTH_SCAN_PERIOD);
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
                bluetooth_stopSound();
                break;

            default:
                LOG_ERROR("event(%d) not processed!", event.event);
                break;
        }
    }
}







