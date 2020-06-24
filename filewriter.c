#include "filewriter.h"

#include <stdio.h>
#include <stdlib.h>

#define TRACK_NO 11

void file_write(Track *track, char *filename, uint8_t *arr, int k) {
  uint8_t end[] = {0x0, 0xff, 0x2f, 0x0};
  track->header[TRACK_NO] = 1;

  int track_size = k + track->meta_size - 4;

  if (track_size > 0xff) {
    track->meta[6] = track_size >> 8;
  }
  track->meta[7] = track_size & 0xff;

  FILE *file = fopen(filename, "wb");
  fwrite(track->header, 1, HEADER_SIZE, file);
  fwrite(track->meta, 1, track->meta_size, file);
  fwrite(arr, 1, k, file);
  fwrite(end, 1, 4, file);
  fclose(file);
}

void write(Track *track, void **notes, char *filename) {
  uint8_t *arr = calloc(20000, sizeof(uint8_t));

  int i = 0;
  while (notes[i] != NULL) {
    i++;
  }

  int k = 0;
  for (int j = 0; j < i; j++) {
    Note *note = (Note *)notes[j];
    Note *last_note = NULL;

    if (j != 0) {
      last_note = (Note *)notes[j - 1];
    }

    if (j != 0) {
      if (last_note->length > 0x7f) {
        arr[k] = (1 << 7) | (last_note->length >> 7);
        k++;
      }
      arr[k] = last_note->length & 0x7f;
    }

    if (k == 0 && note->note == 128) {
      continue;
    }

    k++;

    if (note->note == 128) {
      arr[k] = 0x80;
      arr[k + 1] = last_note->note;
    } else {
      arr[k] = 0x90;
      arr[k + 1] = note->note;
    }
    arr[k + 2] = note->velocity;
    k += 3;
  }

  file_write(track, filename, arr, k);
  free(arr);
}
