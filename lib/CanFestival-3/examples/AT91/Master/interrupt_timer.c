// ----------------------------------------------------------------------------
//         ATMEL Microcontroller Software Support  -  ROUSSET  -
// ----------------------------------------------------------------------------
// DISCLAIMER:  THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
// DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
// OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
// EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// ----------------------------------------------------------------------------
// File Name           : interrupt_timer.c
// Object              : Timer interrupt management
//                     : Use AT91B_LED7 & AT91B_LED8 for status interrupt
// Creation            : JPP   08-Sep-2005
// ----------------------------------------------------------------------------

// Include Standard LIB  files
#include "config.h"

//* Global variable
int timer0_interrupt = 0;

#define TIMER0_INTERRUPT_LEVEL		1

/*-----------------*/
/* Clock Selection */
/*-----------------*/
#define TC_CLKS                  0x7
#define TC_CLKS_MCK2             0x0
#define TC_CLKS_MCK8             0x1
#define TC_CLKS_MCK32            0x2
#define TC_CLKS_MCK128           0x3
#define TC_CLKS_MCK1024          0x4

//*------------------------- Internal Function --------------------------------
//*----------------------------------------------------------------------------
//* Function Name       : AT91F_TC_Open
//* Object              : Initialize Timer Counter Channel and enable is clock
//* Input Parameters    : <tc_pt> = TC Channel Descriptor Pointer
//*                       <mode> = Timer Counter Mode
//*                     : <TimerId> = Timer peripheral ID definitions
//* Output Parameters   : None
//*----------------------------------------------------------------------------
void AT91F_TC_Open ( AT91PS_TC TC_pt, unsigned int Mode, unsigned int TimerId)
{
  unsigned int dummy;

  //* First, enable the clock of the TIMER
  AT91F_PMC_EnablePeriphClock ( AT91C_BASE_PMC, 1<< TimerId ) ;

  //* Disable the clock and the interrupts
  TC_pt->TC_CCR = AT91C_TC_CLKDIS ;
  TC_pt->TC_IDR = 0xFFFFFFFF ;

  //* Clear status bit
  dummy = TC_pt->TC_SR;
  //* Suppress warning variable "dummy" was set but never used
  dummy = dummy;
  //* Set the Mode of the Timer Counter
  TC_pt->TC_CMR = Mode ;

  //* Enable the clock
  TC_pt->TC_CCR = AT91C_TC_CLKEN ;
}
//*------------------------- Interrupt Function -------------------------------

//*----------------------------------------------------------------------------
//* Function Name       : timer0_c_irq_handler
//* Object              : C handler interrupt function calAT91B_LED by the interrupts
//*                       assembling routine
//* Output Parameters   : increment count_timer0_interrupt
//*----------------------------------------------------------------------------
void timer0_c_irq_handler(void)
{
  AT91PS_TC TC_pt = AT91C_BASE_TC0;
  unsigned int dummy;
  //* AcknowAT91B_LEDge interrupt status
  dummy = TC_pt->TC_SR;
  //* Suppress warning variable "dummy" was set but never used
  dummy = dummy;
  timer0_interrupt = TRUE;
}

//*-------------------------- External Function -------------------------------

//*----------------------------------------------------------------------------
//* Function Name       : timer_init
//* Object              : Init timer counter
//* Input Parameters    : none
//* Output Parameters   : TRUE
//*----------------------------------------------------------------------------
void timer_init (unsigned int time)
{
  //* Open timer0
  AT91F_TC_Open(AT91C_BASE_TC0,TC_CLKS_MCK8 | AT91C_TC_WAVESEL_UP_AUTO,AT91C_ID_TC0);

  //* Open Timer 0 interrupt
  AT91F_AIC_ConfigureIt ( AT91C_BASE_AIC, AT91C_ID_TC0, TIMER0_INTERRUPT_LEVEL,AT91C_AIC_SRCTYPE_INT_HIGH_LEVEL, timer0_c_irq_handler);
  AT91C_BASE_TC0->TC_IER = AT91C_TC_CPCS;  //  IRQ enable CPC
  AT91F_AIC_EnableIt (AT91C_BASE_AIC, AT91C_ID_TC0);

  AT91C_BASE_TC0->TC_RC = (unsigned int)(AT91C_MASTER_CLOCK / 8 * time / 1000);

  //* Start timer0
  AT91C_BASE_TC0->TC_CCR = AT91C_TC_SWTRG ;
}
