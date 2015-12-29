#include "defines.h"
#include "serial.h"

/* シリアルコントローラーの数。H8/3069Fは３つ */
#define SERIAL_SCI_NUM 3

/* SCIの定義 */
#define H8_3069F_SCI0 ((volatile struct h8_3069f_sci *) 0xffffb0)
#define H8_3069F_SCI1 ((volatile struct h8_3069f_sci *) 0xffffb8)
#define H8_3069F_SCI2 ((volatile struct h8_3069f_sci *) 0xffffc0)

/* SCIの各種レジスタの定義*/
struct h8_3069f_sci {
  volatile uint8 smr;                   /* シリアル通信のモード設定 */
  volatile uint8 brr;                   /* ボーレートの設定 */
  volatile uint8 scr;                   /* 送受信の有効/無効など */
  volatile uint8 tdr;                   /* 送信したい1文字を書き込む */
  volatile uint8 ssr;                   /* 送信完了/受信完了などを表す */
  volatile uint8 rdr;                   /* 受信した1文字を読み出す */
  volatile uint8 scmr;
};

/* SMR（シリアルモードレジスタ）の設定 */
#define H8_3069F_SCI_SMR_CKS_PER1 (0<<0) /* クロックセレクト*/
#define H8_3069F_SCI_SMR_CKS_PER4 (1<<0)/* ０ならばクロックをそのまま利用する */
#define H8_3069F_SCI_SMR_CKS_PER16 (2<<0) 
#define H8_3069F_SCI_SMR_CKS_PER64 (3<<0)
#define H8_3069F_SCI_SMR_MP (1<<2)     
#define H8_3069F_SCI_SMR_STOP (1<<3)     /* ストップビット長1bitで0,2bitで1*/
#define H8_3069F_SCI_SMR_OE (1<<4)      /* パリティの種類0で偶数、1で奇数パリティ */
#define H8_3069F_SCI_SMR_PE (1<<5)      /* ０でパリティ無効、1で有効 */
#define H8_3069F_SCI_SMR_CHR (1<<6)     /* データ長0で8ビット1で7ビット */
#define H8_3069F_SCI_SMR_CA (1<<7)      /* 0で調歩同期モード、1でクロック同期モード */

/* SCR（シリアルコントロールレジスタ）の設定 */
#define H8_3069F_SCI_SCR_CKE0 (1<<0)    /* クロックイネーブル */
#define H8_3069F_SCI_SCR_CKE1 (1<<1)
#define H8_3069F_SCI_SCR_TEIE (1<<2)
#define H8_3069F_SCI_SCR_MPIE (1<<3)
#define H8_3069F_SCI_SCR_RE (1<<4)      /* 受信イネーブル 1で受信開始 */
#define H8_3069F_SCI_SCR_TE (1<<5)      /* 送信イネーブル 1で送信開始 */
#define H8_3069F_SCI_SCR_RIE (1<<6)     /* 受信割り込みイネーブル 1で受信割り込み有効 */
#define H8_3069F_SCI_SCR_TIE (1<<7)     /* 送信割り込みイネーブル 1で送信割り込み有効 */

/* SSR（シリアルステータスレジスタ）の設定 */
#define H8_3069F_SCI_SSR_MPBT (1<<0)
#define H8_3069F_SCI_SSR_MPB (1<<1)
#define H8_3069F_SCI_SSR_TEND (1<<2)
#define H8_3069F_SCI_SSR_PER (1<<3)
#define H8_3069F_SCI_SSR_FERERS (1<<4)
#define H8_3069F_SCI_SSR_ORER (1<<5)
#define H8_3069F_SCI_SSR_RDRF (1<<6)    /* 受信完了 */
#define H8_3069F_SCI_SSR_TDRE (1<<7)    /* 送信完了 */

static struct {
  volatile struct h8_3069f_sci *sci;
} regs[SERIAL_SCI_NUM] = {
  {H8_3069F_SCI0},
  {H8_3069F_SCI1},
  {H8_3069F_SCI2},                     
};

/* SCIの各種レジスタの初期化 */
int serial_init(int index) {
  volatile struct h8_3069f_sci *sci = regs[index].sci;

  sci->scr = 0;                         /* シリアル送受信無効 */
  sci->smr = 0;
  sci->brr = 64;                                        /* 20MHz,分周比1,9600bps */
  sci->scr = H8_3069F_SCI_SCR_RE | H8_3069F_SCI_SCR_TE; /* シリアル送受信有効 */
  sci->ssr = 0;

  return 0;
}

/* 送信可能かどうかの判定 送信可能なら 1 */
int serial_is_send_enable(int index) {
  volatile struct h8_3069f_sci *sci = regs[index].sci;
  return (sci->ssr & H8_3069F_SCI_SSR_TDRE);
}

/* １文字送信 */
int serial_send_byte(int index, unsigned char c) {
  volatile struct h8_3069f_sci *sci = regs[index].sci;

  /* 送信可能になるまでループで待ち合わせる */
  while (!serial_is_send_enable(index))
	;
  sci->tdr = c;
  sci->ssr &= ~H8_3069F_SCI_SSR_TDRE;   /* 送信完了ビットを落とす */

  return 0;
}
