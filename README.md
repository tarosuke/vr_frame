# vr_core_template

## What is this
This is a project template for vr_core.
Just store source code and "make", it searches source code automatically and build (of course you have to maintain repository).

## How to use

1. Fork this(on GitHub).
2. Clone forked repository (It's your project).
~~~
git clone --recursive https://github.com/tarosuke/vr_core_template.git YOUR_PROJECT
~~~
3. "make submodule update --init" if you forget "--recursive" (in your project).
4. read sample modules to learn how to write code for vr_code (or copy them).
4. "make setup" to setup udev for VR-HMDs(and, reboot or reload udev or somthing).
4. "make -j -C toolbox" to build toolbox.
4. write your code under modules (or somewhere except vr_coce).

## NOTE

* this template recognize YOUR_PROJECT as the project name. the project name used as ...
    * filename of executable
    * preference file will be stored ~/.YOUR_PROJECT

# vr_core_template

## これは何？
これはvr_codeのためのプロジェクトテンプレートです。
コードを置いてmakeするだけで自動的に探してビルドしてくれます(もちろんリポジトリの管理は自分でする必要があります)。

## 使い方

1. (GitHub上で)Forkします。
2. forkしたプロジェクトをcloneします(これがあなたのプロジェクトになります)。
~~~
git clone --recursive https://github.com/tarosuke/vr_core_template.git YOUR_PROJECT)
~~~
3. 「--recursive」を付け忘れたらcloneしたプロジェクトの中で「make update」します。
4. 「make setup」してudevにVR-HMDを設定(で、再起動なり何なり)
4. 「make -j -C toolbox」してtoolboxをビルドします。
4. vr_coreでの書き方を学ぶためにmoduleの下にあるサンプルを読みます(あるいは適当なサンプルをコピーします)。
4. modules以下(あるいはvr_core以外のどこか)にコードを書きます。

## ちゅうい

* このテンプレートは YOUR_PROJECT をプロジェクト名として解釈します。プロジェクト名は以下のように使われます。
    * 実行形式ファイルのファイル名になります。
    * 設定ファイルは「~/.YOUR_PROJECT」に格納されます。
