# sa-is
## 動作確認方法
以下のコマンドを実行すれば動作確認ができる.
```sh
gcc -O3 sais.c backet.c
./a.out 1 < sample.txt
```
なお上記のように、受け取る文字列は何らかのテキストファイルの内容をリダイレクトさせることを