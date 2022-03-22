**目次**

- [はじめに](#はじめに)
  * [C++を使う場合](#cを使う場合)
  * [ビルド環境](#ビルド環境)
- [CMakeの紹介](#cmakeの紹介)
  * [TL;DR](#tldr)
  * [導入](#導入)
  * [CMakeでできること](#cmakeでできること)
  * [CMakeLists.txt](#cmakeliststxt)
- [ライブラリの作成](#ライブラリの作成)
  * [各種設定の追加](#各種設定の追加)
  * [サブフォルダにあるソースの利用](#サブフォルダにあるソースの利用)
  * [In-sourceビルドの防止](#in-sourceビルドの防止)
  * [32bit環境と64bit環境の区別](#32bit環境と64bit環境の区別)
  * [ビルドオプションの設定](#ビルドオプションの設定)
  * [インクルード検索パスの指定](#インクルード検索パスの指定)
  * [プロジェクト名をフォルダ名に](#プロジェクト名をフォルダ名に)
  * [成果物の出力先の決定](#成果物の出力先の決定)
    + [インストール先の指定](#インストール先の指定)
    + [インストール時の挙動の設定](#インストール時の挙動の設定)
    + [追加のファイルのコピー](#追加のファイルのコピー)
  * [ビルドモードの設定](#ビルドモードの設定)
  * [公開する関数の指定](#公開する関数の指定)
    + [MSVC](#msvc)
    + [GCC](#gcc)
    + [Clang](#clang)
    + [出力された公開シンボルの表示](#出力された公開シンボルの表示)
  * [Windows特有の設定](#windows特有の設定)
    + [カレントディレクトリの設定](#カレントディレクトリの設定)
    + [BOMなしUTF-8に関する修正](#bomなしutf-8に関する修正)
    + [その他](#その他)
  * [まとめ](#まとめ)
- [製作したライブラリの利用](#製作したライブラリの利用)
  * [インストール先の指定](#インストール先の指定-1)
  * [ライブラリの取り込み](#ライブラリの取り込み)
  * [ライブラリのリンク](#ライブラリのリンク)
  * [実行ファイルの配置](#実行ファイルの配置)
  * [ランタイムパスの設定](#ランタイムパスの設定)
  * [まとめ](#まとめ-1)
- [おわりに](#おわりに)

# はじめに

C言語でOSに依存せずに利用できるライブラリを作るのは思いのほか面倒なことが多いのでまとめる。
全体的にふざけた書き方になっているが、気が向いたら直すかもしれない。

本稿で製作するライブラリは`tutorial-library`ブランチに、それを利用するプログラムは`tutorial-exec`ブランチにまとめている。

## C++を使う場合
C++でも同じようなことができるが、クラスや関数テンプレートなどのC++による機能ををふんだんに利用した設計は
外部とのインターフェースで困ることが多い。C++の機能の利用ははDLL内部の実装に留め、
ライブラリの利用側に公開するインターフェースはC言語風に書くのがよい｡[^dll]

[^dll]: よい、というかそもそも公開する関数の宣言は`extern "C" { ... }`で囲ってC言語として扱わないと公開不可能である。

## ビルド環境

C言語のコードをプラットフォームに依存せずにビルドする環境として、本稿ではCMakeを用いる。
CMakeではCMakeLists.txtというテキストファイルに独自のスクリプト言語を記述し、
コンパイルするための各種設定を定義することでビルド環境を構築する。

---

# CMakeの紹介

C/C++言語のプログラムをコンパイルする方法はたくさんある。
プログラミングの授業で登場するのはgccだし、
Visual StudioでC++プロジェクトをビルドするのも1つの手だ。
Windowsに限らずいろいろなOSで動作するプログラムを作ろうと思ったら、
OSごとにビルド用の環境は微妙に異なるので開発環境ごと対応が必要である。例えば……

* WindowsのVisual Studioでビルドできるように、ソリューションファイルを作る
* macOSのClangでコンパイルできるようにMakefileスクリプトを整える
* Linuxのgccでコンパイルできるようにコマンドラインパラメータを設定する

知っての通り（？）、C言語のコンパイラは代表的なものだけでもgcc, clang, msvc
(Visual Studioでコンパイルする時のコンパイラ) などたくさんある。
マイコン向けにカスタマイズされたgccなどというものもあれば、
Windows、macOS、Linuxでそれぞれ微妙に挙動が違うなどということもよくある話である。
そういった各種環境それぞれに対してビルド設定を用意するなど面倒くさすぎてありえないので、
ツールを使って自動的にビルド環境を構築しようというのがCMakeを使うモチベーションとなるのである。

注意してほしいのは、CMakeはビルドするための環境を生成するツールであってビルドそのものを行うわけではないことである｡[^cmake]
CMakeを実行すると、WindowsならVisual Studioのソリューションファイルが、LinuxならMakefileのスクリプトができる｡[^xcode]

[^cmake]: 生成したビルド環境を使ってCMakeからビルドを行うためのコマンドはある。
[^xcode]: Xcode？というmacOSで使える統合開発環境があるらしい。CMakeはそれにも対応しているらしい……が、使ったことがないのでさっぱり分からない。

## TL;DR

* CMakeはCMakeLists.txtという名前のテキストファイルに書かれたスクリプトを読んでビルド用のファイルを生成するツール
  * CMakeLists.txtに「どのソースファイルを」「何とリンクして」ビルドする、などという設定を書く
  * CMakeLists.txtに書くべき内容は分かりやすい解説記事がいくつもあるのでそちらを参照のこと
* ビルドの流れ
  * プログラムの置いてあるディレクトリにCMakeLists.txtを作る
  * ターミナルを開いて、そのディレクトリに移動してから`cmake -B build`を実行する
  * ビルド環境（Visual Studioのソリューションファイル群など）が出来上がる
  * 今度は`cmake --build build`とコマンドライン引数を変えてもう一度cmakeしてビルドする

## 導入

正直いってググればいくらでも出てくるので各自で調べてほしいが、
それでは面倒くさがりの諸君は導入そのものを諦めてしまいかねないので最低限の手順を書いておく。
なお実際に試して確認などはしていないので保証はしない。

* **Windows**
  * https://cmake.org/download/ から、Binary distributionsの項へ行き、`cmake-X.XX.X-windows-x86_64.msi`を取ってくる。
  * インストーラを実行し、指示に従う。
  * インストールオプションで、Add CMake to system PATH for ...を選ぶ。
  * all usersかcurrent userかはどちらでも良いと思う。複数人でアカウント分けて同じPCを使うことなんて今時ないでしょ（適当）
* **macOS**
  * ターミナルを開いて、`brew install cmake` らしい
  * ↑ と同じリンクからmacOS用のインストーラを取ってきて入れるのも良さそうだ
* **Linux**
  * Ubuntuならターミナルを開いて、`sudo apt install cmake`  
  * Linux使う人は大体自分でググってなんとかできる人なのでここに書かなくても大丈夫でしょ（適当）

CMakeは基本的にはターミナルからコマンドを打って動かすプログラムであるが、CMake GUIというGUIで動かせるものも用意されている。
私はCMake GUIを使わないのでターミナルの操作に拒絶反応を示す人は各自で調べてもらいたい。

導入がうまくいっていれば、`cmake --version`でCMakeが起動し、バージョン情報が表示されるはずである。

```
> cmake --version
cmake version 3.21.3

CMake suite maintained and supported by Kitware (kitware.com/cmake).
```

## CMakeでできること

CMakeプロジェクトをビルドする手順は以下に示す4段階に分けられる。

1. Configure … CMakeスクリプトを読み込み、ビルドのための設定を構成する
    - コンパイルするソースファイルの列挙、インクルード検索パスの設定、コンパイルオプションの設定など
    - ついでに、依存するライブラリを正しく見つけられるようにスクリプトを書くこともできる
    - `cmake -B build`で行う
1. Generate … Configureで設定した内容を元にビルド環境を生成する
    - `make`するためのMakefileや、Visual Studioのソリューションなどが生成される
    - `cmake -B build`で1.からここまで行われる
1. Build … 生成されたビルド環境でプロジェクトをビルドする
    - `cmake --build build`で行われる
1. Install … ビルドしてできたバイナリファイル等、最終的に使うものを所定の位置に配置する
    - `cmake --install build`で行われる

各ステップで、よく使うオプションがあるので列挙する。

1. Configure ～ Generate
    - `-D` … 構成時のオプションを変更する。`-D 設定項目名=値`と書く。
1. Build
    - `--config` … Visual Studioなど複数のビルド構成をサポートする環境で、
      どの構成でビルドするかを決める。Debugビルドなら`--config Debug`と書く。
1. Install
    - `--config` … Buildのそれと同じ。
    - `--prefix` … インストール先を指定する。`C:\yourlib`に配置する場合は`--prefix "C:\yourlib"`と書く。

CMakeを理解する上で重要な概念がいくつかある。もっと良い解説が参考リンクにあるので読むことを勧める。

- プロジェクト … ソースコードをひとまとめにした処理単位。
  CMakeしろ！と言うとある1つのプロジェクトに対してCMakeされる。
  - ターゲット … 1つのプロジェクトの中で処理されるタスクの単位。
    基本的には1ターゲット = 1バイナリファイル という理解でよい。
    - プロパティ … ターゲットに付けられる詳細設定。
      リンク時にあれをやる、みたいなことを指定できたりする。

> 参考: [CMake入門-基本概念と主な関数 - Qiita](https://qiita.com/sakaeda11/items/fc95f62b68a14ab861dc)

## CMakeLists.txt

CMakeでビルドできるようなプロジェクトを作るには、CMakeLists.txtというスクリプトファイルを置く。
これには最低限以下のような内容を記述する。

```CMake
# # 以降はコメントとなる。ここに書く内容はCMake独自の手続き言語のようなものである。
cmake_minimum_required(VERSION 3.18) # 要求するCMakeの最低バージョン
project(hello_world)                 # プロジェクト名の指定
add_executable(main_app main.cpp)    # 実行ファイルの名前とソースファイルのリスト
```

`cmake_minimum_required`はこのスクリプトを実行するのに必要なCMakeの最低バージョンを示すものである。
2022年3月現在の最新版は3.23だが、よほどのことがなければもう少し古いバージョンでよい。
1つの目安として、Ubuntu 20.04 LTS版のパッケージマネージャから導入できるCMakeはバージョン3.16なので、
特に理由がなければ3.16にするのがよいと思われる。

CMakeLists.txtに書くべき内容の基礎事項については参考リンクに示した記事が非常に分かりやすいのでそちらを参照することを勧める。

---

# ライブラリの作成

ライブラリの作成例として、以下のようなソースコードを考える。  
ライブラリの名前は`yourlib`で、これをリンクすると`yourlib_test_print`、
`yourlib_add`が使える……といういかにもテスト用のソースである。
また、公開したくない関数として`internal_add`を用意している。

<details><summary>source.c (クリックで展開)</summary>

```C
#include "yourlib.h"
#include <stdio.h>

int internal_add(int x, int y) {
    printf("    internal_add(%d, %d)\n", x, y);
    return x + y;
}
int yourlib_add(int x, int y) {
    printf("yourlib_add(%d, %d)\n", x, y);
    return internal_add(x, y);
}
void yourlib_test_print(void) {
    printf("printf called from YourLib\n");
}
```
</details>

<details><summary>yourlib.h (クリックで展開)</summary>

```C
#ifndef YOURLIB_INCLUDED
#define YOURLIB_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

void yourlib_test_print(void);
int yourlib_add(int x, int y);

#ifdef __cplusplus
}
#endif

#endif
```
</details>

`yourlib.h`は外部に公開されることを想定している。
ここで注意することは以下の2点であろう。

- ファイル全体を**インクルードガード**で囲む
- ファイル全体を**リンケージ指定子**で囲む

**インクルードガード**とは、ヘッダファイル先頭の`#ifndef YOURLIB_INCLUDED` ～ `#endif`のことである。
同じソースファイル内[^src]で2回以上このヘッダファイルをインクルードしないようにして、
ヘッダ内にあるものを二重に宣言してしまうことを防止する効果がある。
 
[^src]: 正確には、同じ**翻訳単位**で、である。まあ要するに同じソースファイルで、ということなのだが。

**リンケージ指定子**とは、その次の`extern "C" { ... }`のことである。
このライブラリをC++から利用する場合に、ライブラリの公開関数をC言語として扱うために必要である。
これを忘れると謎のリンカエラーを発してコンパイルできない。
ちなみに、C言語から利用する場合にこれを書くと今度はそんな構文はないと怒られる。
仕方がないのでC++でコンパイル時に定義されるマクロ`__cplusplus`を用いて分岐し、
C言語で利用する場合はなかったことにするとうまくいく。

これをコンパイルするための最低限のCMakeスクリプトは次に示す通りである。
なお、ファイル構成は以下の通りとする。

- yourlib
  - source.c
  - yourlib.h
  - CMakeLists.txt

```CMake
cmake_minimum_required(VERSION 3.16)
project(YourLib)
add_library(yourlib SHARED source.c)
```

このスクリプトが意味することは、

1. 「バージョン3.16以上のCMakeで」
1. 「Visual Studioなどのプロジェクト名を`YourLib`に設定し」
1. 「共有ライブラリ`yourlib`を、source.cから作る」

である。

## 各種設定の追加

ここまでは、`gcc -o yourlib.dll -shared -fPIC source.c`とやるのとそう変わらない。
大きなプロジェクトで利用するには、もっとCMakeで提供される便利な機能を利用するのがよい。  
以下では、次のようなファイル構成を考える。

<details><summary>yourlib (ルートフォルダ、クリックで展開)</summary>

  - CMakeLists.txt
  - bin
    - (ここに生成されたライブラリが置かれる)
  - build
    - (ここにCMakeで生成されるプロジェクトファイル群が置かれる)
  - cmake
    - (CMakeの実行時に使うファイルを置く)
    - cfg
        - file1.csv
        - file2.csv
    - misc
        - misc.txt
    - sample.txt
    - yourlib.def
    - yourlib.exp
    - yourlib.map
  - src
    - CMakeLists.txt
    - source1.c
    - source2.c
    - source1.h
    - source2.h
  - include
    - yourlib.h
</details>

実装に使うソースファイルをsrcフォルダにまとめ、公開するヘッダファイルはincludeフォルダに置いている。
srcフォルダの中にCMakeLists.txtを置くとそこにソースファイルに関係する処理をまとめることができるので置いておく。

さて、CMakeでやりたいこととそれを実現するスクリプトを列挙しよう。

## サブフォルダにあるソースの利用

プロジェクトのルートフォルダにあるCMakeLists.txtにはプロジェクト全体の設定を書き、
srcフォルダにあるCMakeLists.txtにそのソースに関連する内容を記述するとスクリプトを分割できる。

yourlib/CMakeLists.txtにこれを書くと、yourlib/src/CMakeLists.txtが呼び出される

```CMake
add_subdirectory(src)
```

## In-sourceビルドの防止

CMakeには、生成物をプロジェクトルートに直接**だばぁ**っと吐き出すIn-sourceビルドと、
生成物をまとめるディレクトリを作ってその中に吐き出すOut-of-sourceビルドがある。
ビルドコマンドを打つ時、`cmake -B build`とするべきところをうっかり`cmake`としてしまうとIn-sourceビルドとなり非常に後片付けが面倒くさい。
そこで、In-sourceビルドをエラーとすることでこれを防ぐ。  

> 参考: [C/C++プロジェクトをCMakeでビルドする - Qiita](https://qiita.com/Hiroya_W/items/049bfb4c6ad3dfe6ff0c)

```CMake
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR
        "In-source builds are not allowed. "
        "Run 'cmake -B build' instead.")
endif()
```

## 32bit環境と64bit環境の区別

今時のPCは大体64bit OSだが、1つ例外がある。それは、
Windowsの（MinGW-w64ではなく）MinGWでビルドすると64bit環境でもなぜか32bit用バイナリが生成されることだ。
勘弁してくれ。というわけでうっかり32bitのバイナリを生成しないように、警告文を加える。

この設定は共有ライブラリを作成する時に極めて重要となる。
ライブラリをうっかり32bitでコンパイルしてしまっているのに気づかずにそれを利用するプログラムを64bitでコンパイルすると、
なぜかコンパイルは通るのにライブラリの読み込みができないという現象に遭遇する。
コンパイルしてから生成物が32bit用なのか64bit用なのかを判断するのは面倒なので、
予めどちらでコンパイルしているのかを明確にすることでそのようなミスを防ぐ。

```CMake
if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    message(WARNING
        "********* 32bit build warning *********\n"
        "It seems you are trying to build on a 32bit platform.")
endif()
```

> 参考: [visual studio - How to detect if 64 bit MSVC with cmake? - Stack Overflow][cmake64]

## ビルドオプションの設定

本稿では動的ライブラリを作ることを想定しているが、同じライブラリを静的にリンクしたいという需要がある。
そこで、CMakeの実行時に静的にリンクできるようにオプションを設けることで、
同じプロジェクトで動的ライブラリも静的ライブラリも生成できるようにする。

オプションを設定するには`option()`という構文を使用する｡[^opt]
引数は左から順に「オプションが格納される変数名」「説明文」「デフォルト値」である。
 
[^opt]: このような文章を[進次郎構文](https://dic.nicovideo.jp/a/進次郎構文)という。

```CMake
option(YOURLIB_BUILD_SHARED "Build yourlib as a shared library" ON)
```

オプションを指定してビルド環境を生成する時は、`cmake -B build -D YOURLIB_BUILD_SHARED=OFF`のように`-D`オプションを用いる。
今回はオプションの指定により動的・静的を切り替えたいので、それをスクリプトに表す。

```CMake
if (YOURLIB_BUILD_SHARED)
    add_library(${TARGET_NAME} SHARED)
else()
    add_library(${TARGET_NAME} STATIC)
endif()
```

## インクルード検索パスの指定

ライブラリのコンパイル時にインクルードされるヘッダファイルのための検索パスを指定する。

```CMake
target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
```

第2引数の`PRIVATE`は、「このインクルード検索パスの設定を誰が使うか」を示すものである。
誰が、と言われても今ビルドしようとしてるライブラリ以外に使うやつなんていないだろうと思われるが、
実は**このライブラリをリンクする他のCMakeプロジェクト**も利用できるように設定することができる。

この仕様はライブラリの公開用ヘッダファイルを設定する時に威力を発揮する。

```CMake
target_include_directories(${TARGET_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include)
```

第2引数に`PUBLIC`を指定すると、「ライブラリ自身も使うし、ライブラリを利用するプログラムも使う」という意味になる。
このように設定するとライブラリの利用側は改めてライブラリから提供されるヘッダファイルのためにインクルード検索パスを通す必要がなくなり、大変便利である。
ちなみに、`INTERFACE`と指定すると、「ライブラリ自身は使わないが、それを利用するプログラムが使う」という意味になる。

> 参考: [CMakeスクリプトを作成する際のガイドライン - Qiita](https://qiita.com/shohirose/items/5b406f060cd5557814e9#privatepublicinterfaceを適切に使う)

## プロジェクト名をフォルダ名に

本稿で作るCMakeスクリプトはそれなりに分量が多い。
他のプログラムを作るときにもある程度流用できるよう、コピペで済む部分を増やすのが得策である。
そこで、プロジェクトが置かれているフォルダ名を取得してそれをCMake プロジェクトの名前にする。

> 参考: [Automatically use the directory as the project name in CMake - Stack Overflow][autoset]

1行目でカレントディレクトリの名前を取得し、変数`PROJECT_ID`に格納している。
2行目ではスペースが含まれていた場合を考慮して`string`コマンドによりスペースをアンダースコアに置き換えている。
すると、ディレクトリ名から生成されたプロジェクト名が変数`PROJECT_ID`に格納されるので、
3行目でそれをプロジェクト名に設定している。

```CMake
get_filename_component(PROJECT_ID ${CMAKE_CURRENT_SOURCE_DIR} NAME)
string(REPLACE " " "_" PROJECT_ID ${PROJECT_ID})
project(${PROJECT_ID})
```

## 成果物の出力先の決定

何もしないと、コンパイルしてできたファイルはビルドツリー内のどこか勝手な場所に置かれてしまう。
そこで、CMakeのインストール機能を用いて、ビルドした後に生成物を一か所に固めて置く処理を記述する。

`cmake --install build`を実行すると、CMakeプロジェクトの「インストール」を行うことができる。
既定では、`C:\Program Files\yourlib\bin`や、`/usr/bin/yourlib`
など管理者権限が必要な場所に配置しようとしてしまい、うまくいかない。

### インストール先の指定

インストール先の基準となるフォルダはいくつかの方法で指定することができる。

- スクリプト内で`CMAKE_INSTALL_PREFIX`という変数にパスを設定する
- CMakeプロジェクトの構成時に`cmake -B build -DCMAKE_INSTALL_PREFIX="path/to/install"`のように、  
  `-D CMAKE_INSTALL_PREFIX=`というオプションを与える
- インストール時に`cmake --install build --prefix "ここ"`のように、
  `--prefix`オプションを与える

したがって、コマンドライン上でインストール位置を指示された場合はそれを使い、
そうでない場合はデフォルトのパスを用いるようにしたい。
そのため、`CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT` 
を用いて分岐しつつスクリプト内でインストール先を指示する。

```CMake
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR} CACHE PATH "" FORCE)
endif()
```

この例ではインストール先を`${PROJECT_SOURCE_DIR}`、すなわちプロジェクトのソースファイルがある場所に指定している。
`set`の後ろの方にある`CACHE PATH "説明テキスト" FORCE`は、
値を保存して次のConfigureでも（上書きされない限り）使うというものである。

> 参考:
>
> - [CMAKE_INSTALL_PREFIXの初期値を変更する - Qiita](https://qiita.com/kenyabe/items/d25d26778f7280a15bab)
> - [CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT — CMake 3.23.0-rc3 Documentation](https://cmake.org/cmake/help/latest/variable/CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT.html)

具体的な場所の指定には、`GNUInstallDirs`というパッケージを使う。
このパッケージを読み込むと、`CMAKE_INSTALL_○○`という変数が利用可能になり、それが標準的なインストール先を表す。

インストール先をコマンドライン上でカスタマイズできるように変数`YOURLIB_INSTALL_DIR`を用意し、
それに何も入っていない場合にのみ既定値を設定する。
今回は、実行ファイルの場所を示す`CMAKE_INSTALL_BINDIR`（`"bin"`であることが多いらしい）の中に、
OSごと、ビルド種別ごとに生成物のフォルダ分けをする｡[^ge]

[^ge]: `$<～>`という構文を**Generator expression**という。
これは、ビルド環境の構築時ではなくプロジェクトのビルド時に評価される式のことで、
生成物の出力先やビルドモードなど、設定によって柔軟に変化しうるものを一発で取ってくることができる。
その他、コンパイラの種類によって分岐を行う`$<C_COMPILER_ID:～>`などがある。

```CMake
if(NOT YOURLIB_INSTALL_DIR)
    include(GNUInstallDirs)
    set(YOURLIB_LIBRARY_TYPE $<IF:$<BOOL:${YOURLIB_BUILD_SHARED}>,Shared,Static>)
    set(YOURLIB_INSTALL_DIR ${CMAKE_INSTALL_BINDIR}/${CMAKE_SYSTEM_NAME}/${YOURLIB_LIBRARY_TYPE})
endif()
```

### インストール時の挙動の設定

実際にインストール先を設定するには`install()`のオプション`DESTINATION`を使用する。
動的ライブラリの配置には`LIBRARY DESTINATION`と`RUNTIME DESTINATION`の両方を設定する。
WindowsのDLLはRUNTIMEに、Unix系の共有ライブラリはLIBRARYに分類される。
ビルドオプションで静的ライブラリを生成するようにしている場合は当然何も置かれないので、
`if(YOURLIB_BUILD_SHARED)` ～ `endif()`で囲む。

```CMake
if(YOURLIB_BUILD_SHARED)
    install(TARGETS ${TARGET_NAME}
        LIBRARY DESTINATION ${YOURLIB_INSTALL_DIR}
        RUNTIME DESTINATION ${YOURLIB_INSTALL_DIR})
endif()
```

ライブラリ単体をビルドする場合は、静的ライブラリなども分かりやすい場所に置かれてほしいので`ARCHIVE DESTINATION`も追加する。

```CMake
if(${PROJECT_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR})
    install(TARGETS ${TARGET_NAME}
        ARCHIVE DESTINATION ${YOURLIB_INSTALL_DIR})
endif()
```

### 追加のファイルのコピー

生成されたライブラリの他に、設定ファイルなど追加のファイルやフォルダを配置したい場合がある。
そのような場合は`install(FILES)`と`install(DIRECTORY)`を用いてコピーするファイルを指定する。

```CMake
set(FILES_TO_INSTALL ${PROJECT_SOURCE_DIR}/cmake/sample.txt)
install(FILES ${FILES_TO_INSTALL} DESTINATION ${YOURLIB_INSTALL_DIR})
```

フォルダをコピーする場合、パスの最後に`/`を付けるかどうかで意味が変わる。

- `/`がある場合、フォルダ内の全ファイルをコピー
- `/`がない場合、フォルダそのものをコピー

```CMake
set(DIRECTORIES_TO_INSTALL
    ${PROJECT_SOURCE_DIR}/cmake/cfg
    ${PROJECT_SOURCE_DIR}/cmake/misc)
install(DIRECTORY ${DIRECTORIES_TO_INSTALL} DESTINATION ${YOURLIB_INSTALL_DIR})
```

<details><summary>ビルド時に出力先を決める方法</summary>

何も指定しないと、生成されたソリューションファイルのある場所に実行ファイルが置かれたりしてやりにくいので出力先を明示的に指定する、ということもできる。
何を出力するかによって設定する変数が変わる。非常に面倒だが、何の設定で何が変わるかを以下にまとめる。
基本的には以下に示す3つを全部書いておけばとりあえず間違いないだろう。

```CMake
set(OUTPUT_DIR ${PROJECT_SOURCE_DIR}/bin/$<CONFIG>) # 出力先
set_target_properties(${TARGET_NAME} PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR} # 静的ライブラリ用
    LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR} # 動的ライブラリ用
    RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR} # 実行ファイル用
)
```

**出力先に関わる変数**

CMakeの出力先の設定は主に3種類ある。

- **ARCHIVE_OUTPUT_DIRECTORY**  
  - 静的ライブラリの出力場所を決めるもの。
    - Windows
      - エクスポートファイル (yourlib.exp)
      - インポートライブラリ (yourlib.lib, libyourlib.dll.a)
      - 静的ライブラリ (yourlib.lib, libyourlib.a)
      - 静的ライブラリのデバッグ情報 (yourlib.pdb)
    - Linux/macOS
      - 静的ライブラリ (libyourlib.a)
- **LIBRARY_OUTPUT_DIRECTORY**
  - 動的ライブラリの出力場所を決めるもの（**ただしWindowsのDLLを除く**）。
    - Windows
      - **なし**
    - Linux/macOS
      - 共有ライブラリ (libyourlib.so, libyourlib.dylib)
- **RUNTIME_OUTPUT_DIRECTORY**
  - 実行ファイルの出力場所を決めるもの。
    - Windows
      - 実行ファイル (*.exe)
      - **DLLファイル** (yourlib.dll, libyourlib.dll)
      - 実行ファイルとDLLファイルに対応するデバッグ情報 (*.pdb)
    - Linux/macOS
      - 実行ファイル
</details>

<details><summary>ライブラリをコンパイルするとそもそも何が生成されるか</summary>

- **動的ライブラリ**
  - Windows (Visual Studio)
    - DLLファイル (yourlib.dll)
      - ライブラリを利用するプログラムが実行時に読み出すもの。
    - インポートライブラリ (yourlib.lib)
      - ライブラリを利用するプログラムをビルドする時にリンクするもの。
    - エクスポートファイル (yourlib.exp)
      - ライブラリのビルド時に公開された関数のリストがここにまとめられる。  
        2つのライブラリが互いに公開されたシンボルを参照するような場合に使用するらしい。  
        （参考: [インポート ライブラリとエクスポート ファイルの使用 | Microsoft Docs][msvc-exp]）
    - デバッグ情報 (yourlib.pdb)
      - MSVCでデバッグ情報を含むように指定すると生成される。  
        Visual Studioでステップ実行したりする時に参照される。
  - Windows (MinGW)
    - DLLファイル (libyourlib.dll)
    - インポートライブラリ (libyourlib.dll.a)
  - Linux
    - 共有ライブラリ (libyourlib.so)
      - ライブラリを利用するプログラムが実行時に読み出すもの。
  - macOS
    - 共有ライブラリ (libyourlib.dylib)
      - ライブラリを利用するプログラムが実行時に読み出すもの。
- **静的ライブラリ**
  - Windows (Visual Studio)
    - 静的ライブラリ (yourlib.lib)
      - ライブラリを利用するプログラムのビルド時に埋め込まれるもの。
    - デバッグ情報 (yourlib.pdb)
  - Windows (MinGW)
    - 静的ライブラリ (libyourlib.a)
  - Linux
    - 静的ライブラリ (libyourlib.a)
  - macOS
    - 静的ライブラリ (libyourlib.a)
</details>

## ビルドモードの設定

Visual Studioでは既定でDebug ビルドとRelease ビルドというものが用意されていて、
`RUNTIME_OUTPUT_DIRECTORY`などで指定したパスにDebug（またはRelease）というサブフォルダが作られてそこに出力される。
このように、1つの環境で複数のビルドモードを備えているビルド環境をmulti-configuration generatorという。

対して、Makeなどは一度Makefileを作ってしまうとビルドモードを切り替えることは基本的にはできない。
このようなビルド環境をsingle configuration generatorという。

CMakeで用意されているビルドモードを指定すると、Makefile等でも適切にビルドモードに応じたコンパイルオプションが追加されて便利である。
ビルドモードを示す変数`CMAKE_BUILD_TYPE`の既定値は環境により異なる。
multi-configuration generatorではDebugに、single configuration generatorでは空文字列になることが多いようだ。

また、ビルドモードの選択肢そのものは既定では4種類用意されている。

| ビルドモード    | 説明                              |
| ---            | ---                               |
| Debug          | デバッグビルド                     |
| Release        | リリースビルド                     |
| RelWithDebInfo | デバッグ情報付きリリースビルド       |
| MinSizeRel     | ファイルサイズ最適化のリリースビルド |

下2つを使うことは多分ないので、消してしまってよいと思われる。

```CMake
set(CMAKE_CONFIGURATION_TYPES Debug Release)
```

また、ビルドモードの初期値も決めておくと環境依存性が減ると思われる。

```CMake
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()
```

## 公開する関数の指定

作ったライブラリから何の関数を利用可能にするかを決める必要がある。
なんと、CMakeにはこの設定を行う統一的な手段が存在しないようなので、
代表的なコンパイラを対象に直接設定を書く。

[oneTBBのCMakeスクリプト](https://github.com/oneapi-src/oneTBB)を見ると、
どうやら以下のように書くとリンク時のオプションを統一的に記述できるようだ｡[^1tbb]
`LINK_DEPENDS`に公開する関数の設定ファイルを置くと、
再コンパイル時にそれが更新されたのを反映するようになるようだ。

[^1tbb]: Windows、Linuxまでは自力でなんとか調べたが、
macOSで同じことをする方法が分からず困り果てていたところ、このライブラリを発見した。
どうやらこれは他の明らかに対応するのが面倒くさそうなコンパイラにも対応しているようなので、
クロスプラットフォーム開発をする際に大変参考になるものと思われる。

```CMake
set_target_properties(${TARGET_NAME} PROPERTIES
    LINK_FLAGS リンカーに渡すパラメータ
    LINK_DEPENDS リンク時の依存ファイルの指定
)
```

### MSVC

Visual Studioでコンパイルする場合は、
ビルド対象のファイルに**モジュール定義ファイル** (yourlib.def)を追加する。
コンパイラがMSVCである場合だけ追加してほしいので、`if()`で条件分岐する。

コンパイラが特定の物であるかを判断するためには、`CMAKE_C_COMPILER_ID`という変数の値を確認する。

> 参考: [c++ - In CMake, how can I test if the compiler is Clang? - Stack Overflow][compiler]

モジュール定義ファイルのパス`${DEF_FILE}`は空白の入ったパスに対応するためダブルクォートで囲む必要がある。

```CMake
if(${CMAKE_C_COMPILER_ID} STREQUAL MSVC)
    set(DEF_FILE ${PROJECT_SOURCE_DIR}/cmake/${TARGET_NAME}.def)
    set_target_properties(${TARGET_NAME} PROPERTIES
        LINK_FLAGS ${CMAKE_LINK_DEF_FILE_FLAG}"${DEF_FILE}"
        LINK_DEPENDS "${DEF_FILE}"
    )
endif()
```

モジュール定義ファイルは以下のような文法で書かれる公開シンボルのリストである。
最初に`EXPORTS`と書き、その下に公開する関数を列挙すると、それが公開される。
関数でなく変数も公開することが可能で、その場合は変数名の後に`DATA`と書く。

```
; セミコロンで始まる行はコメント
EXPORTS
    yourlib_function1
    yourlib_function2
    yourlib_exported_variable DATA
```

### GCC

GCCでは何も指定しないと既定で全ての関数が公開される。
別にそのままでもよいのだが、内部実装で使う関数も適切な定義さえ与えてしまえば使えてしまうので、
ちょっと都合が悪いことも考えられる。

公開する関数の指定にはリンカオプション`--version-script`に**バージョンスクリプト** (yourlib.map)を指定する。

```CMake
if(${CMAKE_C_COMPILER_ID} STREQUAL GNU)
    set(VERSION_SCRIPT ${PROJECT_SOURCE_DIR}/cmake/${TARGET_NAME}.map)
    set_target_properties(${TARGET_NAME} PROPERTIES
        LINK_FLAGS -Wl,--version-script="${VERSION_SCRIPT}"
        LINK_DEPENDS "${VERSION_SCRIPT}"
    )
endif()
```

バージョンスクリプトは次のような書式のファイルである。

```
{ /* C言語のコメント構文を利用できる */
    global: yourlib_*;
    local: *;
};
```

`global:`というフィールドに公開する関数を列挙する……のだが、
ワイルドカードを使えるので大抵は「ライブラリ名から始まる関数全部」みたいな指定の仕方になると思われる。  
`local:`というフィールドには非公開の関数を列挙することになっているらしい。
わざわざ明示的に指定する意味はなさそうなので、ワイルドカードを置いて「その他全部」を表すのがよい。

### Clang

Clangは少し面倒だ。oneTBBのスクリプトといくつかの実験によれば、

- macOSではAppleClangという特別製Clangが使われる。`CMAKE_C_COMPILER_ID`の値も`AppleClang`である。
- 他のOSでは、OSごとにリンカーオプションが異なる。
    - WindowsならMSVCの設定を渡すとなぜかうまくいく。
    - WindowsにGCCの設定を渡すとうまくいかなかった。
    - LinuxではGCCの設定を渡すとうまくいく
    - 確認していないが、macOSではAppleClangの設定を渡すとうまくいくものと思われる。

以上を踏まえると、コンパイラごとに公開する関数の設定を渡すスクリプトは次のようになるだろう。
ここで、コンパイラの識別に使う式`${CMAKE_C_COMPILER_ID} STREQUAL XXX`の結果を変数に格納するため、
同じ機能を持つ`string(COMPARE EQUAL ${CMAKE_COMPILER_ID} XXX IS_XXX)`を用いる。

```CMake
string(COMPARE EQUAL ${CMAKE_C_COMPILER_ID} MSVC       IS_MSVC)
string(COMPARE EQUAL ${CMAKE_C_COMPILER_ID} Clang      IS_CLANG)
string(COMPARE EQUAL ${CMAKE_C_COMPILER_ID} AppleClang IS_ACLANG)
string(COMPARE EQUAL ${CMAKE_C_COMPILER_ID} GNU        IS_GNU)
if(IS_MSVC OR (WIN32 AND IS_CLANG))
    set(EXPORT_LIST ${PROJECT_SOURCE_DIR}/cmake/${TARGET_NAME}.def)
    set(EXPORT_FLAG ${CMAKE_LINK_DEF_FILE_FLAG})
elseif(IS_ACLANG OR (APPLE AND IS_CLANG))
    set(EXPORT_LIST ${PROJECT_SOURCE_DIR}/cmake/${TARGET_NAME}.exp)
    set(EXPORT_FLAG -Wl,-exported_symbols_list,)
elseif(IS_GNU OR IS_CLANG)
    set(EXPORT_LIST ${PROJECT_SOURCE_DIR}/cmake/${TARGET_NAME}.map)
    set(EXPORT_FLAG -Wl,--version-script=)
endif()
set_target_properties(${TARGET_NAME} PROPERTIES
    LINK_FLAGS ${EXPORT_FLAG}"${EXPORT_LIST}"
    LINK_DEPENDS "${EXPORT_LIST}"
)
```

**macOSにおける設定**

Clangの挙動が面倒くさすぎて解説が後回しになってしまったが、
macOSにおける公開関数の設定ファイルは以下のような書式をとる。
調べた限り、ファイルの拡張子に特に慣例はないようなので、
本稿では対応するファイルをyourlib.expとした。

> 参考: [Dynamic Library Design Guidelines][apple-exports]

```
# #で始まる行（行の途中からでも使える？）はコメント
_yourlib_function1
_yourlib_function2
_yourlib_*
```

ここに書く関数名はソースで定義されている名前の先頭に`_`を付ける必要がある。
また、バージョンスクリプトと同じくワイルドカードを利用できる。したがって、ファイルの中身は往々にしてこうなる。

```
_yourlib_*
```

### 出力された公開シンボルの表示

実際に生成されたライブラリが公開している関数を調べる方法がある。

- **Windows**
  - 「Developer Command Prompt for VS 2019」を起動する。
    - Visual Studioをインストールすると使えるらしい。
      普通のコマンドプロンプトだとPATHが通ってないのかうまくいかない。
  - `dumpbin /exports yourlib.dll`で公開されている関数の一覧が出力される。
  - MinGWしかない場合は`gendef yourlib.dll`とやるとモジュール定義ファイル
    (yourlib.def)が生成されるのでそれを読む方法がある。
- **Linux**
  - `nm -gD libyourlib.so`で確認できる。
- **macOS**
  - `nm -gU libyourlib.dylib`で確認できる。

## Windows特有の設定

Visual Studioで作業する場合、既定では不便な仕様がかなり多いので修正する必要がある。

### カレントディレクトリの設定

CMakeで作成したVisual Studioのプロジェクトでデバッグすると、
カレントディレクトリが不便な位置に設定されているため「実行ファイルからの相対パスでファイルを読む」みたいなことをする時につらい。

~~`if(MSVC)`～`endif()`でMSVCの場合にのみ実行するようにしつつ、~~（← おそらく他の環境では無視されるのでやらなくてよい）
ビルド対象のデバッグ時のカレントディレクトリを実行ファイルのパスに設定する。

```CMake
set_target_properties(${TARGET_NAME}
    PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY
    $<TARGET_FILE_DIR:${TARGET_NAME}>
)
```

### BOMなしUTF-8に関する修正

ソースコードに**うっかり**日本語のコメントを入れたり、**うっかり**日本語の文字列リテラルを含ませたりすると、
Visual Studioでビルドする時にこんなことを言われて怒られる。

> C4819:ファイルは現在のコードページ(932)で表示できない文字を含んでいます。ファイルをUnicode形式で保存してください。

今時の若者なら当然ソースファイルをUTF-8で保存しているだろうし、確認してみても実際にそうなっている。なのに怒られる。困った。ということが起こる。
別に警告が出るだけなら最悪無視してもよいのだが（私は神経質なので無視できないが）、
これとともに謎の文法エラーを起こしてコンパイルが止まったりする。~~「')'が必要です」ってどういうことだ、括弧なら足りてるぞ。~~
調べてみると、Visual StudioはBOMのついたUTF-8しか自動的に認識してくれないらしい。
一方で、Linuxなど他の環境だとBOMのついたUTF-8を正しく認識できなかったりする。無理すぎる。

というわけでVisual Studio環境の場合にコンパイルオプションを追加し、ソースを強制的にUTF-8で読むようにする。  

> 参考: [visual c++ - Possible to force CMake/MSVC to use UTF-8 encoding for source files without a BOM? C4819 - Stack Overflow][vc-utf8]

```CMake
target_compile_options(${TARGET_NAME} PRIVATE $<$<C_COMPILER_ID:MSVC>:/utf-8>)
```

### その他

これらは好みで決めるべきだが、個人的にやっておきたいものである。

`_CRT_SECURE_NO_WARNINGS`を設定すると、Visual Studioでのセキュリティがなんとかいう警告を黙らせることができる。

```CMake
target_compile_definitions(${TARGET_NAME} PRIVATE $<$<C_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>)
```

MinGWのgccでコンパイルすると、Windowsなのにlibなんとか.dllというファイル名となり、慣例に合わない。
そこで、このlibを取り除くための設定を加える。

```CMake
if(WIN32 AND NOT MSVC)
    set_target_properties(${TARGET_NAME} PROPERTIES PREFIX "" IMPORT_PREFIX "")
endif()
```

## まとめ

ここまで多くの設定項目を見てきたが、全部まとめると以下のようなスクリプトが出来上がる。長い。

これをコンパイルするには次のようなコマンドを実行する。
`Release`と書いてある部分がビルドモードの指定で、`Debug`ビルドを用いる場合はそのように書き換える。

```
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
cmake --install build --config Release
```

本稿ではC言語による実装を想定しているが、C++を用いる場合、
ここまで登場した変数でいかにもC言語用の設定だな、というようなもの（`CMAKE_C_COMPILER_ID`など）の
`C`を`CXX`としたもの（`CMAKE_CXX_COMPILER_ID`など）について同様の記述を行う必要がある。例えば、

```CMake
target_compile_definitions(${TARGET_NAME} PRIVATE
    $<$<C_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
    $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>)
```

というような具合である。

<details><summary>yourlib/CMakeLists.txt</summary>

```CMake
cmake_minimum_required(VERSION 3.16)

# プロジェクト名の設定
get_filename_component(PROJECT_ID ${CMAKE_CURRENT_SOURCE_DIR} NAME)
string(REPLACE " " "_" PROJECT_ID ${PROJECT_ID})
project(${PROJECT_ID})

# In-source ビルドの抑止
if(${PROJECT_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR 
        "In-source builds are not allowed. "
        "Run 'cmake -B build' instead.")
endif()

# 32 bit環境でビルドしようとしている場合に警告
if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    message(WARNING
        "********* 32 bit build warning *********\n"
        "It seems you are trying to build on a 32 bit platform.")
endif()

# ビルドオプションの設定
option(YOURLIB_BUILD_SHARED "Build yourlib as a shared library" ON)

# ビルドモードの設定
set(CMAKE_CONFIGURATION_TYPES Debug Release)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# yourlib/src/CMakeLists.txtを呼び出す
add_subdirectory(src)
```
</details>

<details><summary>yourlib/src/CMakeLists.txt</summary>

```CMake
# 作成するライブラリを宣言
set(TARGET_NAME yourlib)
if(YOURLIB_BUILD_SHARED)
    add_library(${TARGET_NAME} SHARED)
else()
    add_library(${TARGET_NAME} STATIC)
endif()

# ソースファイルの登録
target_sources(${TARGET_NAME} PRIVATE
    source1.c
    source2.c
)

# インクルード検索パスの設定
target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(${TARGET_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include)

# 公開する関数の設定
string(COMPARE EQUAL ${CMAKE_C_COMPILER_ID} MSVC       IS_MSVC)
string(COMPARE EQUAL ${CMAKE_C_COMPILER_ID} Clang      IS_CLANG)
string(COMPARE EQUAL ${CMAKE_C_COMPILER_ID} AppleClang IS_ACLANG)
string(COMPARE EQUAL ${CMAKE_C_COMPILER_ID} GNU        IS_GNU)
if(IS_MSVC OR (WIN32 AND IS_CLANG))
    set(EXPORT_LIST ${PROJECT_SOURCE_DIR}/cmake/${TARGET_NAME}.def)
    set(EXPORT_FLAG ${CMAKE_LINK_DEF_FILE_FLAG})
elseif(IS_ACLANG OR (APPLE AND IS_CLANG))
    set(EXPORT_LIST ${PROJECT_SOURCE_DIR}/cmake/${TARGET_NAME}.exp)
    set(EXPORT_FLAG -Wl,-exported_symbols_list,)
elseif(IS_GNU OR IS_CLANG)
    set(EXPORT_LIST ${PROJECT_SOURCE_DIR}/cmake/${TARGET_NAME}.map)
    set(EXPORT_FLAG -Wl,--version-script=)
endif()
set_target_properties(${TARGET_NAME} PROPERTIES
    LINK_FLAGS ${EXPORT_FLAG}"${EXPORT_LIST}"
    LINK_DEPENDS "${EXPORT_LIST}"
)

# インストール先の設定
if(NOT YOURLIB_INSTALL_DIR)
    include(GNUInstallDirs)
    set(YOURLIB_LIBRARY_TYPE $<IF:$<BOOL:${YOURLIB_BUILD_SHARED}>,Shared,Static>)
    set(YOURLIB_INSTALL_DIR ${CMAKE_INSTALL_BINDIR}/${CMAKE_SYSTEM_NAME}/${YOURLIB_LIBRARY_TYPE})
endif()

# 共有ライブラリの配置
if(YOURLIB_BUILD_SHARED)
    install(TARGETS ${TARGET_NAME}
        LIBRARY DESTINATION ${YOURLIB_INSTALL_DIR}
        RUNTIME DESTINATION ${YOURLIB_INSTALL_DIR})
endif()

# ライブラリ単体をビルドする時、インポートライブラリと静的ライブラリの配置
if(${PROJECT_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR})
    install(TARGETS ${TARGET_NAME}
        ARCHIVE DESTINATION ${YOURLIB_INSTALL_DIR})
endif()

# 追加のファイルの配置（フォルダのコピー）
set(DIRECTORIES_TO_INSTALL
    ${PROJECT_SOURCE_DIR}/cmake/cfg
    ${PROJECT_SOURCE_DIR}/cmake/misc)
install(DIRECTORY ${DIRECTORIES_TO_INSTALL} DESTINATION ${YOURLIB_INSTALL_DIR})

# 追加のファイルの配置（ファイルのコピー）
set(FILES_TO_INSTALL
    ${PROJECT_SOURCE_DIR}/cmake/sample.txt)
install(FILES ${FILES_TO_INSTALL} DESTINATION ${YOURLIB_INSTALL_DIR})

# Windows特有の設定
# Visual Studioのデバッガ起動時の作業ディレクトリを設定する
set_target_properties(${TARGET_NAME}
    PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY
    $<TARGET_FILE_DIR:${TARGET_NAME}>
)
# Visual StudioでソースコードをUTF-8で読ませる
target_compile_options(${TARGET_NAME} PRIVATE
    $<$<C_COMPILER_ID:MSVC>:/utf-8>
    $<$<CXX_COMPILER_ID:MSVC>:/utf-8>)
# セキュリティ強化版関数を使えというおせっかい警告を黙らせる
target_compile_definitions(${TARGET_NAME} PRIVATE
    $<$<C_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
    $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>)
# MinGWでlibなんとか.dllになっちゃうのを直す
if(WIN32 AND NOT MSVC)
    set_target_properties(${TARGET_NAME} PROPERTIES PREFIX "" IMPORT_PREFIX "")
endif()
```
</details>

---

# 製作したライブラリの利用

ライブラリを製作したら、次はライブラリを利用したい。
次のコードを用いて先ほど作成したライブラリyourlibの関数を呼び出すことを考える。

<details><summary>main.cpp (クリックで展開)</summary>

```C++
#include <yourlib.h>
#include <iostream>

int main(void) {
    yourlib_test_print();
    auto z = yourlib_add(1, 2);
    std::cout << "yourlib_add(1, 2) = " << z << std::endl;
    return 0;
}
```
</details>

ライブラリを利用する方法は大きく2つある。

- ライブラリのCMakeプロジェクトを取り込んで利用する
- コンパイル済みのバイナリファイルを用意してリンクする

> **コンパイル済みのバイナリファイルとヘッダを自分で用意する場合**
> 
> 手動で依存関係を配置する場合は、`target_link_libraries`と`target_include_directories`を用いてライブラリのパスを指定する。
> ```CMake
> set(TARGET_NAME test)
> add_executable(${TARGET_NAME})
> target_sources(${TARGET_NAME} PRIVATE main.cpp)
> 
> target_include_directories(${TARGET_NAME} PRIVATE path/to/yourlib.h)
> target_link_directories(${TARGET_NAME} PRIVATE path/to/libyourlib/binary)
> target_link_libraries(${TARGET_NAME} PRIVATE yourlib)
> ```

## インストール先の指定

ライブラリを作る時と同様に、実行ファイルの配置先を設定する。

```CMake
if(NOT YOUREXEC_INSTALL_DIR)
    include(GNUInstallDirs)
    set(YOUREXEC_INSTALL_DIR ${CMAKE_INSTALL_BINDIR}/${CMAKE_SYSTEM_NAME})
endif()
```

動的ライブラリが実行ファイルと同じ位置に配置されるように、配置先を示す変数を変更する。

```CMake
set(YOURLIB_INSTALL_DIR ${YOUREXEC_INSTALL_DIR})
```

注意することとして、この設定は**依存関係を読み込む前に**済ませる必要がある。
おそらく、次節の`FetchContent_MakeAvailable`より前に書けば機能すると考えられる。

## ライブラリの取り込み

他のCMakeプロジェクトを取り込むには、`FetchContent`を用いる。

```CMake
include(FetchContent)
FetchContent_Declare(${DEPENDENCY_NAME}
    GIT_REPOSITORY https://github.com/...
    GIT_TAG master)
FetchContent_MakeAvailable(${DEPENDENCY_NAME})
```

`FetchContent_Declare`で依存関係の宣言を行う。
第1引数に識別可能な何かしらの名前を書き、それ以降にどこから取ってくるかなどの項目を書く。
Gitリポジトリをクローンして取ってくる他、指定したURLからダウンロードしてきたり、
SVNなど他のバージョン管理システムを利用することも可能なようだ。

`GIT_REPOSITORY`にリポジトリの場所（ローカルリポジトリでも可）を、
`GIT_TAG`にブランチ名、タグ名、コミットIDなどを指定する。

`FetchContent_MakeAvailable`で宣言した依存関係をCMakeで利用可能にする。
内部的には取ってきたリポジトリのルートにあるCMakeLists.txtを`add_subdirectory()`
で読み込んでいる他､[^fc] ソースのあるフォルダのパスなどが変数に格納される。

[^fc]: ちなみに、FetchContentはGitリポジトリでなくても、
CMakeプロジェクトでなくても読み込むことができる。と公式ドキュメントに書いてある。

## ライブラリのリンク

依存関係を読み込んだら、いつものように実行ファイルを作成するためのスクリプトを書く。
ライブラリをリンクするため、`target_link_libraries()`にライブラリの名前を指定して呼び出す。

```CMake
set(TARGET_NAME test)
add_executable(${TARGET_NAME})
target_sources(${TARGET_NAME} PRIVATE main.cpp)

target_link_libraries(${TARGET_NAME} PRIVATE yourlib)
```

## 実行ファイルの配置

`cmake --install build`した時に行う挙動を設定する。

```CMake
# 実行ファイルと動的ライブラリを置く場所を決める
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR} CACHE PATH "" FORCE)
endif()

install(TARGETS ${TARGET_NAME} RUNTIME DESTINATION ${YOUREXEC_INSTALL_DIR})
```

指定したターゲットに対して、ビルド生成物の配置場所を決めている。
実際の配置場所は`${CMAKE_INSTALL_PREFIX}`を前に付けて得られるパスである。

## ランタイムパスの設定

LinuxとmacOSでは、実行ファイルの場所にある共有ライブラリは検索パスに入っていないため、
このままではライブラリを読み込むことができない。
これらのOSでは**ランタイムパス** (**RPATH**などともいう)という、
実行ファイルごとに追加の検索パスを指定できる仕組みがある。

ランタイムパスの仕様は私にはさっぱり理解できないが、
とりあえず実行ファイルと同じ位置にあるライブラリを見つけられるようにするには
`RPATH`にmacOSなら`@executable`と、Linuxなら`$ORIGIN`と設定すると良いようだ。

```CMake
set_target_properties(${TARGET_NAME} PROPERTIES
    INSTALL_RPATH $<IF:$<PLATFORM_ID:Linux>,$ORIGIN,@executable_path>)
```

`$<IF:条件,真の時,負の時>`に`$<PLATFORM_ID:OSがこれの時に真>`
を組み合わせたGenerator expressionを用いてOSがLinuxであるかどうかを判定している。
また、Windowsにはランタイムパスの仕組みが存在しないため、
`INSTALL_RPATH`プロパティは（おそらく）無視される。
したがって、`if(NOT WIN32)` ～ `endif()`で囲む必要はないと考えられる。

> 参考:
>
> - [Understanding RPATH (with CMake) - development](https://dev.my-gate.net/2021/08/04/understanding-rpath-with-cmake/)  
> - [RPATH handling · Wiki · CMake / Community · GitLab](https://gitlab.kitware.com/cmake/community/-/wikis/doc/cmake/RPATH-handling)  
> - [Creating relocatable Linux executables by setting RPATH with $ORIGIN | by Luke Chen | Medium](https://nehckl0.medium.com/creating-relocatable-linux-executables-by-setting-rpath-with-origin-45de573a2e98)  
> - [バイナリのRPATHの状態を確認する - Qiita](https://qiita.com/koara-local/items/2c26e249e02994230324)

## まとめ

以上をまとめると、CMakeプロジェクトとして存在するライブラリを利用するために書くべきCMakeスクリプトは次のようなものとなる。

<details><summary>CMakeLists.txt (クリックで展開)</summary>

```CMake
cmake_minimum_required(VERSION 3.16)

# プロジェクト名の設定
get_filename_component(PROJECT_ID ${CMAKE_CURRENT_SOURCE_DIR} NAME)
string(REPLACE " " "_" PROJECT_ID ${PROJECT_ID})
project(${PROJECT_ID})

# In-source ビルドの抑止
if(${PROJECT_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR 
        "In-source builds are not allowed. "
        "Run 'cmake -B build' instead.")
endif()

# 32 bit環境でビルドしようとしている場合に警告
if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    message(WARNING
        "********* 32 bit build warning *********\n"
        "It seems you are trying to build on a 32 bit platform.")
endif()

# ビルドモードの設定
set(CMAKE_CONFIGURATION_TYPES Debug Release)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# インストール先の設定
if(NOT YOUREXEC_INSTALL_DIR)
    include(GNUInstallDirs)
    set(YOUREXEC_INSTALL_DIR ${CMAKE_INSTALL_BINDIR}/${CMAKE_SYSTEM_NAME})
endif()

# 依存ライブラリのインストール先を指定
set(YOURLIB_INSTALL_DIR ${YOUREXEC_INSTALL_DIR})

# 依存関係の読み込み
include(FetchContent)
set(DEPENDENCY_NAME dependency)
FetchContent_Declare(${DEPENDENCY_NAME}
    GIT_REPOSITORY ${PROJECT_SOURCE_DIR}
    GIT_TAG tutorial-library)
FetchContent_MakeAvailable(${DEPENDENCY_NAME})

# ビルドターゲットの設定
set(TARGET_NAME test)
add_executable(${TARGET_NAME})
target_sources(${TARGET_NAME} PRIVATE main.cpp)

# 依存関係のリンク
target_link_libraries(${TARGET_NAME} PRIVATE yourlib)

# 実行ファイルと動的ライブラリを置く場所を決める
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR} CACHE PATH "" FORCE)
endif()

# インストールコマンドで実行ファイルを配置するようにする
install(TARGETS ${TARGET_NAME} RUNTIME DESTINATION ${YOUREXEC_INSTALL_DIR})

# 依存関係が動的ライブラリである場合（ライブラリのビルドオプションを参照）
if(YOURLIB_BUILD_SHARED)
    # 動的ライブラリを読めるように、ランタイムパスを設定する
    set_target_properties(${TARGET_NAME} PROPERTIES
        INSTALL_RPATH $<IF:$<PLATFORM_ID:Linux>,$ORIGIN,@executable_path>)
endif()
```
</details>

# おわりに

今まで死ぬほど調べてきて、多分こうやるといい、というような内容をひたすらここに書いた。CMakeはつらい。

---

**余談**

CMakeでビルド対象にするソースファイルは`target_sources()`で列挙するが、
ディレクトリ内のファイルを検索してそのリストを変数に格納する`file(GLOB)`と組み合わせることで
ソースファイルを自動検索するという技がある。

```CMake
add_executable(foo)
file(GLOB SOURCE_FILES "*.c")
target_sources(foo ${SOURCE_FILES})
```

しかし、この技はIDEによってはソースファイルが追加・削除された時に問題を起こすことがあるなどの理由で良くないとされている。

> 参考:
>
> - [Why is cmake file GLOB evil? - Stack Overflow](https://stackoverflow.com/questions/32411963/why-is-cmake-file-glob-evil)  
> - [CMakeスクリプトを作成する際のガイドライン - Qiita](https://qiita.com/shohirose/items/5b406f060cd5557814e9#fileglobを使わない)

**愚痴**

CMakeの公式ドキュメントは本当に読みづらい。知りたい情報にたどり着くための前提知識が多すぎるのでそう感じるのだろうが、
おかげでCMakeの仕様の把握はかなり難易度が高いと言わざるを得ない。みんな大好きStack OverflowやQiitaに先人の知見がまとまっていることも多いが、
詳細な仕様の把握にはテスト用プロジェクトの作成と実験が不可欠である。しんどい……

**参考**

- [CMake入門-基本概念と主な関数 - Qiita](https://qiita.com/sakaeda11/items/fc95f62b68a14ab861dc)
- [勝手に作るCMake入門 その1 基本的な使い方 - かみのメモ](https://kamino.hatenablog.com/entry/cmake_tutorial1)
- [勝手に作るCMake入門 その2 プロジェクトの階層化 - かみのメモ](https://kamino.hatenablog.com/entry/cmake_tutorial2)
- [CMakeの使い方（その１） - Qiita](https://qiita.com/shohirose/items/45fb49c6b429e8b204ac)
- [C/C++プロジェクトをCMakeでビルドする - Qiita](https://qiita.com/Hiroya_W/items/049bfb4c6ad3dfe6ff0c)
- [CMake再入門メモ](https://zenn.dev/rjkuro/articles/054dab5b0e4f40)
- [「自作」ＤＬＬを使おう！！](http://www.kab-studio.biz/Programing/Codian/DLL_Hook_SClass/04.html)
- [CMakeスクリプトを作成する際のガイドライン - Qiita](https://qiita.com/shohirose/items/5b406f060cd5557814e9)
- [CMakeを使ってビルドする(2) | いざどりのtrial and error](https://izadori.net/cmake_2/)
- [oneapi-src/oneTBB: oneAPI Threading Building Blocks (oneTBB)](https://github.com/oneapi-src/oneTBB)

[apple-exports]: https://developer.apple.com/library/archive/documentation/DeveloperTools/Conceptual/DynamicLibraries/100-Articles/DynamicLibraryDesignGuidelines.html#:~:text=File%20listing%20the%20names%20of%20the%20symbols%20to%20export
[autoset]: https://stackoverflow.com/questions/12344368/automatically-use-the-directory-as-the-project-name-in-cmake
[compiler]: https://stackoverflow.com/questions/10046114/in-cmake-how-can-i-test-if-the-compiler-is-clang
[cmake64]: https://stackoverflow.com/questions/39258250/how-to-detect-if-64-bit-msvc-with-cmake
[cmake-guideline]: https://qiita.com/shohirose/items/5b406f060cd5557814e9#privatepublicinterfaceを適切に使う
[msvc-exp]: https://docs.microsoft.com/ja-jp/cpp/build/reference/using-an-import-library-and-export-file?view=msvc-170
[vc-utf8]: https://stackoverflow.com/questions/47690822/possible-to-force-cmake-msvc-to-use-utf-8-encoding-for-source-files-without-a-bo
