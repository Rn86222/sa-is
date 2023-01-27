#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "backet.h"

const char S = 0;
const char L = 1;

typedef struct suffixinfo {
  int* data;
  int first;
  int last;
  char type;
  char isLMS;
} SuffixInfo;

void induced_sort(BacketTable* p_bt, const SuffixInfo* table, const int len) {
  // L-typeの位置を決定
  for (int i = 0; i < p_bt->backet_num; i++) {
    if (p_bt->backet_exists[i]) {
      Backet* p_backet = &(p_bt->backets[i]);
      for (int j = 0; j < p_backet->f_idx; j++) {
        int suffix_idx = p_backet->data[j].idx;
        if (suffix_idx > 0 && table[suffix_idx - 1].type == L)
          BacketTable_f_insert(p_bt, suffix_idx - 1, table[suffix_idx - 1].isLMS, table[suffix_idx - 1].first, table[suffix_idx - 1].last);
      }
      for (int j = p_backet->b_idx + 1; j < p_backet->size; j++) {
        int suffix_idx = p_backet->data[j].idx;
        if (suffix_idx > 0 && table[suffix_idx - 1].type == L)
          BacketTable_f_insert(p_bt, suffix_idx - 1, table[suffix_idx - 1].isLMS, table[suffix_idx - 1].first, table[suffix_idx - 1].last);
      }
    }
  }

  // S-type(LMS)を削除（実際にはb_idxをsize - 1とすればよい）
  // ただし最後のLMSは削除してはいけない
  for (int i = 0; i < p_bt->backet_num; i++) {
    if (p_bt->backet_exists[i] && p_bt->backets[i].data[p_bt->backets[i].size - 1].first != p_bt->backets[i].data[p_bt->backets[i].size - 1].last) { // 長さが1でない
      p_bt->backets[i].b_idx = p_bt->backets[i].size - 1;
      if (p_bt->backets[i].f_idx == 0)
        p_bt->backet_exists[i] = 0;
    }

  }
  // S-typeの位置を決定
  for (int i = p_bt->backet_num - 1; i >= 0; i--) {
    if (p_bt->backet_exists[i]) {
      Backet* p_backet = &(p_bt->backets[i]);
      for (int j = p_backet->size - 1; j > p_backet->b_idx; j--) {
        int suffix_idx = p_backet->data[j].idx;
        if (suffix_idx > 0 && table[suffix_idx - 1].type == S)
          BacketTable_b_insert(p_bt, suffix_idx - 1, table[suffix_idx - 1].isLMS, table[suffix_idx - 1].first, table[suffix_idx - 1].last);
      }
      for (int j = p_backet->f_idx - 1; j >= 0; j--) {
        int suffix_idx = p_backet->data[j].idx;
        if (suffix_idx > 0 && table[suffix_idx - 1].type == S)
          BacketTable_b_insert(p_bt, suffix_idx - 1, table[suffix_idx - 1].isLMS, table[suffix_idx - 1].first, table[suffix_idx - 1].last);
      }
    }
  }
}

void construct_suffix_array(BacketTable* bt, int* str, const int len, const int origin, int* counts) {
  if (len <= 1) {
    BacketTable_f_insert(bt, 0, 0, 0, len - 1);
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
    table[len - i - 1].data = str;
    table[len - i - 1].first = len - i - 1;
    table[len - i - 1].last = len - 1;
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

  BacketTable bt_LMS_substring; // LMS-substringの番号づけ用のBacket Table
  if (origin) {
    BacketTable_init_with_counts(&bt_LMS_substring, counts, 128, str);
  } else {
    BacketTable_init(&bt_LMS_substring, 1, len, str);
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
  for (int i = 0; i < len; i++) {
    if (table[i].isLMS)
      BacketTable_b_insert(&bt_LMS_substring, i, table[i].isLMS, table[i].first, table[i].last);
  }

  // (2) induced_sort()実行
  induced_sort(&bt_LMS_substring, table, len);

  // (3) induced_sort()後のLMSの順番に、対応するLMS-substringに番号をつける(0,1,2,...)
  int* substr_idx_to_LMS_idx; // substringの番号からLMSの元idを取得
  int* LMS_idx_to_substr_idx; // LMSの元idからsubstringの番号を取得
  substr_idx_to_LMS_idx = (int*)malloc(LMS_num * sizeof(int));
  LMS_idx_to_substr_idx = (int*)malloc(len * sizeof(int));
  
  int cnt = 0;
  for (int i = 0; i < bt_LMS_substring.backet_num; i++) {
    Backet* p_backet = &(bt_LMS_substring.backets[i]);
    if (bt_LMS_substring.backet_exists[i]) {
      for (int j = p_backet->b_idx + 1; j < p_backet->size; j++) {
        if (p_backet->data[j].isLMS) {
          substr_idx_to_LMS_idx[cnt] = p_backet->data[j].idx;
          LMS_idx_to_substr_idx[p_backet->data[j].idx] = cnt++;
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
  BacketTable bt_LMS_sort; // LMS-substringのsuffix array用のBacket Table
  BacketTable_init(&bt_LMS_sort, 1, LMS_num, sorted_substr_ids);
  int* tmp; // 引数として渡す用に定義しておく
  construct_suffix_array(&bt_LMS_sort, sorted_substr_ids, LMS_num, 0, tmp);

  // (6) 作成したsuffix arrayから順に対応するLMSを取り出し(LMSのソート)バケットテーブルに入れる
  // このときは取り出した順とは逆に各バケットの下から入れていく

  // まずLMSを順に取り出してsorted_LMS_idsに入れる
  int* sorted_LMS_ids;
  sorted_LMS_ids = (int*)malloc(LMS_num * sizeof(int));
  cnt = 0;
  for (int i = 0; i < bt_LMS_sort.backet_num; i++) {
    if (bt_LMS_sort.backet_exists[i]) {
      Backet* p_backet = &(bt_LMS_sort.backets[i]);
      for (int j = 0; j < p_backet->f_idx; j++) {
        sorted_LMS_ids[cnt++] = substr_idx_to_LMS_idx[p_backet->data[j].string[p_backet->data[j].first]];
      }
      for (int j = p_backet->b_idx + 1; j < p_backet->size; j++) {
        sorted_LMS_ids[cnt++] = substr_idx_to_LMS_idx[p_backet->data[j].string[p_backet->data[j].first]];
      }
    }
  }

  // 取り出した順とは逆に各バケットの下から入れる
  for (int i = LMS_num - 1; i >= 0; i--)
    BacketTable_b_insert(bt, sorted_LMS_ids[i], 1, table[sorted_LMS_ids[i]].first, table[sorted_LMS_ids[i]].last);

  // (7) induced_sort()を実行
  induced_sort(bt, table, len);

  free(substr_idx_to_LMS_idx);
  free(sorted_LMS_ids);
  free(sorted_substr_ids);
  free(LMS_ids);
  free(table);
  BacketTable_destroy(&bt_LMS_substring);
  BacketTable_destroy(&bt_LMS_sort);
}

void sais(const char str[]) {
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

  BacketTable bt_suffix_array; // suffix array(完成)用のBacket Table
  BacketTable_init_with_counts(&bt_suffix_array, counts, 128, int_str);

  construct_suffix_array(&bt_suffix_array, int_str, len, 1, counts);

  BacketTable_print_as_suffix_array(&bt_suffix_array);

  free(int_str);
  BacketTable_destroy(&bt_suffix_array);

}


int main() {
  int max_len = 300000000;
  char* str;
  clock_t start_t, end_t;

  str = (char*)malloc(max_len * sizeof(char));
  scanf("%[\x01-\x7f]", str);
  // printf("%s\n", str);
  
  start_t = clock();
  sais(str);
  end_t = clock();

  printf("time (sec): %f\n", (double)(end_t - start_t) / CLOCKS_PER_SEC);
 
  free(str);
  // sais("mmiissiissiippiifcgvhbjnkyftrfxdgvbhjnkjhujgytfdxkjhghffdghjkhgfdfghjkhgfdswedrftgyhjnmbhgvfcdxsedrftgyhujkmnbvcxsdfcgvbh");
  
  return 0;
}