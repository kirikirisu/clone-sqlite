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

# 5. メモリで表現していたテーブルをファイルに保存する & ファイルに保存していたテーブルをメモリに移す

- Pager
  <br> この構造体からメモリとファイルにアクセスする。メモリはキャッシュに使える。ファイルに書き込む == Disk に保存。コード上では Tabel からアクセスできる。
  <br> file_descriptor でどのファイルで読み書きするかわかる。file_length でどこからファイルに書き込みをすればいいかわかる。

- open()
  <br> ファイルを一意に識別できる数値(ファイルポインタ？ファイルディスクリプタ？)を返す

- lseek(int fd, off_t offset, int whence)
  <br> http://capm-network.com/?tag=C%E8%A8%80%E8%AA%9E%E3%82%B7%E3%82%B9%E3%83%86%E3%83%A0%E3%82%B3%E3%83%BC%E3%83%AB-lseek
  <br> whence を SEEK_END にした場合、ファイルサイズに offset バイト足した位置がファイルオフセット(読み書き位置)になる。

- db_open
  ファイルを開いて、table と pager を初期化している。table の初期化では、ファイルに何個 row が入っているかわかるようにしている。pager にはファイルとやり取りするためにそのファイルの情報を格納し、メモリで row を管理するために pager を初期化している。pages の責務はこれまでと変わっていない。pager の初期化は pager_open で行なっている。

- db_close
  table で使ってたメモリの解放とメモリで管理してた row をファイルに保存している。row のファイルへの保存は pager_flush が担当。
  <br>
  最初の for 文で全て row で埋まっているページをファイルにフラッシュをしている。そして次に残りの中途半端に row が入っているページをフラッシュしている。16 個 row があった場合、14 個目までは 0 ページ目で最初の for 文で処理され、残りの２つは 1 ページ目で次からの文で処理する。

  - pager_flush
    lseek でファイルに保存するページのオフセットを確定している。三つ目の引数の SEEK_SET はシンプルに二つ目の引数のバイトをオフセットにすることを意味する。
    <br>
    最終的に write によりメモリで管理していた page をファイルに書き込む。write している時のオフセットは lseek で決まるっぽい

- get_page
  <br> キャッシュミス: メモリに必要なデータが存在しない場合、ファイルにデータを探しにいくことをさしている。
  <br> row_slot の page_num は引数で渡された row_num 番目の row が何ページ目に存在するかを示している。insert の場合、一番最後の row が何ページ目に存在するか示すことになる。
  <br> num_pages は単純にファイルに保存されているページ数を示す。num_pages より page_num が小さいか同じだった場合、pages の page_num 番目にそのページの row だけファイルから読み込む。
  <br> row をメモリに持ってきたら、新しく追加する row のオフセット(ページ番号とページ内のどこから row を追加するのか)や select する row のオフセットを計算している。

# B-tree, B+ tree??????????????????

- B-tree は、バイナリーツリーの上位互換。B-tree の B は bynary ではなくて balanced の B。挿入や削除などで木のバランスが崩れない、これによって O(log(n))を発揮する。リーフ同士の関係もあるため、範囲取得も得意

- B+tree はテーブルを保存するために B-tree を改良したもの。
  <br> &nbsp; => b-tree はノードにインデック（単一の数値）だけしか格納していない。b+ tree はノードに値の塊（テーブル）を格納できる。その工夫のために、ノードが何種類もあり少しややこしい。

  - Internal Node: 子をもつノード。
    <br> key と子ノードへのポインタをもつ。value は持たない。key はルーティングのために使う。

  - Leaf Node: 末端のノード。
    <br> key と value をもつ。key は value とペアになる。key + value のセットは key によりソートされている。

  - Internarl Node の key の両サイドにポインタがあるため、検索する値と key を比較し、大きければ右のポインタに行き、小さければ左のポインタに行く。Leaf Node までたどり着いたら key により value が見つかる。

  - order
    <br> 内部ノードが持てる子（子へのポインタ）の数。
    <br> order が m の木の場合、その木の内部ノードは最大 m-1 個の key を持てる。また、m 個の子へのポインタを持てる(m == 持つことが出来る子の数 == 子へのポインタの数)。

  - Leaf Node は収まるだけ key+value のセットを持てる。今回の場合、1 ページ分のデータ量を Leaf Node は保持できる(っていうことだと思う)。

  - 6 章まで書いてきたコードとの対応関係
    <br> 1 つの Node: 1 つの page
    <br> &nbsp; => order: 1page の大きさ？
    <br> root node: 0 page 目
    <br> Internal Node の子へのポインター: page 番号

# 6.

- 現在は row をメモリに順番に追加しており、かつメタデータがないため、メモリのスペースはそれほど必要としない。しかし、特定の行を検索する場合はテーブル全体をスキャンする必要がある。また、削除する場合も、例えば page 配列の真ん中の row を消した時にその消した地点から後半全ての row を消したことによってあいたスペースに寄せる必要がある。

- ツリー構造にすることでメモリ上で各 row が連続していなくてもポインタにより繋がっている。削除や挿入の時に row のメモリ上での連続性を意識しなくてよくなる。一方で、row にデータだけでなくポインタなどのメタデータを付与する必要があるため、メモリのスペースを必要とする。

- 各ノードは 1page。そして、そのノードにはメタデータとデータが保存されていく。1 ノード単位ではメモリ上でデータが連続しているため、どっからどこまでがなんのデータなのかわかるようにしている。データはセルの配列で格納される。セルは key と row のセット。

- leaf_node_num_cells の挙動が謎。ノードにセルが何個あるかを示しているのは確定。汎用ポインタに定数を足しているだけでセルの個数が出てくる。nazo
