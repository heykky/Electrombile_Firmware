/*
 * ftp.c
 *
 *  Created on: 2017/01/04
 *      Author: lc
 */
#include "modem.h"
#include "ftp.h"

#define CR  "\r"    //CR (carriage return)
#define AT_FTP_START    "AT+FTPCID=1"

static char ftp_localFileName[32] = {0};
static char ftp_serverFileName[32] = {0};

void ftp_upload_file(char *localFileName, char *serverFileName)
{
    strcpy(ftp_localFileName, localFileName);
    strcpy(ftp_serverFileName, serverFileName);
    modem_AT(AT_FTP_START "\n");
}

void ftp_download_file(char *localFileName, char *serverFileName)
{
    strcpy(ftp_localFileName, localFileName);
    strcpy(ftp_serverFileName, serverFileName);
    modem_AT(AT_FTP_START "\n");
}

void ftp_modem_run(u8 * buf)
{
    if(strstr(buf, "FTPCID"))
    {
        if(strstr(buf, "OK"))
        {
            modem_AT("AT+FTPSERV=\"test.xiaoan110.com\"" CR);
        }
    }
    else if(strstr(buf, "FTPSERV"))
    {
        if(strstr(buf, "OK"))
        {
            modem_AT("AT+FTPUN=\"anonymous\"" CR);
        }
    }
    else if(strstr(buf, "FTPUN"))
    {
        if(strstr(buf, "OK"))
        {
            modem_AT("AT+FTPPUTNAME=\"TEST.bin\"" CR);
        }
    }
    else if(strstr(buf, "FTPPUTNAME"))
    {
        if(strstr(buf, "OK"))
        {
            modem_AT("AT+FTPPUTPATH=\"/home/\"" CR);
        }
    }
    else if(strstr(buf, "FTPPUTPATH"))
    {
        if(strstr(buf, "OK"))
        {
            modem_AT("AT+FTPPUTFRMFS=\"c:\\log.old\"" CR);
        }
    }
    else if (strstr(buf, "FTPGETPATH"))
    {
        if(strstr(buf, "OK"))
        {
            modem_AT("AT+FTPGETTOFS=0,\"log.old\"" CR);
        }
    }
    else if(strstr(buf, "FTPPUTFRMFS: 0"))
    {
        print("put OK");
    }
    else if(strstr(buf, "FTPGETTOFS: 0"))
    {
        print("get OK");
    }
}

void ftp_init(void)
{
    modem_AT("AT+SAPBR=1,1" CR);
}
