#ifndef USER_MAIN_BLUETOOTH_H_
#define USER_MAIN_BLUETOOTH_H_


typedef enum
{
	BLUETOOTH_STATE_EXIST,
	BLUETOOTH_STATE_NOEXIST
}BLUETOOTH_STATE;

void ResetBluetoothState(void);
void app_bluetooth_thread(void *data);




#endif/*USER_MAIN_BLUETOOTH_H_*/

