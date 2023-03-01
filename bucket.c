#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "bucket.h"

void Bucket_init(Bucket* p_self, const int name, const int size, int* data) {
  p_self->data = (Data*)malloc(size * sizeof(Data));
  p_self->f_idx = 0;
  p_self->b_idx = size - 1;
  p_self->size = size;
}

void Bucket_f_insert(Bucket* p_self, const int index, const int isLMS, const int first) {
  assert(p_self->f_idx <= p_self->b_idx);
  p_self->data[p_self->f_idx].isLMS = isLMS;
  p_self->data[p_self->f_idx].first = first;
  p_self->data[p_self->f_idx++].idx = index;
}

void Bucket_destroy(Bucket* p_self) {
  free(p_self->data);
}

void Bucket_b_insert(Bucket* p_self, const int index, const int isLMS, const int first) {
  if (p_self->f_idx > p_self->b_idx)
    printf("%d %d %d\n", p_self->f_idx, p_self->b_idx, index);
  assert(p_self->f_idx <= p_self->b_idx);
  p_self->data[p_self->b_idx].isLMS = isLMS;
  p_self->data[p_self->b_idx].first = first;
  p_self->data[p_self->b_idx--].idx = index;
}

void BucketTable_init(BucketTable* p_self, const int size, const int bucket_num, int* data) {
  p_self->string = data;
  p_self->buckets = (Bucket*)malloc(bucket_num * sizeof(Bucket));
  p_self->bucket_exists = (char*)malloc(bucket_num * sizeof(char));
  p_self->bucket_num = bucket_num;
  for (int i = 0; i < bucket_num; i++) {
    Bucket_init(&(p_self->buckets[i]), i, size, data);
    p_self->bucket_exists[i] = 0;
  }
}

void BucketTable_init_with_counts(BucketTable* p_self, const int* counts, const int bucket_num, int* data) {
  p_self->string = data;
  p_self->buckets = (Bucket*)malloc(bucket_num * sizeof(Bucket));
  p_self->bucket_exists = (char*)malloc(bucket_num * sizeof(char));
  p_self->bucket_num = bucket_num;
  for (int i = 0; i < bucket_num; i++) {
    Bucket_init(&(p_self->buckets[i]), i, counts[i], data);
    p_self->bucket_exists[i] = 0;
  }
}

void BucketTable_f_insert(BucketTable* p_self, const int index, const int isLMS, const int first) {
  int name = p_self->string[first];
  Bucket_f_insert(&(p_self->buckets[name]), index, isLMS, first);
  p_self->bucket_exists[name] = 1;
}

void BucketTable_b_insert(BucketTable* p_self, const int index, const int isLMS, const int first) {
  int name = p_self->string[first];
  Bucket_b_insert(&(p_self->buckets[name]), index, isLMS, first);
  p_self->bucket_exists[name] = 1;
}

void BucketTable_destroy(BucketTable* p_self) {
  for (int i = 0; i < p_self->bucket_num; i++) {
    if (p_self->bucket_exists[i]) {
      Bucket_destroy(&(p_self->buckets[i]));
    }
  }
  free(p_self->buckets);
  free(p_self->bucket_exists);
}

void BucketTable_print(BucketTable* p_self, const int len) {
  for (int i = 0; i < p_self->bucket_num; i++) {
    if (p_self->bucket_exists[i]) {
      Bucket* p_bucket = &(p_self->buckets[i]);
      for (int j = 0; j < p_bucket->f_idx; j++) {
        for (int k = p_bucket->data[j].first; k < len; k++)
          printf("%d ", p_self->string[k]);
        printf(" %d", p_bucket->data[j].isLMS);
        putchar('\n');
      }
      for (int j = p_bucket->b_idx + 1; j < p_bucket->size; j++) {
        for (int k = p_bucket->data[j].first; k < len; k++)
          printf("%d ", p_self->string[k]);
        printf(" %d", p_bucket->data[j].isLMS);
        putchar('\n');
      }
    }
  }
}

void BucketTable_print_as_suffix_array(BucketTable* p_self, const int len) {
  for (int i = 1; i < p_self->bucket_num; i++) {
    if (p_self->bucket_exists[i]) {
      Bucket* p_bucket = &(p_self->buckets[i]);
      for (int j = 0; j < p_bucket->f_idx; j++) {
        for (int k = p_bucket->data[j].first; k < len; k++)
          printf("%c", (char)p_self->string[k]);
        putchar('\n');
      }
      for (int j = p_bucket->b_idx + 1; j < p_bucket->size; j++) {
        for (int k = p_bucket->data[j].first; k < len; k++)
          printf("%c", (char)p_self->string[k]);
        putchar('\n');
      }
    }
  }
}