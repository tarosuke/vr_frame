# vr_frame

## What is this
This is a frameworrk to develop VR application easyer.

## Getting started

At first, clone the repository.
~~~
git clone --recursive https://github.com/tarosuke/vr_core_template.git YOUR_PROJECT
~~~
And, then...
1. "make setup" to setup udev for VR-HMDs.
1. Place your codes inside modules/.

## NOTICE

* This project recognize YOUR_PROJECT (directory name) as the project name. And the project name will be used as ...
    * filename of executable
    * preference file will be stored in ~/.YOUR_PROJECT

# vr_frame

## これは何？
これはVRアプリケーションを簡単に開発するためのフレームワークです。

## 始め方

まずはリポジトリをcloneします。
~~~
git clone --recursive https://github.com/tarosuke/vr_frame.git YOUR_PROJECT
~~~
1. 「make setup」してudevにVR-HMDを設定して修正を有効にする。
1. modules/内にあなたのコードを書きます。

## ちゅうい

* このプロジェクトは YOUR_PROJECT つまりディレクトリ名をプロジェクト名として解釈します。プロジェクト名は以下のように使われます。
    * 実行形式ファイルの名前になります。
    * 設定ファイルは「~/.YOUR_PROJECT」に格納されます。
