/*
 * record.c
 *
 *  Created on: 2017/01/04
 *      Author: lc
 */
#include <string.h>
#include "modem.h"
#include "protocol.h"
#include "fs.h"
#include "ftp.h"
#include "log.h"

#define AT_START_RECORD "AT+CREC=1,\"C:\\record.amr\",0,60,0,0,0" //amr format, 60 seconds limit
#define AT_STOP_RECORD "AT+CREC=2"

eat_bool record_start(void)
{
    unsigned char* cmd = AT_START_RECORD CR;

    fs_delete_file(UPGRADE_FILE_NAME);
    fs_delete_file(RECORDE_FILE_NAME);

    return modem_AT(cmd);
}

eat_bool record_stop(void)
{
    unsigned char* cmd = AT_STOP_RECORD CR;
    return modem_AT(cmd);
}

void record_modem_run(u8 * buf)
{
    char server_Filename[MAX_SERVERFILENAME_LEN] = {0};
    u8 imei[MAX_IMEI_LENGTH + 1] = {0};

    if(strstr(buf, "CREC: 2"))
    {
        eat_get_imei(imei, MAX_IMEI_LENGTH);
        snprintf(server_Filename, MAX_SERVERFILENAME_LEN, "%s_%d.amr", imei, rtc_getTimestamp());
        ftp_upload_file("c:\\record.amr", server_Filename);
        LOG_INFO("start to upload record.amr : %s", server_Filename);
    }
}

