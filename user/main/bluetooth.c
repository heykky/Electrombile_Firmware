#include <eat_periphery.h>
#include <eat_timer.h>
#include <eat_interface.h>
#include <eat_audio.h>
#include <stdio.h>
#include <string.h>


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

typedef enum
{
	BLUETOOTH_STATE_EXIST,
	BLUETOOTH_STATE_NOEXIST
}BLUETOOTH_STATE;


#define BLUETOOTH_SCAN_PERIOD (20 * 1000)


static BLUETOOTH_STATE BluetoothState_now = BLUETOOTH_STATE_NOEXIST;
static BLUETOOTH_STATE BluetoothState_last = BLUETOOTH_STATE_NOEXIST;
static eat_bool PLAY_AUDIO_TYPE;//FALSE: play audio data  TRUE: play audio file


/*
*fun:reset the bluetooth state
*/
void bt_resetBluetoothState(void)
{
    BluetoothState_last = BLUETOOTH_STATE_NOEXIST;
    BluetoothState_now = BLUETOOTH_STATE_NOEXIST;
}

/*
*fun:check the bluetooth id
*/
static int cmd_CheckBluetoothId(u8* buf)
{

    if(strstr(buf,setting.BluetoothId)!=NULL)
    {
        LOG_DEBUG("PASS\n");
        BluetoothState_now = BLUETOOTH_STATE_EXIST;
    }
    else
    {
        LOG_DEBUG("NOPASS\n");
    }

    return 0;
}

/*
*fun:event bluetoothscan timer proc
*/
static void BluetoothScan_proc(void)
{
    if(BluetoothState_now == BLUETOOTH_STATE_EXIST && BluetoothState_last == BLUETOOTH_STATE_NOEXIST)
    {
        if(eat_audio_play_file(AUDIO_FILE_NAME_FOUND, EAT_FALSE, NULL, 100, EAT_AUDIO_PATH_SPK1)!=0)
        {
            eat_audio_play_data(audio_defaultAudioSource_found(), audio_sizeofDefaultAudioSource_found(), EAT_AUDIO_FORMAT_AMR, EAT_AUDIO_PLAY_ONCE, 100, EAT_AUDIO_PATH_SPK1);
            PLAY_AUDIO_TYPE = EAT_FALSE;
        }
        else
        {
            PLAY_AUDIO_TYPE = EAT_TRUE;
        }
        //event when bluetooth is found
    }

    if(BluetoothState_now == BLUETOOTH_STATE_NOEXIST && BluetoothState_last == BLUETOOTH_STATE_EXIST)
    {
        if(eat_audio_play_file(AUDIO_FILE_NAME_LOST, EAT_FALSE, NULL, 100, EAT_AUDIO_PATH_SPK1)!=0)
        {
            eat_audio_play_data(audio_defaultAudioSource_lost(), audio_sizeofDefaultAudioSource_lost(), EAT_AUDIO_FORMAT_AMR, EAT_AUDIO_PLAY_ONCE, 100, EAT_AUDIO_PATH_SPK1);
            PLAY_AUDIO_TYPE = EAT_FALSE;
        }
        else
        {
            PLAY_AUDIO_TYPE = EAT_TRUE;
        }
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
    char* ptr = strstr((const char *) modem_rsp, "+BTSCAN: 0");

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
                        BluetoothScan_proc();
                        eat_timer_start(event.data.timer.timer_id,BLUETOOTH_SCAN_PERIOD);
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
                    cmd_CheckBluetoothId(buf);
                }
                break;

            case EAT_EVENT_AUD_PLAY_FINISH_IND:
                if(PLAY_AUDIO_TYPE == EAT_FALSE)
                {
                    eat_audio_stop_data();
                }
                else if(PLAY_AUDIO_TYPE == EAT_TRUE)
                {
                    eat_audio_stop_file();
                }
                break;

            default:
                LOG_ERROR("event(%d) not processed!", event.event);
                break;
        }
    }
}








