# KILMとは
Implementation of Iterated Learning Model which is proposed by S. Kirby(2002)[1]. 

S. Kirby(2002)が提案したILM(Iterated Learning Model, 繰り返し学習モデル)の実装です．合成性の言語進化モデルに加えて，2つの言語間の非類似度を定量的に評価するlanguage distance[2], ILMの高速化処理clipping[3,4]を実装しています． 

<sup>1</sup>Kirby, S. (2002). Learning, Bottlenecks and the Evolution of Recursive Syntax. In: T. Briscoe. (ed.) Linguistic Evolution through Language Acquisition: Formal and Computational Models, pp.173-203. Cambridge: Cambridge University Press.

<sup>2</sup>Matoba, R., ○Sudo, H., Hagiwara, S., Tojo, S., “Evaluation of the Symmetry Bias in Grammar Acquisition”, International Symposium on Artificial Life and Robotics, Daejeon, Korea, January, 2013

<sup>3</sup>須藤 洸基, 的場 隆一, 萩原 信吾, “繰り返し学習モデルにおける高速化のための文字列省略プロセスの提案”, 日本認知科学会第31回大会, 名古屋大学, 9月, 2014

<sup>4</sup>Ryuichi Matoba, Hiroki Sudo, Makoto Nakamura, Shingo Hagiwara, Satoshi Tojo, “Process Acceleration in the Iterated Learning Model by String Clipping”, In: International Journal of Computer and Communication Engineering, International Academy Publishing, Vol. 4, No. 2, pp100-106, 2015


## 構成要素
フォルダは，rubyの実行ユーティリティ"EXE"，本体ソースコード群"SOURCE", 実行結果用フォルダ"RESULT"で構成されています．
本プログラムを使用する場合は，"SOURCE"フォルダにて各実行環境でmakeを実行して実行ファイルを作り，EXEフォルダの実行ユーティリティをrubyで実行してください．


## 実行環境
下記実行環境をあらかじめ用意してください．

    ruby 2.4
  
    boost 1.59推奨
  
    c++14
  
    gnuplot


## make&実行
makeターゲットはkiとなっています．

    make ki

kilm.exeという実行ファイルができます．kilm.exeにオプションを付けて実行してください．オプションは-hで確認できます．
  
    kilm.exe -h
  

## 実行ユーティリティ
rubyによる実行ユーティリティを使うことで，複数スレッドでのシード値を変更しての実行，統計処理，グラフ描画が簡単にできます，初期設定では，世代数100，実行回数100，スレッド数5にて実行されます．実行回数ごとにシード値は変わります．第一引数に実行結果の入る（入っている）フォルダを指定してください．exe_count.rbでは--path, --dictionaryオプションは指定不要です．EXEフォルダを作業ディレクトリとして実行されることを想定した--dictionaryオプションが自動で付きますので，EXEフォルダにてexe_count.rbを実行してください．

例：
    ruby exe_count.rb ../RESULT --keep-random-rule --analyze --logging --generation 100
