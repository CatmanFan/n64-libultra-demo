#ifndef __CONFIG_AUDIO_H__
#define __CONFIG_AUDIO_H__

/**
 * Enables audio playback.
 */
// #define ENABLE_AUDIO

/**
 * Configure audio bitrate.
 */
#define AUDIO_BITRATE		44100

/**
 * Configure buffer sizes used for audio pointer, SFX and BGM storage.
 */
#define PTR_BUF_SIZE		0x4000 // Sizes taken from NuSystem
#define SFX_BUF_SIZE		0x4000
#define BGM_BUF_SIZE		0x4000

#endif