#include "wfc.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define THRESHOLD 0.00001

bool doubles_equal(double x, double y) { return fabs(x - y) < THRESHOLD; }

void test_entropy(void) {
  Pattern patterns_value[5] = {
      {0, 3, NULL}, {0, 5, NULL}, {0, 1, NULL}, {0, 2, NULL}, {0, 5, NULL},
  };

  Patterns patterns = {5, patterns_value};
  bool *superposition;

  superposition = (bool[]){true, true, true, true, true};
  assert(doubles_equal(entropy(superposition, &patterns), 2.126614));
  superposition = (bool[]){false, false, false, false, false};
  assert(doubles_equal(entropy(superposition, &patterns), -1));
  superposition = (bool[]){true, false, false, false, false};
  assert(doubles_equal(entropy(superposition, &patterns), 0));
  superposition = (bool[]){true, false, true, false, true};
  assert(doubles_equal(entropy(superposition, &patterns), 1.351644));
}

void test_get_lowest_entropy(void) {
  Pattern patterns_value[5] = {
      {0, 3, NULL}, {0, 5, NULL}, {0, 1, NULL}, {0, 2, NULL}, {0, 5, NULL},
  };
  Patterns patterns = {5, patterns_value};
  bool *wave1_values[4] = {(bool[]){true, true, true, true, true},
                           (bool[]){false, false, false, false, false},
                           (bool[]){true, false, false, false, false},
                           (bool[]){true, false, true, false, true}};
  Wave wave1 = {4, 5, wave1_values};
  int result = 0;
  bool err = get_lowest_entropy(&wave1, &patterns, &result);
  assert(err);
  assert(result == 0);

  bool *wave2_values[4] = {(bool[]){true, true, true, true, true},
                           (bool[]){true, true, true, true, true},
                           (bool[]){true, true, false, false, false},
                           (bool[]){true, true, true, false, true}};
  Wave wave2 = {4, 5, wave2_values};
  int result2;
  bool err2 = get_lowest_entropy(&wave2, &patterns, &result2);
  assert(!err2);
  assert(result2 == 2);

  bool *wave3_values[4] = {(bool[]){true, true, true, true, true},
                           (bool[]){true, false, false, false, false},
                           (bool[]){true, true, true, true, false},
                           (bool[]){true, true, true, true, true}};
  Wave wave3 = {4, 5, wave3_values};
  int result3;
  bool err3 = get_lowest_entropy(&wave3, &patterns, &result3);
  assert(!err3);
  assert(result3 == 2);
}

bool ints_equal(void *item1, void *item2) {
  int num1 = *((int *)item1);
  int num2 = *((int *)item2);
  return num1 == num2;
}

void assert_patterns_equal(Patterns *patterns1, Patterns *patterns2,
                           Equality_Func are_equal) {
  assert(patterns1->size == patterns2->size);
  for (int i = 0; i < patterns1->size; i++) {
    Pattern pattern1 = patterns1->values[i];
    Pattern pattern2 = patterns2->values[i];
    assert(pattern1.size == pattern2.size);
    assert(pattern1.frequency == pattern2.frequency);
    assert(pattern_elements_equal(pattern1.elements, pattern2.elements,
                                  pattern1.size, are_equal));
  }
}

void test_get_patterns(void) {
  int nums[5] = {169, 269, 369, 469, 569};

  void *input0[5] = {&nums[0], &nums[1], &nums[2], &nums[3], &nums[4]};
  void *elements00[2] = {input0[0], input0[1]};
  void *elements01[2] = {input0[1], input0[2]};
  void *elements02[2] = {input0[2], input0[3]};
  void *elements03[2] = {input0[3], input0[4]};
  Patterns expected0 = {4, (Pattern[]){{2, 1, elements00},
                                       {2, 1, elements01},
                                       {2, 1, elements02},
                                       {2, 1, elements03}}};
  Patterns *result0 = get_patterns(input0, 5, 2, ints_equal);
  assert_patterns_equal(result0, &expected0, ints_equal);
  free_patterns(result0);

  void *input1[5] = {&nums[0], &nums[1], &nums[0], &nums[1], &nums[0]};
  void *elements10[2] = {input1[0], input1[1]};
  void *elements11[2] = {input1[1], input1[2]};
  Patterns expected1 = {2, (Pattern[]){{2, 2, elements10}, {2, 2, elements11}}};
  Patterns *result1 = get_patterns(input1, 5, 2, ints_equal);
  assert_patterns_equal(result1, &expected1, ints_equal);
  free_patterns(result1);
}

void test_build_propagator(void) {
  int nums[5] = {169, 269, 369, 469, 569};

  void *nums_pointers0[5] = {&nums[0], &nums[1], &nums[2], &nums[3], &nums[4]};
  Patterns *patterns0 = get_patterns(nums_pointers0, 5, 2, ints_equal);
  bool expected_overlap_tensor0[4][4][3] = {
      {
          {0, 1, 0},
          {0, 0, 1},
          {0, 0, 0},
          {0, 0, 0},
      },
      {
          {1, 0, 0},
          {0, 1, 0},
          {0, 0, 1},
          {0, 0, 0},
      },
      {
          {0, 0, 0},
          {1, 0, 0},
          {0, 1, 0},
          {0, 0, 1},
      },
      {
          {0, 0, 0},
          {0, 0, 0},
          {1, 0, 0},
          {0, 1, 0},
      },
  };
  Propagator *result0 = build_propagator(patterns0, 2, ints_equal);
  assert(result0->patterns_size == 4);
  assert(result0->pattern_size == 2);
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      for (int k = 0; k < 3; k++) {
        assert(result0->overlap_tensor[i][j][k] ==
               expected_overlap_tensor0[i][j][k]);
      }
    }
  }
  free_patterns(patterns0);
  free_propagator(result0);

  void *nums_pointers1[5] = {&nums[0], &nums[1], &nums[0], &nums[1], &nums[0]};
  Patterns *patterns1 = get_patterns(nums_pointers1, 5, 2, ints_equal);
  bool expected_overlap_tensor1[2][2][3] = {
      {{0, 1, 0}, {1, 0, 1}},
      {{1, 0, 1}, {0, 1, 0}},
  };
  Propagator *result1 = build_propagator(patterns1, 2, ints_equal);
  assert(result1->patterns_size == 2);
  assert(result1->pattern_size == 2);
  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 2; j++) {
      for (int k = 0; k < 3; k++) {
        assert(result1->overlap_tensor[i][j][k] ==
               expected_overlap_tensor1[i][j][k]);
      }
    }
  }
  free_patterns(patterns1);
  free_propagator(result1);
}

void test_can_overlap(void) {
  int nums[5] = {169, 269, 369, 469, 569};

  void *nums_pointers0[5] = {&nums[0], &nums[1], &nums[2], &nums[3], &nums[4]};
  Patterns *patterns0 = get_patterns(nums_pointers0, 5, 2, ints_equal);
  Propagator *propagator0 = build_propagator(patterns0, 2, ints_equal);

  assert(can_overlap(propagator0, 0, 1, 1));
  assert(can_overlap(propagator0, 1, 2, 1));
  assert(can_overlap(propagator0, 2, 3, 1));

  assert(can_overlap(propagator0, 1, 0, -1));
  assert(can_overlap(propagator0, 2, 1, -1));
  assert(can_overlap(propagator0, 3, 2, -1));

  assert(can_overlap(propagator0, 0, 0, 0));
  assert(can_overlap(propagator0, 1, 1, 0));
  assert(can_overlap(propagator0, 2, 2, 0));
  assert(can_overlap(propagator0, 3, 3, 0));

  assert(!can_overlap(propagator0, 1, 2, 0));
  assert(!can_overlap(propagator0, 1, 2, -1));
  assert(!can_overlap(propagator0, 2, 3, 0));
  assert(!can_overlap(propagator0, 3, 3, -1));
  assert(!can_overlap(propagator0, 0, 0, -1));

  free_propagator(propagator0);
  free_patterns(patterns0);
}

double *get_entropies(Wave *wave, Patterns *patterns) {
  double *entropies = (double *)calloc(wave->size, sizeof(double));
  for (int i = 0; i < wave->size; i++) {
    entropies[i] = entropy(wave->superpositions[i], patterns);
  }
  return entropies;
}

void test_observe(void) {
  int nums[5] = {169, 269, 369, 469, 569};
  int result;
  double *before, *after;
  bool err;

  bool *wave0_values[4] = {
      (bool[]){true, true, true, true}, (bool[]){true, false, false, false},
      (bool[]){true, false, false, false}, (bool[]){true, false, true, false}};
  Wave wave0 = {4, 4, wave0_values};
  void *nums_pointers0[5] = {&nums[0], &nums[1], &nums[2], &nums[3], &nums[4]};
  Patterns *patterns0 = get_patterns(nums_pointers0, 5, 2, ints_equal);
  before = get_entropies(&wave0, patterns0);
  err = observe(&wave0, patterns0, &result);
  after = get_entropies(&wave0, patterns0);
  // assert stuff
  assert(!err);
  for (int i = 0; i < wave0.size; i++) {
    assert(doubles_equal(after[i], i == result ? 0 : before[i]));
  }
  free(after);
  free(before);
  free_patterns(patterns0);

  bool *wave1_values[4] = {
      (bool[]){true, true, true, true}, (bool[]){true, true, true, true},
      (bool[]){true, true, true, true}, (bool[]){true, true, true, true}};
  Wave wave1 = {4, 4, wave1_values};
  void *nums_pointers1[5] = {&nums[0], &nums[1], &nums[2], &nums[3], &nums[4]};
  Patterns *patterns1 = get_patterns(nums_pointers1, 5, 2, ints_equal);
  before = get_entropies(&wave1, patterns1);
  err = observe(&wave1, patterns1, &result);
  after = get_entropies(&wave1, patterns1);
  // assert stuff
  assert(!err);
  for (int i = 0; i < wave1.size; i++) {
    assert(doubles_equal(after[i], i == result ? 0 : before[i]));
  }
  free(before);
  free(after);
  free_patterns(patterns1);
}

void test_propagator() {
  int nums[5] = {169, 269, 369, 469, 569};

  void *nums_pointers0[5] = {&nums[0], &nums[1], &nums[2], &nums[3], &nums[4]};
  Patterns *patterns0 = get_patterns(nums_pointers0, 5, 2, ints_equal);
  Propagator *propagator0 = build_propagator(patterns0, 2, ints_equal);
  bool *wave0_values[4] = {
      (bool[]){true, false, false, false}, (bool[]){true, true, true, true},
      (bool[]){true, true, true, true}, (bool[]){true, true, true, true}};
  Wave wave0 = {4, 4, wave0_values};
  propagate(propagator0, &wave0, 0);
  bool *expected0_values[4] = {
      (bool[]){true, false, false, false}, (bool[]){false, true, false, false},
      (bool[]){false, false, true, false}, (bool[]){false, false, false, true}};
  Wave expected0 = {4, 4, expected0_values};
  for (int i = 0; i < expected0.size; i++) {
    for (int j = 0; j < expected0.superposition_size; j++) {
      assert(wave0.superpositions[i][j] == expected0.superpositions[i][j]);
    }
  }

  free_patterns(patterns0);
  free_propagator(propagator0);

  void *nums_pointers1[5] = {&nums[0], &nums[1], &nums[1], &nums[0], &nums[0]};
  Patterns *patterns1 = get_patterns(nums_pointers1, 5, 2, ints_equal);
  Propagator *propagator1 = build_propagator(patterns1, 2, ints_equal);
  bool *wave1_values[4] = {
      (bool[]){true, true, true, true}, (bool[]){false, false, true, false},
      (bool[]){true, true, true, true}, (bool[]){true, true, true, true}};
  Wave wave1 = {4, 4, wave1_values};
  propagate(propagator1, &wave1, 1);
  bool *expected1_values[4] = {
      (bool[]){true, true, false, false}, (bool[]){false, false, true, false},
      (bool[]){true, false, false, true}, (bool[]){true, true, true, true}};
  Wave expected1 = {4, 4, expected1_values};
  for (int i = 0; i < expected1.size; i++) {
    for (int j = 0; j < expected1.superposition_size; j++) {
      assert(wave1.superpositions[i][j] == expected1.superpositions[i][j]);
    }
  }

  free_patterns(patterns1);
  free_propagator(propagator1);
}

void test_wave_collapsed() {
  bool err;

  bool *wave0_values[4] = {
      (bool[]){true, false, false, false}, (bool[]){true, true, true, true},
      (bool[]){true, true, true, true}, (bool[]){true, true, true, true}};
  Wave wave0 = {4, 4, wave0_values};
  assert(!wave_collapsed(&wave0, &err));
  assert(!err);

  bool *wave1_values[4] = {
      (bool[]){false, false, false, false}, (bool[]){true, true, true, true},
      (bool[]){true, true, true, true}, (bool[]){true, true, true, true}};
  Wave wave1 = {4, 4, wave1_values};
  assert(wave_collapsed(&wave1, &err));
  assert(err);

  bool *wave2_values[4] = {
      (bool[]){true, false, false, false}, (bool[]){false, true, false, false},
      (bool[]){false, false, true, false}, (bool[]){false, false, false, true}};
  Wave wave2 = {4, 4, wave2_values};
  assert(wave_collapsed(&wave2, &err));
  assert(!err);
}

void test_wfc() {
  int nums[5] = {169, 269, 369, 469, 569};

  void *nums_pointers0[11] = {
      &nums[0], &nums[1], &nums[2], &nums[1], &nums[2], &nums[3],
      &nums[2], &nums[0], &nums[1], &nums[1], &nums[0],
  };
  void **output0;
  bool err = wfc(nums_pointers0, 11, &output0, ints_equal, 2, 20, 2);
  if (!err) {
    for (int i = 0; i < 20; i++) {
      printf("%d\n", *(int *)output0[i]);
    }
  }

  free(output0);
}

int main(void) {
  test_entropy();
  test_get_lowest_entropy();
  test_get_patterns();
  test_build_propagator();
  test_can_overlap();
  test_observe();
  test_propagator();
  test_wave_collapsed();
  test_wfc();
  return 0;
}
