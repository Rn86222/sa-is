#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "backet.h"

void Backet_init(Backet* p_self, const int name, const int size, int* data) {
  p_self->data = (Data*)malloc(size * sizeof(Data));
  p_self->f_idx = 0;
  p_self->b_idx = size - 1;
  p_self->size = size;
}

void Backet_f_insert(Backet* p_self, const int index, const int isLMS, const int first) {
  assert(p_self->f_idx <= p_self->b_idx);
  p_self->data[p_self->f_idx].isLMS = isLMS;
  p_self->data[p_self->f_idx].first = first;
  p_self->data[p_self->f_idx++].idx = index;
}

void Backet_destroy(Backet* p_self) {
  free(p_self->data);
}

void Backet_b_insert(Backet* p_self, const int index, const int isLMS, const int first) {
  if (p_self->f_idx > p_self->b_idx)
    printf("%d %d %d\n", p_self->f_idx, p_self->b_idx, index);
  assert(p_self->f_idx <= p_self->b_idx);
  p_self->data[p_self->b_idx].isLMS = isLMS;
  p_self->data[p_self->b_idx].first = first;
  p_self->data[p_self->b_idx--].idx = index;
}

void BacketTable_init(BacketTable* p_self, const int size, const int backet_num, int* data) {
  p_self->string = data;
  p_self->backets = (Backet*)malloc(backet_num * sizeof(Backet));
  p_self->backet_exists = (char*)malloc(backet_num * sizeof(char));
  p_self->backet_num = backet_num;
  for (int i = 0; i < backet_num; i++) {
    Backet_init(&(p_self->backets[i]), i, size, data);
    p_self->backet_exists[i] = 0;
  }
}

void BacketTable_init_with_counts(BacketTable* p_self, const int* counts, const int backet_num, int* data) {
  p_self->string = data;
  p_self->backets = (Backet*)malloc(backet_num * sizeof(Backet));
  p_self->backet_exists = (char*)malloc(backet_num * sizeof(char));
  p_self->backet_num = backet_num;
  for (int i = 0; i < backet_num; i++) {
    Backet_init(&(p_self->backets[i]), i, counts[i], data);
    p_self->backet_exists[i] = 0;
  }
}

void BacketTable_f_insert(BacketTable* p_self, const int index, const int isLMS, const int first) {
  int name = p_self->string[first];
  Backet_f_insert(&(p_self->backets[name]), index, isLMS, first);
  p_self->backet_exists[name] = 1;
}

void BacketTable_b_insert(BacketTable* p_self, const int index, const int isLMS, const int first) {
  int name = p_self->string[first];
  Backet_b_insert(&(p_self->backets[name]), index, isLMS, first);
  p_self->backet_exists[name] = 1;
}

void BacketTable_destroy(BacketTable* p_self) {
  for (int i = 0; i < p_self->backet_num; i++) {
    if (p_self->backet_exists[i]) {
      Backet_destroy(&(p_self->backets[i]));
    }
  }
  free(p_self->backets);
  free(p_self->backet_exists);
}

void BacketTable_print(BacketTable* p_self, const int len) {
  for (int i = 0; i < p_self->backet_num; i++) {
    if (p_self->backet_exists[i]) {
      Backet* p_backet = &(p_self->backets[i]);
      for (int j = 0; j < p_backet->f_idx; j++) {
        for (int k = p_backet->data[j].first; k < len; k++)
          printf("%d ", p_self->string[k]);
        printf(" %d", p_backet->data[j].isLMS);
        putchar('\n');
      }
      for (int j = p_backet->b_idx + 1; j < p_backet->size; j++) {
        for (int k = p_backet->data[j].first; k < len; k++)
          printf("%d ", p_self->string[k]);
        printf(" %d", p_backet->data[j].isLMS);
        putchar('\n');
      }
    }
  }
}

void BacketTable_print_as_suffix_array(BacketTable* p_self, const int len) {
  for (int i = 1; i < p_self->backet_num; i++) {
    if (p_self->backet_exists[i]) {
      Backet* p_backet = &(p_self->backets[i]);
      for (int j = 0; j < p_backet->f_idx; j++) {
        for (int k = p_backet->data[j].first; k < len; k++)
          printf("%c", (char)p_self->string[k]);
        putchar('\n');
      }
      for (int j = p_backet->b_idx + 1; j < p_backet->size; j++) {
        for (int k = p_backet->data[j].first; k < len; k++)
          printf("%c", (char)p_self->string[k]);
        putchar('\n');
      }
    }
  }
}