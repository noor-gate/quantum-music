#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "filereader.h"

Note *initialise_note(void) {
  Note *note = malloc(sizeof(note));
  note->length = 0;
  return note;
}

uint32_t calculate_delta_time(uint8_t *track_arr, int *j) {
  uint32_t delta_time = 0;

  while (track_arr[*j] & 0x80) {
    delta_time |= track_arr[*j] & 0x7f;
    delta_time = delta_time << 0x7;
    *j += 1;
  }

  delta_time |= track_arr[*j];
  *j += 1;
  return delta_time;
}

int get_meta(Track *track, uint8_t *track_arr) {
  int i = 0;
  while (((track_arr[i + 1] >> 4) != 0x9) || ((track_arr[i]) >> 4) != 0) {
    i++;
  }
  memcpy(track->meta, track_arr, i);
  return i;
}

int get_events(Track *track, uint8_t *track_arr, uint32_t track_end) {

  int j = get_meta(track, track_arr);
  int note_no = 0;

  while (j < track_end) {
    // uint32_t delta_time = 0;
    // delta_time = calculate_delta_time(track_arr, &j);

    if (track_arr[j] >> 4 != 0x9 && track_arr[j] >> 4 != 0x8) {
      j++;
      continue;
    }

    Note *event = initialise_note();

    if (track_arr[j] >> 4 == 0x9) {
      event->note = track_arr[j + 1];
      event->velocity = track_arr[j + 2];
      j += 3; // go to start of next delta time

      // calculate time note stays on for
      int k = j;
      uint32_t note_time = 0;

      while (true) {
        note_time += calculate_delta_time(track_arr, &k);
        if (track_arr[k] >> 4 == 0x8 && track_arr[k + 1] == event->note) {
          break;
        }
        k += 3;
      }

      event->length = note_time;

    } else {
      // add silence note
      event->note = 128;
      event->velocity = track_arr[j + 2];
      j += 3; // go to start of next delta time;
      int k = j;
      event->length = calculate_delta_time(track_arr, &k);
    }

    track->notes[note_no] = event;
    note_no++;
  }
  return note_no;
}

uint8_t *get_track(uint8_t *data, uint32_t start, uint32_t end) {
  uint8_t *track = calloc(20000, sizeof(uint8_t));
  for (int i = start; i < end; i++) {
    track[i - start] = data[i];
  }
  return track;
}

void get_header(Track *track, uint8_t *data) {
  for (int i = 0; i < HEADER_SIZE; i++) {
    track->header[i] = data[i];
  }
}

void get_lyrics(Track *track, uint32_t lyric_index, uint8_t *data) {
  for (int i = HEADER_SIZE; i < lyric_index; i++) {
    track->lyrics[i - HEADER_SIZE] = data[i];
  }
}

int check_lyric(uint8_t *data) {
  int i = 0;
  while (data[i] != 0x0 || data[i - 1] != 0x2f || data[i - 2] != 0xff) {
    i++;
  }
  i++;
  if (data[i] == 0x4d) {
    return i;
  }
  return 0;
}

uint8_t *file_read(const char *filename) {
  FILE *file = fopen(filename, "rb");
  if (!file) {
    perror("Error opening file");
  }

  uint8_t *data = malloc(sizeof(uint8_t) * 10000);

  fread(data, sizeof(uint8_t), 20000, file);
  fclose(file);
  return data;
}

uint32_t get_start(Track *track, uint8_t *data) {
  uint32_t lyric_index = check_lyric(data);
  uint32_t track_start = HEADER_SIZE;

  if (lyric_index) {
    get_lyrics(track, lyric_index, data);
    track_start = lyric_index;
  }

  return track_start;
}

uint32_t get_end(Track *track, uint8_t *data, uint32_t track_start) {
  uint32_t track_end = 0;
  uint32_t size = 0;

  for (int i = 4; i < 8; i++) {
    uint32_t shift = data[i + track_start];
    size += shift << (8 * (7 - i));
  }

  track_end += size + 8 + track_start;
  track->size = size;
  return track_end;
}

void get_meta_size(Track *track, uint8_t *track_arr) {
  int i = 0;
  while ((track_arr[i + 1] >> 4) != 0x9 || ((track_arr[i]) >> 4) != 0) {
    i++;
  }
  track->meta_size = i;
}

Track *initialise_track() {
  Track *track = malloc(sizeof(Track));
  track->meta = malloc(200 * sizeof(uint8_t));
  track->notes = malloc(sizeof(Note *) * 1000);
  track->header = malloc(HEADER_SIZE * sizeof(uint8_t));
  track->lyrics = malloc(1000 * sizeof(uint8_t));
  return track;
}

void free_track(Track *track, int note_no) {
  for (int i = 0; i < note_no; i++) {
    free(track->notes[i]);
  }
  free(track->notes);
  free(track->meta);
  free(track->header);
  free(track->lyrics);
  free(track);
}

Track *fill_track(char *filename) {
  Track *track = initialise_track();
  uint8_t *data = file_read(filename);
  get_header(track, data);

  uint32_t track_start = get_start(track, data);
  uint32_t track_end = get_end(track, data, track_start);
  uint8_t *track_arr = get_track(data, track_start, track_end);

  get_meta_size(track, track_arr);
  get_events(track, track_arr, track_end);

  free(track_arr);
  free(data);

  return track;
}
