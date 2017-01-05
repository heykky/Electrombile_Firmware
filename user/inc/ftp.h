/*
 * ftp.h
 *
 *  Created on: 2017/01/04
 *      Author: lc
 */
#ifndef __USER_FTP_H__
#define __USER_FTP_H__
void ftp_init(void);
void ftp_modem_run(u8 * buf);
void ftp_download_file(char *localFileName, char *serverFileName);
void ftp_upload_file(char *localFileName, char *serverFileName);



#endif/*_USER_FTP_H_*/
