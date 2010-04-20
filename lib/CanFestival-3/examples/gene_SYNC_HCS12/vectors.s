;;/* M68HC11 Interrupt vectors table
;;   Copyright (C) 1999 Free Software Foundation, Inc.
;;   Written by Stephane Carrez (stcarrez@worldnet.fr)	
;;
;;This file is free software; you can redistribute it and/or modify it
;;under the terms of the GNU General Public License as published by the
;;Free Software Foundation; either version 2, or (at your option) any
;;later version.
;;
;;In addition to the permissions in the GNU General Public License, the
;;Free Software Foundation gives you unlimited permission to link the
;;compiled version of this file with other programs, and to distribute
;;those programs without any restriction coming from the use of this
;;file.  (The General Public License restrictions do apply in other
;;respects; for example, they cover modification of the file, and
;;distribution when not linked into another program.)
;;
;;This file is distributed in the hope that it will be useful, but
;;WITHOUT ANY WARRANTY; without even the implied warranty of
;;MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
;;General Public License for more details.
;;
;;You should have received a copy of the GNU General Public License
;;along with this program; see the file COPYING.  If not, write to
;;the Free Software Foundation, 59 Temple Place - Suite 330,
;;Boston, MA 02111-1307, USA.  
;; ----------------------------------------------
;; Modified by  Francis Dupin for MC9S12DP256.  April 2003
;; francis.dupin@inrets.fr
;;*/

	.sect .text
	.globl _start
        .globl can4HdlTra
 	.globl can4HdlRcv
	.globl can4HdlErr
	.globl can4HdlWup
	.globl can3HdlTra
 	.globl can3HdlRcv
	.globl can3HdlErr
 	.globl can3HdlWup
	.globl can2HdlTra
 	.globl can2HdlRcv
	.globl can2HdlErr
 	.globl can2HdlWup
        .globl can1HdlTra
 	.globl can1HdlRcv
	.globl can1HdlErr
 	.globl can1HdlWup
	.globl can0HdlTra
	.globl can0HdlRcv
	.globl can0HdlErr
 	.globl can0HdlWup
	.globl timerOvflHdl 
	.globl timer3IC 
	.globl timer2IC
	.globl timer1IC
	.globl timer0IC
	  
;; Default interrupt handler.
	.sect .text
def:
	rti

;; 
;; Interrupt vectors are in a specific section that is
;; mapped at 0xff00. For the example program, the reset handler
;; points to the generic crt0 entry point.
;;
	.sect .vectors
	.globl vectors
vectors:
       
        .word def               ; ff00
        .word def               ; ff02
        .word def               ; ff04
        .word def               ; ff06
        .word def               ; ff08
        .word def               ; ff0a
        .word def               ; ff0c
        .word def               ; ff0e
        .word def               ; ff10
        .word def               ; ff12
        .word def               ; ff14
        .word def               ; ff16
        .word def               ; ff18
        .word def               ; ff1a
        .word def               ; ff1c
        .word def               ; ff1e
        .word def               ; ff20
        .word def               ; ff22
        .word def               ; ff24
        .word def               ; ff26
        .word def               ; ff28
        .word def               ; ff2a
        .word def               ; ff2c
        .word def               ; ff2e
        .word def               ; ff30
        .word def               ; ff32
        .word def               ; ff34
        .word def               ; ff36
        .word def               ; ff38
        .word def               ; ff3a
        .word def               ; ff3c
        .word def               ; ff3e
        .word def               ; ff40
        .word def               ; ff42
        .word def               ; ff44
        .word def               ; ff46
        .word def               ; ff48
        .word def               ; ff4a
        .word def               ; ff4c
        .word def               ; ff4e
        .word def               ; ff50
        .word def               ; ff52
        .word def               ; ff54
        .word def               ; ff56
        .word def               ; ff58
        .word def               ; ff5a
        .word def               ; ff5c
        .word def               ; ff5e
        .word def               ; ff60
        .word def               ; ff62
        .word def               ; ff64
        .word def               ; ff66
        .word def               ; ff68
        .word def               ; ff6a
        .word def               ; ff6c
        .word def               ; ff6e
        .word def               ; ff70
        .word def               ; ff72
        .word def               ; ff74
        .word def               ; ff76
        .word def               ; ff78
        .word def               ; ff7a
        .word def               ; ff7c
        .word def               ; ff7e
        .word def               ; ff80
        .word def               ; ff82
        .word def               ; ff84
        .word def               ; ff86
        .word def               ; ff88
        .word def               ; ff8a
        .word def               ; ff8c
        .word def               ; ff8e
	
	;; MSCAN
        .word can4HdlTra	; ff90 (MSCAN4 Transmit) 
        .word can4HdlRcv	; ff92 (MSCAN4 Receive)  
        .word can4HdlErr	; ff94 (MSCAN4 Errors)   
        .word can4HdlWup	; ff96 (MSCAN4 Wake up)  
        .word can3HdlTra	; ff98 (MSCAN3 Transmit) 
        .word can3HdlRcv	; ff9a (MSCAN3 Receive)  
        .word can3HdlErr	; ff9c (MSCAN3 Errors)   
        .word can3HdlWup	; ff9e (MSCAN3 Wake up)  
        .word can2HdlTra	; ffa0 (MSCAN2 Transmit) 
        .word can2HdlRcv	; ffa2 (MSCAN2 Receive)  
        .word can2HdlErr	; ffa4 (MSCAN2 Errors)   
        .word can2HdlWup	; ffa6 (MSCAN2 Wake up)  
        .word can1HdlTra	; ffa8 (MSCAN1 Transmit) 
        .word can1HdlRcv	; ffaa (MSCAN1 Receive)  
        .word can1HdlErr	; ffac (MSCAN1 Errors)   
        .word can1HdlWup	; ffae (MSCAN1 Wake up)  
        .word can0HdlTra	; ffb0 (MSCAN0 Transmit) 
	.word can0HdlRcv	; ffb2 (MSCAN0 Receive)  
        .word can0HdlErr        ; ffb4 (MSCAN0 Errors)   
        .word can0HdlWup        ; ffb6 (MSCAN0 Wake up)      

      	.word def               ; ffb8     
        .word def               ; ffba 
        .word def               ; ffbc
        .word def               ; ffbe

	.word def		; ffc0
	.word def		; ffc2
	.word def		; ffc4
	.word def		; ffc6
	.word def		; ffc8
	.word def          	; ffca (Modulus Down Counter underflow)
	.word def		; ffcc
	.word def		; ffce
	.word def		; ffd0
	.word def		; ffd2
	.word def		; ffd4

	;; SCI
	.word def		; ffd6

	;; SPI
	.word def						; ffd8
	.word def						; ffda (PAII)
	.word def						; ffdc (PAOVI)
	.word def						;ffde (TOI)

	;; Timer Output Compare
	.word def		; ffe0 Timer Chanel 7
	.word def		; ffe2 Timer Chanel 6
	.word def		; ffe4 Timer Chanel 5
	.word timer4Hdl         ; ffe6 Timer Chanel 4 used by Canopen
	.word def		; ffe8 Timer Chanel 3 
	.word def		; ffea Timer Chanel 2
	.word def		; ffec Timer Chanel 1
	.word def		; ffee Timer Chanel 0

	;;  Misc
	.word def               ; fff0 (RTII)
	.word def		; fff2 (IRQ)
	.word def		; fff4 (XIRQ)
	.word def               ; fff6 (SWI)
	.word def		; fff8 (ILL)
	.word def               ; fffa (COP Failure)
	.word def		; fffc (COP Clock monitor)
	.word _start		; fffe (reset)

