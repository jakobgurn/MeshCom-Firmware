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

// I2S-Handle
i2s_port_t i2s_num = I2S_NUM;

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

    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT, // Mono
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = 0,
        .dma_buf_count = 8,
        .dma_buf_len = 64,
        .use_apll = false,
        .tx_desc_auto_clear = true,
        .fixed_mclk = 0
    };

    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_BCLK,
        .ws_io_num = I2S_LRC,
        .data_out_num = I2S_DOUT,
        .data_in_num = -1
    };

    i2s_driver_install(i2s_num, &i2s_config, 0, NULL);
    i2s_set_pin(i2s_num, &pin_config);

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
 * create tone and play via I2S
 */
void playTone(int duration_ms, int volume_percent) {
    int samples_per_period = SAMPLE_RATE / TONE_FREQ;
    int half_period = samples_per_period / 2;

    // buffer for 1 ms Ton (16bit Samples)
    int16_t buffer[BUFFER_SIZE];

    // fill buffer
    for (int i = 0; i < BUFFER_SIZE; i++) {
        int pos_in_period = i % samples_per_period;

        if (pos_in_period < half_period) {
            buffer[i] = (3000 * volume_percent) / 100;  // positiv Value (Amplitude)
        } else {
            buffer[i] = (-3000 * volume_percent) / 100; // negativ Value
        }
    }

    int total_ms = duration_ms;
    size_t bytes_written;

    // send buffer until duration is et
    while (total_ms > 0) {
        i2s_write(i2s_num, buffer, sizeof(buffer), &bytes_written, portMAX_DELAY);
        total_ms--;
    }
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

    String morseCode;

    switch (character)
    {
        case 'a': case 'A':
            morseCode = ".-";
            break;
        case 'b': case 'B':
            morseCode = "-...";
            break;
        case 'c': case 'C':
            morseCode = "-.-.";
            break;
        case 'd': case 'D':
            morseCode = "-..";
            break;
        case 'e': case 'E':
            morseCode = ".";
            break;
        case 'f': case 'F':
            morseCode = "..-.";
            break;
        case 'g': case 'G':
            morseCode = "--.";
            break;
        case 'h': case 'H':
            morseCode = "....";
            break;
        case 'i': case 'I':
            morseCode = "..";
            break;
        case 'j': case 'J':
            morseCode = ".---";
            break;
        case 'k': case 'K':
            morseCode = "-.-";
            break;
        case 'l': case 'L':
            morseCode = ".-..";
            break;
        case 'm': case 'M':
            morseCode = "--";
            break;
        case 'n': case 'N':
            morseCode = "-.";
            break;
        case 'o': case 'O':
            morseCode = "---";
            break;
        case 'p': case 'P':
            morseCode = ".--.";
            break;
        case 'q': case 'Q':
            morseCode = "--.-";
            break;
        case 'r': case 'R':
            morseCode = ".-.";
            break;
        case 's': case 'S':
            morseCode = "...";
            break;
        case 't': case 'T':
            morseCode = "-";
            break;
        case 'u': case 'U':
            morseCode = "..-";
            break;
        case 'v': case 'V':
            morseCode = "...-";
            break;
        case 'w': case 'W':
            morseCode = ".--";
            break;
        case 'x': case 'X':
            morseCode = "-..-";
            break;
        case 'y': case 'Y':
            morseCode = "-.--";
            break;
        case 'z': case 'Z':
            morseCode = "--..";
            break;
        case '0':
            morseCode = "-----";
            break;
        case '1':
            morseCode = ".----";
            break;
        case '2':
            morseCode = "..---";
            break;
        case '3':
            morseCode = "...--";
            break;
        case '4':
            morseCode = "....-";
            break;
        case '5':
            morseCode = ".....";
            break;
        case '6':
            morseCode = "-....";
            break;
        case '7':
            morseCode = "--...";
            break;
        case '8':
            morseCode = "---..";
            break;
        case '9':
            morseCode = "----.";
            break;
        case '.':
            morseCode = ".-.-.-";
            break;
        case ',':
            morseCode = "--..--";
            break;
        case '?':
            morseCode = "..--..";
            break;
        case '!':
            morseCode = "-.-.--";
            break;
        case ':':
            morseCode = "---...";
            break;
        case ';':
            morseCode = "-.-.-.";
            break;
        case '-':
            morseCode = "-....-";
            break;
        case '_':
            morseCode = "..--.-";
            break;
        case '(':
            morseCode = "-.--.";
            break;
        case ')':
            morseCode = "-.--.-";
            break;
        case '\'':
            morseCode = ".----.";
            break;
        case '=':
            morseCode = "-...-";
            break;
        case '+':
            morseCode = ".-.-.";
            break;
        case '/':
            morseCode = "-..-.";
            break;
        case '@':
            morseCode = ".--.-.";
            break;
        case '"':
            morseCode = ".-..-.";
        default:
            return;
    } 

    for (int i = 0; morseCode[i] != '\0'; i++) {
        if (morseCode[i] == '.')
        {
            playTone(DOT_DURATION, (volume * 100) / 22);
        }
        else if (morseCode[i] == '-')
        {
            playTone(DASH_DURATION, (volume * 100) / 22);
        }
        playTone(SYMBOL_PAUSE, 0); // pause (silence) between symbols
    }
    playTone(LETTER_PAUSE, 0); // pause (silence) between letters
}

/**
 * play a CW character
 */
void play_cw(const char character)
{
    return play_cw(character, 20);
}

/**
 * play CW start 
 */
void play_cw_start()
{
    if (meshcom_settings.node_mute)
    {
        if (bDEBUG)
        {
            Serial.println("[audio]...muted");
        }
        return;
    }

    const char *morseCode = "-.-.-";

    for (int i = 0; morseCode[i] != '\0'; i++) {
        if (morseCode[i] == '.')
        {
            playTone(DOT_DURATION, (15 * 100) / 22);
        }
        else if (morseCode[i] == '-')
        {
            playTone(DASH_DURATION, (15 * 100) / 22);
        }
        playTone(SYMBOL_PAUSE, 0); // pause (silence) between symbols
    }
    playTone(LETTER_PAUSE, 0); // pause (silence) between letters
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
