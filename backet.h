#ifndef _BACKET_H_
#define _BACKET_H_

typedef struct data {
  int first;
  int idx;
  char isLMS;
} Data;

typedef struct backet {
  int f_idx; // 上からinsertするときの場所
  int b_idx; // 下からinsertするときの場所
  int size;
  Data* data;
} Backet;

typedef struct backettable {
  Backet* backets;
  char *backet_exists;
  int backet_num;
  int* string;
} BacketTable;

void Backet_init(Backet* p_self, const int name, const int size, int* data);
void Backet_f_insert(Backet* p_self, const int index, const int isLMS, const int first);
void Backet_b_insert(Backet* p_self, const int index, const int isLMS, const int first);
void Backet_destroy(Backet* p_self);

void BacketTable_init(BacketTable* p_self, const int size, const int backet_num, int* data);
void BacketTable_init_with_counts(BacketTable* p_self, const int* counts, const int backet_num, int* data);
void BacketTable_f_insert(BacketTable* p_self, const int index, const int isLMS, const int first);
void BacketTable_b_insert(BacketTable* p_self, const int index, const int isLMS, const int first);
void BacketTable_destroy(BacketTable* p_self);

void BacketTable_print(BacketTable* p_self, const int len);
void BacketTable_print_as_suffix_array(BacketTable* p_self, const int len);


#endif // _BACKET_H_