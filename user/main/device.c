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
enum
{
    DEVICE_GET_DEVICEINFO   = 0,
    DEVICE_GET_LOCATE       = 1,
    DEVICE_SET_AUTOLOCK     = 2,
    DEVICE_GET_AUTOLOCK     = 3,
    DEVICE_SET_DEFENDSTATE  = 4,
    DEVICE_GET_DEFENDSTATE  = 5,
    DEVICE_GET_BATTERY      = 6,
    DEVICE_SET_BATTERYTYPE  = 7,
}DEVICE_CMD_NAME;

typedef int (*DEVICE_PROC)(const void*, const char *);
typedef struct
{
    char cmd;
    DEVICE_PROC pfn;
}DEVICE_MSG_PROC;

static int device_GetDeviceInfo(const void* req, const char *param)
{
    //TODO SEND DEVICE INFO MSG TO SERVER
}

static DEVICE_MSG_PROC deviceProcs[] =
{
    {DEVICE_GET_DEVICEINFO, device_GetDeviceInfo},
};

int cmd_device_handler(const void* msg)
{
    int i =0;
    char cmd = 0;
    char *param = NULL;
    cJSON *json_cmd = NULL;
    cJSON *json_root = NULL;
    cJSON *json_param = NULL;
    MSG_DEVICE_REQ *req = (MSG_DEVICE_REQ *)msg;

    json_root = cJSON_Parse(req->data);
    if(!json_root)
    {
        LOG_ERROR("content is not json type");
        return;
    }

    json_cmd = cJSON_GetObjectItem(json_root, "c");
    if(!json_cmd)
    {
        cJSON_Delete(json_root);
        LOG_ERROR("no cmd in content");
        return;
    }
    cmd = json_cmd->valueint;

    json_param = cJSON_GetObjectItem(json_root, "param");
    if(json_param)
    {
        param = cJSON_PrintUnformatted(json_param);
    }
    cJSON_Delete(json_root);

    for (i = 0; i < sizeof(deviceProcs) / sizeof(deviceProcs[0]); i++)
    {
        if (deviceProcs[i].cmd == json_cmd->valueint)
        {
            DEVICE_PROC pfn = deviceProcs[i].pfn;
            if (pfn)
            {
                pfn(msg, param);
                if(param) free(param);
                return 0;
            }
            else
            {
                LOG_ERROR("Message %d not processed!", cmd);
                if(param) free(param);
                return -1;
            }
        }
    }

    LOG_ERROR("unknown device type %d!", cmd);
    if(param) free(param);
    return -1;
}
