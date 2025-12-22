/*
 * Venom Audio D-Bus Client
 * Client library for connecting to org.venom.Audio service
 */

#ifndef VENOM_AUDIO_CLIENT_H
#define VENOM_AUDIO_CLIENT_H

#include <stdbool.h>

/* Audio state structure */
typedef struct {
    int volume;         /* 0-100 (or 0-150 with overamplification) */
    bool muted;
    int mic_volume;
    bool mic_muted;
    int max_volume;     /* 100 or 150 */
} VenomAudioState;

/* Initialize D-Bus connection to audio service */
bool venom_audio_init(void);

/* Cleanup */
void venom_audio_cleanup(void);

/* Get current audio state */
bool venom_audio_get_state(VenomAudioState* state);

/* Volume control */
int  venom_audio_get_volume(void);
bool venom_audio_set_volume(int volume);

/* Mute control */
bool venom_audio_get_muted(void);
bool venom_audio_set_muted(bool muted);
bool venom_audio_toggle_mute(void);

/* Volume adjustment */
bool venom_audio_volume_up(int step);
bool venom_audio_volume_down(int step);

#endif /* VENOM_AUDIO_CLIENT_H */
