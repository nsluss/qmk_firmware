#include "process_music.h"

bool music_activated = false;
uint8_t starting_note = 0x0C;
int offset = 7;

// music sequencer
static bool music_sequence_recording = false;
static bool music_sequence_playing = false;
static float music_sequence[16] = {0};
static uint8_t music_sequence_count = 0;
static uint8_t music_sequence_position = 0;

static uint16_t music_sequence_timer = 0;
static uint16_t music_sequence_interval = 100;

bool process_music(uint16_t keycode, keyrecord_t *record) {

    if (keycode == AU_ON && record->event.pressed) {
      audio_on();
      return false;
    }

    if (keycode == AU_OFF && record->event.pressed) {
      audio_off();
      return false;
    }

    if (keycode == AU_TOG && record->event.pressed) {
        if (is_audio_on())
        {
            audio_off();
        }
        else
        {
            audio_on();
        }
      return false;
    }

    if (keycode == MU_ON && record->event.pressed) {
        music_on();
        return false;
    }

    if (keycode == MU_OFF && record->event.pressed) {
        music_off();
        return false;
    }

    if (keycode == MU_TOG && record->event.pressed) {
        if (music_activated)
        {
            music_off();
        }
        else
        {
            music_on();
        }
        return false;
    }

    if (keycode == MUV_IN && record->event.pressed) {
        voice_iterate();
        music_scale_user();
        return false;
    }

    if (keycode == MUV_DE && record->event.pressed) {
        voice_deiterate();
        music_scale_user();
        return false;
    }

    if (music_activated) {

      if (keycode == KC_LCTL && record->event.pressed) { // Start recording
        stop_all_notes();
        music_sequence_recording = true;
        music_sequence_playing = false;
        music_sequence_count = 0;
        return false;
      }

      if (keycode == KC_LALT && record->event.pressed) { // Stop recording/playing
        stop_all_notes();
        music_sequence_recording = false;
        music_sequence_playing = false;
        return false;
      }

      if (keycode == KC_LGUI && record->event.pressed) { // Start playing
        stop_all_notes();
        music_sequence_recording = false;
        music_sequence_playing = true;
        music_sequence_position = 0;
        music_sequence_timer = 0;
        return false;
      }

      if (keycode == KC_UP) {
        if (record->event.pressed)
            music_sequence_interval-=10;
        return false;
      }

      if (keycode == KC_DOWN) {
        if (record->event.pressed)
            music_sequence_interval+=10;
        return false;
      }

      float freq = ((float)220.0)*pow(2.0, -5.0)*pow(2.0,(starting_note + SCALE[record->event.key.col + offset])/12.0+(MATRIX_ROWS - record->event.key.row));
      if (record->event.pressed) {
        play_note(freq, 0xF);
        if (music_sequence_recording) {
          music_sequence[music_sequence_count] = freq;
          music_sequence_count++;
        }
      } else {
        stop_note(freq);
      }

      if (keycode < 0xFF) // ignores all normal keycodes, but lets RAISE, LOWER, etc through
        return false;
    }
  return true;
}

bool is_music_on(void) {
    return (music_activated != 0);
}

void music_toggle(void) {
    if (!music_activated) {
        music_on();
    } else {
        music_off();
    }
}

void music_on(void) {
    music_activated = 1;
    music_on_user();
}

void music_off(void) {
    music_activated = 0;
    stop_all_notes();
}


__attribute__ ((weak))
void music_on_user() {}

__attribute__ ((weak))
void audio_on_user() {}

__attribute__ ((weak))
void music_scale_user() {}

void matrix_scan_music(void) {
  if (music_sequence_playing) {
    if ((music_sequence_timer == 0) || (timer_elapsed(music_sequence_timer) > music_sequence_interval)) {
      music_sequence_timer = timer_read();
      stop_note(music_sequence[(music_sequence_position - 1 < 0)?(music_sequence_position - 1 + music_sequence_count):(music_sequence_position - 1)]);
      play_note(music_sequence[music_sequence_position], 0xF);
      music_sequence_position = (music_sequence_position + 1) % music_sequence_count;
    }
  }
}
