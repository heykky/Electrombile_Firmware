#include <eat_periphery.h>
#include <eat_timer.h>
#include <eat_interface.h>
#include <eat_audio.h>
#include <math.h>
#include <stdio.h>
#include <string.h>


#include "thread_msg.h"
#include "thread.h"
#include "setting.h"
#include "data.h"
#include "log.h"
#include "mem.h"
#include "timer.h"
#include "modem.h"
#include "bluetooth.h"
#include "fs.h"
#include "audio_source.h"


#define BLUETOOTH_TIMER_PERIOD (15*1000)
#define MAX_RECORDNAME_SIZE 128


static char AcceptAddress[18] = "2c:8a:72:fb:8f:f9";
static BLUETOOTH_STATE BluetoothState_now = BLUETOOTH_STATE_NOEXIST;
static BLUETOOTH_STATE BluetoothState_last = BLUETOOTH_STATE_NOEXIST;

/*
*fun:check the bluetooth address
*/
static int cmd_CheckBluetoothAddress(u8* buf)
{
    char GetAddress[18];

    if(sscanf(buf, "%*s%*[^,],%*[^,],%*[^,],%[^,]", GetAddress)==1)
    {
        if(strcmp(AcceptAddress,GetAddress)==0)
        {
            LOG_DEBUG("Bluetooth address is %s\n",GetAddress);
            BluetoothState_now = BLUETOOTH_STATE_EXIST;
        }
        else
        {
            LOG_DEBUG("Bluetooth address is %s\n",GetAddress);
        }
    }
    return 0;
}

/*
*fun:event bluetoothscan timer proc
*/
static void app_bluescan_proc(void)
{
    if(BluetoothState_now == BLUETOOTH_STATE_EXIST && BluetoothState_last == BLUETOOTH_STATE_NOEXIST)
    {
        eat_audio_play_data(audio_BluetoothIsFound, sizeof(audio_BluetoothIsFound), EAT_AUDIO_FORMAT_AMR, EAT_AUDIO_PLAY_ONCE, 100, EAT_AUDIO_PATH_SPK1);
        //event when bluetooth is found
    }

    if(BluetoothState_now == BLUETOOTH_STATE_NOEXIST && BluetoothState_last == BLUETOOTH_STATE_EXIST)
    {
        eat_audio_play_data(audio_BluetoothIsLost, sizeof(audio_BluetoothIsLost), EAT_AUDIO_FORMAT_AMR, EAT_AUDIO_PLAY_ONCE, 100, EAT_AUDIO_PATH_SPK1);
        //event when bluetooth is lost
    }

    BluetoothState_last = BluetoothState_now;
    BluetoothState_now = BLUETOOTH_STATE_NOEXIST;
    modem_AT("AT+BTSCAN=1,10\r");
}

/*
*fun:judge the bluetoothscan command
*/
static eat_bool IsBluetoothScan(char* modem_rsp)
{
    char* ptr = strstr((const char *) modem_rsp, "+BTSCAN:");

    if(ptr)
    {
        return EAT_TRUE;
    }

    return EAT_FALSE;
}


void app_bluetooth_thread(void *data)
{
    EatEvent_st event;
    MSG_THREAD* msg = 0;
    u8 buf[256] = {0};
	u16 len = 0;

	LOG_INFO("bluetooth thread start.");

    modem_AT("AT+BTPOWER=1\r");

    LOG_INFO("TIMER_BLUETOOTH_SCAN start.");
    eat_timer_start(TIMER_BLUETOOTH_SCAN,BLUETOOTH_TIMER_PERIOD);

    while(EAT_TRUE)
	{
        eat_get_event_for_user(THREAD_BLUETOOTH, &event);
        switch(event.event)
        {
            case EAT_EVENT_TIMER:
                switch (event.data.timer.timer_id)
                {
                    case TIMER_BLUETOOTH_SCAN:
                        app_bluescan_proc();
                        eat_timer_start(event.data.timer.timer_id,BLUETOOTH_TIMER_PERIOD);
                        break;

                    default:
                        LOG_ERROR("timer(%d) expire!", event.data.timer.timer_id);
                        break;
                }
                break;

            case EAT_EVENT_MDM_READY_RD:
                memset(buf,0,256);
                len = eat_modem_read(buf, 256);
            	if (!len)
            	{
            	    LOG_ERROR("modem received nothing.");
            	    break;
            	}
                LOG_DEBUG("modem recv: %s", buf);
                if(IsBluetoothScan(buf))
                {
                    cmd_CheckBluetoothAddress(buf);
                }
                break;

            case EAT_EVENT_AUD_PLAY_FINISH_IND:
                eat_audio_stop_data();
                break;

            default:
                LOG_ERROR("event(%d) not processed!", event.event);
                break;
        }
    }
}








