/*
 * Vaxp Audio D-Bus Client
 * Implementation - connects to org.vaxp.Audio D-Bus service
 */

#include "audio_client.h"
#include <gio/gio.h>
#include <stdio.h>

#define AUDIO_BUS_NAME    "org.vaxp.Audio"
#define AUDIO_OBJECT_PATH "/org/vaxp/Audio"
#define AUDIO_INTERFACE   "org.vaxp.Audio"

static GDBusProxy* audio_proxy = NULL;

/* ============================================================================
 * Initialization
 * ============================================================================ */

bool vaxp_audio_init(void) {
    if (audio_proxy) return true;  /* Already initialized */
    
    GError* error = NULL;
    audio_proxy = g_dbus_proxy_new_for_bus_sync(
        G_BUS_TYPE_SESSION,
        G_DBUS_PROXY_FLAGS_NONE,
        NULL,
        AUDIO_BUS_NAME,
        AUDIO_OBJECT_PATH,
        AUDIO_INTERFACE,
        NULL,
        &error
    );
    
    if (error) {
        fprintf(stderr, "Audio: Failed to connect to D-Bus: %s\n", error->message);
        g_error_free(error);
        return false;
    }
    
    if (!audio_proxy) {
        fprintf(stderr, "Audio: D-Bus proxy is NULL\n");
        return false;
    }
    
    printf("Audio: Connected to %s\n", AUDIO_BUS_NAME);
    return true;
}

void vaxp_audio_cleanup(void) {
    if (audio_proxy) {
        g_object_unref(audio_proxy);
        audio_proxy = NULL;
    }
}

/* ============================================================================
 * Helper for D-Bus calls
 * ============================================================================ */

static GVariant* call_method(const char* method, GVariant* params) {
    if (!audio_proxy) {
        if (!vaxp_audio_init()) return NULL;
    }
    
    GError* error = NULL;
    GVariant* result = g_dbus_proxy_call_sync(
        audio_proxy,
        method,
        params,
        G_DBUS_CALL_FLAGS_NONE,
        -1,  /* Default timeout */
        NULL,
        &error
    );
    
    if (error) {
        fprintf(stderr, "Audio: %s failed: %s\n", method, error->message);
        g_error_free(error);
        return NULL;
    }
    
    return result;
}

/* ============================================================================
 * Volume Control
 * ============================================================================ */

int vaxp_audio_get_volume(void) {
    GVariant* result = call_method("GetVolume", NULL);
    if (!result) return 0;
    
    gint volume;
    g_variant_get(result, "(i)", &volume);
    g_variant_unref(result);
    
    return volume;
}

bool vaxp_audio_set_volume(int volume) {
    if (volume < 0) volume = 0;
    if (volume > 150) volume = 150;
    
    GVariant* result = call_method("SetVolume", g_variant_new("(i)", volume));
    if (!result) return false;
    
    gboolean success;
    g_variant_get(result, "(b)", &success);
    g_variant_unref(result);
    
    return success;
}

/* ============================================================================
 * Mute Control
 * ============================================================================ */

bool vaxp_audio_get_muted(void) {
    GVariant* result = call_method("GetMuted", NULL);
    if (!result) return false;
    
    gboolean muted;
    g_variant_get(result, "(b)", &muted);
    g_variant_unref(result);
    
    return muted;
}

bool vaxp_audio_set_muted(bool muted) {
    GVariant* result = call_method("SetMuted", g_variant_new("(b)", muted));
    if (!result) return false;
    
    gboolean success;
    g_variant_get(result, "(b)", &success);
    g_variant_unref(result);
    
    return success;
}

bool vaxp_audio_toggle_mute(void) {
    bool current = vaxp_audio_get_muted();
    return vaxp_audio_set_muted(!current);
}

/* ============================================================================
 * Volume Adjustment
 * ============================================================================ */

bool vaxp_audio_volume_up(int step) {
    int current = vaxp_audio_get_volume();
    return vaxp_audio_set_volume(current + step);
}

bool vaxp_audio_volume_down(int step) {
    int current = vaxp_audio_get_volume();
    return vaxp_audio_set_volume(current - step);
}

/* ============================================================================
 * Get Full State
 * ============================================================================ */

bool vaxp_audio_get_state(VaxpAudioState* state) {
    if (!state) return false;
    
    state->volume = vaxp_audio_get_volume();
    state->muted = vaxp_audio_get_muted();
    
    /* Get max volume */
    GVariant* result = call_method("GetMaxVolume", NULL);
    if (result) {
        g_variant_get(result, "(i)", &state->max_volume);
        g_variant_unref(result);
    } else {
        state->max_volume = 100;
    }
    
    /* Get mic state */
    result = call_method("GetMicVolume", NULL);
    if (result) {
        g_variant_get(result, "(i)", &state->mic_volume);
        g_variant_unref(result);
    } else {
        state->mic_volume = 0;
    }
    
    result = call_method("GetMicMuted", NULL);
    if (result) {
        gboolean muted;
        g_variant_get(result, "(b)", &muted);
        state->mic_muted = muted;
        g_variant_unref(result);
    } else {
        state->mic_muted = false;
    }
    
    return true;
}
