/*
 * device.c
 *
 *  Created on: 2016/12/15
 *      Author: lc
 */

#include <stdio.h>
#include <string.h>

#include <eat_interface.h>
#include "cJSON.h"

#include "client.h"
#include "socket.h"
#include "msg.h"
#include "log.h"
#include "uart.h"
#include "setting.h"
#include "thread.h"
#include "thread_msg.h"
#include "timer.h"
#include "msg_queue.h"
#include "device.h"
#include "record.h"
#include "protocol.h"
#include "telecontrol.h"

enum
{
    DEVICE_GET_DEVICEINFO    = 0,
    DEVICE_GET_LOCATE        = 1,
    DEVICE_SET_AUTOLOCK      = 2,
    DEVICE_GET_AUTOLOCK      = 3,
    DEVICE_SET_DEFENDSTATE   = 4,
    DEVICE_GET_DEFENDSTATE   = 5,
    DEVICE_GET_BATTERY       = 6,
    DEVICE_SET_BATTERYTYPE   = 7,
    DEVICE_START_RECORD      = 8,
    DEVICE_STOP_RECORD       = 9,
    DEVICE_SET_BLUETOOTHID   = 10,
    DEVICE_DOWNLOAD_AUDIOFILE= 12,
    DEVICE_SET_BLUETOOTHSW   = 13,
    DEVICE_START_ALARM       = 14,
    DEVICE_CONTROL_LOCK      = 15,
}DEVICE_CMD_NAME;

typedef int (*DEVICE_PROC)(const void*, cJSON*);
typedef struct
{
    char cmd;
    DEVICE_PROC pfn;
}DEVICE_MSG_PROC;

static int device_responseOK(const void* req)
{
    char *buf = "{\"code\":0}";
    int length = sizeof(MSG_DEVICE_RSP) + strlen(buf);

    MSG_DEVICE_RSP *msg = alloc_device_msg(req, length);
    if(!msg)
    {
        LOG_ERROR("device inner error");
        return -1;
    }
    strncpy(msg->data, buf, strlen(buf));

    socket_sendDataDirectly(msg, length);
    return 0;
}

static int device_responseERROR(const void* req)
{
    char *buf = "{\"code\":110}";
    int length = sizeof(MSG_DEVICE_RSP) + strlen(buf);

    MSG_DEVICE_RSP *msg = alloc_device_msg(req, length);
    if(!msg)
    {
        LOG_ERROR("device inner error");
        return -1;
    }
    strncpy(msg->data, buf, strlen(buf));

    socket_sendDataDirectly(msg, length);
    return 0;
}


static int device_GetDeviceInfo(const void* req, cJSON *param)
{
    //TODO SEND DEVICE INFO MSG TO SERVER
    return 0;
}
static int device_GetLocation(const void* req, cJSON *param)
{
    //TODO SEND DEVICE INFO MSG TO SERVER
    return 0;
}

static int device_SetAutolock(const void* req, cJSON *param)
{
    //TODO SEND DEVICE INFO MSG TO SERVER
    return 0;
}

static int device_GetAutolock(const void* req, cJSON *param)
{
    //TODO SEND DEVICE INFO MSG TO SERVER
    return 0;
}

static int device_SetDeffend(const void* req, cJSON *param)
{
    //TODO SEND DEVICE INFO MSG TO SERVER
    return 0;
}

static int device_GetDeffend(const void* req, cJSON *param)
{
    //TODO SEND DEVICE INFO MSG TO SERVER
    return 0;
}

static int device_GetBattery(const void* req, cJSON *param)
{
    //TODO SEND DEVICE INFO MSG TO SERVER
    return 0;
}

static int device_SetBatteryType(const void* req, cJSON *param)
{
    //TODO SEND DEVICE INFO MSG TO SERVER
    return 0;
}

static int device_StartRecord(const void* req, cJSON *param)
{
    record_start();

    return device_responseOK(req);
}

static int device_StopRecord(const void* req, cJSON *param)
{
    record_stop();

    return device_responseOK(req);
}

static int device_SetBluetoothId(const void* req, cJSON *param)
{
    cJSON *bluetoothId = NULL;
    u8 msgLen = sizeof(MSG_THREAD);
    MSG_THREAD *msg = NULL;

    msg = allocMsg(msgLen);
    if(!msg)
    {
        return device_responseERROR(req);
    }

    msg->cmd = CMD_THREAD_BLUETOOTHRESET;
    msg->length = 0;

    if(!param)
    {
        return device_responseERROR(req);
    }
    bluetoothId = cJSON_GetObjectItem(param, "bluetoothId");
    set_bluetooth_id(bluetoothId->valuestring);

    sendMsg(THREAD_BLUETOOTH, msg, msgLen);

    return device_responseOK(req);
}

static int device_DownloadAudioFile(const void* req, cJSON *param)
{
    cJSON *use = NULL;
    cJSON *fileName = NULL;
    char fileNameString [32]= {0};
    if(!param)
    {
        return device_responseERROR(req);
    }
    use = cJSON_GetObjectItem(param, "use");
    fileName = cJSON_GetObjectItem(param, "fileName");
    strncpy(fileNameString, fileName->valuestring,32);
    if(use->valueint == 0)
    {
        ftp_download_file("close_audio.amr", fileNameString);
    }
    else if(use->valueint == 1)
    {
        ftp_download_file("far_audio.amr", fileNameString);
    }
    return device_responseOK(req);
}

static int device_SetBlutoothSwitch(const void* req, cJSON *param)
{
    cJSON *bluetoothSwitch = NULL;

    if(!param)
    {
        return device_responseERROR(req);
    }

    bluetoothSwitch = cJSON_GetObjectItem(param, "sw");
    set_bluetooth_switch(bluetoothSwitch->valueint);

    return device_responseOK(req);
}

static int device_StartAlarm(const void* req, cJSON *param)
{
    u8 msgLen = sizeof(MSG_THREAD);
    MSG_THREAD *msg = NULL;

    msg = allocMsg(msgLen);
    if(!msg)
    {
        return device_responseERROR(req);
    }
    msg->cmd = CMD_THREAD_STARTALARM;
    msg->length = 0;

    sendMsg(THREAD_BLUETOOTH, msg, msgLen);

    return device_responseOK(req);
}

static int device_ControlCarLocked(const void* req, cJSON *param)
{
    cJSON *Switch = NULL;

    if(!param)
    {
        return device_responseERROR(req);
    }

    Switch = cJSON_GetObjectItem(param, "sw");
    if(1 == Switch->valueint)
    {
        telecontrol_lock();
    }
    else
    {
        telecontrol_unlock();
    }

    return device_responseOK(req);
}

static DEVICE_MSG_PROC deviceProcs[] =
{
    {DEVICE_GET_DEVICEINFO,    device_GetDeviceInfo},
    {DEVICE_GET_LOCATE,        device_GetLocation},
    {DEVICE_SET_AUTOLOCK,      device_SetAutolock},
    {DEVICE_GET_AUTOLOCK,      device_GetAutolock},
    {DEVICE_SET_DEFENDSTATE,   device_SetDeffend},
    {DEVICE_GET_DEFENDSTATE,   device_GetDeffend},
    {DEVICE_GET_BATTERY,       device_GetBattery},
    {DEVICE_SET_BATTERYTYPE,   device_SetBatteryType},
    {DEVICE_START_RECORD,      device_StartRecord},
    {DEVICE_STOP_RECORD,       device_StopRecord},
    {DEVICE_SET_BLUETOOTHID,   device_SetBluetoothId},
    {DEVICE_DOWNLOAD_AUDIOFILE,device_DownloadAudioFile},
    {DEVICE_SET_BLUETOOTHSW,   device_SetBlutoothSwitch},
    {DEVICE_START_ALARM,       device_StartAlarm},
    {DEVICE_CONTROL_LOCK,      device_ControlCarLocked}
};

int cmd_device_handler(const void* msg)
{
    int i =0;
    char cmd = 0;
    cJSON *json_cmd = NULL;
    cJSON *json_root = NULL;
    cJSON *json_param = NULL;
    MSG_DEVICE_REQ *req = (MSG_DEVICE_REQ *)msg;

    json_root = cJSON_Parse(req->data);
    if(!json_root)
    {
        LOG_ERROR("content is not json type");
        return -1;
    }

    json_cmd = cJSON_GetObjectItem(json_root, "c");
    if(!json_cmd)
    {
        cJSON_Delete(json_root);
        LOG_ERROR("no cmd in content");
        return -1;
    }
    cmd = json_cmd->valueint;

    json_param = cJSON_GetObjectItem(json_root, "param");

    for (i = 0; i < sizeof(deviceProcs) / sizeof(deviceProcs[0]); i++)
    {
        if (deviceProcs[i].cmd == json_cmd->valueint)
        {
            DEVICE_PROC pfn = deviceProcs[i].pfn;
            if (pfn)
            {
                pfn(msg, json_param);
                return 0;
            }
            else
            {
                LOG_ERROR("Message %d not processed!", cmd);
                return -1;
            }
        }
    }

    LOG_ERROR("unknown device type %d!", cmd);
    return -1;
}
