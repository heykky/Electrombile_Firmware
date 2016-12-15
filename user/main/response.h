/*
 * response.h
 *
 *  Created on: 2016年2月4日
 *      Author: jk
 */

#ifndef USER_MAIN_RESPONSE_H_
#define USER_MAIN_RESPONSE_H_

int cmd_Login_rsp(const void* msg);
int cmd_Ping_rsp(const void* msg);
int cmd_SimInfo_rsp(const void* msg);
int cmd_Alarm_rsp(const void* msg);
int cmd_Sms_rsp(const void* msg);

int cmd_DefendOn_rsp(const void* msg);
int cmd_DefendOff_rsp(const void* msg);
int cmd_DefendGet_rsp(const void* msg);
int cmd_Seek_rsp(const void* msg);
int cmd_Location_rsp(const void* msgLocation);
int cmd_AutodefendSwitchSet_rsp(const void* msg);
int cmd_AutodefendSwitchGet_rsp(const void* msg);
int cmd_AutodefendPeriodSet_rsp(const void* msg);
int cmd_AutodefendPeriodGet_rsp(const void* msg);
int cmd_Server_rsp(const void* msg);
int cmd_Timer_rsp(const void* msg);
int cmd_Battery_rsp(const void* msg);
int cmd_SetServer_rsp(const void* msg);
int cmd_Reboot_rsp(const void* msg);
int cmd_SetBatteryType_rsp(const void* msg);

int cmd_UpgradeStart_rsp(const void* msg);
int cmd_UpgradeData_rsp(const void* msg);
int cmd_UpgradeEnd_rsp(const void* msg);
int cmd_DeviceInfo_rsp(const void* msg);

int cmd_LogInfo_rsp(const void * msg);
int cmd_GSMSignal_rsp(const void * msg);
int cmd_GPSSignal_rsp(const void * msg);
int cmd_433Signal_rsp(const void * msg);
int cmd_GetSetting_rsp(const void * msg);
int cmd_GetBattery_rsp(const void* msg);
int cmd_GetAT_rsp(const void* msg);


#define MAX_DEBUG_BUF_LEN 256   //128 for loginfo is not enough


#endif /* USER_MAIN_RESPONSE_H_ */
