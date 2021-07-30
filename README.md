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

# 3. 一つの row の中身はメモリ上で連続して並べておく

- uint32_t == unsigned int
- Row のメンバ（attribute）のサイズを算出し、オフセットを決めることでシリアライズ（直列化）した時に
  どこからどこまでがどのメンバの値なのかわかるようにしている。

  - この場合、id = 4byte, username = 32byte, email 255byte になるため、
    メモリ上の配列の 0~3byte 番目は id、4~35byte 番目は username、36~290byte 番目は email が入っていることがわかるようにしている。
    <br>配列のサイズは 291byte になる

- memcpy
  - void *memcpy(void *buf1, const void \*buf2, size_t n);
    <br>buf2 の先頭から n 文字分を buf1 にコピーする。

# 4. 構造体でテーブルを表現する

- page? page size?
  <br>paging というメモリ管理の方法がある。
  <br>http://www.cs.gunma-u.ac.jp/~nakano/OS16/vm.html
  <br>https://www.computerhope.com/jargon/p/paging.htm

- void\*について
  　<br> 単にアドレスを格納する型。どんな型の変数のアドレスでも格納できる。汎用ポインタと呼ばれる。

- new_table()
  <br>page を 100 個用意している。pages の 0 番目から 99 番目まで NULL が入る。NULL にはのちに void*が入る。void*は malloc により返された値が入る。

- page の管理

  - ROWS_PER_PAGE = PAGE_SIZE / ROW_SIZE = 4096 / 291 = 14.07

    <br>1 つの page に 14 個 row を格納できる事になる。page は 100 個用意してあるため、合計 1400 個のデータをメモリ上に保存できる。コード上では、構造体 Table の pages に row を追加していく。

  - row_slot()
    <br> row_num / POWS_PER_PAGE で row を格納する pages 配列の番号を割り出す。row がすでに 14 個格納されていれば、pages 配列の 0 番目の page は埋まっていることになる。埋まっていたら新しく PAGE_SIZE 分だけメモリを確保し、あららしく確保した領域の先頭のアドレスを pages のそれぞれの page に割り当てている。

    <br> row_num % ROWS_PER_PAGE は 0 ~ 13 を繰り返す。page の中にすでに何個 row が入っているかを表す。

    <br> byte_offset は単に page の何バイト目まで row が入っているかを表す。

    <br> page は malloc により作られている。そのため、page には 新しく確保された領域の先頭アドレスだけ入っている。つまりこの関数は、確保した領域の何バイト目から row を追加するのかを計算している。

    <br> まとめると、row が page に 14 個追加されるごとに新しい page を一つ作っている。新しい page が作られるということは、新しく作られた page は今まで作られた page とメモリ上で連続していない事になる。それぞれの page の先頭アドレスは、malloc により返された値となっていて、pages 配列のそれぞれの要素に格納されていく。
