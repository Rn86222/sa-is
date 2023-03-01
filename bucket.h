#ifndef _BUCKET_H_
#define _BUCKET_H_

typedef struct data {
  int first;
  int idx;
  char isLMS;
} Data;

typedef struct bucket {
  int f_idx; // 上からinsertするときの場所
  int b_idx; // 下からinsertするときの場所
  int size;
  Data* data;
} Bucket;

typedef struct buckettable {
  Bucket* buckets;
  char *bucket_exists;
  int bucket_num;
  int* string;
} BucketTable;

void Bucket_init(Bucket* p_self, const int name, const int size, int* data);
void Bucket_f_insert(Bucket* p_self, const int index, const int isLMS, const int first);
void Bucket_b_insert(Bucket* p_self, const int index, const int isLMS, const int first);
void Bucket_destroy(Bucket* p_self);

void BucketTable_init(BucketTable* p_self, const int size, const int bucket_num, int* data);
void BucketTable_init_with_counts(BucketTable* p_self, const int* counts, const int bucket_num, int* data);
void BucketTable_f_insert(BucketTable* p_self, const int index, const int isLMS, const int first);
void BucketTable_b_insert(BucketTable* p_self, const int index, const int isLMS, const int first);
void BucketTable_destroy(BucketTable* p_self);

void BucketTable_print(BucketTable* p_self, const int len);
void BucketTable_print_as_suffix_array(BucketTable* p_self, const int len);


#endif // _BUCKET_H_