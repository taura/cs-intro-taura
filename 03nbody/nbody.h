/** 
 * @file nbody.h nbody共通ヘッダファイル
 */

typedef float real;

/**
   3次元ベクタ
 */
struct vec {
  real x[3];
  vec(real a, real b, real c) {
    x[0] = a; x[1] = b; x[2] = c;
  }
  vec(real a) {
    x[0] = a; x[1] = a; x[2] = a;
  }
  vec() { }
};

/**
   cfg_record 系の状態
 */
typedef struct {
  long step;                    /** ステップ */
  long n;                       /** 粒子数 */
  real t;                       /** 時刻 */
  real T;                       /** 終了時刻 */
  real U;                       /** ポテンシャル */
  real K;                       /** 運動エネルギー */
  long idx;                     /** 粒子番号 */
  real m;                       /** 粒子質量 */
  vec pos;                      /** 粒子位置 */
  vec vel;                      /** 粒子速度 */
  vec acc;                      /** 粒子加速度 */
} cfg_record;
