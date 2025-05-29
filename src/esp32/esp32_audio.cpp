/**
 * @file        esp32_audio.cpp
 * @brief       functions for handling audio playback on ESP32
 * @author      Ing. Jakob Gurnhofer (OE3GJC)
 * @license     MIT
 * @copyright   Copyright (c) 2025 ICSSW.org
 * @date        2025-05-28
 */

#include "esp32_audio.h"
#include <configuration.h>
#include <Audio.h>
#include <SD.h>
#include <driver/i2s.h>
#include <esp32/esp32_flash.h>

#include <loop_functions_extern.h>
 
Audio audio;
SemaphoreHandle_t audioSemaphore;

/**
 * initializes audio
 */
void init_audio()
{
    if (bDEBUG)
    {
        Serial.println("[audio]...initializing");
    }

    audioSemaphore = xSemaphoreCreateBinary();
    xSemaphoreGive(audioSemaphore);
    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
}

/**
 * play a supported file from SD in the background
 */
bool play_file_from_sd(const char *filename, int volume)
{
    if (meshcom_settings.node_mute)
    {
        if (bDEBUG)
        {
            Serial.println("[audio]...muted");
        }
        return true;
    }
    if (xSemaphoreTake(audioSemaphore, 0) == pdTRUE)
    {
        if (SD.exists(filename))
        {
            audio.setVolume(volume);
            audio.connecttoFS(SD, filename);

            if (bDEBUG)
            {
                Serial.printf("[audio]...playing %s in background\n", filename);
            }

            xTaskCreatePinnedToCore(
                play_function,
                "audio play task",
                4 * 1024,
                NULL,
                1,
                NULL,
                1
            );
            return true;
        }
        else
        {
            Serial.printf("[audio]...file %s not found on SD\n", filename);
            return false;
        }
    }
    else
    {
        Serial.println("[audio]...currently playing another file");
        return true;
    }
}

/**
 * play a supported file from SD in the background
 */
bool play_file_from_sd(const char *filename)
{
    return play_file_from_sd(filename, 20);
}

/**
 * play a supported file from SD
 */
bool play_file_from_sd_blocking(const char *filename, int volume)
{
    if (meshcom_settings.node_mute)
    {
        if (bDEBUG)
        {
            Serial.println("[audio]...muted");
        }
        return true;
    }
    if (SD.exists(filename))
    {
        audio.setVolume(volume);
        audio.connecttoFS(SD, filename);

        if (bDEBUG)
        {
            Serial.printf("[audio]...playing %s\n", filename);
        }
        while (audio.isRunning()) {
            audio.loop();
        }
        audio.stopSong();
        return true;
    }
    else
    {
        Serial.printf("[audio]...file %s not found on SD\n", filename);
        return false;
    }
}

/**
 * play a supported file from SD
 */
bool play_file_from_sd_blocking(const char *filename)
{
    return play_file_from_sd_blocking(filename, 20);
}

/**
 * play a CW character
 */
void play_cw(const char character, int volume)
{
    if (meshcom_settings.node_mute)
    {
        if (bDEBUG)
        {
            Serial.println("[audio]...muted");
        }
        return;
    }

}

/**
 * play a CW character
 */
void play_cw(const char character)
{
    return play_cw(character, 20);
}

/**
 * function for task to play audio in background
 */
void play_function(void *parameter)
{
    while (audio.isRunning()) {
        audio.loop();
        vTaskDelay(10);
    }
    audio.stopSong();

    xSemaphoreGive(audioSemaphore);

    vTaskSuspend(NULL);
}
