$nocompile
'###############################################################################
'##                                                                           ##
'##                        L C D   B A R   G R A P H                          ##
'##                       ===========================                         ##
'##                                                                           ##
'##  Description: Draws a horizontal bar graph with variable size on a        ##
'##               LC display. You can choose between 5 different styles       ##
'##               or add your own.                                            ##
'##               The function accepts values from 0 to 100 (byte).           ##
'##                                                                           ##
'##  Author:      Matthias (at) Braunecker (.at)                              ##
'##  Version:     1.0                                        Date: June 2011  ##
'##                                                                           ##
'##  Free for personal use, credits and a short mail with your project info   ##
'##  and/or comments, improvements etc. to this library would be nice.        ##
'##                                                                           ##
'###############################################################################


' use the default style if not specified
#if Not Varexist( "lcd_bar_style")
   Const Lcd_bar_style = 0
#endif

Declare Sub Lcd_bar(byval Percent As Byte , Byval Chars As Byte)

' init the special characters
Gosub Lcd_bar_setstyle

Goto Lcd_bar_jumpover

' called to define the special characters, useful if you want to use other
' special characters while no bargraph is displayed.
Lcd_bar_setstyle:
   ' Simple straight lines
   #if Lcd_bar_style = 0
      Const Lcd_bar_mode = 0
      Deflcdchar 0 , 32 , 32 , 32 , 21 , 21 , 32 , 32 , 32
      Deflcdchar 1 , 32 , 16 , 16 , 21 , 21 , 16 , 16 , 32
      Deflcdchar 2 , 32 , 8 , 8 , 29 , 29 , 8 , 8 , 32
      Deflcdchar 3 , 32 , 4 , 4 , 21 , 21 , 4 , 4 , 32
      Deflcdchar 4 , 32 , 2 , 2 , 23 , 23 , 2 , 2 , 32
      Deflcdchar 5 , 32 , 1 , 1 , 21 , 21 , 1 , 1 , 32
      Const Lcd_bar_leading = 0
      Const Lcd_bar_trailing = 0
   #endif

   ' Serif
   #if Lcd_bar_style = 1
      Const Lcd_bar_mode = 0
      Deflcdchar 0 , 32 , 32 , 32 , 21 , 21 , 32 , 32 , 32
      Deflcdchar 1 , 24 , 16 , 16 , 21 , 21 , 16 , 16 , 24
      Deflcdchar 2 , 28 , 8 , 8 , 29 , 29 , 8 , 8 , 28
      Deflcdchar 3 , 14 , 4 , 4 , 21 , 21 , 4 , 4 , 14
      Deflcdchar 4 , 7 , 2 , 2 , 23 , 23 , 2 , 2 , 7
      Deflcdchar 5 , 3 , 1 , 1 , 21 , 21 , 1 , 1 , 3
      Const Lcd_bar_leading = 0
      Const Lcd_bar_trailing = 0
   #endif

   ' Arrow
   #if Lcd_bar_style = 2
      Const Lcd_bar_mode = 0
      Deflcdchar 0 , 32 , 32 , 32 , 32 , 32 , 32 , 21 , 21
      Deflcdchar 1 , 32 , 24 , 24 , 16 , 16 , 32 , 21 , 21
      Deflcdchar 2 , 32 , 28 , 28 , 8 , 8 , 32 , 21 , 21
      Deflcdchar 3 , 32 , 14 , 14 , 4 , 4 , 32 , 21 , 21
      Deflcdchar 4 , 32 , 7 , 7 , 2 , 2 , 32 , 21 , 21
      Deflcdchar 5 , 32 , 3 , 3 , 1 , 1 , 32 , 21 , 21
      Const Lcd_bar_leading = 0
      Const Lcd_bar_trailing = 0
   #endif

   ' Block
   #if Lcd_bar_style = 3
      Const Lcd_bar_mode = 0
      Deflcdchar 0 , 32 , 32 , 32 , 32 , 32 , 32 , 32 , 32
      Deflcdchar 1 , 16 , 16 , 16 , 16 , 16 , 16 , 16 , 16
      Deflcdchar 2 , 24 , 24 , 24 , 24 , 24 , 24 , 24 , 24
      Deflcdchar 3 , 28 , 28 , 28 , 28 , 28 , 28 , 28 , 28
      Deflcdchar 4 , 30 , 30 , 30 , 30 , 30 , 30 , 30 , 30
      Deflcdchar 5 , 31 , 31 , 31 , 31 , 31 , 31 , 31 , 31
      Const Lcd_bar_leading = 5
      Const Lcd_bar_trailing = 0
   #endif

   ' Standard Chars
   #if Lcd_bar_style = 4
      Const Lcd_bar_mode = 1
      Const Lcd_bar_char1 = 165
      Const Lcd_bar_char2 = 253
      Const Lcd_bar_char3 = 43
      Const Lcd_bar_char4 = 42
      Const Lcd_bar_char5 = 35
      Const Lcd_bar_leading = 35
      Const Lcd_bar_trailing = 32
   #endif

   Cls
Return

' draws a nice bar graph
Sub Lcd_bar(percent As Byte , Chars As Byte)
   Local Position As Word , Offset As Byte , Index As Byte

   ' calculate the arrow position
   Position = Chars * 5                                     ' resolution
   Position = Position * Percent                            ' calc absolute position
   If Percent = 100 Then Decr Position
   Position = Position / 100
   Offset = Position Mod 5                                  ' relative position
   Offset = Offset + 1
'   If Percent = 100 Then Incr Position
   Position = Position / 5                                  ' character position

   ' draw leading characters
   For Index = 1 To Position
      Lcd Chr(lcd_bar_leading)
   Next

   ' draw the arrow
   #if Lcd_bar_mode = 0
      Lcd Chr(offset)
   #else
      Select Case Offset
      Case 1:
         Lcd Chr(lcd_bar_char1)
      Case 2:
         Lcd Chr(lcd_bar_char2)
      Case 3:
         Lcd Chr(lcd_bar_char3)
      Case 4:
         Lcd Chr(lcd_bar_char4)
      Case 5:
         Lcd Chr(lcd_bar_char5)
      End Select
   #endif

   ' trailing characters
   Position = Position + 2
   For Index = Position To Chars
      Lcd Chr(lcd_bar_trailing)
   Next
End Sub

Lcd_bar_jumpover: