#include <stdint.h>

#ifndef FILEREADER_H
#define FILEREADER_H

#define HEADER_SIZE 14

typedef struct {
  uint16_t note;
  uint32_t length;
  uint32_t velocity;
} Note;

typedef struct {
  uint8_t *header;
  uint8_t *lyrics;
  uint32_t size;
  uint32_t meta_size;
  uint8_t *meta;
  Note **notes;
} Track;

Track *fill_track(char *filename);
void free_track(Track *track, int note_no);

#endif
