# Sapporo.cpp オープンソースカンファレンスHokkaido2017 展示用プログラム

制作：H.Hiro [https://github.com/maraigue](https://github.com/maraigue)

# このソフトウェアについて

C++で簡易HTTPサーバを立ち上げ、所定のディレクトリ（dataディレクトリを指定しています）に入っているファイルを選択し見られるようにしたものです。  
HTMLのiframeを利用しており、ウェブブラウザで見られるもの（画像、PDFファイルなど）を切り替えて表示するのに便利です。

# 利用方法

作者は動作確認はUbuntu 16.04でのみ行っています。Windowsではおそらく動きません。

- [Boost](http://www.boost.org/)をインストールします。
- それが利用可能な状況で、`make`を行います。
- `server`という実行ファイルが生成されたら実行します。コマンドラインオプションは実行ファイルの表示内容を参照してください。

# 利用条件

このコードは、[Boost.asio](https://github.com/boostorg/asio)のサンプルコード "example/cpp11/http/server/
" を元に作成しています。（これを持ってきて必要な箇所を変えただけのものです。）  
このコードも、元のサンプルコードと同じく、Boost License 1.0に基づいて利用可とします。全文（英語）は BOOST\_LICENSE\_1\_0.txt をご覧ください。

