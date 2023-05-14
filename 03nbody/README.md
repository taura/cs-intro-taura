# nbody

# コンパイル

```
$ make
```

するとほとんど最適化されていない N体問題のコード (nbody_seq.h) でシミュレーションをするプログラム ./exe/nbody_seq.exe ができる. 機械語(アセンブラ)は asm/nbody_seq.s に生成される

# 実行

## -h でヘルプを表示

```
$ ./exe/nbody_seq.exe -h
./exe/nbody_seq.exe: invalid option -- 'h'
usage:
  ./exe/nbody_seq.exe [options]:
options:
 -n N : simulate N particles (1024)
 -t dt : time step (0.001000)
 -T T : simulation end time (0.100000)
 -s N : simulate N steps (0)
 -r rv : virial ratio (0.500000)
 -e eps : softening parameter (0.001000)
 -o FILE : dump output to FILE ((null))
 -d A:B:C : dump between step A up to step B-1, with interval C (i.e., dump at step A, A+C, A+2C, ...) (0:0:1)
 -x X : set random seed to X, to change initial configuration (9876543210987654)```

## 無引数で実行すると 1024 粒子のシミュレーションを時間刻み 0.001 で, 時刻 0.1まで(100ステップ)実行 (数秒で終了する)

```
namopa:02nbody$ ./exe/nbody_seq.exe
n_particles: 1024, n_steps: 100, dt: 0.001000, T: 0.100000, rv: 0.500000, eps: 0.001000, output: None, dump: 0:100:1, seed: 9876543210987654
iteration: 0
sync: 110 clocks = 0.000000 sec
interact_all: 11165812 clocks = 0.005287 sec
kinetic: 16544 clocks = 0.000008 sec
update: 8450 clocks = 0.000004 sec
interact_all: 0.093818 ppi/clocks
iteration: 0.093607 ppi/clocks
clock frequency: 2.112005 GHz
iteration: 1

   ...

iteration: 99
sync: 76 clocks = 0.000000 sec
interact_all: 7378226 clocks = 0.003493 sec
kinetic: 9350 clocks = 0.000004 sec
update: 4594 clocks = 0.000002 sec
interact_all: 0.141979 ppi/clocks
iteration: 0.141710 ppi/clocks
clock frequency: 2.112009 GHz
all iterations: 0.122476 ppi/clocks
```

* 実行した結果に興味がある場合は結果をファイルに出力し, アニメーションが出来る. その手順

```
# Oakbridge CXにログインする際に -X を指定
$ ssh -X t50xxx@obcx.cc.u-tokyo.ac.jp
$ cd ... このフォルダに移動 ...
$ ./exe/nbody_seq.exe -o a.dat -T 1.0
$ ./anime.py a.dat
```

* これで, Macの端末に窓が現れて, 以下 https://gitlab.eidos.ic.i.u-tokyo.ac.jp/tau/cs-overview-taura/blob/master/02nbody/movie/nbody.mp4 にあるような粒子のアニメーションが始まるはず

もし長時間の動きを見たいなら, ちゃんとジョブを計算ノードに投入すること

# 高速化

* 高速化を目指す演習ではステップを多数回繰り返す必要はない
* 5ステップ程度実行して毎iterationの時間や速度 (ppi/clocks) が同じ程度の数字を示していれば十分

```
$ ./exe/nbody_seq.exe -s 5
```

* そのもとで各フェーズの実行時間を短くする, 特に interact_all の時間を短くすることが目標

* 高速化, 特に並列化は, ある程度粒子数が大きくないとほとんど意味がない
* デフォルトの1024粒子に対して高速化を目指す必要もないし意味がないので, 適宜粒子数を増やして実行せよ
* 目標は表示される以下の数字を大きくすること
```
interact_all: 0.115958 ppi/clocks
iteration: 0.073191 ppi/clocks
```
* ppi は particle particle interaction の略で, 1粒子対の計算に何 reference clock かかったかという数字
 * interact_allの方は 相互作用フェーズのみで時間を測ったときの性能
 * iterationの方はそれ以外のフェーズも時間に含めた上での性能
 * 粒子数が大きくなると殆どの時間は interact_all の中で費やされるようになる(ここだけ粒子数$N$の2乗に比例する時間がかかるため)
 
# この演習の目標

* この ppi/clocks を向上させる
 * SIMD
 * 命令レベル並列性
 * マルチコア並列性
を駆使する. もちろんさらにMPIを使っても良い

# 作業方法

* nbody_seq.h を好きな名前にコピーする
```
$ cp nbody_seq.h nbody_simd.h
```

* Makefile は nbody_xxx.h というファイルを見つけると勝手に exe/nbody_xxx.exe と asm/nbody_xxx.s を生成してくれる

* 新しいファイルではおそらく以下の関数を変更することになる

 * interact_all (全粒子間の力の計算)
 * interact2 (2粒子間の力の計算)

実行時間に深刻に影響するのはこれだけなのでこれだけをいじっても目的の8割は達成できる

* ただし以下も高速化するほうが良いだろう

 * kinetic (運動エネルギーの計算)
 * update (粒子の位置の更新)

* プログラムのmain関数は nbody.cc というファイルにある
* そこから全体としてプログラムがどう動いているか, それらのinteract_allなどの関数がどう呼ばれているかを知りたい場合はデバッガを使うと良い

 * Makefile をいじって, -O0 -g でコンパイルされるようにする (-O3 は無効に)
 * gdb ./exe/nbody_seq.exe
  (gdb) break main
  (gdb) run -s 1


