/*
This file is part of CanFestival, a library implementing CanOpen Stack. 

Copyright (C): Edouard TISSERANT and Francis DUPIN

See COPYING file for copyrights details.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
/*
Definition of the i/o ports of the Motorola HCS12
*/

/*
On HCS12, ports are maped by default at 0X0000. (see ports.s)
It can also be done at link stage with an option in
command line : for example to map at 0X8000
--defsym _io_ports8=0X8000 --defsym _io_ports16=0X8000
Or in the file memory.x :
PROVIDE (_io_ports8 = 0X8000);
PROVIDE (_io_ports16 = 0X8000);
(see portsaccess.h)
I wonder if the value in INITRG must not be changed too 
at run time.
*/

#ifndef __HCS12_PORTS__
#define __HCS12_PORTS__

#define  PORTA     0x0000        
#define  PORTB     0x0001    
#define  DDRA      0x0002    
#define  DDRB      0x0003    

#define  PORTE     0x0008    
#define  DDRE      0x0009    
#define  PEAR      0x000A    
#define  MODE      0x000B    
#define  PUCR      0x000C    
#define  RDRIV     0x000D    
#define  EBICTL    0x000E    

#define  INITRM    0x0010    
#define  INITRG    0x0011    
#define  INITEE    0x0012    
#define  MISC      0x0013    
#define  MTST0     0x0014    
#define  ITCR      0x0015    
#define  ITEST     0x0016    
#define  MTST1     0x0017    

#define  PARTIDH   0x001A    
#define  PARTIDL   0x001B    
#define  MEMSIZ0   0x001C    
#define  MEMSIZ1   0x001D    
#define  INTCR     0x001E    
#define  HPRIO     0x001F    

#define  BKPCT0    0x0028    
#define  BKPCT1    0x0029    
#define  BKP0X     0x002A    
#define  BKP0H     0x002B    
#define  BKP0L     0x002C    
#define  BKP1X     0x002D    
#define  BKP1H     0x002E    
#define  BKP1L     0x002F    
#define  PPAGE     0x0030    

#define  PORTK     0x0032    
#define  DDRK      0x0033    
#define  SYNR      0x0034    
#define  REFDV     0x0035    
#define  CTFLG     0x0036    
#define  CRGFLG    0x0037    
#define  CRGINT    0x0038    
#define  CLKSEL    0x0039    
#define  PLLCTL    0x003A    
#define  RTICTL    0x003B    
#define  COPCTL    0x003C    
#define  FORBYP    0x003D    
#define  CTCTL     0x003E 
#define  ARMCOP    0x003F    
#define  TIOS      0x0040    
#define  TCFORC    0x0041    
#define  TOC7M     0x0042    
#define  TOC7D     0x0043    
#define  TCNTH     0x0044
#define  TCNTL     0x0045
#define  TSCR1     0x0046    
#define  TTOV      0x0047    
#define  TCTL1     0x0048    
#define  TCTL2     0x0049    
#define  TCTL3     0x004A    
#define  TCTL4     0x004B    
#define  TIE       0x004C    
#define  TSCR2     0x004D    
#define  TFLG1     0x004E    
#define  TFLG2     0x004F    
#define  TC0H      0x0050
#define  TC0L      0x0051
#define  TC1H      0x0052
#define  TC1L      0x0053
#define  TC2H      0x0054
#define  TC2L      0x0055
#define  TC3H      0x0056
#define  TC3L      0x0057
#define  TC4H      0x0058
#define  TC4L      0x0059
#define  TC5H      0x005A
#define  TC5L      0x005B
#define  TC6H      0x005C
#define  TC6L      0x005D
#define  TC7H      0x005E
#define  TC7L      0x005F
#define  PACTL     0x0060    
#define  PAFLG     0x0061    
#define  PACN3H    0x0062
#define  PACN2L    0x0063
#define  PACN1H    0x0064
#define  PACN0L    0x0065
#define  MCCTL     0x0066    
#define  MCFLG     0x0067    
#define  ICPAR     0x0068    
#define  DLYCT     0x0069    
#define  ICOVW     0x006A    
#define  ICSYS     0x006B    

#define  TIMTST    0x006D    

#define  PBCTL     0x0070    
#define  PBFLG     0x0071    
#define  PA3H      0x0072    
#define  PA2H      0x0073    
#define  PA1H      0x0074    
#define  PA0H      0x0075    
#define  MCCNTH    0x0076
#define  MCCNTL    0x0077
#define  TC0HH     0x0078
#define  TC0HL     0x0079
#define  TC1HH     0x007A
#define  TC1HL     0x007B
#define  TC2HH     0x007C
#define  TC2HL     0x007D
#define  TC3HH     0x007E

#define  ATD0CTL0  0x0080    
#define  ATD00CTL1 0x0081     
#define  ATD0CTL2  0x0082     
#define  ATD0CTL3  0x0083    
#define  ATD0CTL4  0x0084    
#define  ATD0CTL5  0x0085    
#define  ATD0STAT0 0x0086    
#define  ATD0STAT1 0x0087    
#define  ATD0TEST0 0x0088    
#define  ATD0TEST1 0x0089    

#define  ATD0DIEN  0x008D    

#define  PORTAD0   0x008F    
#define  ATD0DR0H  0x0090    
#define  ATD0DR0L  0x0091    
#define  ATD0DR1H  0x0092    
#define  ATD0DR1L  0x0093    
#define  ATD0DR2H  0x0094    
#define  ATD0DR2L  0x0095    
#define  ATD0DR3H  0x0096   
#define  ATD0DR3L  0x0097    
#define  ATD0DR4H  0x0098    
#define  ATD0DR4L  0x0099    
#define  ATD0DR5H  0x009A    
#define  ATD0DR5L  0x009B    
#define  ATD0DR6H  0x009C    
#define  ATD0DR6L  0x009D    
#define  ATD0DR7H  0x009E    
#define  ATD0DR7L  0x009F    
#define  PWME      0x00A0    
#define  PWMPOL    0x00A1    
#define  PWMCLK    0x00A2    
#define  PWMPRCLK  0x00A3    
#define  PWMCAE    0x00A4    
#define  PWMCTL    0x00A5    
#define  PWMTST    0x00A6    
#define  PWMPRSC   0x00A7    
#define  PWMSCLA   0x00A8    
#define  PWMSCLB   0x00A9    
#define  PWMSCNTA  0x00AA    
#define  PWMSCNTB  0x00AB    
#define  PWMCNT0   0x00AC    
#define  PWMCNT1   0x00AD    
#define  PWMCNT2   0x00AE    
#define  PWMCNT3   0x00AF    
#define  PWMCNT4   0x00B0    
#define  PWMCNT5   0x00B1    
#define  PWMCNT6   0x00B2    
#define  PWMCNT7   0x00B3    
#define  PWMPER0   0x00B4    
#define  PWMPER1   0x00B5    
#define  PWMPER2   0x00B6    
#define  PWMPER3   0x00B7    
#define  PWMPER4   0x00B8    
#define  PWMPER5   0x00B9    
#define  PWMPER6   0x00BA    
#define  PWMPER7   0x00BB    
#define  PWMDTY0   0x00BC    
#define  PWMDTY1   0x00BD    
#define  PWMDTY2   0x00BE    
#define  PWMDTY3   0x00BF    
#define  PWMDTY4   0x00C0    
#define  PWMDTY5   0x00C1    
#define  PWMDTY6   0x00C2    
#define  PWMDTY7   0x00C3    
#define  PWMSDN    0x00C4   

#define  SCI0BDH   0x00C8    
#define  SCI0BDL   0x00C9    
#define  SC0CR1    0x00CA    
#define  SCI0CR2   0x00CB    
#define  SCI0SR1   0x00CC    
#define  SC0SR2    0x00CD    
#define  SCI0DRH   0x00CE    
#define  SCI0DRL   0x00CF    
#define  SCI1BDH   0x00D0    
#define  SCI1BDL   0x00D1    
#define  SC1CR1    0x00D2    
#define  SCI1CR2   0x00D3    
#define  SCI1SR1   0x00D4    
#define  SC1SR2    0x00D5    
#define  SCI1DRH   0x00D6    
#define  SCI1DRL   0x00D7    
#define  SPI0CR1   0x00D8    
#define  SPI0CR2   0x00D9    
#define  SPI0BR    0x00DA    
#define  SPI0SR    0x00DB   

#define  SPI0DR    0x00DD    

#define  IBAD      0x00E0    
#define  IBFD      0x00E1    
#define  IBCR      0x00E2    
#define  IBSR      0x00E3    
#define  IICDR     0x00E4    

#define  DLCBCR1   0x00E8    
#define  DLCBSVR   0x00E9    
#define  DLCBCR2   0x00EA    
#define  DLCBDR    0x00EB    
#define  DLCBARD   0x00EC    
#define  DLCBRSR   0x00ED    
#define  DLCSCR    0x00EE    
#define  DLCBSTAT  0x00EF    
#define  SPI1CR1   0x00F0    
#define  SPI1CR2   0x00F1    
#define  SPI1BR    0x00F2  
#define  SPI1SR    0x00F3    

#define  SPI1DR    0x00F5    

#define  SPI2CR1   0x00F8    
#define  SPI2CR2   0x00F9    
#define  SPI2BR    0x00FA    
#define  SPI2SR    0x00FB    

#define  SPI2DR    0x00FD    

#define  FCLKDIV   0x0100     
#define  FSEC      0x0101    

#define  FCNFG     0x0103    
#define  FPROT     0x0104    
#define  FSTAT     0x0105    
#define  FCMD      0x0106    

#define  ECLKDIV   0x0110   

#define  ECNFG     0x0113    
#define  EPROT     0x0114    
#define  ESTAT     0x0115    
#define  ECMD      0x0116    

#define  ATD1CTL0  0x0120    
#define  ATD1CTL1  0x0121    
#define  ATD1CTL2  0x0122    
#define  ATD1CTL3  0x0123    
#define  ATD1CTL4  0x0124    
#define  ATD1CTL5  0x0125    
#define  ATD1STAT0 0x0126    
#define  ATD1STAT1 0x0127    
#define  ATD1TEST0 0x0128    
#define  ATD1TEST1 0x0129    

#define  ATDDIEN   0x012D    

#define  PORTAD1   0x012F    
#define  ATD1DR0H  0x0130    
#define  ATD1DR0L  0x0131    
#define  ATD1DR1H  0x0132    
#define  ATD1DR1L  0x0133    
#define  ATD1DR2H  0x0134    
#define  ATD1DR2L  0x0135    
#define  ATD1DR3H  0x0136   
#define  ATD1DR3L  0x0137    
#define  ATD1DR4H  0x0138    
#define  ATD1DR4L  0x0139    
#define  ATD1DR5H  0x013A    
#define  ATD1DR5L  0x013B    
#define  ATD1DR6H  0x013C    
#define  ATD1DR6L  0x013D    
#define  ATD1DR7H  0x013E    
#define  ATD1DR7L  0x013F    
#define  CAN0CTL0  0x0140    
#define  CAN0CTL1  0x0141    
#define  CAN0BTR0  0x0142    
#define  CAN0BTR1  0x0143    
#define  CAN0RFLG  0x0144    
#define  CAN0RIER  0x0145    
#define  CAN0TFLG  0x0146    
#define  CAN0TIER  0x0147    
#define  CAN0TARQ  0x0148    
#define  CAN0TAAK  0x0149    
#define  CAN0TBSEL 0x014A    
#define  CAN0IDAC  0x014B    

#define  CAN0RXERR 0x014E     
#define  CAN0TXERR 0x014F    
#define  CAN0IDAR0 0x0150    
#define  CAN0IDAR1 0x0151    
#define  CAN0IDAR2 0x0152    
#define  CAN0IDAR3 0x0153    
#define  CAN0IDMR0 0x0154    
#define  CAN0IDMR1 0x0155    
#define  CAN0IDMR2 0x0156    
#define  CAN0IDMR3 0x0157    
#define  CAN0IDAR4 0x0158    
#define  CAN0IDAR5 0x0159    
#define  CAN0IDAR6 0x015A    
#define  CAN0IDAR7 0x015B    
#define  CAN0IDMR4 0x015C    
#define  CAN0IDMR5 0x015D    
#define  CAN0IDMR6 0x015E    
#define  CAN0IDMR7 0x015F 
#define  CAN0RXFG0 0x0160
#define  CAN0RXFG1 0x0161
#define  CAN0RXFG2 0x0162
#define  CAN0RXFG3 0x0163
#define  CAN0RXFG4 0x0164
#define  CAN0RXFG5 0x0165
#define  CAN0RXFG6 0x0166
#define  CAN0RXFG7 0x0167
#define  CAN0RXFG8 0x0168
#define  CAN0RXFG9 0x0169
#define  CAN0RXFGA 0x016A
#define  CAN0RXFGB 0x016B
#define  CAN0RXFGC 0x016C
#define  CAN0RXFGD 0x016D
#define  CAN0RXFGE 0x016E
#define  CAN0RXFGF 0x016F
#define  CAN0TXFG0 0x0170
#define  CAN0TXFG1 0x0171
#define  CAN0TXFG2 0x0172
#define  CAN0TXFG3 0x0173
#define  CAN0TXFG4 0x0174
#define  CAN0TXFG5 0x0175
#define  CAN0TXFG6 0x0176
#define  CAN0TXFG7 0x0177
#define  CAN0TXFG8 0x0178
#define  CAN0TXFG9 0x0179
#define  CAN0TXFGA 0x017A
#define  CAN0TXFGB 0x017B
#define  CAN0TXFGC 0x017C
#define  CAN0TXFGD 0x017D
#define  CAN0TXFGE 0x017E
#define  CAN0TXFGF 0x017F 
   
#define  CAN1CTL0  0x0180    
#define  CAN1CTL1  0x0181    
#define  CAN1BTR0  0x0182      
#define  CAN1BTR1  0x0183    
#define  CAN1RFLG  0x0184    
#define  CAN1RIER  0x0185    
#define  CAN1TFLG  0x0186    
#define  CAN1TIER  0x0187    
#define  CAN1TARQ  0x0188    
#define  CAN1TAAK  0x0189    
#define  CAN1TBSEL 0x018A    
#define  CAN1IDAC  0x018B      
#define  CAN1RXERR 0x018E    
#define  CAN1TXERR 0x018F    
#define  CAN1IDAR0 0x0190    
#define  CAN1IDAR1 0x0191    
#define  CAN1IDAR2 0x0192    
#define  CAN1IDAR3 0x0193    
#define  CAN1IDMR0 0x0194    
#define  CAN1IDMR1 0x0195    
#define  CAN1IDMR2 0x0196    
#define  CAN1IDMR3 0x0197    
#define  CAN1IDAR4 0x0198    
#define  CAN1IDAR5 0x0199   
#define  CAN1IDAR6 0x019A    
#define  CAN1IDAR7 0x019B    
#define  CAN1IDMR4 0x019C    
#define  CAN1IDMR5 0x019D    
#define  CAN1IDMR6 0x019E    
#define  CAN1IDMR7 0x019F

#define  CAN1RXFG0 0x01A0
#define  CAN1RXFG1 0x01A1
#define  CAN1RXFG2 0x01A2
#define  CAN1RXFG3 0x01A3
#define  CAN1RXFG4 0x01A4
#define  CAN1RXFG5 0x01A5
#define  CAN1RXFG6 0x01A6
#define  CAN1RXFG7 0x01A7
#define  CAN1RXFG8 0x01A8
#define  CAN1RXFG9 0x01A9
#define  CAN1RXFGA 0x01AA
#define  CAN1RXFGB 0x01AB
#define  CAN1RXFGC 0x01AC
#define  CAN1RXFGD 0x01AD
#define  CAN1RXFGE 0x01AE
#define  CAN1RXFGF 0x01AF
#define  CAN1TXFG0 0x01B0
#define  CAN1TXFG1 0x01B1
#define  CAN1TXFG2 0x01B2
#define  CAN1TXFG3 0x01B3
#define  CAN1TXFG4 0x01B4
#define  CAN1TXFG5 0x01B5
#define  CAN1TXFG6 0x01B6
#define  CAN1TXFG7 0x01B7
#define  CAN1TXFG8 0x01B8
#define  CAN1TXFG9 0x01B9
#define  CAN1TXFGA 0x01BA
#define  CAN1TXFGB 0x01BB
#define  CAN1TXFGC 0x01BC
#define  CAN1TXFGD 0x01BD
#define  CAN1TXFGE 0x01BE
#define  CAN1TXFGF 0x01BF   
#define  CAN2CTL0  0x01C0    
#define  CAN2CTL1  0x01C1    
#define  CAN2BTR0  0x01C2    
#define  CAN2BTR1  0x01C3    
#define  CAN2RFLG  0x01C4    
#define  CAN2RIER  0x01C5    
#define  CAN2TFLG  0x01C6    
#define  CAN2TIER  0x01C7    
#define  CAN2TARQ  0x01C8    
#define  CAN2TAAK  0x01C9    
#define  CAN2TBSEL 0x01CA    
#define  CAN2IDAC  0x01CB    

#define  CAN2RXERR 0x01CE   
#define  CAN2TXERR 0x01CF     
#define  CAN2IDAR0 0x01D0    
#define  CAN2IDAR1 0x01D1    
#define  CAN2IDAR2 0x01D2    
#define  CAN2IDAR3 0x01D3    
#define  CAN2IDMR0 0x01D4    
#define  CAN2IDMR1 0x01D5    
#define  CAN2IDMR2 0x01D6    
#define  CAN2IDMR3 0x01D7    
#define  CAN2IDAR4 0x01D8    
#define  CAN2IDAR5 0x01D9    
#define  CAN2IDAR6 0x01DA    
#define  CAN2IDAR7 0x01DB    
#define  CAN2IDMR4 0x01DC    
#define  CAN2IDMR5 0x01DD    
#define  CAN2IDMR6 0x01DE    
#define  CAN2IDMR7 0x01DF
#define  CAN2RXFG0 0x01E0
#define  CAN2RXFG1 0x01E1
#define  CAN2RXFG2 0x01E2
#define  CAN2RXFG3 0x01E3
#define  CAN2RXFG4 0x01E4
#define  CAN2RXFG5 0x01E5
#define  CAN2RXFG6 0x01E6
#define  CAN2RXFG7 0x01E7
#define  CAN2RXFG8 0x01E8
#define  CAN2RXFG9 0x01E9
#define  CAN2RXFGA 0x01EA
#define  CAN2RXFGB 0x01EB
#define  CAN2RXFGC 0x01EC
#define  CAN2RXFGD 0x01ED
#define  CAN2RXFGE 0x01EE
#define  CAN2RXFGF 0x01EF
#define  CAN2TXFG0 0x01F0
#define  CAN2TXFG1 0x01F1
#define  CAN2TXFG2 0x01F2
#define  CAN2TXFG3 0x01F3
#define  CAN2TXFG4 0x01F4
#define  CAN2TXFG5 0x01F5
#define  CAN2TXFG6 0x01F6
#define  CAN2TXFG7 0x01F7
#define  CAN2TXFG8 0x01F8
#define  CAN2TXFG9 0x01F9
#define  CAN2TXFGA 0x01FA
#define  CAN2TXFGB 0x01FB
#define  CAN2TXFGC 0x01FC
#define  CAN2TXFGD 0x01FD
#define  CAN2TXFGE 0x01FE
#define  CAN2TXFGF 0x01FF

#define  CAN3CTL0  0x0200    
#define  CAN3CTL1  0x0201    
#define  CAN3BTR0  0x0202    
#define  CAN3BTR1  0x0203   
#define  CAN3RFLG  0x0204    
#define  CAN3RIER  0x0205    
#define  CAN3TFLG  0x0206    
#define  CAN3TIER  0x0207    
#define  CAN3TARQ  0x0208    
#define  CAN3TAAK  0x0209    
#define  CAN3TBSEL 0x020A    
#define  CAN3IDAC  0x020B    

#define  CAN3RXERR 0x020E    
#define  CAN3TXERR 0x020F    
#define  CAN3IDAR0 0x0210    
#define  CAN3IDAR1 0x0211    
#define  CAN3IDAR2 0x0212    
#define  CAN3IDAR3 0x0213    
#define  CAN3IDMR0 0x0214    
#define  CAN3IDMR1 0x0215    
#define  CAN3IDMR2 0x0216    
#define  CAN3IDMR3 0x0217    
#define  CAN3IDAR4 0x0218    
#define  CAN3IDAR5 0x0219    
#define  CAN3IDAR6 0x021A    
#define  CAN3IDAR7 0x021B    
#define  CAN3IDMR4 0x021C    
#define  CAN3IDMR5 0x021D    
#define  CAN3IDMR6 0x021E    
#define  CAN3IDMR7 0x021F 

#define  CAN3RXFG0 0x0220
#define  CAN3RXFG1 0x0221
#define  CAN3RXFG2 0x0222
#define  CAN3RXFG3 0x0223
#define  CAN3RXFG4 0x0224
#define  CAN3RXFG5 0x0225
#define  CAN3RXFG6 0x0226
#define  CAN3RXFG7 0x0227
#define  CAN3RXFG8 0x0228
#define  CAN3RXFG9 0x0229
#define  CAN3RXFGA 0x022A
#define  CAN3RXFGB 0x022B
#define  CAN3RXFGC 0x022C
#define  CAN3RXFGD 0x022D
#define  CAN3RXFGE 0x022E
#define  CAN3RXFGF 0x022F
#define  CAN3TXFG0 0x0230
#define  CAN3TXFG1 0x0231
#define  CAN3TXFG2 0x0232
#define  CAN3TXFG3 0x0233
#define  CAN3TXFG4 0x0234
#define  CAN3TXFG5 0x0235
#define  CAN3TXFG6 0x0236
#define  CAN3TXFG7 0x0237
#define  CAN3TXFG8 0x0238
#define  CAN3TXFG9 0x0239
#define  CAN3TXFGA 0x023A
#define  CAN3TXFGB 0x023B
#define  CAN3TXFGC 0x023C
#define  CAN3TXFGD 0x023D
#define  CAN3TXFGE 0x023E
#define  CAN3TXFGF 0x023F

#define  PTT       0x0240    
#define  PTIT      0x0241    
#define  DDRT      0x0242    
#define  RDRT      0x0243    
#define  PERT      0x0244    
#define  PPST      0x0245    

#define  PTS       0x0248    
#define  PTIS      0x0249    
#define  DDRS      0x024A    
#define  RDRS      0x024B    
#define  PERS      0x024C    
#define  PPSS      0x024D    
#define  WOMS      0x024E    

#define  PTM       0x0250    
#define  PTIM      0x0251    
#define  DDRM      0x0252    
#define  RDRM      0x0253    
#define  PERM      0x0254    
#define  PPSM      0x0255    
#define  WOMM      0x0256    

#define  PTP       0x0258    
#define  PTIP      0x0259    
#define  DDRP      0x025A    
#define  RDRP      0x025B    
#define  PERP      0x025C    
#define  PPSP      0x025D    
#define  PIEP      0x025E    
#define  PIFP      0x025F    
#define  PTH       0x0260    
#define  PTIH      0x0261    
#define  DDRH      0x0262    
#define  RDRH      0x0263    
#define  PERH      0x0264    
#define  PPSH      0x0265    
#define  PIEH      0x0266  
#define  PIFH      0x0267    
#define  PTJ       0x0268    
#define  PTIJ      0x0269    
#define  DDRJ      0x026A    
#define  RDRJ      0x026B    
#define  PERJ      0x026C    
#define  PPSJ      0x026D    
#define  PIEJ      0x026E    
#define  PIFJ      0x026F    

#define  CAN4CTL0  0x0280    
#define  CAN4CTL1  0x0281    
#define  CAN4BTR0  0x0282    
#define  CAN4BTR1  0x0283    
#define  CAN4RFLG  0x0284    
#define  CAN4RIER  0x0285    
#define  CAN4TFLG  0x0286    
#define  CAN4TIER  0x0287    
#define  CAN4TARQ  0x0288    
#define  CAN4TAAK  0x0289    
#define  CAN4TBSEL 0x028A    
#define  CAN4IDAC  0x028B    

#define  CAN4RXERR 0x028E    
#define  CAN4TXERR 0x028F    
#define  CAN4IDAR0 0x0290    
#define  CAN4IDAR1 0x0291    
#define  CAN4IDAR2 0x0292    
#define  CAN4IDAR3 0x0293    
#define  CAN4IDMR0 0x0294    
#define  CAN4IDMR1 0x0295    
#define  CAN4IDMR2 0x0296    
#define  CAN4IDMR3 0x0297    
#define  CAN4IDAR4 0x0298    
#define  CAN4IDAR5 0x0299    
#define  CAN4IDAR6 0x029A    
#define  CAN4IDAR7 0x029B    
#define  CAN4IDMR4 0x029C    
#define  CAN4IDMR5 0x029D    
#define  CAN4IDMR6 0x029E    
#define  CAN4IDMR7 0x029F  

#define  CAN4RXFG0 0x02A0
#define  CAN4RXFG1 0x02A1
#define  CAN4RXFG2 0x02A2
#define  CAN4RXFG3 0x02A3
#define  CAN4RXFG4 0x02A4
#define  CAN4RXFG5 0x02A5
#define  CAN4RXFG6 0x02A6
#define  CAN4RXFG7 0x02A7
#define  CAN4RXFG8 0x02A8
#define  CAN4RXFG9 0x02A9
#define  CAN4RXFGA 0x02AA
#define  CAN4RXFGB 0x02AB
#define  CAN4RXFGC 0x02AC
#define  CAN4RXFGD 0x02AD
#define  CAN4RXFGE 0x02AE
#define  CAN4RXFGF 0x02AF
#define  CAN4TXFG0 0x02B0
#define  CAN4TXFG1 0x02B1
#define  CAN4TXFG2 0x02B2
#define  CAN4TXFG3 0x02B3
#define  CAN4TXFG4 0x02B4
#define  CAN4TXFG5 0x02B5
#define  CAN4TXFG6 0x02B6
#define  CAN4TXFG7 0x02B7
#define  CAN4TXFG8 0x02B8
#define  CAN4TXFG9 0x02B9
#define  CAN4TXFGA 0x02BA
#define  CAN4TXFGB 0x02BB
#define  CAN4TXFGC 0x02BC
#define  CAN4TXFGD 0x02BD
#define  CAN4TXFGE 0x02BE
#define  CAN4TXFGF 0x02BF



/*********************************************** 
Constants for MSCAN
************************************************
*/

#define CAN0      0x0140  /* First address for MSCAN0                    */
#define CAN1      0x0180  /* First address for MSCAN1                    */
#define CAN2      0x01C0  /* First address for MSCAN2                    */
#define CAN3      0x0200  /* First address for MSCAN3                    */
#define CAN4      0x0280  /* First address for MSCAN4                    */

#define CANCTL0   0X00    /* Control register 0                          */
#define CANCTL1   0X01    /* Control register 1                          */
#define CANBTR0   0X02    /* Bus timing register 0                       */
#define CANBTR1   0X03    /* Bus timing register 1                       */
#define CANRFLG   0X04    /* Receiver flag register                      */
#define CANRIER   0X05		/* Enable interruptions when reception    		 */	 
#define CANTFLG   0X06    /* Transmit flag register                      */
#define CANTIER   0X07		/* Enable interruptions when transmission 		 */	 
#define CANTBSEL  0X0A    /* Select foreground buffer for transmit       */
#define CANIDAC   0x0B    /* Identifier acceptance control register      */
#define CANIDAR0  0x10    
#define CANIDAR1  0x11    
#define CANIDAR2  0x12    
#define CANIDAR3  0x13    
#define CANIDMR0  0x14    
#define CANIDMR1  0x15    
#define CANIDMR2  0x16    
#define CANIDMR3  0x17    
#define CANIDAR4  0x18    
#define CANIDAR5  0x19    
#define CANIDAR6  0x1A    
#define CANIDAR7  0x1B    
#define CANIDMR4  0x1C    
#define CANIDMR5  0x1D    
#define CANIDMR6  0x1E    
#define CANIDMR7  0x1F  

/* Received foreground buffer */
#define CANRCVID  0x20    /* Base of identifier registers                */
#define CANRCVDTA 0x24    /* Base of data       registers                */
#define CANRCVLEN 0x2C    /* Data length        register                 */

/* Transmit foreground buffer */
#define CANTRSID  0x30    /* Base of identifier registers                */
#define CANTRSDTA 0x34    /* Base of data       registers                */
#define CANTRSLEN 0x3C    /* Data length        register                 */
#define CANTRSPRI 0x3D    /* Priority           register                 */    	


/* Constants for serials port */
#define  SCI0      0x00C8
#define  SCI1      0x00D0 

#define  SCIBDH    0x00   
#define  SCIBDL    0x01    
#define  SCICR1    0x02    
#define  SCICR2    0x03    
#define  SCISR1    0x04    
#define  SCISR2    0x05    
#define  SCIDRH    0x06    
#define  SCIDRL    0x07 

	   
#endif  /* __HCS12_PORTS__  */	   
		   
		   
		   
		   
		   
		   
		   
		   
		   
		   
		   
