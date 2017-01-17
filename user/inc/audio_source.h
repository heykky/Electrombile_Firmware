#ifndef USER_AUDIO_SOURCE_H_
#define USER_AUDIO_SOURCE_H_

const u8* audio_defaultAudioSource_found(void);
const u8* audio_defaultAudioSource_lost(void);
const u8* audio_defaultAlarm(void);

unsigned int audio_sizeofDefaultAudioSource_found(void);
unsigned int audio_sizeofDefaultAudioSource_lost(void);
unsigned int audio_sizeofDefaultAlarm(void);


#endif /* USER_AUDIO_SOURCE_H_ */

