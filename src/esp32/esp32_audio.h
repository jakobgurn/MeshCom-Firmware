/**
 * @file        esp32_audio.h
 * @brief       functions for handling audio playback on ESP32
 * @author      Ing. Jakob Gurnhofer (OE3GJC)
 * @license     MIT
 * @copyright   Copyright (c) 2025 ICSSW.org
 * @date        2025-05-28
 */

#ifndef _ESP32_AUDIO_H_
#define _ESP32_AUDIO_H_

#include <Audio.h>

extern Audio audio;

void init_audio();
bool play_file_from_sd(const char *filename);
bool play_file_from_sd(const char *filename, int volume);
bool play_file_from_sd_blocking(const char *filename);
bool play_file_from_sd_blocking(const char *filename, int volume);
void play_cw(const char character);
void play_cw(const char character, int volume);
void play_function(void *parameter);

#endif