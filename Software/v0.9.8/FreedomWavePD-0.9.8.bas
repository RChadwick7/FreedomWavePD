
   '
   '  **********************************
   '  *  Parkinson's Gloves Ver 0.9.8  *
   '  *         FreedomWavePD          *
   '  *   (c) 2023 Charles Bailey      *
   '  *  Compiled with Bascom 2.0.8.6  *
   '  **********************************
   '
' ATMega328 has 3 timers. 0 and 2 are 8 bit, 1 is 16 bit. Fingers use Timer1.

' Fingers are arranged this way:
'    Left Hand       Right Hand
'      0  1  2  3  4  5  6  7

' 0 is left pinky
' 1 is left ring
' 2 is left middle
' 3 is left index
' 4 is right index
' 5 is right middle
' 6 is right ring
' 7 is right pinky
'
' 8 is left thumb
' 9 is right thumb

$programmer = 22                                            'ARDUINO (using stk500v1 protocol)
$regfile = "m328pdef.dat"                                   'Set the AVR to use - ATMega328.
$crystal = 16000000                                         'Set the AVR clock.
   '
$hwstack = 120                                               'Set the capacity of the hardware stack.
$swstack = 120                                               'Set the capacity of the software stack.
$framesize = 180                                             'Set the capacity of the frame area.

Config RND = 32

UCSR0B = 0 ' Disables the serial port, left enabled by some bootloaders



'###############################################################################
'##             V A R I A B L E S  T O  S A V E  T O  E E P R O M             ##
'###############################################################################



'
 Dim AutoStart as Byte      'Not a mistake, bit values have to be bytes in the menu system
 Dim Treatment_time as Byte
 Dim Vib_Intensity as Byte
 Dim JitterSetting as Word
 Dim HandMirror as Byte     'Not a mistake, bit values have to be bytes in the menu system.
 Dim ERM_LRA_Setting as String * 3
 Dim OL_CL_Setting as String * 6
 Dim Drive_Voltage as Single
 Dim LRA_OPEN_LOOP  as Byte 'Not a mistake, bit values have to be bytes in the menu system
 Dim ERM_OPEN_LOOP as Byte  'Not a mistake, bit values have to be bytes in the menu system

 Dim Is_LRA as Byte         'Not a mistake, bit values have to be bytes in the menu system
 Dim Is_OL as Byte          'Not a mistake, bit values have to be bytes in the menu system

' Dim Temp_Voltage as Single
 ' ------------------------------------------------------------------------------------------------


'###############################################################################
'##                    D I M E N S I O N  V A R I A B L E S                   ##
'###############################################################################

'Config Minute/Hour timer
'Each period of 166.66ms x 36 = 1 minute, almost exactly

 Dim PeriodCount as Byte
 Dim MinuteTimer as Byte
 Dim SWVer as String *  5
 Dim ReadRegister as Byte

Config PortD = output  ' Set PortD for fingers output

' This turns off the output so the Init routine vibrations will stop when done.
PORTD = &H00

 Dim ___rseed as word

'Config Timing Variables
 Dim PulseStart As Word
 Dim PulseLength As Word
 Dim PulseStarting as bit
 Dim PeriodLength as Word
 Dim FirstHand as Byte
 Dim SecondHand as Byte
 Dim OnCount as Byte  ' The number of cycles the 4 finger pattern repeats. Standard is 3
 Dim OffCount as Byte ' The number of cycles of 4 finger patterns to rest. Standard is 2
 Dim MaxOn as Byte
 Dim MaxOff as Byte
 Dim LastFinger as Byte

' Jitter is +/- 20ms. 40ms total.0 to 624! This is used in timing loop..
 Dim PulseStartWJitter as Word

'Config Random Variables
 Dim ArrayLength as Byte
 Dim ArrayLengthPlusOne as Byte
 Dim i as Byte
 Dim j as Byte
 Dim Finger(8) as Byte
 Dim holdi as Byte
 Dim Holdj as byte

 Dim NeedRandom as Bit
 Dim FingerCount as Byte

 Dim LoopArray as Byte

 Dim i2cvalue1 as Byte
 Dim i2cvalue2 as Byte
 Dim i2cvalue3 as Byte

 Dim ChipType as String * 8
 Dim AdcVal As Word         ' This is to detect the Autostart jumper
 Dim TreatmentActive as Bit ' If this is a 1, then we are actively doing treatment, or should be

 Dim LRA_Frequency as Word
 Dim RATED_VOLTAGE as Byte
 Dim OD_CLAMP as Byte
 Dim OL_LRA_PERIOD as Byte       ' Make this a single?
 Dim Temp2 as Single
 Dim Temp3 as Single


 Dim Old_Is_LRA as Byte
 Dim Old_Is_OL  As Byte
 Dim Old_Vib_Intensity as Byte
 Dim Old_Drive_Voltage as Single
 Dim Old_LRA_Frequency as Word


 Dim SAMPLE_TIME as Word


 Dim Menu_value_active_old As Bit ' Used by menu system to determine when menu changed something

'###############################################################################
'##                   S E T  V A R I O U S   V A R I A B L E S                ##
'###############################################################################

' ------------------
   SWVer = "0.9.8"        '< UPDATE THIS WITH EACH VERSION
' ------------------

FingerCount = 1
Sample_Time = 300

MaxOn = 3 * 8' It's multiplied by two as this gets run at starting AND stopping pulse
MaxOff = 2 * 3 ' Same reason. This works, not sure why (Remember 66ms!!)
MaxOff = MaxOff + 1 ' This makes timing right for the off period
OnCount = 0
OffCount = 0

PulseStart = 35  ' Start of pulse. Jitter is only ADDED to this, so not critical.
PulseLength = 1562 ' This is 100ms (1526)
PeriodLength = 2604 ' This is 166.6ms (2604) (Multiply ms by 15.624 to get time value.
PulseStarting = 1

ArrayLength = 4  ' 0 to 3                This determines if we are doing 4 fingers mirrored or 8 fingers
ArrayLengthPlusOne = ArrayLength + 1

'NeedRandom = 1   ' Logically it should be here, but output different, not necessarily better.

Config PortD = output  ' Set PortD for fingers output

' We set the initial array numbers:
        Finger(1) = 0
        Finger(2) = 1
        Finger(3) = 2
        Finger(4) = 3
        Finger(5) = 4
        Finger(6) = 5
        Finger(7) = 6
        Finger(8) = 7

'###############################################################################
'##           C O N F I G U R E   R O T A R Y   E N C O D E R                 ##
'###############################################################################

Encoder_switch Alias PinC.2
Dim B As Byte
Dim Encoder_switch_old As Bit
Dim Encoder_turn_left As Byte , Encoder_turn_right As Byte
PortC.0 = 1                                              ' pullup encoder a,
PortC.1 = 1                                              ' pullup encoder b,
PortC.2 = 1                                              ' pullup encoder switch
Dim skip_flagL as byte    ' This is used for this specific encoder. Otherwise, one notch registers as two!
Dim skip_flagR as byte    ' This is used for this specific encoder. Otherwise, one notch registers as two!


'###############################################################################
'##                C O N F I G U R E   M E N U   T I M E R                    ##
'###############################################################################

Const Ticker_hwtimer = 2                                    ' Choose which hardware timer to use
Const Ticker_frequency = 1000                               ' set the timer resolution
Const Tickers = 2                                           ' # of software timers to use
$include "tickers.inc"
Const Timer_readswitches = 1
Const Timer_valueupdate = 2


'###############################################################################
'##                       C O N F I G U R E   L C D                           ##
'###############################################################################

$lib "i2c_twi.lib"                                       'Incorporate the hardware I2C/TWI library.
Config Twi = 100000                                      'I2C bus clock = 100KHz
Config Scl = Portc.5                                     'You must specify the SCL pin name.
Config Sda = Portc.4                                     'You must specify the SDA pin name.
I2cinit                                                  'Initialize the SCL and SDA lines of the I2C bus.
Reset Watchdog
Dim Pcf8574_lcd As Byte : Pcf8574_lcd = &H4E             'PCF8574 slave address. (&H40,&H42,&H44,&H46,&H48,&H4A,&H4C,&H4E)
Dim Backlight As Byte : Backlight = 1                    'LCD backlight control. (0: off, 1: on)
$lib "lcd_i2c_PCF8574.LIB"                               'Incorporate the library of I2C LCD PCF8574 Adapter.
Config Lcd = 16x2                                        'Set the LCD to 16 characters and 2 lines.
Initlcd                                                  'Initialize the LCD.


'###############################################################################
'##                     C R E A T E   R A N D O M   S E E D                   ##
'###############################################################################

CONFIG ADC = Single, PRESCALER = Auto,  REFERENCE = Avcc

Reset Watchdog

Dim Adc_word as word
Dim Bit_num as Byte
Dim Rnd_word as word

For I = 1 To 72
Reset Watchdog
   For Bit_num = 0 To 7
      Start ADC
      Adc_word = Getadc(Bit_num)
      Stop ADC
      Rotate Rnd_word , Left
      Rnd_word = Rnd_word Xor Adc_word
   Next
Next


___rseed = Rnd_word


'###############################################################################
'##                     C O N F I G U R E  T I M E R S                        ##
'###############################################################################

Compare1B = PulseStart       ' The first time we don't bother with jitter
Compare1A = PeriodLength

Config Timer1 = Timer , Prescale = 1024  , Clear_timer = 1
On Compare1B StartPulse

Enable Compare1a
Enable Compare1b
Enable Interrupts
Stop Timer1
Reset Watchdog


'###############################################################################
'##                    C O N F I G U R E  W A T C H D O G                     ##
'###############################################################################

CONFIG WATCHDOG = 2048

Start Watchdog
Reset Watchdog


'###############################################################################
'##      L C D   M E N U                                                      ##
'###############################################################################

Macro Menu_include_data

' include the data file created with the menu designer
 $Include "PARKINSONS_menu_data4.inc"
End Macro

'Include just the Menu.inc
$include "menu.inc"

'###############################################################################
'##    M E N U    V A R I A B L E S                                           ##
'###############################################################################

Dim Test_string As String * Menu_value_string_width         ' max string length is stored in a constant
Dim Tempbyte As Byte


'###############################################################################
'##      M E N U   I N I T                                                    ##
'###############################################################################

Encoder_switch_old = Encoder_switch

Ticker_time(timer_readswitches) = 20                        ' 20 ms debounce
Ticker_enabled(timer_readswitches) = True
Ticker_time(timer_valueupdate) = 500                        ' 500 ms read-only value update
Ticker_enabled(timer_valueupdate) = True

Menu_init


'###############################################################################
'##                  Run System Self Check of All Devices                     ##
'###############################################################################

 Gosub TestSystem


'###############################################################################
'##  P R O G R A M   C H I P S   F R O M   S A V E D   M E N U   V A L U E S  ##
'###############################################################################

 Gosub NewSetupChips


'###############################################################################
'##                         S T A R T U P   S C R E E N                       ##
'###############################################################################

Cls
Lcd " FreedomWavePD "
Lowerline
Lcd "Ver" ; SWVer ; " "
Lcd "*"
Lcd  ___rseed
Lcd "*"

Reset Watchdog

Gosub StartupSound


'###############################################################################
'##                    D E T E C T   A U T O S T A R T                        ##
'###############################################################################

CONFIG ADC = Single, PRESCALER = Auto,  REFERENCE = INTERNAL_1.1
AdcVal = Getadc(6)

   If AdcVal < 10 Then           ' Autostart jumper is in place
     AutoStart = 1
   End If

ADMUX.REFS1 = 0                   ' Switches ADC reference to Avcc



   If AutoStart = 1 Then
     TreatmentActive = 1
     Gosub StartTreatment
   End If


 Config PortD = output           ' Set PortD for fingers output


'########################################################################################################
'##                                         M A I N   L O O P                                          ##
'########################################################################################################


MainLoop:

  Gosub SetSavedVars             ' We set the ERM/LRA and OPEN/CLOSED Loop variables


' Here we check to see if variables were changed from the menu. If so, we run the NewSetupChips routine
  If Is_LRA <> Old_Is_LRA or Is_OL <> Old_Is_OL or Vib_Intensity <> Old_Vib_Intensity or Drive_Voltage <> Old_Drive_voltage or LRA_Frequency <> Old_LRA_Frequency Then ' If there was any change in the menu
    Gosub NewSetupChips

  Old_Is_LRA = Is_LRA
  Old_Is_OL = Is_OL
  Old_Vib_Intensity = Vib_Intensity
  Old_Drive_Voltage = Drive_Voltage
  Old_LRA_Frequency = LRA_Frequency
  End If

  Reset Watchdog

   If NeedRandom = 1 then
      Gosub MakeRandom
      NeedRandom = 0
   EndIf

' Here we check counter to see if a minute has passed
If TreatmentActive = 1 Then
 If PeriodCount > 17 Then        ' The magic number is 18 (Was 36)

    PeriodCount = 0
    Incr MinuteTimer
    Gosub Draw_homescreen

 EndIf
EndIf

   Menu
Reset Watchdog

' Here we handle the encoder

  B = Encoder(pinC.0 , PinC.1 , Left , Right , 0)

  WaitMS 1    ' Helps to debounce


  '     Here we select what to do with each menu choice

   Select Case Ticker_get_interrupt()
   Case Timer_readswitches:
      ' encoder switch pressed?
      If Encoder_switch = True And Encoder_switch_old = False Then
         Tempbyte = Menu_enter()
         Select Case Tempbyte
         Case Menu_exit:                                    ' menu closed
            Gosub Draw_homescreen

            Case 2:
            ' Starts Treatment
         If TreatmentActive = 1 Then
            CLS
            LCD " Already Active"
         Else
            Cls
            LCD "Treatment Start"
            TreatmentActive = 1
            Gosub StartTreatment

            Reset Watchdog
            Waitms 400
            Reset Watchdog
            Waitms 400
            Reset Watchdog
            Waitms 400
            Reset Watchdog
            Waitms 400

            Test_string = "Executed"                        ' assign the byte variable with a value
            Gosub Draw_homescreen
   '           Gosub DrawOperationScreen

            Menu_show 1
         End If

            Case 3:
            ' Stops Treatment

             Gosub StopTreatment
           '  Gosub CompletedSound
             TreatmentActive = 0

            Reset Watchdog
            Waitms 400
            Reset Watchdog
            Waitms 400
            Reset Watchdog
            Waitms 400
            Reset Watchdog
            Waitms 400
            Reset Watchdog
            Waitms 400
            Reset Watchdog
            Waitms 400
            Reset Watchdog
            Waitms 400
            Reset Watchdog
            Waitms 400
            Test_string = "Executed"                        ' assign the byte variable with a value

            Menu_show 1


            Case 4:
            ' Shows ABOUT Screen
            Cls
             LCD " FreedomWavePD"
             Lowerline
             LCD " Version " ; SWVer
            Reset Watchdog
            Waitms 400
            Reset Watchdog
            Waitms 400
            Reset Watchdog
            Waitms 400
            Reset Watchdog
            Waitms 400
            Reset Watchdog
            Waitms 400
            Reset Watchdog
            Waitms 400
            Reset Watchdog
            Waitms 400
            Reset Watchdog
            Waitms 400
            Test_string = "Executed"                        ' assign the byte variable with a value

            Menu_show 1



            Case 9
            ' Loads Default Settings, in case someone really screws up settings.
            Gosub SetDefaults
            Cls
            LCD "Defaults Loaded"
            Waitms 400
            Reset Watchdog
            Waitms 400
            Reset Watchdog
            Waitms 400
            Reset Watchdog
            Waitms 400
            Test_string = "Executed"                        ' assign the byte variable with a value

            Menu_show 1



         End Select
      End If



 ' ----------
      Encoder_switch_old = Encoder_switch

      ' encoder turns left
      If 0 < Encoder_turn_left Then
         Decr Encoder_turn_left
         Menu_backward
      End If

      ' encoder turns right
      If 0 < Encoder_turn_right Then
         Decr Encoder_turn_right
         Menu_forward
      End If
' -----------

   Case Timer_valueupdate
      ' force the read-only value currently displayed to update
      Menu_check_update

   End Select


Goto MainLoop



'*****************************************************************************************
'**                                S U B R O U T I N E S                                **
'*****************************************************************************************



'###############################################################################
'##      D R A W  T H E  H O M E  S C R E E N                                 ##
'###############################################################################



Draw_homescreen:   ' Makesw the main screen. Varies depending on if treatment is going on

If TreatmentActive = 1  Then ' If therapy is going on make a menu that shows minutes left
   CLS
   Cursor Off
   Locate 1 , 1
   LCD "Minutes: " ; MinuteTimer
   Lowerline
   LCD "Treatment Active"
    If MinuteTimer  => Treatment_time  Then    ' Here we check to see if treatment time is up.
      Gosub StopTreatment
    EndIf

Else

   Cls
   Cursor Off
   Locate 1 , 1
   Lcd " FreedomWavePD"
   Locate 2 , 1
   LCD " Version " ; SWVer
End If
Return

DrawOperationScreen:     ' What is this? Do we need it? Can we use it?



Return


'###############################################################################
'##                        Handle the Rotary Encoder                          ##
'###############################################################################

Left:
               If skip_flagL = 1 then
                  skip_flagL = 0
               Else
                  Incr Encoder_turn_left
                  skip_flagL = 1
               End if

Return

Right:
               If skip_flagR = 1 then
                  skip_flagR = 0
               Else
                  Incr Encoder_turn_right
                  skip_flagR = 1
               End if
Return



'###############################################################################
'##        Set default values in case settings get really messed up           ##
'###############################################################################

SetDefaults:
' Sets the default values

 AutoStart = 0
 Treatment_time    = 120    'This sets 2 hours
 Vib_Intensity     = 100
 JitterSetting     = 625
 HandMirror        = 1
 ERM_LRA_Setting   = "LRA"
 Drive_Voltage     = 1.5
 LRA_OPEN_LOOP     = 0
 ERM_OPEN_LOOP     = 0
 LRA_Frequency     = 250

Return



'###############################################################################
'##             Create the 4 or 8 random sequence of fingers                  ##
'###############################################################################
 MakeRandom:


' We need a start:
        Finger(1) = 0
        Finger(2) = 1
        Finger(3) = 2
        Finger(4) = 3
        Finger(5) = 4
        Finger(6) = 5
        Finger(7) = 6
        Finger(8) = 7



 Do
         For i = ArrayLength to 2 step - 1   ' For an ArrayLength of 4, this counts backwards from 3 to 1
            j = RND(i)
            Incr j
            holdi = Finger(i)
            holdj = Finger(j)
            Finger(i) = holdj
            Finger(j) = holdi

         Next i
 Reset Watchdog
 Loop until LastFinger <> Finger(1)    ' If the last played finger will be the same as the next first finger, do again

 LastFinger = Finger(ArrayLength)


 Return


'###############################################################################
'##           Here is the Interrupt Service Request (ISR) routine             ##
'###############################################################################
'

 StartPulse:

If OnCount < MaxOn Then  ' If we are on the ON Cycle
OffCount = 0
 Incr OnCount
' Incr PeriodCount

      If PulseStarting = 1 Then   ' We turn on the selected finger on the first hand
            FirstHand = Finger(FingerCount)
            PortD.FirstHand = 1
            SecondHand = FirstHand + 4

' Here we select how we mirror the second hand

         If HandMirror = 1 Then
            Select Case FirstHand
             Case 0 : SecondHand = 7
             Case 1 : SecondHand = 6
             Case 2 : SecondHand = 5
             Case 3 : SecondHand = 4
            End Select
         Else
            Select Case FirstHand
             Case 0 : SecondHand = 4
             Case 1 : SecondHand = 5
             Case 2 : SecondHand = 6
             Case 3 : SecondHand = 7
            End Select
         Endif

                ' Here we turn on the specific finger on the second hand
            PORTD.SecondHand = 1
            COMPARE1B = PulseStartWJitter + PulseLength  ' Setting up timer to turn off pin
            PulseStarting = 0

      Else     ' We turn off the selected pins

            FirstHand = Finger(FingerCount)
            PortD.FirstHand = 0        ' We turn off the selected pins
            SecondHand = FirstHand + 4
            PORTD.SecondHand = 0
            Gosub MakeJitter ' We need a better place for this!
            COMPARE1B = PulseStartWJitter ' Setting up timer for the next turn on. ADD JITTER HERE.
            PulseStarting = 1
            Incr FingerCount

            If FingerCount = ArrayLengthPlusOne Then  ' If we have gone through all fingers
               Fingercount = 1                         ' We start over
               NeedRandom = 1                          ' We need a new random finger sequence
            EndIf

      EndIf
Else           ' If we are on the OFF cycle

  If OffCount < MaxOff Then  ' If we are still on the OFF Cycle
   Incr OffCount

  Else
   Incr PeriodCount ' This counts to a minute
   Oncount = 0
  EndIf
EndIf
 Reset Watchdog
Return


'###############################################################################
'##                    Here is where we START treatment                       ##
'###############################################################################

StartTreatment:

            OnCount = 0
            OffCount = 0
            TreatmentActive = 1
            MinuteTimer = 0
            PulseStarting = 1       ' If set to 0, weird problems with pulses not turning off.
            FingerCount = 1
            Reset Watchdog
            Start Timer1 ' This starts treatment
            Gosub StartupSound
            Reset Watchdog

Return




'###############################################################################
'##                     Here is where we STOP treatment                       ##
'###############################################################################

StopTreatment:

            Cls
            LCD " Treatment Done  "
            Lowerline
            LCD "  Wait 2 hours":

            TreatmentActive = 0
            MinuteTimer = 0
            Reset Watchdog
            Stop Timer1   ' This stops treatment
            PORTD = &H00  ' Thismakes sure no fingers are left vibrating when stopped
            PeriodCount = 0
            MinuteTimer = 0
'            PulseStarting = 0
            FingerCount = 1
            OnCount = 0
            OffCount = 0
            Gosub CompletedSound
            Reset Watchdog


Return



'###############################################################################
'##                Here we calculate the jitter for each finger               ##
'###############################################################################

MakeJitter:

PulseStartWJitter = RND(JitterSetting)
Incr PulseStartWJitter

Return


'###############################################################################
'##               Here are routines to make various sounds                    ##
'###############################################################################


'Startup Sound
StartupSound:
Reset Watchdog
Sound portc.3, 255, 600
  waitms 20
Sound portc.3, 255, 500
  waitms 20
Reset Watchdog
Sound portc.3, 265, 400
  waitms 20
Sound portc.3, 280, 300
Reset Watchdog
Return


'Completed sound
CompletedSound:
Reset Watchdog
Sound portc.3, 370, 300
  waitms 20
Sound portc.3, 380, 400
  waitms 20
Reset Watchdog
Sound portc.3, 390, 500
  waitms 20
Sound portc.3, 400, 600
  waitms 20
Reset Watchdog
Sound portc.3, 370, 300
  waitms 20
Sound portc.3, 380, 400
  waitms 20
Reset Watchdog
Sound portc.3, 390, 500
  waitms 20
Sound portc.3, 400, 600
Reset Watchdog
Return

' LCD Test Error
Beep5Times:
Reset Watchdog
Sound portc.3, 900, 300
  wait 1
Reset Watchdog
Sound portc.3, 900, 300
  wait 1
Reset Watchdog
Sound portc.3, 900, 300
  wait 1
Reset Watchdog
Sound portc.3, 900, 300
  wait 1
Reset Watchdog
Sound portc.3, 900, 300
  wait 1
Reset Watchdog

Return




'###############################################################################
'## Here we program all the driver chips with i2cvaluex data                  ##
'##                                                                           ##
'##   i2cvalue1  This should always be B4, the address of the DRV2605L        ##
'##   i2cvalue2  Selects the specific register                                ##
'##   i2cvalue3  Write this value to the register                             ##
'###############################################################################

SetChips:



Reset Watchdog

                  i2cstart  ' Channel 1
                  i2cWByte &HE0        ' This selects the TCA9548A
                  i2cWByte &B00000001  ' This selects channel 1 of the TCA9548A
                  i2cstop
                  i2cstart
                  i2cWByte i2cvalue1
                  i2cWByte i2cvalue2
                  i2cWByte i2cvalue3
                  i2cstop


                  i2cstart  ' Channel 2
                  i2cWByte &HE0        ' This selects the TCA9548A
                  i2cWByte &B00000010  ' This selects channel 2 of the TCA9548A
                  i2cstop
                  i2cstart
                  i2cWByte i2cvalue1
                  i2cWByte i2cvalue2
                  i2cWByte i2cvalue3
                  i2cstop


                  i2cstart  ' Channel 3
                  i2cWByte &HE0        ' This selects the TCA9548A
                  i2cWByte &B00000100  ' This selects channel 3 of the TCA9548A
                  i2cstop
                  i2cstart
                  i2cWByte i2cvalue1
                  i2cWByte i2cvalue2
                  i2cWByte i2cvalue3
                  i2cstop

                  i2cstart  ' Channel 4
                  i2cWByte &HE0        ' This selects the TCA9548A
                  i2cWByte &B00001000  ' This selects channel 4 of the TCA9548A
                  i2cstop
                  i2cstart
                  i2cWByte i2cvalue1
                  i2cWByte i2cvalue2
                  i2cWByte i2cvalue3
                  i2cstop

                  i2cstart  ' Channel 5
                  i2cWByte &HE0        ' This selects the TCA9548A
                  i2cWByte &B00010000  ' This selects channel 5 of the TCA9548A
                  i2cstop
                  i2cstart
                  i2cWByte i2cvalue1
                  i2cWByte i2cvalue2
                  i2cWByte i2cvalue3
                  i2cstop

                  i2cstart  ' Channel 6
                  i2cWByte &HE0        ' This selects the TCA9548A
                  i2cWByte &B00100000  ' This selects channel 6 of the TCA9548A
                  i2cstop
                  i2cstart
                  i2cWByte i2cvalue1
                  i2cWByte i2cvalue2
                  i2cWByte i2cvalue3
                  i2cstop

                  i2cstart  ' Channel 7
                  i2cWByte &HE0        ' This selects the TCA9548A
                  i2cWByte &B01000000  ' This selects channel 7 of the TCA9548A
                  i2cstop
                  i2cstart
                  i2cWByte i2cvalue1
                  i2cWByte i2cvalue2
                  i2cWByte i2cvalue3
                  i2cstop

                  i2cstart  ' Channel 8
                  i2cWByte &HE0        ' This selects the TCA9548A
                  i2cWByte &B10000000  ' This selects channel 8 of the TCA9548A
                  i2cstop
                  i2cstart
                  i2cWByte i2cvalue1
                  i2cWByte i2cvalue2
                  i2cWByte i2cvalue3
                  i2cstop
 Reset Watchdog
Return



'###############################################################################
'##                        H A R D W A R E   T E S T S                        ##
'###############################################################################

TestSystem:

' Test the PCF8574 LCD - Beep 5 times if it fails.
Reset Watchdog
LCD "Testing PCF8574"
                  i2cstart
                  i2cWByte &H4E  ' This selects the PCF8574
                  If Err <> 0 Then Gosub Beep5Times  ' Error reading the PCF8574
                  i2cstop
Cls
'----------

' Test the TCA9548A I2C Multiplexer - This will eventually be able to be disabled for devices without the TCA9548A

' First we write to the TCA9548A register
        For LoopArray = 0 to 7
        Reset Watchdog
        LCD "Testing TCA9548A" ; LoopArray
                  i2cstart
                  i2cWByte &HE0 ' This selects the TCA9548A to WRITE
                  i2cWByte LoopArray ' This selects channel 1 of the TCA9548A
                  i2cstop
' Now we read the register
                  i2cstart
                  i2cWByte &HE1 ' This selects the TCA9548A to READ
                  I2CRbyte ReadRegister , nack'
                  i2cstop
                  Cls
                  If ReadRegister <> LoopArray Then
                    LCD "Error 1." ; ReadRegister ; "." ; LoopArray
                    wait 1
                  End If
              '    wait 1
        Next LoopArray

'----------

' Test each DRV2605L. BASIC tests, we can do better tests during config and autocalibrate.
' TO DO - Add selecting lines 0-7 to verify those lines. Select a line, see if the input of
' the DRV2605L changes.


Reset Watchdog

LCD "Testing DRV2605L 1"
                  i2cstart
                  i2cWByte &HE0 ' This selects the TCA9548A
                  i2cWByte &B00000001  ' This selects channel 1 of the TCA9548A
                  i2cstop
                  Gosub TestDrvChip
                  Cls

LCD "Testing DRV2605L 2"
                  i2cstart
                  i2cWByte &HE0 ' This selects the TCA9548A
                  i2cWByte &B00000010  ' This selects channel 1 of the TCA9548A
                  i2cstop
                  Gosub TestDrvChip
                  Cls

LCD "Testing DRV2605L 3"
                  i2cstart
                  i2cWByte &HE0 ' This selects the TCA9548A
                  i2cWByte &B00000100  ' This selects channel 1 of the TCA9548A
                  i2cstop
                  Gosub TestDrvChip
                  Cls

LCD "Testing DRV2605L 4"
                  i2cstart
                  i2cWByte &HE0 ' This selects the TCA9548A
                  i2cWByte &B00001000  ' This selects channel 1 of the TCA9548A
                  i2cstop
                  Gosub TestDrvChip
                  Cls

LCD "Testing DRV2605L 5"
                  i2cstart
                  i2cWByte &HE0 ' This selects the TCA9548A
                  i2cWByte &B00010000  ' This selects channel 1 of the TCA9548A
                  i2cstop
                  Gosub TestDrvChip
                  Cls

LCD "Testing DRV2605L 6"
                  i2cstart
                  i2cWByte &HE0 ' This selects the TCA9548A
                  i2cWByte &B00100000  ' This selects channel 1 of the TCA9548A
                  i2cstop
                  Gosub TestDrvChip
                  Cls

LCD "Testing DRV2605L 7"
                  i2cstart
                  i2cWByte &HE0 ' This selects the TCA9548A
                  i2cWByte &B01000000  ' This selects channel 1 of the TCA9548A
                  i2cstop
                  Gosub TestDrvChip
                  Cls

LCD "Testing DRV2605L 8"
                  i2cstart
                  i2cWByte &HE0 ' This selects the TCA9548A
                  i2cWByte &B10000000  ' This selects channel 1 of the TCA9548A
                  i2cstop
                  Gosub TestDrvChip
                  Cls

Return


'###############################################################################
'##                     D R I V E R   C H I P   T E S T S                     ##
'###############################################################################

TestDrvChip:
'This is where we test each DRV2605L
' For now, just read 7-5 status register, make sure chip is correct

Reset Watchdog


   I2CStart
   I2CWbyte &HB4'              transmit to the chip
   I2CWbyte &H00'              Status Register
   I2CRepStart
   I2CWbyte &HB5
   I2CRbyte ReadRegister , nack'
   I2CStop

Cls

ReadRegister = ReadRegister And &b11100000 ' Get the top 3 bits, which show what chip it is
Shift ReadRegister , Right , 5

Reset Watchdog


' 3 = DRV2605 (contains licensed ROM library, does not contain RAM)
' 4 = DRV2604 (contains RAM, does not contain licensed ROM library)
' 6 = DRV2604L (low-voltage version of the DRV2604 device)
' 7 = DRV2605L (low-voltage version of the DRV2605 device)


If ReadRegister = 7 Then
ChipType = "DRV2605L"
ElseIf ReadRegister = 6 then
ChipType = "DRV2604L"
ElseIf ReadRegister = 4 then
ChipType = "DRV2604"
ElseIf ReadRegister = 3 then
ChipType = "DRV2605"
Else
LCD "Error2-" ; ReadRegister
Waitms 500
End If


Reset Watchdog


Return


'###############################################################################
'##    This takes the values from the menu and configures the driver chips    ##
'###############################################################################
NewSetupChips:
'


      If ERM_LRA_Setting = "LRA" Then ' If it's LRA




        If OL_CL_Setting = "OPEN" Then ' Here we do Open Loop LRA -------------------------

          Gosub CalcLRA_OL_Freq

          ' Now we need to SET the LRA_PERIOD register

         i2cvalue1 = &HB4            ' This should always be B4, the address of the DRV2605L
         i2cvalue2 = &H20            ' Selects the OL_LRA_PERIOD
         i2cvalue3 = OL_LRA_PERIOD   ' Write this value to the register
         Gosub SetChips             ' This sends the config to all driver chips

          Gosub CalcLRA_OL_Volt

          ' Now we need to Set the OD_CLAMP register

         i2cvalue1 = &HB4        ' This should always be B4, the address of the DRV2605L
         i2cvalue2 = &H17        ' Selects the OD_CLAMP
         i2cvalue3 = OD_CLAMP    ' Write this value to the register
         Gosub SetChips

          ' Now we need to run Autocalibrate

         i2cvalue1 = &HB4'              transmit to the chip
         i2cvalue2 = &H01'              Mode Register
         i2cvalue3 = &H07'              Sets bit 7 to 1, autocalibrate
         Gosub SetChips

         i2cvalue1 = &HB4'              transmit to the chip
         i2cvalue2 = &H0C'              GO Register
         i2cvalue3 = &H01'              Sets GO Register to 1, start Autocalibration
         Gosub SetChips

          ' We set LRA Mode in the driver IC
         i2cvalue1 = &HB4       ' This should always be B4, the address of the DRV2605L
         i2cvalue2 = &H1A       ' Selects the . Bit 7 is ERM/LRA
         i2cvalue3 = &HB6       ' We write a 1 to select LRA   WAS 01, changed to B6
         Gosub SetChips


          ' Here we set the library 0x03

         i2cvalue1 = &HB4'              transmit to the chip
         i2cvalue2 = &H03'              Library Selection
         i2cvalue3 = &H80'              Selects the LRA Library
         Gosub SetChips

'         ' Here we set the Mode register 0x01

         i2cvalue1 = &HB4'              transmit to the chip
         i2cvalue2 = &H01'              Mode Register
         i2cvalue3 = &H03'              Sets Mode Register to 2, external trigger,
         Gosub SetChips


        Else ' Here we do Closed Loop LRA Voltage  ----------------------------------------
          ' Now we need to Set the RATED_VOLTAGE

          Gosub CalcLRA_CL_Volt

         i2cvalue1 = &HB4'              transmit to the chip
         i2cvalue2 = &H16'              RATED_VOLTAGE Register
         i2cvalue3 = RATED_VOLTAGE'


         Gosub SetChips

          ' Now we need to set the OD_CLAMP to twice RATED_VOLTAGE

         Gosub CalcLRA_OD_CLAMP

         i2cvalue1 = &HB4'              transmit to the chip
         i2cvalue2 = &H17'              OD_CLAMP Register
         i2cvalue3 = OD_CLAMP'          Set OD_CLAMP
         Gosub SetChips



          ' Here we set the library 0x03

         i2cvalue1 = &HB4'              transmit to the chip
         i2cvalue2 = &H03'              Library Selection
         i2cvalue3 = &H80'              Selects the LRA Library
         Gosub SetChips

'         ' Here we set the Mode register 0x01

         i2cvalue1 = &HB4'              transmit to the chip
         i2cvalue2 = &H01'              Mode Register
         i2cvalue3 = &H02'              Sets Mode Register to 2, external trigger,
         Gosub SetChips

          ' We set LRA Mode in the driver IC
         i2cvalue1 = &HB4       ' This should always be B4, the address of the DRV2605L
         i2cvalue2 = &H1A       ' Selects the . Bit 7 is ERM/LRA
         i2cvalue3 = &HB6       ' We write a 1 to select LRA   WAS 01, changed to B6
         Gosub SetChips


        End If

     ' These are done for LRA, either open or closed loop:

          ' Here we set the library 0x03

         i2cvalue1 = &HB4'              transmit to the chip
         i2cvalue2 = &H03'              Library Selection
         i2cvalue3 = &H80'              Selects the LRA Library
         Gosub SetChips

          ' Now we need to run Autocalibration

         i2cvalue1 = &HB4'              transmit to the chip
         i2cvalue2 = &H01'              Mode Register
         i2cvalue3 = &B00000111'        Sets byte to 7, autocalibrate
         Gosub SetChips

         i2cvalue1 = &HB4'              transmit to the chip
         i2cvalue2 = &H0C'              GO Register
         i2cvalue3 = &H01'              Sets GO Register to 1, start Autocalibration
         Gosub SetChips


      Else ' We have ERM -------------------------------------------------------------------

          ' Re set the driver IC to ERM mode

         i2cvalue1 = &HB4       ' This should always be B4, the address of the DRV2605L
         i2cvalue2 = &H1A       ' Selects the . Bit 7 is ERM/LRA
         i2cvalue3 = &H00       ' We write a 0 to select ERM
         Gosub SetChips

          ' Need to set OD_CLAMP

         Gosub CalcERM_OL_Volt

         i2cvalue1 = &HB4'              transmit to the chip
         i2cvalue2 = &H17'              OD_CLAMP Register
         i2cvalue3 = OD_CLAMP'          Set OD_CLAMP
         Gosub SetChips

          ' Now we need to run Autocalibrate

         i2cvalue1 = &HB4'              transmit to the chip
         i2cvalue2 = &H01'              Mode Register
         i2cvalue3 = &H07'              Sets bit 7 to 1, autocalibrate
         Gosub SetChips

         i2cvalue1 = &HB4'              transmit to the chip
         i2cvalue2 = &H0C'              GO Register
         i2cvalue3 = &H01'              Sets GO Register to 1, start Autocalibration
         Gosub SetChips

          ' Here we set the Mode register 0x01

         i2cvalue1 = &HB4'              transmit to the chip
         i2cvalue2 = &H01'              Mode Register
         i2cvalue3 = &H02'              Sets Mode Register to 2, external trigger,
         Gosub SetChips


      End If

' Now we configre everything that is common to all modes

          ' Here we set the Mode register 0x01

         i2cvalue1 = &HB4'              transmit to the chip
         i2cvalue2 = &H01'              Mode Register
         i2cvalue3 = &H02'              Sets Mode Register to 3, External Trigger. Can try 1 and 3.
         Gosub SetChips


Return



'###############################################################################
'##       Here we do the calculations for various driver chip settings        ##
'###############################################################################

' Calculates the LRA Open Loop Frequency.
CalcLRA_OL_Freq: ' Inputs LRA_Frequency and outputs frequency as OL_LRA_PERIOD (7 Bits)

  Temp2 = LRA_Frequency * 0.00009849
  Temp2 = 1 / Temp2
  Temp2 = INT(Temp2)
  If Temp2 > 127 then   ' We make sure the size isn't bigger than 7 bits
     Temp2 = 127
  End If

  OL_LRA_PERIOD = Temp2


Return


' ----------------------------------------
' Calculates the LRA Open Loop Voltage.
CalcLRA_OL_Volt:   ' Inputs Drive_Voltage and outputs OD_CLAMP (Byte)

  Temp3 = Drive_Voltage / 0.02132
  Temp3 = Temp3 * Temp3
  Temp2 = LRA_Frequency * 0.000800
  Temp2 = 1 - Temp2
  Temp3 = Temp3 / Temp2
  Temp2 = SQR(Temp3)
  OD_CLAMP = INT(Temp2)


' We need to Multiply Vib_Intensity into this
  Temp2 = Vib_Intensity * 0.01
  Temp2 = OD_CLAMP * Temp2
  OD_CLAMP = INT(Temp2)


Return


' ----------------------------------------
' Calculates the LRA Closed Loop Voltage.
CalcLRA_CL_Volt: ' Inputs DRIVE_Voltage and outputs RATED_VOLTAGE (Byte)

  Temp2 = 4 * Sample_Time
  Temp2 = Temp2 + 300
  Temp2 = Temp2 * 0.000001
  Temp2 = 1 - Temp2
  Temp2 = SQR(Temp2)
  Temp2 = DRIVE_Voltage * Temp2
  Temp2 = Temp2 / 0.02058
  RATED_VOLTAGE = INT(Temp2)

' We need to Multiply Vib_Intensity into this
  Temp2 = Vib_Intensity * 0.01
  Temp2 = RATED_VOLTAGE * Temp2
  RATED_VOLTAGE = INT(Temp2)


Return

' ----------------------------------------
' Calculates the ERM Open Loop Voltage.
CalcERM_OL_Volt:  ' Inputs DRIVE_Voltage and outputs OD_CLAMP(Byte)     This runs when it should

  Temp2 = DRIVE_Voltage / 0.02159
  OD_CLAMP = INT(Temp2)

' We need to Multiply Vib_Intensity into this
  Temp2 = Vib_Intensity * 0.01
  Temp2 = OD_CLAMP * Temp2
  OD_CLAMP = INT(Temp2)


Return


' ----------------------------------------
' Calculates the LRA CLOSED Loop Overdrive Voltage.
CalcLRA_OD_CLAMP:  ' Inputs DRIVE_Voltage and outputs OD_CLAMP(Byte)      *********** RUNS WHEN LRA CLOSED  OR OPEN!!!

  Temp2 = DRIVE_VOLTAGE / 0.02122
  Temp2 = Temp2 * 1.8  ' This isn't part of the normal formula. This sets the Clamp voltage at about 1.8 times drive voltage
  OD_CLAMP = INT(Temp2)


Return


'###############################################################################
'##      S E T  T H E  E R M / L R A  A N D  L O O P   V A R I A B L E S      ##
'###############################################################################

SetSavedVars:
' Here we set LRA and Open Loop variables. Wish this could be more efficient

   If Is_LRA = 1 Then
     ERM_LRA_Setting = "LRA"
   Else
     ERM_LRA_Setting = "ERM"
   End If

   If Is_OL = 1 Then
     OL_CL_Setting = "OPEN"
   Else
     OL_CL_Setting = "CLOSED"
   End If

Return