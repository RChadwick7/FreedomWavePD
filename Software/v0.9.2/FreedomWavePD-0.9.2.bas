
   '
   '  **********************************
   '  *  Parkinson's Gloves Ver 0.9.2  *
   '  *         FreedomWavePD          *
   '  *   (c) 2023 Charles Bailey      *
   '  *  Compiled with Bascom 2.0.8.6  *
   '  **********************************
   '
' ATMega328 has 3 timers. 0 and 2 are 8 bit, 1 is 16 bit

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
$hwstack = 48                                               'Set the capacity of the hardware stack.
$swstack = 80                                               'Set the capacity of the software stack.
$framesize = 80                                             'Set the capacity of the frame area.

$LIB "I2C_TWI.LBX"

Config RND = 32
CONFIG BASE = 0 ' This makes arrays start at 0 instead of 1





CONFIG WATCHDOG = 2048

Start Watchdog
Reset Watchdog

'Config Minute/Hour timer
'Each period of 166.66ms x 36 = 1 minute, almost exactly

Dim PeriodCount as Byte
Dim MinuteTimer as Byte

Dim ReadRegister as Byte
Dim ConfigFinger as Byte
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
Dim Jitter as Word ' Jitter is +/- 20ms. 40ms total.0 to 624! This is used in timing loop.
Dim JitterSetting as Word ' This is the jitter value set.
Dim Jitterms as Byte      ' This is the jitter value set, in ms. Used for menu.
Dim PulseStartWJitter as Word

' These variables control realtime function
Dim TreatmentOn as Bit

 Reset Watchdog

' Jitter values range from 0 to 625
JitterSetting =  625

MaxOn = 3 * 8' It's multiplied by two as this gets run at starting AND stopping pulse
MaxOff = 2 * 3 ' Same reason. This works, not sure why (Remember 66ms!!)
MaxOff = MaxOff + 1 ' This makes timing right for the off period
OnCount = 0
OffCount = 0

PulseStart = 35  ' Start of pulse. Jitter is only ADDED to this, so not critical.
PulseLength = 1562 ' This is 100ms (1526)
PeriodLength = 2604 ' This is 166.6ms (2604) (Multiply ms by 15.624 to get time value.
PulseStarting = 1

'Config Random Variables
Dim ArrayLength as Byte
Dim ArrayLengthMinusOne as Byte
Dim i as Byte
Dim j as Byte
Dim k as Byte
Dim RandomMax as byte
Dim Finger(8) as Byte
Dim holdi as Byte
Dim Holdj as byte

Dim NeedRandom as Bit
Dim FingerCount as Byte
FingerCount = 0

Dim LoopArray as Byte
Dim LoopArray2 as Byte

' Variables for Menu
'-------------------------------------
ArrayLength = 4  ' 0 to 3                This determines if we are doing 4 fingers mirrored or 8 fingers
ArrayLengthMinusOne = ArrayLength - 1
' We set the initial array numbers:
Finger(0) = 0
Finger(1) = 1
Finger(2) = 2
Finger(3) = 3
Finger(4) = 4
Finger(5) = 5
Finger(6) = 6
Finger(7) = 7

' For the Encoder
Dim B As Byte
Dim skip_flag as Byte

Dim ChipType as String * 8

Reset Watchdog

' ---------------------------------------------
'  * PCF8574 I2C LCD Adapter settings *
'----------------------------------------------
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
'-------------------------------

Config PortD = output  ' Set PortD for fingers output



'-------------------------------------------------------
' Here we make a more random seed using the ADC
'
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


' **************************************************************************************************
' Here we do hardware tests

' Test the PCF8574 LCD - Beep 5 times if it fails.
Reset Watchdog
LCD "Testing PCF8574"
                  i2cstart
                  i2cWByte &H4E  ' This selects the PCF8574
                  If Err <> 0 Then Gosub Beep5Times  ' Error reading the PCF8574
                  i2cstop
Cls


' Test the TCA9548A I2C Multiplexer - This will eventually be able to be disabled for devices without the TCA9548A
'
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




' Test each DRV2605L. BASIC tests, we can do better tests during config and autocalibrate.
' TO DO - Add selecting lines 0-7 to verify those lines. Select a line, see if the input of
' the DRV2605L changes.

For LoopArray = 1 to 8
Reset Watchdog
LCD "Testing DRV2605L" ; LoopArray
                  i2cstart
                  i2cWByte &HE0 ' This selects the TCA9548A
                  i2cWByte LoopArray ' This selects  the channel of the TCA9548A
                  i2cstop
                  Gosub TestDrvChip
                  Cls

Next LoopArray

'***********************************************************************************************************

'-----------------------------------------------------------
'-----------------------------------------------------------
' Title Screen
Cls
Lcd "FreedomWavePD "
Lowerline
Lcd "Ver1.9.2 "
Lcd "*"
Lcd  ___rseed
Lcd "*"

Reset Watchdog




Gosub StartupSound


'
' Set up the Menu
'
Dim Menu(12) as String * 20
Dim MenuChoice as Byte
Menu(1) = "First Menu Opt  "
Menu(2) = "Second Menu Opt "
Menu(3) = "Third Menu Opt  "
Menu(4) = "Fourth Menu Opt "
Menu(5) = "Fifth Menu Opt  "
Menu(6) = "Sixth Menu Opt  "
Menu(7) = "Seventh Menu Opt"
Menu(8) = "Eighth Menu Opt "
Menu(9) = "Ninth Menu Opt  "
Menu(10) = "Tenth Menu Opt  "
Menu(11) = "Eleventh Menu Op"

' --------------------------------------------------

Gosub ProgramChips ' We configure each of the ERM/LRA driver chips
Reset Watchdog

' --------------------------------------------------

' Here we configure the timers

Compare1B = PulseStart       ' The first time we don't bother with jitter
Compare1A = PeriodLength

Config Timer1 = Timer , Prescale = 1024  , Clear_timer = 1
On Compare1B StartPulse

Enable Compare1a
Enable Compare1b
Enable Interrupts
Start Timer1

Reset Watchdog



'---------------------------------------------------
'***************************************************
MainLoop:

' If we need to run random, we do it here

'Start Timer1 ' This is just for testing!

Reset Watchdog

   If NeedRandom = 1 then
      gosub MakeRandom
      NeedRandom = 0
      EndIf
' Here we check counter to see if a minute has passed
 If PeriodCount > 17 Then        ' The magic number is 18 (Was 36)
 CLS
    PeriodCount = 0
   Incr MinuteTimer
   LCD "Minutes: " ; MinuteTimer
    If MinuteTimer > 90 then
     CLS
     LCD " Treatment Done  "
     Lowerline
     LCD "  Wait 2 hours"
     Stop Timer1
     Gosub CompletedSound
    EndIf
 EndIf

      goto mainloop

'***************************************************



'--------------------------------------------------------
' Create the 4 or 8 random sequence of fingers
'--------------------------------------------------------
 MakeRandom:


' We need a start:
         Finger(0) = 0
         Finger(1) = 1
         Finger(2) = 2
         Finger(3) = 3
         Finger(4) = 4
         Finger(5) = 5
         Finger(6) = 6
         Finger(7) = 7

 RunRandom:
         For i = ArrayLength -1 to 1 step - 1   ' For an ArrayLength of 4, this coulnts backwards from 3 to 1
            j = RND(i)

            holdi = Finger(i)
            holdj = Finger(j)
            Finger(i) = holdj
            Finger(j) = holdi

         Next i
If LastFinger = Finger(0) then goto RunRandom    ' If the last played finger will be the same as the next first finger

 LastFinger = Finger(ArrayLengthMinusOne)

' Now we print the result for testing purposes
'for k = 0 to ArrayLength - 1
'lcd Finger(k) ;
'next k
'lcd ""
'waitms 100

      Return


'***************************************************
' Here are the ISR routines
'***************************************************
'
 StartPulse:
' Gosub MakeJitter
' PulseStartWJitter = PulseStart ' Put it here temporarily for testing
'If OnCount < MaxOn and PulseStarting = 1 Then  ' If we are on the ON Cycle
If OnCount < MaxOn Then  ' If we are on the ON Cycle
OffCount = 0
 Incr OnCount
' Incr PeriodCount

         If PulseStarting = 1 Then   ' We turn on the selected pins
            FirstHand = Finger(FingerCount)
            PortD.FirstHand = 1
            SecondHand = FirstHand + 4
' This is where we figure out how to mirror the hands
'       right       left
'         0   +4     4
'         1   +4     5
'         2   +4     6
'         3   +4     7
'
'         0 on goto  7
'         1          6
'         2          5
'         3          4



' We can EVENTUALLY add the below lines, conditional on mirroring.
'Select Case FirstHand
'           Case 1 : SecondHand = 8
'           Case 2 : SecondHand = 7
'           Case 3 : SecondHand = 6
'           Case 4 : SecondHand = 5
'End Select

' I THINK this is right, the above didn't work. TEST! Enable when selected in menu.
'Select Case FirstHand
'           Case 0 : SecondHand = 7
'           Case 1 : SecondHand = 6
'           Case 2 : SecondHand = 5
'           Case 3 : SecondHand = 4
'End Select


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
            FingerCount = FingerCount + 1

            ArrayLengthMinusOne = ArrayLength - 1
            If FingerCount = ArrayLength then   ' If we have gone through all fingers
               Fingercount = 0                         ' We start over
               NeedRandom = 1                          ' We need a new random finger sequence
            EndIf
         EndIf
Else     ' If we are on the OFF cycle

  If OffCount < MaxOff Then  ' If we are still on the OFF Cycle
   Incr OffCount

  Else
   Incr PeriodCount ' This counts to a minute
   Oncount = 0
  EndIf
EndIf

Return




' *****************************************************
' * Here we read the encoder MENU - GOES IN MAIN LOOP *
' *****************************************************


               PortC = &B11                                             ' activate pull up registers
               skip_flag = 1

'    ****************
               Locate 1, 1
               LCD "Parkinson's Test"
               Locate 2, 1
               LCD "Ver. 0.11       "

               Do

                  B = Encoder(pinC.0 , PinC.1 , Left , Right , 0)

  '                                               ^--- 1 means wait for change which blocks programflow

  '                               ^--------^---------- labels which are called

  '              ^-------^---------------------------- port PINs

'  WaitMS 5

                  Debounce PinC.2 , 0 , ButtonPush , Sub
'Debounce PinC.2 , 1 , ButtonUnPush , Sub

'                    ^----- label to branch to

'               ^---------- Branch when PIND.0 goes low(0)

'         ^---------------- Examine PIND.0








               Loop
               End

               ButtonPush:
                  Locate 2,1
                  LCD "Button Pushed"
               Return

'ButtonUnPush:
'Locate 2,1
'LCD "              "
'Return

               Left:
               If skip_flag = 1 then
                  skip_flag = 0
                  Return
               End if
               Locate 1 , 1
               Decr MenuChoice
               If MenuChoice < 1 then MenuChoice = 1
               LCD Menu(MenuChoice)
               Locate 2, 10
               LCD "       "
               Locate 2, 1
               LCD "MenuChoice " + STR(MenuChoice)
               skip_flag = 1
               Return

               Right:
               If skip_flag = 1 then
                  skip_flag = 0
                  Return
               End if
               Locate 1 , 1
               Incr MenuChoice
               If MenuChoice > 11 then MenuChoice = 11
               LCD Menu(MenuChoice)
               Locate 2, 10
               LCD "       "
               Locate 2, 1
               LCD "MenuChoice " + STR(MenuChoice)

               skip_flag = 1
               Return

               End

' Menu Choices
'
' ****************
' Start Treastment
' Treatment Time     (2 Hours)
' Pattern           (Mirrored Not mirrored, repeating and slowly changing
' Random Jitter %    (25%)
' Inbetween time
' Burst Length       (100ms)









' -------------------------------------------------------------------
' Set up the output chips
ProgramChips:

Reset Watchdog
  wait 1 ' This is to give the driver chips a chance to boot up
' We select the correct chip on the TCA9548A
Reset Watchdog


                  ConfigFinger = 1
                  i2cstart  ' Channel 1
                  portc.5 = 1
                  Portc.4 = 1
                  i2cWByte &HE0 ' This selects the TCA9548A
                  i2cWByte &H01 ' This selects channel 1 of the TCA9548A
                  i2cstop
                  Gosub ChipSetup
Reset Watchdog
                  ConfigFinger = 2
                  i2cstart ' Channel 2
                  portc.5 = 1
                  Portc.4 = 1
                  i2cWByte &HE0 ' This selects the TCA9548A
                  i2cWByte &H02 ' This selects channel 2 of the TCA9548A
                  i2cstop
                  Gosub ChipSetup
Reset Watchdog
                  ConfigFinger = 3
                  i2cstart ' Channel 3
                  portc.5 = 1
                  Portc.4 = 1
                  i2cWByte &HE0 ' This selects the TCA9548A
                  i2cWByte &H04 ' This selects channel 3 of the TCA9548A
                  i2cstop
                  Gosub ChipSetup
 Reset Watchdog
                  ConfigFinger = 4
                  i2cstart ' Channel 4
                  portc.5 = 1
                  Portc.4 = 1
                  i2cWByte &HE0 ' This selects the TCA9548A
                  i2cWByte &H08 ' This selects channel 4 of the TCA9548A
                  i2cstop
                  Gosub ChipSetup
 Reset Watchdog
                  ConfigFinger = 5
                  i2cstart  ' Channel 5
                  portc.5 = 1
                  Portc.4 = 1
                  i2cWByte &HE0 ' This selects the TCA9548A
                  i2cWByte &H10 ' This selects channel 5 of the TCA9548A
                  i2cstop
                  Gosub ChipSetup
Reset Watchdog
                  ConfigFinger = 6
                  i2cstart  ' Channel 6
                  portc.5 = 1
                  Portc.4 = 1
                  i2cWByte &HE0 ' This selects the TCA9548A
                  i2cWByte &H20 ' This selects channel 6 of the TCA9548A
                  i2cstop
                  Gosub ChipSetup
Reset Watchdog
                  ConfigFinger = 7
                  i2cstart  ' Channel 7
                  portc.5 = 1
                  Portc.4 = 1
                  i2cWByte &HE0 ' This selects the TCA9548A
                  i2cWByte &H40 ' This selects channel 7 of the TCA9548A
                  i2cstop
                  Gosub ChipSetup
Reset Watchdog
                  ConfigFinger = 8
                  i2cstart  ' Channel 8
                  portc.5 = 1
                  Portc.4 = 1
                  i2cWByte &HE0 ' This selects the TCA9548A
                  i2cWByte &H80 ' This selects channel 8 of the TCA9548A
                  i2cstop
                  Gosub ChipSetup
Reset Watchdog
               Return





'------------------------------------------------------------
' Here we calculate the jitter for each finger
'------------------------------------------------------------

MakeJitter:

'cls
PulseStartWJitter = RND(JitterSetting)
'LCD "Jitter:";PulseStartWJitter


Return



ChipSetup:  ' THIS particular routine is for LRA
'  DRV2605L Address is B4
' Whatever done here gets sent to ALL 8 chips
Reset Watchdog
' FIRST, We Autocalibrate
' Here we set register 0x16, the voltage register
  I2CStart
  I2CWbyte &HB4'              Transmit to the chip
  I2CWbyte &H16'              RatedVoltage Register
  I2CWbyte &H53'              Sets voltage to 2V         (I got 95 Decimal, 5F Hex. Hmmm...)
  I2CStop

' Here we set register 0x17, the overvoltage register
  I2CStart
  I2CWbyte &HB4'              Transmit to the chip
  I2CWbyte &H17'              Overdrive Clamp Register
  I2CWbyte &H89'              Sets voltage to 3V Peak
  I2CStop

  ' Here we set register 0x1A, the Feedback Control register
  I2CStart
  I2CWbyte &HB4'              transmit to the chip
  I2CWbyte &H1A'              RatedVoltage Register
  I2CWbyte &HB6'              Sets Register - 10110110 - LRA, BrakeFactor, LoopGain, BEMFGain
  I2CStop

  ' Here we set register 0x1B   ?????????????
  I2CStart
  I2CWbyte &HB4'              transmit to the chip
  I2CWbyte &H1B'              RatedVoltage Register
  I2CWbyte &H93'              Sets voltage to 2V
  I2CStop

  ' Here we set register 0x1C   ?????????????
  I2CStart
  I2CWbyte &HB4'              transmit to the chip
  I2CWbyte &H1C'              RatedVoltage Register
  I2CWbyte &HF5'              Sets voltage to 2V
  I2CStop

  ' Here we set register 0x1D   ?????????????
  I2CStart
  I2CWbyte &HB4'              transmit to the chip
  I2CWbyte &H1D'              RatedVoltage Register
  I2CWbyte &H80'              Sets voltage to 2V
  I2CStop

  ' Here we set register 0x01, to Autocalibrate
  I2CStart
  I2CWbyte &HB4'              transmit to the chip
  I2CWbyte &H01'              Mode Register
  I2CWbyte &H07'              Sets bit 7 to 1, autocalibrate
  I2CStop

    ' Here we set register 0x0C, to GO
  I2CStart
  I2CWbyte &HB4'              transmit to the chip
  I2CWbyte &H0C'              GO Register
  I2CWbyte &H01'              Sets GO Register to 1, start Autocalibration
  I2CStop
Reset Watchdog
' Now we read the registers
 DO
' Here we poll the GO BIT for 0, indicating autocalibration is complete
  I2CStart
  I2CWbyte &HB4'              transmit to the chip
  I2CWbyte &H0C'              GO Register
  I2CRepStart
  I2CWbyte &HB5
  I2CRbyte ReadRegister , nack'
  I2CStop
Reset Watchdog

Loop until ReadRegister.0 = 0

' Now we check to make sure autocalibrate didn't fail:
  I2CStart
  I2CWbyte &HB4'              transmit to the chip
  I2CWbyte &H00'              Status Register
  I2CRepStart
  I2CWbyte &HB5
  I2CRbyte ReadRegister , nack'
  I2CStop

  If ReadRegister.2 = 0 then
   lcd "*"
  Else
   Cls
   LCD "Finger "
   LCD  ConfigFinger
   LCD " Failed!"
   LCD "Please Check Wiring"
  Endif


'---------------
' NOW, we Initialize the chip

' LAST HURDLE!!!!! WE NEED TO SET TO PWM MODE!!!!!!!!!!

    ' Here we set Control1 register 0x1B
  I2CStart
  I2CWbyte &HB4'              ???
  I2CWbyte &H1B'              ???
  I2CWbyte &H13'              ???
  I2CStop

    ' Here we set Control2 register 0x1C
  I2CStart
  I2CWbyte &HB4'              ???
  I2CWbyte &H1C'              ???
  I2CWbyte &HF5'              \
  I2CStop

    ' Here we set Control3 register 0x1D
  I2CStart
  I2CWbyte &HB4'              ???
  I2CWbyte &H1D'              ???
  I2CWbyte &H80'              ???
  I2CStop

    ' Here we set the library 0x03
  I2CStart
  I2CWbyte &HB4'              transmit to the chip
  I2CWbyte &H03'              Library Selection
  I2CWbyte &H80'              Selects the LRA Library
  I2CStop

    ' Here we set the Mode register 0x01
  I2CStart
  I2CWbyte &HB4'              transmit to the chip
  I2CWbyte &H01'              Mode Register
  I2CWbyte &H03'              Sets Mode Register to 2, external trigger,
  I2CStop

return


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



CompletedSound:
'Completed sound
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
LCD "Error" ; ReadRegister
Waitms 500
End If

LCD "Chip " ; LoopArray ; " " ; ChipType
Waitms 400

Reset Watchdog


Return