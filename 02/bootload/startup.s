	.h8300h
	.section .text
	.global _start
#	.type  _start,@function
_start: #ここからスタート
	mov.l #0xffff00,sp #スタック・ポインタの設定
	jsr @_main #main()にジャンプ
1:
	bra 1b
