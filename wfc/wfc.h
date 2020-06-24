#ifndef WFC_H
#define WFC_H

#include <stdbool.h>
#include <stdlib.h>

typedef bool (*Equality_Func)(void *item1, void *item2);

typedef struct {
  size_t size;
  int index;
  int *values;
} Stack;

typedef struct {
  size_t size;
  int frequency;
  void **elements;
} Pattern;

typedef struct {
  size_t size;
  Pattern *values;
} Patterns;

typedef struct {
  size_t patterns_size;
  size_t pattern_size;
  bool ***overlap_tensor; // tensor = 3D "matrix" in this context
} Propagator;

typedef struct {
  size_t size;
  size_t superposition_size;
  bool **superpositions;
} Wave;

// wave

Wave *init_wave(size_t size, size_t superposition_size);

void free_wave(Wave *wave);

// entropy

int rand_gen(int limit);

double entropy(bool *superposition, Patterns *patterns);

bool get_lowest_entropy(Wave *wave, Patterns *patterns, int *result);

// patterns

bool pattern_elements_equal(void **pattern1, void **pattern2,
                            size_t pattern_size, Equality_Func are_equal);

void **get_elements(void **input, size_t position, size_t count);

int find_pattern(void **elements, Patterns *patterns, Equality_Func are_equal);

Patterns *get_patterns(void **input, size_t input_size, size_t pattern_size,
                       Equality_Func are_equal);

void free_patterns(Patterns *patterns);

// propagator

Propagator *init_propagator(size_t patterns_size, size_t pattern_size);

bool legal_offset(Pattern known_pattern, Pattern contender_pattern, int offset,
                  Equality_Func are_equal);

Propagator *build_propagator(Patterns *patterns, size_t pattern_size,
                             Equality_Func are_equal);

bool can_overlap(Propagator *propagator, int known_pattern,
                 int contender_pattern, int offset);

void free_propagator(Propagator *propagator);

// main loop functions
bool observe(Wave *wave, Patterns *patterns, int *result);

void push(Stack *stack, int value);

int pop(Stack *stack);

bool collapsed(bool *superposition, size_t size);

void propagate(Propagator *propagator, Wave *wave, int updated_element);

// returns true if was unable to generate output
void **wave_to_output(Wave *wave, Patterns *patterns);

bool wave_collapsed(Wave *wave, bool *err);

bool wfc(void **input, size_t input_size, void ***output,
         Equality_Func are_equal, int pattern_size, size_t output_size,
         unsigned int seed);

#endif
