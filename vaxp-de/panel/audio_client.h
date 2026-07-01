/*
 * Vaxp Audio D-Bus Client
 * Client library for connecting to org.vaxp.Audio service
 */

#ifndef VAXP_AUDIO_CLIENT_H
#define VAXP_AUDIO_CLIENT_H

#include <stdbool.h>

/* Audio state structure */
typedef struct {
    int volume;         /* 0-100 (or 0-150 with overamplification) */
    bool muted;
    int mic_volume;
    bool mic_muted;
    int max_volume;     /* 100 or 150 */
} VaxpAudioState;

/* Initialize D-Bus connection to audio service */
bool vaxp_audio_init(void);

/* Cleanup */
void vaxp_audio_cleanup(void);

/* Get current audio state */
bool vaxp_audio_get_state(VaxpAudioState* state);

/* Volume control */
int  vaxp_audio_get_volume(void);
bool vaxp_audio_set_volume(int volume);

/* Mute control */
bool vaxp_audio_get_muted(void);
bool vaxp_audio_set_muted(bool muted);
bool vaxp_audio_toggle_mute(void);

/* Volume adjustment */
bool vaxp_audio_volume_up(int step);
bool vaxp_audio_volume_down(int step);

#endif /* VAXP_AUDIO_CLIENT_H */
