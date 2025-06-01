//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2025 Gibbon
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//  System interface for music using FMOD Core API.
//  Adapted from SDL_mixer based i_music.c from Chocolate Doom.
//  Includes lazy initialization for the FMOD system.
//  The mus2mid conversion step is still required externally.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// FMOD Core API header
#include "fmod.h"
#include "fmod_errors.h"

// --- Basic type definitions (normally from doomtype.h or similar) ---
#ifndef DOOMTYPE_H_REPLACEMENT
#define DOOMTYPE_H_REPLACEMENT
typedef int dboolean;
#define true 1
#define false 0
#endif // DOOMTYPE_H_REPLACEMENT

// --- Music module structure definition ---
typedef struct {
    void* possible_devices;
    int num_possible_devices;
    dboolean(*InitMusic)(void);
    void (*ShutdownMusic)(void);
    void (*SetMusicVolume)(int volume);
    void (*PauseSong)(void);
    void (*ResumeSong)(void);
    void* (*RegisterSong)(void* data, int len);
    void (*UnRegisterSong)(void* handle);
    void (*PlaySong)(void* handle, dboolean looping);
    void (*StopSong)(void);
    dboolean(*MusicIsPlaying)(void);
    void (*Poll)(void);
} music_module_t;

// --- External mus2mid function (from mmus2mid.c or similar) ---
extern void mus2mid(unsigned char* data);

// --- FMOD Global Variables ---
static FMOD_SYSTEM* fmod_system = NULL;
static FMOD_SOUND* current_fmod_song_handle = NULL;
static FMOD_CHANNEL* music_fmod_channel = NULL;

static dboolean fmod_music_system_initialized = false; // Guard for initialization
static float current_fmod_playback_volume = 1.0f;

// --- FMOD Error Checking Macro ---
static void FMOD_ERRCHECK_IMPL(FMOD_RESULT result, const char* file, int line) {
    if (result != FMOD_OK) {
        fprintf(stderr, "FMOD error! Code: %d, Message: %s, File: %s, Line: %d\n",
            result, FMOD_ErrorString(result), file, line);
    }
}
#define FMOD_ERRCHECK(result) FMOD_ERRCHECK_IMPL(result, __FILE__, __LINE__)

static dboolean I_FMOD_InitMusic(void);

// --- Music Interface Functions (FMOD Implementation) ---

static dboolean I_FMOD_InitMusic(void) {
    FMOD_RESULT result;

    // If already initialized, do nothing.
    if (fmod_music_system_initialized && fmod_system) {
        return true;
    }

    if (fmod_system) {
        FMOD_System_Release(fmod_system);
        fmod_system = NULL;
    }

    result = FMOD_System_Create(&fmod_system, FMOD_VERSION);
    FMOD_ERRCHECK(result);
    if (result != FMOD_OK) {
        fmod_system = NULL;
        return false;
    }

    result = FMOD_System_Init(fmod_system, 32, FMOD_INIT_NORMAL, NULL);
    FMOD_ERRCHECK(result);
    if (result != FMOD_OK) {
        FMOD_System_Release(fmod_system);
        fmod_system = NULL;
        return false;
    }

    fmod_music_system_initialized = true;
    fprintf(stdout, "FMOD Music System Initialized.\n");
    return true;
}

static void I_FMOD_ShutdownMusic(void) {
    if (!fmod_music_system_initialized || !fmod_system) {
        return;
    }
    if (music_fmod_channel) {
        FMOD_Channel_Stop(music_fmod_channel);
        music_fmod_channel = NULL;
    }
    if (current_fmod_song_handle) {
        FMOD_Sound_Release(current_fmod_song_handle);
        current_fmod_song_handle = NULL;
    }
    FMOD_System_Close(fmod_system);
    FMOD_System_Release(fmod_system);
    fmod_system = NULL;
    fmod_music_system_initialized = false; // Reset flag
    fprintf(stdout, "FMOD Music System Shutdown.\n");
}

static void I_FMOD_SetMusicVolume(int volume) {
    if (!fmod_music_system_initialized) {
        if (!I_FMOD_InitMusic()) {
            fprintf(stderr, "FMOD SetMusicVolume: Auto-initialization failed.\n");
            return;
        }
    }
    current_fmod_playback_volume = (float)volume / 127.0f;
    if (current_fmod_playback_volume < 0.0f) current_fmod_playback_volume = 0.0f;
    if (current_fmod_playback_volume > 1.0f) current_fmod_playback_volume = 1.0f;

    if (music_fmod_channel) {
        FMOD_RESULT result = FMOD_Channel_SetVolume(music_fmod_channel, current_fmod_playback_volume);
        FMOD_ERRCHECK(result);
    }
}

static void I_FMOD_PauseSong(void) {
    if (!fmod_music_system_initialized) {
        if (!music_fmod_channel) return;
        if (!I_FMOD_InitMusic()) return;
    }
    if (!music_fmod_channel) return;
    FMOD_RESULT result = FMOD_Channel_SetPaused(music_fmod_channel, true);
    FMOD_ERRCHECK(result);
}

static void I_FMOD_ResumeSong(void) {
    if (!fmod_music_system_initialized) {
        if (!music_fmod_channel) return;
        if (!I_FMOD_InitMusic()) return;
    }
    if (!music_fmod_channel) return;
    FMOD_RESULT result = FMOD_Channel_SetPaused(music_fmod_channel, false);
    FMOD_ERRCHECK(result);
}

static void* I_FMOD_RegisterSong(void* data, int len) {
    FMOD_SOUND* sound_object;
    FMOD_RESULT result;
    (void)len;

    if (!fmod_music_system_initialized) {
        if (!I_FMOD_InitMusic()) {
            fprintf(stderr, "FMOD RegisterSong: Auto-initialization failed.\n");
            return NULL;
        }
    }
    if (!fmod_system || !data) {
        return NULL;
    }

    mus2mid((unsigned char*)data);

    if (current_fmod_song_handle) {
        if (music_fmod_channel) {
            FMOD_Channel_Stop(music_fmod_channel);
            music_fmod_channel = NULL;
        }
        FMOD_Sound_Release(current_fmod_song_handle);
        current_fmod_song_handle = NULL;
    }

    result = FMOD_System_CreateSound(fmod_system, "DOOMSONG.MID", FMOD_CREATESTREAM | FMOD_LOOP_OFF, 0, &sound_object);
    FMOD_ERRCHECK(result);
    if (result != FMOD_OK) {
        fprintf(stderr, "FMOD Error: Could not load 'DOOMSONG.MID'.\n");
        return NULL;
    }
    current_fmod_song_handle = sound_object;
    return sound_object;
}

static void I_FMOD_UnRegisterSong(void* handle) {
    if (!fmod_music_system_initialized || !handle) return;

    FMOD_SOUND* sound_to_release = (FMOD_SOUND*)handle;
    if (sound_to_release == current_fmod_song_handle && music_fmod_channel) {
        FMOD_SOUND* playing_sound = NULL;
        FMOD_Channel_GetCurrentSound(music_fmod_channel, &playing_sound);
        if (playing_sound == sound_to_release) {
            FMOD_Channel_Stop(music_fmod_channel);
            music_fmod_channel = NULL;
        }
    }
    FMOD_RESULT result = FMOD_Sound_Release(sound_to_release);
    FMOD_ERRCHECK(result);
    if (sound_to_release == current_fmod_song_handle) {
        current_fmod_song_handle = NULL;
    }
}

static void I_FMOD_PlaySong(void* handle, dboolean looping) {
    FMOD_RESULT result;
    if (!fmod_music_system_initialized) {
        if (!I_FMOD_InitMusic()) {
            fprintf(stderr, "FMOD PlaySong: Auto-initialization failed.\n");
            return;
        }
    }
    if (!fmod_system || !handle) {
        return;
    }

    FMOD_SOUND* sound_to_play = (FMOD_SOUND*)handle;
    if (music_fmod_channel) {
        FMOD_Channel_Stop(music_fmod_channel);
        music_fmod_channel = NULL;
    }

    FMOD_MODE sound_mode;
    FMOD_Sound_GetMode(sound_to_play, &sound_mode);
    if (looping) {
        sound_mode &= ~FMOD_LOOP_OFF;
        sound_mode |= FMOD_LOOP_NORMAL;
        FMOD_Sound_SetLoopCount(sound_to_play, -1);
    }
    else {
        sound_mode &= ~FMOD_LOOP_NORMAL;
        sound_mode |= FMOD_LOOP_OFF;
        FMOD_Sound_SetLoopCount(sound_to_play, 0);
    }
    result = FMOD_Sound_SetMode(sound_to_play, sound_mode);
    FMOD_ERRCHECK(result);

    result = FMOD_System_PlaySound(fmod_system, sound_to_play, NULL, false, &music_fmod_channel);
    FMOD_ERRCHECK(result);
    if (result == FMOD_OK && music_fmod_channel) {
        FMOD_Channel_SetVolume(music_fmod_channel, current_fmod_playback_volume);
    }
    else {
        music_fmod_channel = NULL;
    }
}

static void I_FMOD_StopSong(void) {
    // No auto-init for stop. If system isn't up, there's nothing to stop.
    if (!fmod_music_system_initialized || !music_fmod_channel) return;
    FMOD_RESULT result = FMOD_Channel_Stop(music_fmod_channel);
    FMOD_ERRCHECK(result);
    music_fmod_channel = NULL;
}

static dboolean I_FMOD_MusicIsPlaying(void) {
    FMOD_BOOL is_playing = false;
    FMOD_RESULT result;

    if (!fmod_music_system_initialized) {
        return false;
    }
    if (!music_fmod_channel) {
        return false;
    }

    result = FMOD_Channel_IsPlaying(music_fmod_channel, &is_playing);
    if (result != FMOD_OK && result != FMOD_ERR_INVALID_HANDLE && result != FMOD_ERR_CHANNEL_STOLEN) {
        FMOD_ERRCHECK(result);
    }
    return is_playing ? true : false;
}

static void I_FMOD_Poll(void) {
    if (!fmod_music_system_initialized) {
        if (!I_FMOD_InitMusic()) {
            return;
        }
    }
    if (fmod_system) {
        FMOD_RESULT result = FMOD_System_Update(fmod_system);
        if (result != FMOD_OK && result != FMOD_ERR_NET_SOCKET_ERROR) {
            FMOD_ERRCHECK(result);
        }
    }
}

// --- The Music Module Export ---
music_module_t music_fmod_module = {
    NULL,
    0,
    I_FMOD_InitMusic,       // Can still be called explicitly if desired
    I_FMOD_ShutdownMusic,
    I_FMOD_SetMusicVolume,
    I_FMOD_PauseSong,
    I_FMOD_ResumeSong,
    I_FMOD_RegisterSong,
    I_FMOD_UnRegisterSong,
    I_FMOD_PlaySong,
    I_FMOD_StopSong,
    I_FMOD_MusicIsPlaying,
    I_FMOD_Poll
};
