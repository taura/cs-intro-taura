# mm

# コンパイル

```
$ make
```

するとほとんど最適化されていない 行列積のコード (mm_seq.h) からプログラム ./exe/mm_seq.exe ができる. 機械語(アセンブラ)は asm/mm_seq.s に生成される

# 実行

## -h でヘルプを表示

```
$ ./exe/mm_seq.exe -h
./exe/mm_seq.exe: invalid option -- 'h'
usage:
  ./exe/mm_seq.exe [options]:
perform matrix matrix multiply of (M,N) += (M,K) * (K,N) 
options:
 -M num : set M to num (16)
 -N num : set N to num (32)
 -K num : set K to num (128)
 -r num : repeat num times (1)
 -x X : set random seed to X, to change initial configuration (9876543210987654)

```

## 無引数で実行すると A: 16 x 128 B: 128 x 32 の行列の掛け算を(1回)行う

```
$ ./exe/mm_seq.exe
A = 16 x 128 (8192 bytes)
B = 128 x 32 (16384 bytes)
C = 16 x 32 (2048 bytes)
repeat C += A * B 1 times
131072 flops, total 26624 bytes
120244 clocks
219013 cpu clocks
0.000 nsec
1.090 flops/clock = 0.068 vfmadds/clock
0.598 flops/cpu clock = 0.037 vfmadds/cpu clock
2.568629 GFLOPS
OK: max relative error = 0.000000
```

上記の
```
0.598 flops/clock
```

が性能指標で, これを大きくするのが目標. 行列のサイズや繰り返し数(-r)を, 測定がしやすいように適宜変更して良い. プログラムは任意サイズの行列を処理するようにしておくことが求められるが, 適宜「◯◯の倍数である」などという仮定を置いても良い.

なおわかりやすい性能指標は,  0.037 vfmadds/cpu clock を 2.000 に近づけること(プロセッサ性能の限界)


# 高速化

* 上記の flops/clock を向上させる
 * SIMD
 * 命令レベル並列性
 * マルチコア並列性
を駆使する. もちろんさらにMPIを使っても良い


# 作業方法

* mm_seq.h を好きな名前にコピーする
```
$ cp mm_seq.h mm_simd.h
```

* Makefile は mm_xxx.h というファイルを見つけると勝手に exe/mm_xxx.exe と asm/mm_xxx.s を生成してくれる

* プログラムのmain関数は mm.cc というファイルにある
* そこから全体としてプログラムがどう動いているかを知りたい場合はデバッガを使うと良い

 * Makefile をいじって, -O0 -g でコンパイルされるようにする (-O3 は無効に)
 * gdb ./exe/mm_seq.exe
  (gdb) break main
  (gdb) run -s 1


