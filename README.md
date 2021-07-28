# 参考・出典

https://cstack.github.io/db_tutorial/

# 1. getline()

- getline(char s[], int lim)
  1 行分の文字を読み取り記憶する。
  入力された文字は配列 s に格納される
  記憶する文字数は lim する
- ssize_t
  変数に入る値が負の値になる場合がある時に使う

# 2. prepare_statement & excute_statement

- prepare_statement が sqlite のバイトコードを出力する SQL Command Processor を表現している
- excute_statement が sqlite のバイトコードを実行する Virtural Machine を表現している

prepare_statement でクエリ文をバイトコードにコンパイルする理由
-> バイトコードをキャッシュしてパフォーマンスを向上させるため
-> コンパイルの際に構文チェックもして、VM で構文エラーを心配しなくていいようにするため
