#include "../wfc/wfc.h"
#include "filereader.h"
#include "filewriter.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define DEFAULT_OUTPUT_SIZE 500
#define DEFAULT_PATTERN_SIZE 3
#define DEFAULT_SEED 0
#define MAX_FAILURES 10

bool equality(void *note1, void *note2) {
  Note *note_one = (Note *)note1;
  Note *note_two = (Note *)note2;

  return (note_one->note == note_two->note) &&
         (note_one->length == note_two->length) &&
         (note_one->velocity == note_two->velocity);
}

int main(int argc, char **argv) {
  if (argc < 2 || 5 < argc) {
    printf(
        "Usage: %s <input_midi_file> [output_length] [pattern_size] [seed]\n",
        argv[0]);
    return EXIT_FAILURE;
  }

  Track *track = fill_track(argv[1]);

  int input_size = 0;
  while (track->notes[input_size] != NULL) {
    input_size++;
  }

  void **input = (void **)calloc(input_size, sizeof(Note *));

  for (int i = 0; i < input_size; i++) {
    input[i] = (void *)track->notes[i];
  }

  // parse arguments
  int output_size = DEFAULT_OUTPUT_SIZE;
  if (2 < argc) {
    output_size = (int)strtol(argv[2], NULL, 10);
  }

  int pattern_size = DEFAULT_PATTERN_SIZE;
  if (3 < argc) {
    pattern_size = (int)strtol(argv[3], NULL, 10);
  }

  unsigned int seed = DEFAULT_SEED;
  if (4 < argc) {
    seed = (unsigned int)strtol(argv[4], NULL, 10);
  }

  void **notes = (void **)calloc(output_size, sizeof(Note *));

  bool err;
  int attempts = 0;
  do {
    err = wfc(input, input_size, &notes, equality, pattern_size, output_size,
              seed);
    printf("%s\n", err ? "ERROR" : "SUCCESS");
  } while (err && attempts < MAX_FAILURES);

  write(track, notes, "out.mid");

  free(input);
  free(notes);
  free_track(track, input_size);

  return EXIT_SUCCESS;
}
