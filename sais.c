#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "bucket.h"

#define MAX_LEN 300000000 // 文字列の最大長

const char S = 0;
const char L = 1;

typedef struct suffixinfo {
  // int first;
  char type;
  char isLMS;
} SuffixInfo;

void induced_sort(BucketTable* p_bt, const SuffixInfo* table, const int len) {
  // L-typeの位置を決定
  for (int i = 0; i < p_bt->bucket_num; i++) {
    if (p_bt->bucket_exists[i]) {
      Bucket* p_bucket = &(p_bt->buckets[i]);
      for (int j = 0; j < p_bucket->f_idx; j++) {
        int suffix_idx = p_bucket->data[j].idx;
        if (suffix_idx > 0 && table[suffix_idx - 1].type == L)
          BucketTable_f_insert(p_bt, suffix_idx - 1, table[suffix_idx - 1].isLMS, suffix_idx - 1);
      }
      for (int j = p_bucket->b_idx + 1; j < p_bucket->size; j++) {
        int suffix_idx = p_bucket->data[j].idx;
        if (suffix_idx > 0 && table[suffix_idx - 1].type == L)
          BucketTable_f_insert(p_bt, suffix_idx - 1, table[suffix_idx - 1].isLMS, suffix_idx - 1);
      }
    }
  }

  // S-type(LMS)を削除（実際にはb_idxをsize - 1とすればよい）
  // ただし最後のLMSは削除してはいけない
  for (int i = 0; i < p_bt->bucket_num; i++) {
    if (p_bt->bucket_exists[i] && p_bt->buckets[i].data->first != len - 1) { // 長さが1でない
      p_bt->buckets[i].b_idx = p_bt->buckets[i].size - 1;
      if (p_bt->buckets[i].f_idx == 0)
        p_bt->bucket_exists[i] = 0;
    }

  }
  // S-typeの位置を決定
  for (int i = p_bt->bucket_num - 1; i >= 0; i--) {
    if (p_bt->bucket_exists[i]) {
      Bucket* p_bucket = &(p_bt->buckets[i]);
      for (int j = p_bucket->size - 1; j > p_bucket->b_idx; j--) {
        int suffix_idx = p_bucket->data[j].idx;
        if (suffix_idx > 0 && table[suffix_idx - 1].type == S)
          BucketTable_b_insert(p_bt, suffix_idx - 1, table[suffix_idx - 1].isLMS, suffix_idx - 1);
      }
      for (int j = p_bucket->f_idx - 1; j >= 0; j--) {
        int suffix_idx = p_bucket->data[j].idx;
        if (suffix_idx > 0 && table[suffix_idx - 1].type == S)
          BucketTable_b_insert(p_bt, suffix_idx - 1, table[suffix_idx - 1].isLMS, suffix_idx - 1);
      }
    }
  }
}

void construct_suffix_array(BucketTable* bt, int* str, const int len, const int origin, int* counts) {
  if (len <= 1) {
    BucketTable_f_insert(bt, 0, 0, 0);
    return;
  }

  SuffixInfo* table; // 各suffixを管理するための表
  int* LMS_ids = (int*)malloc(len * sizeof(int));
  int LMS_num = 0;

  table = (SuffixInfo*)malloc(len * sizeof(SuffixInfo));
  table[0].isLMS = 0;
  table[len - 1].type = S;

  // tableの作成
  for (int i = 0; i < len; i++) {
    if (i > 0) {
      if (str[len - i - 1] < str[len - i]) {
        table[len - i - 1].type = S;
      } else if (str[len - i - 1] > str[len - i]) {
        table[len - i - 1].type = L;
      } else {
        table[len - i - 1].type = table[len - i].type;
      }
    }
    table[len - i - 1].isLMS = i < len - 1 && table[len - i - 2].type == L && table[len - i - 1].type == S;
  }
  for (int i = 1; i < len; i++) {
    table[i].isLMS = table[i - 1].type == L && table[i].type == S;
    if (table[i].isLMS) {
      LMS_ids[LMS_num++] = i;
    }
  }

  BucketTable bt_LMS_substring; // LMS-substringの番号づけ用のBucket Table
  if (origin) {
    BucketTable_init_with_counts(&bt_LMS_substring, counts, 128, str);
  } else {
    BucketTable_init(&bt_LMS_substring, 1, len, str);
  }

  /*
    (1) LMSを先頭の1文字についてバケットソート
    (2) induced_sort()実行
    (3) induced_sort()後のLMSの順番に、対応するLMS-substringに番号をつける(0,1,2,...)
    (4) この数字を文字だと思って、元の文字列に出現していた順に並べて文字列を作る(例. 2310)
    (5) この文字列に対しconstruct_suffix_array()
    (6) 作成したsuffix arrayから順に対応するLMSを取り出し(LMSのソート)バケットテーブルに入れる
    (7) induced_sort()を実行
    終わり
  */

  // (1) LMSを先頭の1文字についてバケットソート
  for (int i = 0; i < LMS_num; i++) {
    BucketTable_b_insert(&bt_LMS_substring, LMS_ids[i], 1, LMS_ids[i]);
  }

  // (2) induced_sort()実行
  induced_sort(&bt_LMS_substring, table, len);

  // (3) induced_sort()後のLMSの順番に、対応するLMS-substringに番号をつける(0,1,2,...)
  int* substr_idx_to_LMS_idx; // substringの番号からLMSの元idを取得
  int* LMS_idx_to_substr_idx; // LMSの元idからsubstringの番号を取得
  substr_idx_to_LMS_idx = (int*)malloc(LMS_num * sizeof(int));
  LMS_idx_to_substr_idx = (int*)malloc(len * sizeof(int));
  
  int cnt = 0;
  for (int i = 0; i < bt_LMS_substring.bucket_num; i++) {
    Bucket* p_bucket = &(bt_LMS_substring.buckets[i]);
    if (bt_LMS_substring.bucket_exists[i]) {
      for (int j = p_bucket->b_idx + 1; j < p_bucket->size; j++) {
        if (p_bucket->data[j].isLMS) {
          substr_idx_to_LMS_idx[cnt] = p_bucket->data[j].idx;
          LMS_idx_to_substr_idx[p_bucket->data[j].idx] = cnt++;
        }
      }
    }
  }

  // (4) この数字を文字だと思って、元の文字列に出現していた順に並べて文字列を作る(例. 2310)
  // 実際にはintの配列である(例. {2,3,1,0})
  int* sorted_substr_ids;
  sorted_substr_ids = (int*)malloc(len * sizeof(int));
  cnt = 0;
  for (int i = 0; i < LMS_num; i++) {
    sorted_substr_ids[cnt++] = LMS_idx_to_substr_idx[LMS_ids[i]];
  }
  free(LMS_idx_to_substr_idx);

  // (5) この文字列に対しconstruct_suffix_array()を再帰呼び出し
  BucketTable bt_LMS_sort; // LMS-substringのsuffix array用のBucket Table
  BucketTable_init(&bt_LMS_sort, 1, LMS_num, sorted_substr_ids);
  int* tmp; // 引数として渡す用に定義しておく
  construct_suffix_array(&bt_LMS_sort, sorted_substr_ids, LMS_num, 0, tmp);

  // (6) 作成したsuffix arrayから順に対応するLMSを取り出し(LMSのソート)バケットテーブルに入れる
  // このときは取り出した順とは逆に各バケットの下から入れていく

  // まずLMSを順に取り出してsorted_LMS_idsに入れる
  int* sorted_LMS_ids;
  sorted_LMS_ids = (int*)malloc(LMS_num * sizeof(int));
  cnt = 0;
  for (int i = 0; i < bt_LMS_sort.bucket_num; i++) {
    if (bt_LMS_sort.bucket_exists[i]) {
      Bucket* p_bucket = &(bt_LMS_sort.buckets[i]);
      for (int j = 0; j < p_bucket->f_idx; j++) {
        sorted_LMS_ids[cnt++] = substr_idx_to_LMS_idx[sorted_substr_ids[p_bucket->data[j].first]];
      }
      for (int j = p_bucket->b_idx + 1; j < p_bucket->size; j++) {
        sorted_LMS_ids[cnt++] = substr_idx_to_LMS_idx[sorted_substr_ids[p_bucket->data[j].first]];
      }
    }
  }

  // 取り出した順とは逆に各バケットの下から入れる
  for (int i = LMS_num - 1; i >= 0; i--)
    BucketTable_b_insert(bt, sorted_LMS_ids[i], 1, sorted_LMS_ids[i]);

  // (7) induced_sort()を実行
  induced_sort(bt, table, len);

  free(substr_idx_to_LMS_idx);
  free(sorted_LMS_ids);
  free(sorted_substr_ids);
  free(LMS_ids);
  free(table);

  BucketTable_destroy(&bt_LMS_substring);
  BucketTable_destroy(&bt_LMS_sort);
}

void sais(const char str[], const char display) {
  
  int len = strlen(str) + 1; // 末尾の'\0'を考慮して1足しておく;
  int counts[128]; // 各文字の出現回数を数える(各バケットのサイズを決めるのに使う)
  for (int i = 0; i < 128; i++) {
    counts[i] = 0;
  }
  int* int_str;
  int_str = (int*)malloc(len * sizeof(int));
  for (int i = 0; i < len - 1; i++) {
    int_str[i] = (int)str[i];
    counts[int_str[i]]++;
  }
  int_str[len - 1] = 0; // = (int)'\0'
  counts[0] = 1; // '\0'の出現回数は1

  BucketTable bt_suffix_array; // suffix array(完成)用のBucket Table
  BucketTable_init_with_counts(&bt_suffix_array, counts, 128, int_str);

  construct_suffix_array(&bt_suffix_array, int_str, len, 1, counts);

  if (display == '1')
    BucketTable_print_as_suffix_array(&bt_suffix_array, len - 1);

  free(int_str);
  BucketTable_destroy(&bt_suffix_array);

}


int main(int argc, const char *argv[]) {

  if (argc <= 1)
    return 0;

  clock_t start_t, end_t;

  char* str;
  str = (char*)malloc(MAX_LEN * sizeof(char));
  scanf("%[\x01-\x7f]", str);

  start_t = clock();

  sais(str, argv[1][0]);

  end_t = clock();

  printf("time (sec): %f\n", (double)(end_t - start_t) / CLOCKS_PER_SEC);
 
  free(str);

  return 0;
}