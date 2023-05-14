# 
# Makefile nbody用
#

# .h ファイル
hs := $(wildcard nbody_*.h)

# .h ファイル に対応して実行可能ファイルを作る
# nbody_simd.h というファイルを作ったら, exe/nbody_simd.exe ができる
exes := $(patsubst %.h,exe/%.exe,$(hs))
asms := $(patsubst %.h,asm/%.s,$(hs))

#
# Cコンパイラオプション
#
CFLAGS :=
#プログラムの追跡をしたい場合, -O3をコメントアウト(#)して -O0 -g を有効にせよ
#CFLAGS += -O0 -g
CFLAGS += -O3
CFLAGS += -DNDEBUG
CFLAGS += -march=native
CFLAGS += -Wall
CFLAGS += -Wextra
CFLAGS += -fopenmp

# perfコマンドのようなイベントをプログラムから取得したい場合は
# 01perf のプログラムを参考にプログラムを変更して, 以下を追加
#CFLAGS += -I../01perf/
#CFLAGS += -I../libpfm-4.10.1/include
#LDFLAGS :=
#LDFLAGS += -L../libpfm-4.10.1/lib
#LDFLAGS += -Wl,-R$(shell realpath ../libpfm-4.10.1/lib)

all : $(exes) $(asms)

$(exes) : exe/%.exe : %.h nbody.h nbody.cc exe/created
	g++ -o $@ -DNBODYx_H=\"$<\" $(CFLAGS) nbody.cc $(LDFLAGS) $(LIBS)

$(asms) : asm/%.s : %.h nbody.h nbody.cc asm/created
	g++ -o $@ -DNBODYx_H=\"$<\" $(CFLAGS) nbody.cc -S 

exe/created :
	mkdir -p $@
asm/created :
	mkdir -p $@

clean :
	rm -rf exe asm
