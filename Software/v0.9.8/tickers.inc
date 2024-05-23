$nocompile
'###############################################################################
'##                                                                           ##
'##            U N I V E R S A L   S O F T W A R E   T I M E R                ##
'##            ===============================================                ##
'##                                                                           ##
'##  Description: This is an easy-to-use universal software timer library,    ##
'##               for use with any hardware timer, featuring automatic        ##
'##               determination of needed timer parameters on precompile,     ##
'##               up to 256 independant software timers, each can be          ##
'##               enabled/disabled and an interrupt queue if more than one    ##
'##               sw interrupt occured at once.                               ##
'##                                                                           ##
'##  Author:      Matthias (at) Braunecker (.at)                              ##
'##  Version:     1.3                                        Date: July 2009  ##
'##                                                                           ##
'##  Free for personal use, credits and a short mail with your project info   ##
'##  and/or comments, improvements etc. to this library would be nice.        ##
'##                                                                           ##
'###############################################################################

' How to use this (the values are just for example):
' --------------------------------------------------

' Initialisation:
'    $crystal = 16000000
'    [...]                                                       ' your Code
'    Const False = 0: Const True = 1                             ' for better reading

'    Const Ticker_hwtimer = 0                                    ' Choose which hardware timer to use
'    Const Ticker_frequency = 10000                              ' set the timer resolution
'    Const Tickers = 2                                           ' # of software timers to use
'    $include "tickers.bas"
'    Ticker_time(1) = 25                                         ' Interrupt Ticks to trigger SW-Timer 1
'    Ticker_time(2) = 500                                        ' SW-Timer 2 and so on...
'    Ticker_enabled(1) = True                                    ' Enable/Disable SW-Timer
'    Ticker_enabled(2) = False
'    [...]
'    Enable Interrupts

' Main Loop:
'    Do
'       [...]
'       Select Case Ticker_get_interrupt()                       ' make sure that this function is called at least once per
                                                                 ' hw timer overflow, so don't make the µC hanging around
'       Case 1:                                                  ' Ticker 1 triggered an interrupt
'          [...]
'       Case 2:                                                  ' Ticker 2 interrupt
'          [...]
'       End Select
'       [...]
'    Loop

' 1.2:
'     * Added Ticker_systemtime (Variable to count the uptime) and
'       Ticker_use_systemtime (to enable/disable it, standard enabled)


Declare Function Ticker_get_interrupt() As Byte
Declare Sub Ticker_enable(byval Tickernr As Byte)

#if Varexist( "false") = 0
   Const False = 0
#endif
#if Varexist( "true") = False
   Const True = 1
#endif

#if Varexist( "Ticker_use_systemtime") = False
   Const Ticker_use_systemtime = True
#endif

'###############################################################################
'##      T I M E R   S E T T I N G S   C A L C U L A T I O N                  ##
'###############################################################################
' calculation based on a work by "Walter", MCS Forum, added prescaler calculation
Const Ticker_cyclespertick = _xtal / Ticker_frequency                           ' clock cycles needed for desired frequency
#if Ticker_hwtimer = 0                                                          ' Set the timer register size
   Const Ticker_register = 256
#endif
#if Ticker_hwtimer = 1
   Const Ticker_register = 65536
#endif
#if Ticker_hwtimer = 2
   Const Ticker_register = 256
#endif

#if(ticker_cyclespertick / 1) < Ticker_register                                 ' calculate the right prescale value
   Const Ticker_prescale = 1
#else
   #if(ticker_cyclespertick / 8) < Ticker_register
      Const Ticker_prescale = 8
   #else
      #if(ticker_cyclespertick / 64) < Ticker_register
         Const Ticker_prescale = 64
      #else
         #if(ticker_cyclespertick / 256) < Ticker_register
            Const Ticker_prescale = 256
         #else
            #if(ticker_cyclespertick / 1024) < Ticker_register
               Const Ticker_prescale = 1024
            #endif
         #endif
      #endif
   #endif
#endif

Const Ticker_tickfrequency = _xtal / Ticker_prescale                            ' frequency of Timer-Ticks
Const Ticker_ticktime = 1 / Ticker_tickfrequency                                ' time of one Timer-Tick in sec
Const Ticker_desiredtime = 1 / Ticker_frequency                                 ' desired time in sec
Const Ticker_count = Ticker_desiredtime / Ticker_ticktime                       ' number of Ticks for desired time
Const Ticker_preload = Ticker_register - Ticker_count                           ' start-value for the timer


'###############################################################################
'##      V A R I A B L E S                                                    ##
'###############################################################################
Dim Ticker_counter(tickers) As Word                                             ' tick counter
Dim Ticker_time(tickers) As Word                                                ' desired intervall time
Dim Ticker_interrupt(10) As Byte                                                ' interrupt stack
Dim Ticker_pointer As Byte                                                      ' interrupt stack pointer
Dim Ticker_tmp As Byte                                                          ' internal temp variable
Dim Ticker_enabled(tickers) As Byte                                             ' enable/disable ticker
Dim Ticker_newtick As Bit                                                       ' hw timer interrupt flag
#if Ticker_use_systemtime = True
   Dim Ticker_systemtime As Dword
#endif


'###############################################################################
'##      S E T T I N G   U P   H W - T I M E R                                ##
'###############################################################################
#if Ticker_hwtimer = 0
   Ticker_timer Alias Timer0
   Config Ticker_timer = Timer , Prescale = Ticker_prescale
   On Timer0 Ticker_isr Nosave
   Enable Timer0
#endif
#if Ticker_hwtimer = 1
   Ticker_timer Alias Timer1
   Config Ticker_timer = Timer , Prescale = Ticker_prescale
   On Timer1 Ticker_isr Nosave
   Enable Timer1
#endif
#if Ticker_hwtimer = 2
   Ticker_timer Alias Timer2
   Config Ticker_timer = Timer , Prescale = Ticker_prescale
   On Timer2 Ticker_isr Nosave
   Enable Timer2
#endif

Ticker_timer = Ticker_preload
Goto Ticker_runprog                                                             ' don't execute the functions


'###############################################################################
'##      F U N C T I O N S                                                    ##
'###############################################################################
Sub Ticker_enable(byval Tickernr As Byte)                                       ' enables and resets a sw-timer
   Ticker_enabled(tickernr) = True
   Ticker_counter(tickernr) = 0
End Sub

'-------------------------------------------------------------------------------
Function Ticker_get_interrupt() As Byte                                         ' Main function
   If Ticker_newtick = True Then                                                ' hw timer interrupt occured
      Ticker_newtick = False                                                    ' reset the flag
      #if Ticker_use_systemtime = True
         Incr Ticker_systemtime
      #endif
      For Ticker_tmp = 1 To Tickers                                             ' check all sw tickers
         If Ticker_enabled(ticker_tmp) = 1 Then Incr Ticker_counter(ticker_tmp) ' if ticker enabled, increment tick count
         If Ticker_counter(ticker_tmp) = Ticker_time(ticker_tmp) Then           ' tick count has reached desired time
            Incr Ticker_pointer
            Ticker_interrupt(ticker_pointer) = Ticker_tmp                       ' store the interrupt in queue
            Ticker_counter(ticker_tmp) = 0                                      ' reset ticker counter
         End If
      Next
   End If
   If 0 < Ticker_pointer Then                                                   ' interrupt is waiting in queue
      Ticker_get_interrupt = Ticker_interrupt(ticker_pointer)
      Ticker_interrupt(ticker_pointer) = 0                                      ' interrupt processed, delete it
      Decr Ticker_pointer
   Else
      Ticker_get_interrupt = False                                              ' no interrupt waiting
   End If
End Function

'-------------------------------------------------------------------------------
Ticker_isr:                                                                     ' hw timer interrupt service routine
$asm
   push R23
   IN R23, SREG
   push R23
   PUSH R24
   PUSH R25
$end Asm
   Ticker_timer = Ticker_preload                                                ' reset the timer register with calculated preload value
   Ticker_newtick = True                                                        ' set flag
$asm
   POP R25
   pop R24
   POP R23
   Out Sreg , R23
   pop R23
$end Asm
Return

Ticker_runprog: