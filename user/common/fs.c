/*
 * fs.c
 *
 *  Created on: 2016年2月19日
 *      Author: jk
 */


#include <string.h>

#include <eat_fs.h>

#include "fs.h"
#include "uart.h"
#include "log.h"
#include "debug.h"
#include "utils.h"

#define SYSTEM_DRIVE    "C:\\"
#define TF_DRIVE        "D:\\"

#define CMD_STRING_LS   "ls"
#define CMD_STRING_RM   "rm"
#define CMD_STRING_CAT  "cat"
#define CMD_STRING_TAIL  "tail"



#define MAX_FILENAME_LEN    32
static int fs_ls(const unsigned char* cmdString, unsigned short length)
{
    FS_HANDLE fh;
    EAT_FS_DOSDirEntry fileinfo;
    WCHAR filename_w[MAX_FILENAME_LEN];
    char filename[MAX_FILENAME_LEN];
    SINT64 size = 0;
    int rc = 0;
    char directory[MAX_FILENAME_LEN] = {0};
    WCHAR directory_w[MAX_FILENAME_LEN] = {0};

    const unsigned char *directoryname = string_bypass(cmdString, "ls");
    directoryname = string_trimLeft(directoryname);
    strncpy(directory, directoryname, MAX_FILENAME_LEN);
    string_trimRight(directory);

    if(0 < strlen(directory))
    {
        strcat(directory,"\\*.*");
        ascii2unicode(directory_w,directory);
        print("%s:%s:%s",cmdString, directoryname, directory);
        fh = eat_fs_FindFirst(directory_w, 0, 0, &fileinfo, filename_w, sizeof(filename_w));
    }
    else
    {

        fh = eat_fs_FindFirst(L"C:\\*.*", 0, 0, &fileinfo, filename_w, sizeof(filename_w));
    }


    if (fh > 0)
    {

        do
        {
            unicode2ascii(filename, filename_w);

            //filename, file size, file attr, date
            print("%-16s\t %d\t %c%c%c%c%c\t %d-%d-%d %d:%d:%d\r\n",
                    filename,   //fileinfo.FileName,
                    fileinfo.FileSize,
                    fileinfo.Attributes & FS_ATTR_DIR ? 'D' : '-',
                    fileinfo.Attributes & FS_ATTR_READ_ONLY ? 'R' : '-',
                    fileinfo.Attributes & FS_ATTR_HIDDEN ? 'H' : '-',
                    fileinfo.Attributes & FS_ATTR_SYSTEM ? 'S' : '-',
                    fileinfo.Attributes & FS_ATTR_ARCHIVE ? 'A' : '-',
                    fileinfo.CreateDateTime.Year1980 + 1980,
                    fileinfo.CreateDateTime.Month,
                    fileinfo.CreateDateTime.Day,
                    fileinfo.CreateDateTime.Hour,
                    fileinfo.CreateDateTime.Minute,
                    fileinfo.CreateDateTime.Second2);
        }while (eat_fs_FindNext(fh, &fileinfo, filename_w, sizeof(filename_w)) == EAT_FS_NO_ERROR);
    }

    eat_fs_FindClose(fh);


    rc = eat_fs_GetDiskFreeSize(EAT_FS, &size);
    if (rc == EAT_FS_NO_ERROR)
    {
        print("\r\n\t free disk size:%lld\r\n", size);
    }
    else
    {
        print("\r\n\t free disk size:---(error:%d)\r\n", rc);
    }

    return 0;
}


/*
 * cmd format: rm file.txt
 * must have a parameter, do not support wildcard
 */
static int fs_rm(const unsigned char* cmdString, unsigned short length)
{
    const unsigned char* filename = strstr(cmdString, CMD_STRING_RM) + strlen(CMD_STRING_RM);
    int rc = EAT_FS_NO_ERROR;
    WCHAR filename_w[MAX_FILENAME_LEN];


    filename = string_trimLeft(filename);
    string_trimRight((unsigned char*)filename);
    if (strlen(filename) == 0)
    {
        LOG_INFO("parameter not correct");
        return 0;
    }

    ascii2unicode(filename_w, filename);      //FIXME: overflow bug: the filename length may exceed MAX_FILENAME_LEN

    rc = eat_fs_Delete(filename_w);
    if (rc == EAT_FS_FILE_NOT_FOUND)
    {
        print("file %s not found", filename);
    }
    else if(rc == EAT_FS_NO_ERROR)
    {
        print("delete file Success");
    }
    else
    {
        print("delete file %s fail, return code is %d", filename, rc);
    }

    return rc;
}

/*
 * cmd format: cat file.txt
 * must have a parameter, do not support wildcard
 */
static int fs_cat(const unsigned char* cmdString, unsigned short length)
{
    const unsigned char* filename = strstr(cmdString, CMD_STRING_CAT) + strlen(CMD_STRING_CAT);
    int rc = EAT_FS_NO_ERROR;
    WCHAR filename_w[MAX_FILENAME_LEN];


    filename = string_trimLeft(filename);
    string_trimRight((unsigned char*)filename);

    if (strlen(filename) == 0)
    {
        print("parameter not correct");
        return 0;
    }

    ascii2unicode(filename_w, filename);      //FIXME: overflow bug: the filename length may exceed MAX_FILENAME_LEN

    //TODO: to be completed
    return rc;
}


static int fs_tail(const unsigned char* cmdString, unsigned short length)
{
#define MAX_TAIL_SIZE   1024
    const unsigned char* filename = strstr(cmdString, CMD_STRING_TAIL) + strlen(CMD_STRING_TAIL);
    int rc = EAT_FS_NO_ERROR;
    WCHAR filename_w[MAX_TAIL_SIZE];
    FS_HANDLE fh;
    unsigned int filesize = 0;
    char buf[MAX_TAIL_SIZE] = {0};

    filename = string_trimLeft(filename);
    string_trimRight((unsigned char*)filename);

    if (strlen(filename) == 0)
    {
        print("parameter not correct");
        return 0;
    }

    ascii2unicode(filename_w, filename);      //FIXME: overflow bug: the filename length may exceed MAX_FILENAME_LEN

    fh = eat_fs_Open(filename_w, FS_READ_ONLY);
    if(EAT_FS_FILE_NOT_FOUND == fh)
    {
        print("log file not exists.");
        return -1;
    }

    if (fh < EAT_FS_NO_ERROR)
    {
        print("open file failed, eat_fs_Open return %d!", fh);
        return -1;
    }

    rc = eat_fs_GetFileSize(fh, &filesize);
    if (rc < EAT_FS_NO_ERROR)
    {
        eat_fs_Close(fh);
        print("get file size failed: return %d", rc);
        return -1;
    }

    rc = eat_fs_Seek(fh, filesize > MAX_TAIL_SIZE ? filesize - MAX_TAIL_SIZE : 0, EAT_FS_FILE_BEGIN);
    if (rc < EAT_FS_NO_ERROR)
    {
        print("seek file pointer failed:%d", rc);
        eat_fs_Close(fh);

        return -1;
    }

    rc = eat_fs_Read(fh, buf, MAX_TAIL_SIZE, NULL);
    if (rc < EAT_FS_NO_ERROR)
    {
        print("read file failed:%d", rc);
        eat_fs_Close(fh);

        return -1;
    }

    print("%s\n", buf);

    eat_fs_Close(fh);

    return rc;
}

void fs_initial(void)
{
    regist_cmd(CMD_STRING_LS, fs_ls);
    regist_cmd(CMD_STRING_RM, fs_rm);
    regist_cmd(CMD_STRING_CAT, fs_cat);
    regist_cmd(CMD_STRING_TAIL, fs_tail);
}

SINT64 fs_getDiskFreeSize(void)
{
    SINT64 size = 0;

    int rc = eat_fs_GetDiskFreeSize(EAT_FS, &size);
    if(rc == EAT_FS_NO_ERROR)
    {
        LOG_DEBUG("Get free disk size success,and the free disk size is %lld", size);
    }
    else
    {
        LOG_ERROR("Get free disk size failed, rc = %d",rc);
        return -1;
    }

    return size;
}

int fs_factory(void)
{
    int rc = 0;
    int result = 0;

    rc = eat_fs_Delete(SETTINGFILE_NAME);
    if (rc == EAT_FS_FILE_NOT_FOUND || rc >= EAT_FS_NO_ERROR)
    {
        LOG_DEBUG("delete setting file ok");
    }
    else
    {
        LOG_DEBUG("delete setting file error: %d",rc);
        result = -1;
    }

    rc = eat_fs_Delete(LOG_FILE_BAK);
    if (rc == EAT_FS_FILE_NOT_FOUND || rc >= EAT_FS_NO_ERROR)
    {
        LOG_DEBUG("delete log.old file ok");
    }
    else
    {
        LOG_DEBUG("delete log.old file error: %d",rc);
        result = -1;
    }

    rc = eat_fs_Delete(LOG_FILE_NAME);
    if (rc == EAT_FS_FILE_NOT_FOUND || rc >= EAT_FS_NO_ERROR)
    {
        LOG_DEBUG("delete log.txt file ok");
    }
    else
    {
        LOG_DEBUG("delete log.txt file error: %d",rc);
        result = -1;
    }

    rc = eat_fs_Delete(ITINERARYFILE_NAME);
    if (rc == EAT_FS_FILE_NOT_FOUND || rc >= EAT_FS_NO_ERROR)
    {
        LOG_DEBUG("delete itinerary file ok");
    }
    else
    {
        LOG_DEBUG("delete itinerary file error: %d",rc);
        result = -1;
    }

    return result;
}

