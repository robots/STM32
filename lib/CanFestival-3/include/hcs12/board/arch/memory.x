
/* L'espace mémoire du MCS12DP256 en "Normal single ship */
/* ----------------------------------------------------- */
/* 0X0000 - 0X0399 : Registres
   0X0400 - 0X0FFF : 4KO EEPROM
   0X1000 - 0X3FFF : 12KO RAM
   0X4000 - 0XFFFF : 48KO FLASH (accès par pages jusau'à 256 KO 
                     entre 0X8000 et 0XBFFF)

 francis.dupin@inrets.fr                   
*/   

/*
----------------------------------------------------------------
Programme dans la flash, données dans la RAM interne
----------------------------------------------------------------
*/

/* Fixed definition of the available memory banks.
   See generic emulation script for a user defined configuration.  */
MEMORY
{
  /* Registres softs utilisés par gcc. Ne jamais modifier */
  page0 (rwx) : ORIGIN = 0x0, LENGTH = 256

  /* Programme placé en flash, de 4000 à FF00-1 */
  /* (Les vecteurs d'interruption sont en flash entre 0XFF00 et 0XFFFF */
  text  (rx)  : ORIGIN = 0x4000, LENGTH = 0xFF00 - 0x4000
 
  /* Données en RAM (12k), de 0X1000 à 0X3FFF*/
  data        : ORIGIN = 0x1000, LENGTH = 0x4000 - 0x1000
}
/* Setup the stack on the top of the data memory bank.  */
PROVIDE (_stack = 0x3FFF);


vectors_addr = 0xff00;


