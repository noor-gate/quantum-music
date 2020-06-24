#include "wfc.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define STACK_END -1

Wave *init_wave(size_t size, size_t superposition_size) {
  Wave *wave = (Wave *)calloc(1, sizeof(Wave));
  wave->size = size;
  wave->superposition_size = superposition_size;
  wave->superpositions = (bool **)calloc(size, sizeof(bool *));
  for (int i = 0; i < size; i++) {
    wave->superpositions[i] = (bool *)calloc(superposition_size, sizeof(bool));
    for (int j = 0; j < superposition_size; j++) {
      wave->superpositions[i][j] = true;
    }
  }
  return wave;
}

void free_wave(Wave *wave) {
  for (int i = 0; i < wave->size; i++) {
    free(wave->superpositions[i]);
  }
  free(wave->superpositions);
  free(wave);
}

int rand_gen(int limit) {
  int divisor = RAND_MAX / (limit + 1);
  int retval;
  do {
    retval = rand() / divisor;
  } while (retval > limit);
  return retval;
}

double entropy(bool *superposition, Patterns *patterns) {
  double total_frequency = 0;
  for (int i = 0; i < patterns->size; i++) {
    if (superposition[i]) {
      total_frequency += (double)patterns->values[i].frequency;
    }
  }
  if (total_frequency == 0) {
    return -1;
  }
  double sum = 0;
  for (int i = 0; i < patterns->size; i++) {
    if (superposition[i]) {
      double chance = (double)patterns->values[i].frequency / total_frequency;
      sum += chance * log2(1 / chance);
    }
  }
  return sum;
}

bool get_lowest_entropy(Wave *wave, Patterns *patterns, int *result) {
  int lowest_entropy_superposition = 0;
  double lowest_entropy = entropy(wave->superpositions[0], patterns);
  for (int i = 0; i < wave->size; i++) {
    double current_entropy = entropy(wave->superpositions[i], patterns);
    if (current_entropy == -1) {
      return true;
    }
    if (current_entropy == 0) {
      continue;
    }
    if ((current_entropy == lowest_entropy && rand_gen(2)) ||
        current_entropy < lowest_entropy) {
      lowest_entropy_superposition = i;
      lowest_entropy = current_entropy;
    }
  }
  *result = lowest_entropy_superposition;
  return false;
}

// returns true if the elements are equal only
bool pattern_elements_equal(void **elements1, void **elements2,
                            size_t pattern_size, Equality_Func are_equal) {
  for (int i = 0; i < pattern_size; i++) {
    if (!are_equal(elements1[i], elements2[i])) {
      return false;
    }
  }
  return true;
}

void **get_elements(void **input, size_t position, size_t count) {
  void **elements = (void **)calloc(count, sizeof(void *));
  for (int i = 0; i < count; i++) {
    elements[i] = input[position + i];
  }
  return elements;
}

int find_pattern(void **elements, Patterns *patterns, Equality_Func are_equal) {
  for (int i = 0; i < patterns->size; i++) {
    if (pattern_elements_equal(elements, patterns->values[i].elements,
                               patterns->values[i].size, are_equal)) {
      return i;
    }
  }
  return -1;
}

Patterns *get_patterns(void **input, size_t input_size, size_t pattern_size,
                       Equality_Func are_equal) {
  Patterns *patterns = (Patterns *)calloc(1, sizeof(Patterns));
  patterns->size = 0;
  patterns->values = (Pattern *)calloc(patterns->size, sizeof(Pattern));
  for (int i = 0; i < input_size - (pattern_size - 1); i++) {
    void **elements = get_elements(input, i, pattern_size);
    int repeat = find_pattern(elements, patterns, are_equal);
    if (repeat != -1) {
      free(elements);
      patterns->values[repeat].frequency++;
      continue;
    }
    patterns->size++;
    patterns->values =
        (Pattern *)realloc(patterns->values, patterns->size * sizeof(Pattern));
    patterns->values[patterns->size - 1].size = pattern_size;
    patterns->values[patterns->size - 1].frequency = 1;
    patterns->values[patterns->size - 1].elements = elements;
  }
  return patterns;
}

void free_patterns(Patterns *patterns) {
  for (int i = 0; i < patterns->size; i++) {
    free(patterns->values[i].elements);
  }
  free(patterns->values);
  free(patterns);
}

Propagator *init_propagator(size_t patterns_size, size_t pattern_size) {
  Propagator *propagator = (Propagator *)calloc(1, sizeof(Propagator));
  propagator->patterns_size = patterns_size;
  propagator->pattern_size = pattern_size;
  propagator->overlap_tensor = (bool ***)calloc(patterns_size, sizeof(bool *));
  for (int i = 0; i < patterns_size; i++) {
    propagator->overlap_tensor[i] =
        (bool **)calloc(patterns_size, sizeof(bool *));
    for (int j = 0; j < patterns_size; j++) {
      propagator->overlap_tensor[i][j] =
          (bool *)calloc(2 * pattern_size - 1, sizeof(bool));
    }
  }
  return propagator;
}

bool legal_offset(Pattern known_pattern, Pattern contender_pattern, int offset,
                  Equality_Func are_equal) {
  for (int i = 0; i < contender_pattern.size; i++) {
    if (i - offset < 0 || contender_pattern.size <= i - offset) {
      continue;
    }
    if (!are_equal(known_pattern.elements[i],
                   contender_pattern.elements[i - offset])) {
      return false;
    }
  }
  return true;
}

Propagator *build_propagator(Patterns *patterns, size_t pattern_size,
                             Equality_Func are_equal) {
  Propagator *propagator = init_propagator(patterns->size, pattern_size);
  for (int i = 0; i < patterns->size; i++) {
    Pattern pattern1 = patterns->values[i];
    for (int j = 0; j < patterns->size; j++) {
      Pattern pattern2 = patterns->values[j];
      for (int k = 0; k < 2 * pattern_size - 1; k++) {
        propagator->overlap_tensor[i][j][k] =
            legal_offset(pattern1, pattern2, k - (pattern_size - 1), are_equal);
      }
    }
  }
  return propagator;
}

void free_propagator(Propagator *propagator) {
  for (int i = 0; i < propagator->patterns_size; i++) {
    for (int j = 0; j < propagator->patterns_size; j++) {
      free(propagator->overlap_tensor[i][j]);
    }
    free(propagator->overlap_tensor[i]);
  }
  free(propagator->overlap_tensor);
  free(propagator);
}

bool can_overlap(Propagator *propagator, int known_pattern,
                 int contender_pattern, int offset) {
  int index = offset + propagator->pattern_size - 1;
  bool out_of_bounds = index < 0 || 2 * propagator->pattern_size - 1 <= index;
  return !out_of_bounds &&
         propagator->overlap_tensor[known_pattern][contender_pattern][index];
}

bool observe(Wave *wave, Patterns *patterns, int *result) {
  bool err = get_lowest_entropy(wave, patterns, result);
  if (err) {
    return true;
  }
  bool *superposition = wave->superpositions[*result];
  int *indices = (int *)calloc(wave->superposition_size, sizeof(int));
  int index_count = 0;
  int total_frequency = 0;
  for (int i = 0; i < wave->superposition_size; i++) {
    if (superposition[i]) {
      indices[index_count] = i;
      index_count++;
      total_frequency += patterns->values[i].frequency;
    }
  }
  int rand_num = rand_gen(total_frequency - 1);
  bool collapsed = false;
  for (int i = 0; i < index_count; i++) {
    if (!collapsed) {
      rand_num -= patterns->values[indices[i]].frequency;
      if (rand_num <= 0) {
        collapsed = true;
        continue;
      }
    }
    superposition[indices[i]] = false;
  }
  free(indices);
  return false;
}

Stack *init_stack() {
  Stack *stack = (Stack *)calloc(1, sizeof(Stack));
  stack->index = 0;
  stack->values = (int *)calloc(1, sizeof(int));
  return stack;
}

void free_stack(Stack *stack) {
  free(stack->values);
  free(stack);
}

bool contains(Stack *stack, int value) {
  for (int i = 0; i <= stack->index; i++) {
    if (stack->values[i] == value) {
      return true;
    }
  }
  return false;
}

void push(Stack *stack, int value) {
  stack->index++;
  if (stack->size <= stack->index) {
    stack->size = stack->index + 1;
    stack->values = (int *)realloc(stack->values, stack->size * sizeof(int));
  }
  stack->values[stack->index] = value;
}

int pop(Stack *stack) {
  if (stack->index < 0) {
    return -1;
  }
  stack->index--;
  return stack->values[stack->index + 1];
}

bool collapsed(bool *superposition, size_t size) {
  int count = 0;
  for (int i = 0; i < size; i++) {
    count += (int)superposition[i];
  }
  return count == 1;
}

void propagate(Propagator *propagator, Wave *wave, int updated_element) {
  Stack *stack = init_stack();
  push(stack, updated_element);
  int pattern_size = propagator->pattern_size;
  for (int index = pop(stack); index != STACK_END; index = pop(stack)) {
    bool *superposition = wave->superpositions[index];
    for (int offset = -(pattern_size - 1); offset < pattern_size; offset++) {
      if (index + offset < 0 || wave->size <= index + offset || offset == 0) {
        continue;
      }
      bool *other = wave->superpositions[index + offset];
      if (collapsed(other, wave->superposition_size)) {
        continue;
      }
      bool has_changed = false;
      for (int i = 0; i < wave->superposition_size; i++) {
        if (!other[i]) {
          continue;
        }
        bool illegal = true; // assume it's illegal until proven otherwise
        for (int j = 0; j < wave->superposition_size; j++) {
          if (!superposition[j]) {
            continue;
          }
          if (can_overlap(propagator, j, i, offset)) {
            illegal = false;
          }
        }
        if (illegal) {
          other[i] = false;
          has_changed = true;
        }
      }
      if (has_changed && !contains(stack, index + offset)) {
        push(stack, index + offset);
      }
    }
  }
  free_stack(stack);
}

void **wave_to_output(Wave *wave, Patterns *patterns) {
  void **output = (void **)calloc(wave->size, sizeof(void *));
  for (int i = 0; i < wave->size; i++) {
    int index = 0;
    for (int j = 0; j < wave->superposition_size; j++) {
      if (wave->superpositions[i][j]) {
        index = j;
        break;
      }
    }
    output[i] = patterns->values[index].elements[0];
  }
  return output;
}

bool wave_collapsed(Wave *wave, bool *err) {
  bool collapsed = true;
  for (int i = 0; i < wave->size; i++) {
    int count = 0;
    for (int j = 0; j < wave->superposition_size; j++) {
      count += (int)wave->superpositions[i][j];
    }
    if (count == 0) {
      *err = true;
      return true;
    }
    if (count != 1) {
      collapsed = false;
    }
  }
  *err = false;
  return collapsed;
}

bool wfc(void **input, size_t input_size, void ***output,
         Equality_Func are_equal, int pattern_size, size_t output_size,
         unsigned int seed) {
  srand(seed);
  Patterns *patterns = get_patterns(input, input_size, pattern_size, are_equal);
  Propagator *propagator = build_propagator(patterns, pattern_size, are_equal);
  Wave *wave = init_wave(output_size, patterns->size);
  bool err = false;
  while (!wave_collapsed(wave, &err)) {
    int updated_element;
    observe(wave, patterns, &updated_element);
    propagate(propagator, wave, updated_element);
  }
  *output = wave_to_output(wave, patterns);
  free_wave(wave);
  free_propagator(propagator);
  free_patterns(patterns);
  return err;
}
