' LCD Menu Library
' Version 1.5, Nov 2017

$nocompile

#if Not Varexist( "True")
   Const False = 0 : Const True = 1
#endif

Const Menu_debuglevel = 0                                   ' 0: no debug
                                                             ' 1: standard debug
                                                             ' 2: deep debug (not used)
' entry types enumeration
Const Menu_exit = 1
Const Menu_link = 2
Const Menu_value = 3
Const Menu_function = 4

' variable types enumeration
Const Menu_value_bit = 1
Const Menu_value_byte = 2
Const Menu_value_word = 3
Const Menu_value_integer = 4
Const Menu_value_dword = 5
Const Menu_value_long = 6
Const Menu_value_single = 7
Const Menu_value_double = 8
Const Menu_value_string = 9

Const Menu_value_type_mask = &H0F
Const Menu_value_flag_eeprom_preserve = 2 ^ 4
'Const Menu_value_flag_reserved = 2 ^ 5
'Const Menu_value_flag_reserved = 2 ^ 6
'Const Menu_value_flag_reserved = 2 ^ 7

' string edit special characters
Const Menu_character_backspace = 127
Const Menu_character_finish = 219

' looks nice.
Const Readfrom = 0
Const Writeto = 1

Const Menu_flag_readonly = 0
Const Menu_flag_eeprom_preserve = 1
'Const Menu_flag_reserved = 2
'Const Menu_flag_reserved = 3
'Const Menu_flag_reserved = 4
'Const Menu_flag_reserved = 5
'Const Menu_flag_reserved = 6
'Const Menu_flag_reserved = 7

#if Not Varexist( "Menu_eeprom_start")
   ' EEPROM data start address, some devices have problems on address 0
   Const Menu_eeprom_start = 1
#endif


'###############################################################################
'##      M E N U   D A T A                                                    ##
'###############################################################################
Menu_include_data

#if Not Varexist( "Menu_eeprom_preserve_use")
   Const Menu_eeprom_preserve_use = False
#endif
#if Menu_eeprom_preserve_use = True
   ' points to the first free memory location after the menu data block
   Const Menu_eeprom_end = Menu_eeprom_start + Menu_eeprom_size + 1
#endif

' Display string sizes
Const Menu_name_string_width = Lcd_width - 2
Const Menu_value_string_width = Lcd_width - 3


'###############################################################################
'##      S U B R O U T I N E S                                                ##
'###############################################################################
Declare Sub Menu()
Declare Sub Menu_forward()
Declare Sub Menu_backward()
Declare Function Menu_enter() As Byte
Declare Sub Menu_show(byval Entry_id As Byte)
Declare Sub Menu_hide()
Declare Function Centertext(byref Text As String , Byval Width As Byte) As String * Lcd_width
#if Menu_values_use = True
   Declare Sub Menu_data(byval Direction As Byte)
   Declare Sub Menu_load_value_data()
   Declare Function Menu_searchentry_byvalue(byval Valueid As Byte) As Byte
#endif


'###############################################################################
'##      V A R I A B L E S                                                    ##
'###############################################################################
Dim Menu_tempbyte As Byte

Dim Menu_active As Bit                                      ' menu is currently displayed
Dim Menu_redraw As Bit                                      ' menu screen needs to be redrawn
Dim Menu_current_entry As Byte                              ' points to the selected entry
Dim Menu_name_string As String * Menu_name_string_width     ' holds the text associated with the entry

Dim Menu_entry_type(menu_entries_count) As Byte             ' link, value, function, exit
Dim Menu_entry_name(menu_entries_count) As Byte             ' text id
Dim Menu_entry_before(menu_entries_count) As Byte           ' pointer to the preceding entry
Dim Menu_entry_next(menu_entries_count) As Byte             ' pointer to the following entry
Dim Menu_entry_childid(menu_entries_count) As Byte          ' link: pointer to another entry; value: value id; function: function id
Dim Menu_entry_flags(menu_entries_count) As Byte            ' b0: Read-Only Value

#if Menu_values_use = True
   Dim Menu_value_varpointer(menu_values_count) As Word     ' memory address of the associated variable
   Dim Menu_value_type(menu_values_count) As Byte           ' data type
   Dim Menu_value_childid(menu_values_count) As Byte        ' 0: read only; >0: value data id
   #if Menu_debuglevel > 0
      Print "Values: " ; Menu_values_count
   #endif
#endif

' Value data: min, max, step size
#if Menu_value_bit_count > 0
   #if Menu_debuglevel > 0
      Print "Bit Values: " ; Menu_value_bit_count
   #endif
#endif
#if Menu_value_byte_count > 0
   Dim Menu_value_min_byte(menu_value_byte_count) As Byte
   Dim Menu_value_max_byte(menu_value_byte_count) As Byte
   Dim Menu_value_step_byte(menu_value_byte_count) As Byte
   #if Menu_debuglevel > 0
      Print "Byte Values: " ; Menu_value_byte_count
   #endif
#endif
#if Menu_value_word_count > 0
   Dim Menu_value_min_word(menu_value_word_count) As Word
   Dim Menu_value_max_word(menu_value_word_count) As Word
   Dim Menu_value_step_word(menu_value_word_count) As Word
   #if Menu_debuglevel > 0
      Print "Word Values: " ; Menu_value_word_count
   #endif
#endif
#if Menu_value_integer_count > 0
   Dim Menu_value_min_integer(menu_value_integer_count) As Integer
   Dim Menu_value_max_integer(menu_value_integer_count) As Integer
   Dim Menu_value_step_integer(menu_value_integer_count) As Integer
   #if Menu_debuglevel > 0
      Print "Integer Values: " ; Menu_value_integer_count
   #endif
#endif
#if Menu_value_dword_count > 0
   Dim Menu_value_min_dword(menu_value_dword_count) As Dword
   Dim Menu_value_max_dword(menu_value_dword_count) As Dword
   Dim Menu_value_step_dword(menu_value_dword_count) As Dword
   #if Menu_debuglevel > 0
      Print "Dword Values: " ; Menu_value_dword_count
   #endif
#endif
#if Menu_value_long_count > 0
   Dim Menu_value_min_long(menu_value_long_count) As Long
   Dim Menu_value_max_long(menu_value_long_count) As Long
   Dim Menu_value_step_long(menu_value_long_count) As Long
   #if Menu_debuglevel > 0
      Print "Long Values: " ; Menu_value_long_count
   #endif
#endif
#if Menu_value_single_count > 0
   Dim Menu_value_min_single(menu_value_single_count) As Single
   Dim Menu_value_max_single(menu_value_single_count) As Single
   Dim Menu_value_step_single(menu_value_single_count) As Single
   #if Menu_debuglevel > 0
      Print "Single Values: " ; Menu_value_single_count
   #endif
#endif
#if Menu_value_double_count > 0
   Dim Menu_value_min_double(menu_value_double_count) As Double
   Dim Menu_value_max_double(menu_value_double_count) As Double
   Dim Menu_value_step_double(menu_value_double_count) As Double
   #if Menu_debuglevel > 0
      Print "Double Values: " ; Menu_value_double_count
   #endif
#endif
#if Menu_value_string_count > 0
   #if Menu_debuglevel > 0
      Print "String Values: " ; Menu_value_string_count
   #endif
#endif

#if Menu_values_use = True
   Dim Menu_current_value As Byte                           ' points to the selected value

   ' determine the max size of the used data types
   #if Menu_value_string_count > 0
      Const Menu_data_size = Menu_value_string_width + 1
      Const Menu_temp_data_size = 8
   #else
      #if Menu_value_double_count > 0
         Const Menu_data_size = 8
      #else
         #if Menu_value_single_count > 0 Or Menu_value_long_count > 0 Or Menu_value_dword_count > 0
            Const Menu_data_size = 4
         #else
            #if Menu_value_integer_count > 0 Or Menu_value_word_count > 0
               Const Menu_data_size = 2
            #else
               Const Menu_data_size = 1
            #endif
         #endif
      #endif
      Const Menu_temp_data_size = Menu_data_size
   #endif

   ' reserve some memory to store the value(s) to display
   Dim Menu_dummy_data(menu_data_size) As Byte
   Dim Menu_display_value_byte As Byte At Menu_dummy_data(1) Overlay
   Dim Menu_display_value_word As Word At Menu_dummy_data(1) Overlay
   Dim Menu_display_value_integer As Integer At Menu_dummy_data(1) Overlay
   Dim Menu_display_value_dword As Dword At Menu_dummy_data(1) Overlay
   Dim Menu_display_value_long As Long At Menu_dummy_data(1) Overlay
   Dim Menu_display_value_single As Single At Menu_dummy_data(1) Overlay
   Dim Menu_display_value_double As Double At Menu_dummy_data(1) Overlay
   Dim Menu_display_value_string As String * Menu_value_string_width At Menu_dummy_data(1) Overlay

   #if Menu_values_edit_use = True
      Dim Menu_value_active As Bit                          ' edit mode is active

      #if Menu_value_string_count > 0
         Dim Menu_string_table_index As Byte                ' index of the selected character in the table
         Dim Menu_string_table_character As Byte            ' selected character
      #endif

      ' reserve some memory for the value edit function
      Dim Menu_temp_data(menu_temp_data_size) As Byte
      Dim Menu_temp_value_byte As Byte At Menu_temp_data(1) Overlay
      Dim Menu_temp_value_word As Word At Menu_temp_data(1) Overlay
      Dim Menu_temp_value_integer As Integer At Menu_temp_data(1) Overlay
      Dim Menu_temp_value_dword As Dword At Menu_temp_data(1) Overlay
      Dim Menu_temp_value_long As Long At Menu_temp_data(1) Overlay
      Dim Menu_temp_value_single As Single At Menu_temp_data(1) Overlay
      Dim Menu_temp_value_double As Double At Menu_temp_data(1) Overlay
   #endif
#endif


'###############################################################################
'##      L C D   B A R G R A P H                                              ##
'###############################################################################
#if Menu_value_byte_count > 0 Or Menu_value_word_count > 0 Or Menu_value_integer_count > 0 Or Menu_value_dword_count > 0 Or Menu_value_long_count > 0 Or Menu_value_single_count > 0 Or Menu_value_double_count > 0
   Const Menu_lcd_bar_width = Lcd_width - 9
   $include "lcd_bar.inc"
#endif


'###############################################################################
'##      I N I T I A L I S A T I O N                                          ##
'###############################################################################
Macro Menu_init
   ' load the menu structure
   Restore Menu_entries
   For Menu_tempbyte = 1 To Menu_entries_count
      Read Menu_entry_type(menu_tempbyte)
      Read Menu_entry_name(menu_tempbyte)
      Read Menu_entry_before(menu_tempbyte)
      Read Menu_entry_next(menu_tempbyte)
      Read Menu_entry_childid(menu_tempbyte)
      #if Menu_debuglevel > 0
         Print "Type:   " ; Menu_entry_type(menu_tempbyte)
         Print "Name:   " ; Menu_entry_name(menu_tempbyte)
         Print "Before: " ; Menu_entry_before(menu_tempbyte)
         Print "Next:   " ; Menu_entry_next(menu_tempbyte)
         Print "Child:  " ; Menu_entry_childid(menu_tempbyte)
         Print ""
      #endif
   Next
   Menu_current_entry = Menu_default_entry

   #if Menu_values_use = True
      If Menu_entry_type(menu_current_entry) = Menu_value Then Menu_current_value = Menu_entry_childid(menu_current_entry)

      ' load value data (variable, min, max, step, ...)
      Menu_varpointers
      Menu_load_value_data
      ' load edit chars
      #if Menu_values_edit_use = True And Menu_value_string_count > 0
         Menu_string_table_index = 1
         Menu_string_table_character = Lookup(menu_string_table_index , Menu_string_characters)
      #endif
   #endif

   Cursor Off Noblink
   'Menu_active = False
End Macro

Goto Menu_runprog                                           ' jump over the subroutines


'###############################################################################
'##      S U B R O U T I N E S                                                ##
'###############################################################################
' Main function, to be called in your main loop every time
' updates the display if necessary
Sub Menu()
   #if Menu_value_byte_count > 0 Or Menu_value_word_count > 0 Or Menu_value_integer_count > 0 Or Menu_value_dword_count > 0 Or Menu_value_long_count > 0 Or Menu_value_single_count > 0 Or Menu_value_double_count > 0
      Local Single1 As Single , Single2 As Single
   #endif
   If Menu_active = True And Menu_redraw = True Then
      ' update is needed
      Cls
      Locate 1 , 1
      Menu_name_string = Lookupstr(menu_entry_name(menu_current_entry) , Menu_string_constants)       ' load the entry text
      Lcd Chr(60) ; Centertext(menu_name_string , Menu_name_string_width)       ' and display it in the center
      Locate 1 , Lcd_width
      Lcd Chr(62)

      #if Menu_values_use = True
         If Menu_entry_type(menu_current_entry) = Menu_value Then
            Menu_data Readfrom                              ' load the value from the associated variable

            #if Menu_value_byte_count > 0 Or Menu_value_word_count > 0 Or Menu_value_integer_count > 0 Or Menu_value_dword_count > 0 Or Menu_value_long_count > 0 Or Menu_value_single_count > 0 Or Menu_value_double_count > 0
               If Menu_value_bit < Menu_value_type(menu_current_value) And Menu_value_type(menu_current_value) < Menu_value_string Then
                  ' value type is a number
                  Menu_tempbyte = Menu_value_childid(menu_current_value)

                  ' calc the procentual position
                  Select Case Menu_value_type(menu_current_value)
                  #if Menu_value_byte_count > 0
                     Case Menu_value_byte:
                        Single1 = Menu_display_value_byte - Menu_value_min_byte(menu_tempbyte)
                        Single2 = Menu_value_max_byte(menu_tempbyte) - Menu_value_min_byte(menu_tempbyte)
                  #endif
                  #if Menu_value_word_count > 0
                     Case Menu_value_word:
                        Single1 = Menu_display_value_word - Menu_value_min_word(menu_tempbyte)
                        Single2 = Menu_value_max_word(menu_tempbyte) - Menu_value_min_word(menu_tempbyte)
                  #endif
                  #if Menu_value_integer_count > 0
                     Case Menu_value_integer:
                        Single1 = Menu_display_value_integer - Menu_value_min_integer(menu_tempbyte)
                        Single2 = Menu_value_max_integer(menu_tempbyte) - Menu_value_min_integer(menu_tempbyte)
                  #endif
                  #if Menu_value_dword_count > 0
                     Case Menu_value_dword:
                        Menu_temp_value_dword = Menu_display_value_dword - Menu_value_min_dword(menu_tempbyte)
                        Single1 = Menu_temp_value_dword
                        Menu_temp_value_dword = Menu_value_max_dword(menu_tempbyte) - Menu_value_min_dword(menu_tempbyte)
                        Single2 = Menu_temp_value_dword
                  #endif
                  #if Menu_value_long_count > 0
                     Case Menu_value_long:
                        Single1 = Menu_display_value_long - Menu_value_min_long(menu_tempbyte)
                        Single2 = Menu_value_max_long(menu_tempbyte) - Menu_value_min_long(menu_tempbyte)
                  #endif
                  #if Menu_value_single_count > 0
                     Case Menu_value_single:
                        Single1 = Menu_display_value_single - Menu_value_min_single(menu_tempbyte)
                        Single2 = Menu_value_max_single(menu_tempbyte) - Menu_value_min_single(menu_tempbyte)
                  #endif
                  #if Menu_value_double_count > 0
                     Case Menu_value_double:
                        Menu_temp_value_double = Menu_display_value_double - Menu_value_min_double(menu_tempbyte)
                        Single1 = Menu_temp_value_double
                        Menu_temp_value_double = Menu_value_max_double(menu_tempbyte) - Menu_value_min_double(menu_tempbyte)
                        Single2 = Menu_temp_value_double
                  #endif
                  End Select
                  Single1 = Single1 * 100
                  Single1 = Single1 / Single2
                  ' draw the bargraph
                  Menu_tempbyte = Single1
                  Locate 2 , 10
                  Lcd_bar Menu_tempbyte , Menu_lcd_bar_width
               End If
            #endif

            #if Menu_values_edit_use = True
               ' some brackets visible while in edit mode
               If Menu_value_active = True Then
                  Locate 2 , 1
                  Lcd Chr(91)
               Else
                  Locate 2 , 2
               End If
            #else
               Locate 2 , 2
            #endif

            ' display the current value
            Select Case Menu_value_type(menu_current_value)
            #if Menu_value_bit_count > 0
               Case Menu_value_bit:
                  If Menu_display_value_byte.0 = 0 Then
                     Lcd Lcd_bit_display_off
                  Else
                     Lcd Lcd_bit_display_on
                  End If
            #endif
            #if Menu_value_byte_count > 0
               Case Menu_value_byte:
                  Lcd Menu_display_value_byte
            #endif
            #if Menu_value_word_count > 0
               Case Menu_value_word:
                  Lcd Menu_display_value_word
            #endif
            #if Menu_value_integer_count > 0
               Case Menu_value_integer
                  Lcd Menu_display_value_integer
            #endif
            #if Menu_value_dword_count > 0
               Case Menu_value_dword:
                  Lcd Menu_display_value_dword
            #endif
            #if Menu_value_long_count > 0
               Case Menu_value_long:
                  Lcd Menu_display_value_long
            #endif
            #if Menu_value_single_count > 0
               Case Menu_value_single:
                  Menu_name_string = Fusing(menu_display_value_single , "###.##")
                  Lcd Menu_name_string
            #endif
            #if Menu_value_double_count > 0
               Case Menu_value_double:
                  Single1 = Menu_display_value_double
                  Menu_name_string = Fusing(single1 , "###.#####")
                  Lcd Menu_name_string
            #endif
            #if Menu_value_string_count > 0
               Case Menu_value_string:
                  Lcd Menu_display_value_string
            #endif
            End Select

            #if Menu_values_edit_use = True
               If Menu_value_active = True Then
                  If Menu_value_type(menu_current_value) <> Menu_value_string Then
                     Lcd " "
                     #if Menu_value_string_count > 0
                        ' in string edit mode
                        Else
                           'Menu_tempbyte = Len(menu_display_value_string) + 2
                           Menu_tempbyte = Len(menu_display_value_string)
                           If Menu_tempbyte = Menu_value_string_width And Menu_string_table_character <> Menu_character_backspace And Menu_string_table_character <> Menu_character_finish Then
                              Menu_tempbyte = Menu_tempbyte + 1
                           Else
                              Menu_tempbyte = Menu_tempbyte + 2
                           End If
                           'If Menu_string_table_character = Menu_character_backspace Or Menu_string_table_character = Menu_character_finish Then Incr Menu_tempbyte
                           Locate 2 , Menu_tempbyte
                           Lcd Chr(menu_string_table_character)
                     #endif
                  End If
                  Lcd Chr(93)                               ' edit mode brackets close
                  Shiftcursor Left
                  Shiftcursor Left
               End If
            #endif
         End If
      #endif
      Menu_redraw = False
   End If
End Sub

' navigate through the menue forward or edit (increment) a value
Sub Menu_forward()
   If Menu_active = True Then
      Menu_redraw = True
      #if Menu_values_edit_use = True
         If Menu_value_active = False Then
      #endif
            ' get the next entry id
            Menu_current_entry = Menu_entry_next(menu_current_entry)
      #if Menu_values_use = True
            If Menu_entry_type(menu_current_entry) = Menu_value Then
               ' get the value id
               Menu_current_value = Menu_entry_childid(menu_current_entry)
            End If
      #endif
      #if Menu_values_edit_use = True
         Else
            ' in edit mode
            Menu_data Readfrom
            Menu_tempbyte = Menu_value_childid(menu_current_value)
            ' increment the value
            Select Case Menu_value_type(menu_current_value)
            #if Menu_value_bit_count > 0
               Case Menu_value_bit
                  Menu_display_value_byte = True
            #endif
            #if Menu_value_byte_count > 0
               Case Menu_value_byte
                  Menu_temp_value_byte = Menu_value_max_byte(menu_tempbyte) - Menu_display_value_byte
                  If Menu_temp_value_byte < Menu_value_step_byte(menu_tempbyte) Then
                     Menu_display_value_byte = Menu_value_max_byte(menu_tempbyte)
                  Else
                     Menu_display_value_byte = Menu_display_value_byte + Menu_value_step_byte(menu_tempbyte)
                  End If
            #endif
            #if Menu_value_word_count > 0
               Case Menu_value_word
                  Menu_temp_value_word = Menu_value_max_word(menu_tempbyte) - Menu_display_value_word
                  If Menu_temp_value_word < Menu_value_step_word(menu_tempbyte) Then
                     Menu_display_value_word = Menu_value_max_word(menu_tempbyte)
                  Else
                     Menu_display_value_word = Menu_display_value_word + Menu_value_step_word(menu_tempbyte)
                  End If
            #endif
            #if Menu_value_integer_count > 0
               Case Menu_value_integer
                  Menu_temp_value_integer = Menu_value_max_integer(menu_tempbyte) - Menu_display_value_integer
                  If Menu_temp_value_integer < Menu_value_step_integer(menu_tempbyte) Then
                     Menu_display_value_integer = Menu_value_max_integer(menu_tempbyte)
                  Else
                     Menu_display_value_integer = Menu_display_value_integer + Menu_value_step_integer(menu_tempbyte)
                  End If
            #endif
            #if Menu_value_dword_count > 0
               Case Menu_value_dword
                  Menu_temp_value_dword = Menu_value_max_dword(menu_tempbyte) - Menu_display_value_dword
                  If Menu_temp_value_dword < Menu_value_step_dword(menu_tempbyte) Then
                     Menu_display_value_dword = Menu_value_max_dword(menu_tempbyte)
                  Else
                     Menu_display_value_dword = Menu_display_value_dword + Menu_value_step_dword(menu_tempbyte)
                  End If
            #endif
            #if Menu_value_long_count > 0
               Case Menu_value_long
                  Menu_temp_value_long = Menu_value_max_long(menu_tempbyte) - Menu_display_value_long
                  If Menu_temp_value_long < Menu_value_step_long(menu_tempbyte) Then
                     Menu_display_value_long = Menu_value_max_long(menu_tempbyte)
                  Else
                     Menu_display_value_long = Menu_display_value_long + Menu_value_step_long(menu_tempbyte)
                  End If
            #endif
            #if Menu_value_single_count > 0
               Case Menu_value_single
                  Menu_temp_value_single = Menu_value_max_single(menu_tempbyte) - Menu_display_value_single
                  If Menu_temp_value_single < Menu_value_step_single(menu_tempbyte) Then
                     Menu_display_value_single = Menu_value_max_single(menu_tempbyte)
                  Else
                     Menu_display_value_single = Menu_display_value_single + Menu_value_step_single(menu_tempbyte)
                  End If
            #endif
            #if Menu_value_double_count > 0
               Case Menu_value_double
                  Menu_temp_value_double = Menu_value_max_double(menu_tempbyte) - Menu_display_value_double
                  If Menu_temp_value_double < Menu_value_step_double(menu_tempbyte) Then
                     Menu_display_value_double = Menu_value_max_double(menu_tempbyte)
                  Else
                     Menu_display_value_double = Menu_display_value_double + Menu_value_step_double(menu_tempbyte)
                  End If
            #endif
            #if Menu_value_string_count > 0
               Case Menu_value_string
                  ' select the next string edit char
                  Do
                     Incr Menu_string_table_index
                     Menu_string_table_character = Lookup(menu_string_table_index , Menu_string_characters)
                  Loop Until Menu_string_table_character <> 0
                  If Menu_string_table_character = 254 Then Menu_string_table_index = 1
                  Menu_string_table_character = Lookup(menu_string_table_index , Menu_string_characters)
            #endif
            End Select
            ' save back to the variable
            If Menu_entry_flags(menu_current_entry).menu_flag_readonly = False Then Menu_data Writeto
         End If
      #endif
   End If
End Sub

' navigate through the menue backward or edit (decrement) a value
Sub Menu_backward()
   If Menu_active = True Then
      Menu_redraw = True
      #if Menu_values_edit_use = True
         If Menu_value_active = False Then
      #endif
            ' get the preceeding entry id
            Menu_current_entry = Menu_entry_before(menu_current_entry)
      #if Menu_values_use = True
            If Menu_entry_type(menu_current_entry) = Menu_value Then
               ' get the value id
               Menu_current_value = Menu_entry_childid(menu_current_entry)
            End If
      #endif
      #if Menu_values_edit_use = True
         Else
            ' in edit mode
            Menu_data Readfrom
            Menu_tempbyte = Menu_value_childid(menu_current_value)
            ' decrement the value
            Select Case Menu_value_type(menu_current_value)
            #if Menu_value_bit_count > 0
               Case Menu_value_bit
                  Menu_display_value_byte = False
            #endif
            #if Menu_value_byte_count > 0
               Case Menu_value_byte
                  Menu_temp_value_byte = Menu_display_value_byte - Menu_value_min_byte(menu_tempbyte)
                  If Menu_temp_value_byte < Menu_value_step_byte(menu_tempbyte) Then
                     Menu_display_value_byte = Menu_value_min_byte(menu_tempbyte)
                  Else
                     Menu_display_value_byte = Menu_display_value_byte - Menu_value_step_byte(menu_tempbyte)
                  End If
            #endif
            #if Menu_value_word_count > 0
               Case Menu_value_word
                  Menu_temp_value_word = Menu_display_value_word - Menu_value_min_word(menu_tempbyte)
                  If Menu_temp_value_word < Menu_value_step_word(menu_tempbyte) Then
                     Menu_display_value_word = Menu_value_min_word(menu_tempbyte)
                  Else
                     Menu_display_value_word = Menu_display_value_word - Menu_value_step_word(menu_tempbyte)
                  End If
            #endif
            #if Menu_value_integer_count > 0
               Case Menu_value_integer
                  Menu_temp_value_integer = Menu_display_value_integer - Menu_value_min_integer(menu_tempbyte)
                  If Menu_temp_value_integer < Menu_value_step_integer(menu_tempbyte) Then
                     Menu_display_value_integer = Menu_value_min_integer(menu_tempbyte)
                  Else
                     Menu_display_value_integer = Menu_display_value_integer - Menu_value_step_integer(menu_tempbyte)
                  End If
            #endif
            #if Menu_value_dword_count > 0
               Case Menu_value_dword
                  Menu_temp_value_dword = Menu_display_value_dword - Menu_value_min_dword(menu_tempbyte)
                  If Menu_temp_value_dword < Menu_value_step_dword(menu_tempbyte) Then
                     Menu_display_value_dword = Menu_value_min_dword(menu_tempbyte)
                  Else
                     Menu_display_value_dword = Menu_display_value_dword - Menu_value_step_dword(menu_tempbyte)
                  End If
            #endif
            #if Menu_value_long_count > 0
               Case Menu_value_long
                  Menu_temp_value_long = Menu_display_value_long - Menu_value_min_long(menu_tempbyte)
                  If Menu_temp_value_long < Menu_value_step_long(menu_tempbyte) Then
                     Menu_display_value_long = Menu_value_min_long(menu_tempbyte)
                  Else
                     Menu_display_value_long = Menu_display_value_long - Menu_value_step_long(menu_tempbyte)
                  End If
            #endif
            #if Menu_value_single_count > 0
               Case Menu_value_single
                  Menu_temp_value_single = Menu_display_value_single - Menu_value_min_single(menu_tempbyte)
                  If Menu_temp_value_single < Menu_value_step_single(menu_tempbyte) Then
                     Menu_display_value_single = Menu_value_min_single(menu_tempbyte)
                  Else
                     Menu_display_value_single = Menu_display_value_single - Menu_value_step_single(menu_tempbyte)
                  End If
            #endif
            #if Menu_value_double_count > 0
               Case Menu_value_double
                  Menu_temp_value_double = Menu_display_value_double - Menu_value_min_double(menu_tempbyte)
                  If Menu_temp_value_double < Menu_value_step_double(menu_tempbyte) Then
                     Menu_display_value_double = Menu_value_min_double(menu_tempbyte)
                  Else
                     Menu_display_value_double = Menu_display_value_double - Menu_value_step_double(menu_tempbyte)
                  End If
            #endif
            #if Menu_value_string_count > 0
               Case Menu_value_string
                  ' select preceeding string edit char
                  Do
                     Decr Menu_string_table_index
                     Menu_string_table_character = Lookup(menu_string_table_index , Menu_string_characters)
                  Loop Until Menu_string_table_character <> 0
                  If Menu_string_table_character = 254 Then
                     Do
                        Incr Menu_string_table_index
                        Menu_string_table_character = Lookup(menu_string_table_index , Menu_string_characters)
                     Loop Until Menu_string_table_character = 254
                     Decr Menu_string_table_index
                     Menu_string_table_character = Lookup(menu_string_table_index , Menu_string_characters)
                  End If
            #endif
            End Select
            ' save back the data
            If Menu_entry_flags(menu_current_entry).menu_flag_readonly = False Then Menu_data Writeto
         End If
      #endif
   End If
End Sub

' make a wild guess
Function Menu_enter() As Byte
   #if Menu_eeprom_preserve_use = True
      Local Eeprom_address As Word , Value_index As Byte
      Eeprom_address = Menu_eeprom_start + 1
   #endif
   Menu_redraw = True
   Menu_enter = 0                                           ' no action

   If Menu_active = False Then
      Menu_show Menu_default_entry                          ' display menu if inactive
   Else
      #if Menu_values_edit_use = True
         If Menu_value_active = False Then
      #endif
      Select Case Menu_entry_type(menu_current_entry)
         #if Menu_values_edit_use = True
            Case Menu_value:                                ' enter edit mode
               If Menu_entry_flags(menu_current_entry).menu_flag_readonly = False Then
                  Menu_value_active = True
                  Cursor On Blink
               End If
         #endif
         Case Menu_link:                                    ' link to another item
            Menu_current_entry = Menu_entry_childid(menu_current_entry)
            #if Menu_values_use = True
               If Menu_entry_type(menu_current_entry) = Menu_value Then
                  Menu_current_value = Menu_entry_childid(menu_current_entry)
               End If
            #endif
         Case Menu_exit:                                    ' exit the menu
            Menu_enter = Menu_exit
            Menu_hide
         Case Menu_function:                                ' function execute
            Menu_active = False
            Menu_enter = Menu_entry_childid(menu_current_entry)
            Cls
      End Select
      #if Menu_values_edit_use = True
         Else                                               ' exit edit mode or string edit mode
            #if Menu_value_string_count > 0
               If Menu_value_type(menu_current_value) = Menu_value_string Then
                  Select Case Menu_string_table_character
                  Case Menu_character_backspace             ' backspace selected, delete last character from string
                     Menu_tempbyte = Len(menu_display_value_string) - 1
                     If 0 < Menu_tempbyte Then
                        Menu_display_value_string = Left(menu_display_value_string , Menu_tempbyte)
                     Else
                        Menu_display_value_string = ""
                     End If
                  Case Menu_character_finish                ' string edit finished
                     Menu_value_active = False
                     Cursor Off Noblink
                  Case Else                                 ' append currently selected char to string
                     Menu_tempbyte = Len(menu_display_value_string)
                     If Menu_tempbyte = Menu_value_string_width Then
                        Decr Menu_tempbyte
                        Menu_display_value_string = Left(menu_display_value_string , Menu_tempbyte)
                     End If
                     Menu_display_value_string = Menu_display_value_string + Chr(menu_string_table_character)
                  End Select
                  Menu_data Writeto                         ' write back to the variable
               Else
            #endif
                  Menu_value_active = False                 ' exit variable edit mode
                  Cursor Off Noblink
            #if Menu_value_string_count > 0
               End If
            #endif
            #if Menu_eeprom_preserve_use = True             ' store current variable value in EEPROM
               If Menu_entry_flags(menu_current_entry).menu_flag_eeprom_preserve = True Then
                  For Menu_tempbyte = 1 To Menu_entries_count       ' search value address
                     If Menu_entry_flags(menu_tempbyte).menu_flag_eeprom_preserve = True Then
                        Value_index = Menu_entry_childid(menu_tempbyte)
                        If Value_index = Menu_current_value Then Exit For       ' reached current used value, address found
                        Select Case Menu_value_type(value_index)       ' jump to next value position in EEPROM
                           #if Menu_value_bit_count > 0 Or Menu_value_byte_count > 0
                              Case Menu_value_bit To Menu_value_byte
                                 Eeprom_address = Eeprom_address + 1
                           #endif
                           #if Menu_value_word_count > 0 Or Menu_value_integer_count > 0
                              Case Menu_value_word To Menu_value_integer
                                 Eeprom_address = Eeprom_address + 2
                           #endif
                           #if Menu_value_dword_count > 0 Or Menu_value_long_count > 0 Or Menu_value_single_count > 0
                              Case Menu_value_dword To Menu_value_single
                                 Eeprom_address = Eeprom_address + 4
                           #endif
                           #if Menu_value_double_count > 0
                              Case Menu_value_double
                                 Eeprom_address = Eeprom_address + 8
                           #endif
                           #if Menu_value_string_count > 0
                              Case Menu_value_string
                                 Eeprom_address = Eeprom_address + Menu_value_string_width
                           #endif
                        End Select
                     End If
                  Next
                  Select Case Menu_value_type(menu_current_value)       ' save it
                     #if Menu_value_bit_count > 0 Or Menu_value_byte_count > 0
                        Case Menu_value_bit To Menu_value_byte
                           Writeeeprom Menu_display_value_byte , Eeprom_address
                     #endif
                     #if Menu_value_word_count > 0 Or Menu_value_integer_count > 0
                        Case Menu_value_word To Menu_value_integer
                           Writeeeprom Menu_display_value_word , Eeprom_address
                     #endif
                     #if Menu_value_dword_count > 0 Or Menu_value_long_count > 0 Or Menu_value_single_count > 0
                        Case Menu_value_dword To Menu_value_single
                           Writeeeprom Menu_display_value_dword , Eeprom_address
                     #endif
                     #if Menu_value_double_count > 0
                        Case Menu_value_double
                           Writeeeprom Menu_display_value_double , Eeprom_address
                     #endif
                     #if Menu_value_string_count > 0
                        Case Menu_value_string
                           Writeeeprom Menu_display_value_string , Eeprom_address
                     #endif
                  End Select
               End If
            #endif
         End If
      #endif
   End If
End Function

' displays the menu with the provided ID, if 0, the default entry will be used
Sub Menu_show(byval Entry_id As Byte)
   If Entry_id <> 0 Then
      Menu_current_entry = Entry_id
   End If
   #if Menu_debuglevel > 1
      Print "Showing Menu: EntryID " ; Menu_current_entry
   #endif
   Menu_active = True
   Menu_redraw = True
   #if Menu_values_use = True
      If Menu_entry_type(entry_id) = Menu_value Then Menu_current_value = Menu_entry_childid(entry_id)
   #endif
End Sub

' hides the menu
Sub Menu_hide()
   Menu_active = False
   Menu_current_entry = Menu_default_entry
End Sub

' returns the centered text within the specified width or left orientated if
' the string width exceeds the provided width
Function Centertext(byref Text As String , Byval Width As Byte) As String * Lcd_width
   Menu_tempbyte = Len(text)
   If Width < Menu_tempbyte Then Menu_tempbyte = Width
   Menu_tempbyte = Width - Menu_tempbyte
   Shift Menu_tempbyte , Right , 1

   If 0 < Menu_tempbyte Then
      Centertext = Space(menu_tempbyte)
   Else
      Centertext = ""
   End If
   Centertext = Centertext + Text
End Function

#if Menu_values_use = True
   ' transfers the value data from the variable in the menu memory and vice versa
   Sub Menu_data(byval Direction As Byte)
      Local Readaddress As Word , Writeaddress As Word

      ' set read- and write addresses
      If Direction = Readfrom Then
         Readaddress = Menu_value_varpointer(menu_current_value)
         Writeaddress = Varptr(menu_dummy_data(1))
      Else
         Readaddress = Varptr(menu_dummy_data(1))
         Writeaddress = Menu_value_varpointer(menu_current_value)
      End If

      ' determine the data size
      Select Case Menu_value_type(menu_current_value)
      Case Menu_value_bit To Menu_value_byte:
         Menu_tempbyte = 1
      Case Menu_value_word To Menu_value_integer:
         Menu_tempbyte = 2
      Case Menu_value_dword To Menu_value_single:
         Menu_tempbyte = 4
      Case Menu_value_double:
         Menu_tempbyte = 8
      Case Menu_value_string:
         Menu_tempbyte = Menu_value_string_width + 1
      End Select

      ' copy the data
      While Menu_tempbyte <> 0
         Direction = Inp(readaddress)
         Out Writeaddress , Direction
         Incr Readaddress
         Incr Writeaddress
         Decr Menu_tempbyte
      Wend
   End Sub

   ' loads the menu value data (Type, Step/Readonly, [Init, Min, Max]) and sets
   ' the initial value if applicable
   Sub Menu_load_value_data()
      Local Value_index As Byte , Value_sub_index As Byte
      Local Use_startvalue As Byte , Entry_index As Byte
      #if Menu_eeprom_preserve_use = True
         Local Eeprom_address As Word , Eeprom_initialized As Byte
         Eeprom_address = Menu_eeprom_start
         Readeeprom Eeprom_initialized , Eeprom_address     ' get EEPROM checksum
         Eeprom_address = Eeprom_address + 1
      #endif
      Value_index = 0

      Restore Menu_values
      For Menu_current_value = 1 To Menu_values_count
         Incr Value_index
         Read Menu_value_type(value_index)
         Entry_index = Menu_searchentry_byvalue(menu_current_value)
         #if Menu_eeprom_preserve_use = True
            If Menu_value_type(value_index) >= Menu_value_flag_eeprom_preserve Then       ' EEPROM flag set
               Menu_entry_flags(entry_index).menu_flag_eeprom_preserve = True
            End If
            Menu_value_type(value_index) = Menu_value_type(value_index) And Menu_value_type_mask
         #endif
         Read Value_sub_index
         Menu_value_childid(value_index) = Value_sub_index

         Select Case Menu_value_type(value_index)
         #if Menu_value_bit_count > 0
            Case Menu_value_bit
               Read Menu_tempbyte                           ' read step size
               If Menu_tempbyte <> 0 Then                   ' editable value
                  Read Menu_display_value_byte              ' read init value
                  #if Menu_eeprom_preserve_use = True
                     If Menu_entry_flags(entry_index).menu_flag_eeprom_preserve = True Then
                        If Eeprom_initialized = Menu_eeprom_checksum Then
                           Readeeprom Menu_display_value_byte , Eeprom_address       ' load init value from EEPROMif checksum correct
                        Else
                           Writeeeprom Menu_display_value_byte , Eeprom_address       ' store init value
                        End If
                        Eeprom_address = Eeprom_address + 1
                     End If
                  #endif
                  Use_startvalue = True                     ' write value back to variable SRAM location
               Else
                  Menu_entry_flags(entry_index).menu_flag_readonly = True       ' read only value
               End If
               #if Menu_debuglevel > 0
                  Print "Bit: " ; Menu_display_value_byte
               #endif
         #endif
         #if Menu_value_byte_count > 0
            Case Menu_value_byte
               Read Menu_value_step_byte(value_sub_index)
               Read Menu_value_min_byte(value_sub_index)
               Read Menu_value_max_byte(value_sub_index)
               If Menu_value_step_byte(value_sub_index) <> 0 Then
                  Read Menu_display_value_byte
                  #if Menu_eeprom_preserve_use = True
                     If Menu_entry_flags(entry_index).menu_flag_eeprom_preserve = True Then
                        If Eeprom_initialized = Menu_eeprom_checksum Then
                           Readeeprom Menu_display_value_byte , Eeprom_address
                        Else
                           Writeeeprom Menu_display_value_byte , Eeprom_address
                        End If
                        Eeprom_address = Eeprom_address + 1
                     End If
                  #endif
                  Use_startvalue = True
               Else
                  Menu_entry_flags(entry_index).menu_flag_readonly = True
               End If
               #if Menu_debuglevel > 0
                  Print "Byte Count: " ; Value_sub_index ; "   Start: " ; Menu_display_value_byte ; "   Min: " ; Menu_value_min_byte(value_sub_index) ; "   Max: " ; Menu_value_max_byte(value_sub_index) ; "   Step: " ; Menu_value_step_byte(value_sub_index)
               #endif
         #endif
         #if Menu_value_word_count > 0
            Case Menu_value_word
               Read Menu_value_step_word(value_sub_index)
               Read Menu_value_min_word(value_sub_index)
               Read Menu_value_max_word(value_sub_index)
               If Menu_value_step_word(value_sub_index) <> 0 Then
                  Read Menu_display_value_word
                  #if Menu_eeprom_preserve_use = True
                     If Menu_entry_flags(entry_index).menu_flag_eeprom_preserve = True Then
                        If Eeprom_initialized = Menu_eeprom_checksum Then
                           Readeeprom Menu_display_value_word , Eeprom_address
                        Else
                           Writeeeprom Menu_display_value_word , Eeprom_address
                        End If
                        Eeprom_address = Eeprom_address + 2
                     End If
                  #endif
                  Use_startvalue = True
               Else
                  Menu_entry_flags(entry_index).menu_flag_readonly = True
               End If
               #if Menu_debuglevel > 0
                  Print "Word Count: " ; Value_sub_index ; "   Start: " ; Menu_display_value_word ; "   Min: " ; Menu_value_min_word(value_sub_index) ; "   Max: " ; Menu_value_max_word(value_sub_index) ; "   Step: " ; Menu_value_step_word(value_sub_index)
               #endif
         #endif
         #if Menu_value_integer_count > 0
            Case Menu_value_integer
               Read Menu_value_step_integer(value_sub_index)
               Read Menu_value_min_integer(value_sub_index)
               Read Menu_value_max_integer(value_sub_index)
               If Menu_value_step_integer(value_sub_index) <> 0 Then
                  Read Menu_display_value_integer
                  #if Menu_eeprom_preserve_use = True
                     If Menu_entry_flags(entry_index).menu_flag_eeprom_preserve = True Then
                        If Eeprom_initialized = Menu_eeprom_checksum Then
                           Readeeprom Menu_display_value_integer , Eeprom_address
                        Else
                           Writeeeprom Menu_display_value_integer , Eeprom_address
                        End If
                        Eeprom_address = Eeprom_address + 2
                     End If
                  #endif
                  Use_startvalue = True
               Else
                  Menu_entry_flags(entry_index).menu_flag_readonly = True
               End If
               #if Menu_debuglevel > 0
                  Print "Integer Count: " ; Value_sub_index ; "   Start: " ; Menu_display_value_integer ; "   Min: " ; Menu_value_min_integer(value_sub_index) ; "   Max: " ; Menu_value_max_integer(value_sub_index) ; "   Step: " ; Menu_value_step_integer(value_sub_index)
               #endif
         #endif
         #if Menu_value_dword_count > 0
            Case Menu_value_dword
               Read Menu_value_step_dword(value_sub_index)
               Read Menu_value_min_dword(value_sub_index)
               Read Menu_value_max_dword(value_sub_index)
               If Menu_value_step_dword(value_sub_index) <> 0 Then
                  Read Menu_display_value_dword
                  #if Menu_eeprom_preserve_use = True
                     If Menu_entry_flags(entry_index).menu_flag_eeprom_preserve = True Then
                        If Eeprom_initialized = Menu_eeprom_checksum Then
                           Readeeprom Menu_display_value_dword , Eeprom_address
                        Else
                           Writeeeprom Menu_display_value_dword , Eeprom_address
                        End If
                        Eeprom_address = Eeprom_address + 4
                     End If
                  #endif
                  Use_startvalue = True
               Else
                  Menu_entry_flags(entry_index).menu_flag_readonly = True
               End If
               #if Menu_debuglevel > 0
                  Print "Dword Count: " ; Value_sub_index ; "   Start: " ; Menu_display_value_dword ; "   Min: " ; Menu_value_min_dword(value_sub_index) ; "   Max: " ; Menu_value_max_dword(value_sub_index) ; "   Step: " ; Menu_value_step_dword(value_sub_index)
               #endif
         #endif
         #if Menu_value_long_count > 0
            Case Menu_value_long
               Read Menu_value_step_long(value_sub_index)
               Read Menu_value_min_long(value_sub_index)
               Read Menu_value_max_long(value_sub_index)
               If Menu_value_step_long(value_sub_index) <> 0 Then
                  Read Menu_display_value_long
                  #if Menu_eeprom_preserve_use = True
                     If Menu_entry_flags(entry_index).menu_flag_eeprom_preserve = True Then
                        If Eeprom_initialized = Menu_eeprom_checksum Then
                           Readeeprom Menu_display_value_long , Eeprom_address
                        Else
                           Writeeeprom Menu_display_value_long , Eeprom_address
                        End If
                        Eeprom_address = Eeprom_address + 4
                     End If
                  #endif
                  Use_startvalue = True
               Else
                  Menu_entry_flags(entry_index).menu_flag_readonly = True
               End If
               #if Menu_debuglevel > 0
                  Print "Long Count: " ; Value_sub_index ; "   Start: " ; Menu_display_value_long ; "   Min: " ; Menu_value_min_long(value_sub_index) ; "   Max: " ; Menu_value_max_long(value_sub_index) ; "   Step: " ; Menu_value_step_long(value_sub_index)
               #endif
         #endif
         #if Menu_value_single_count > 0
            Case Menu_value_single
               Read Menu_value_step_single(value_sub_index)
               Read Menu_value_min_single(value_sub_index)
               Read Menu_value_max_single(value_sub_index)
               If Menu_value_step_single(value_sub_index) <> 0 Then
                  Read Menu_display_value_single
                  #if Menu_eeprom_preserve_use = True
                     If Menu_entry_flags(entry_index).menu_flag_eeprom_preserve = True Then
                        If Eeprom_initialized = Menu_eeprom_checksum Then
                           Readeeprom Menu_display_value_single , Eeprom_address
                        Else
                           Writeeeprom Menu_display_value_single , Eeprom_address
                        End If
                        Eeprom_address = Eeprom_address + 4
                     End If
                  #endif
                  Use_startvalue = True
               Else
                  Menu_entry_flags(entry_index).menu_flag_readonly = True
               End If
               #if Menu_debuglevel > 0
                  Print "Single Count: " ; Value_sub_index ; "   Start: " ; Menu_display_value_single ; "   Min: " ; Menu_value_min_single(value_sub_index) ; "   Max: " ; Menu_value_max_single(value_sub_index) ; "   Step: " ; Menu_value_step_single(value_sub_index)
               #endif
         #endif
         #if Menu_value_double_count > 0
            Case Menu_value_double
               Read Menu_value_step_double(value_sub_index)
               Read Menu_value_min_double(value_sub_index)
               Read Menu_value_max_double(value_sub_index)
               If Menu_value_step_double(value_sub_index) <> 0 Then
                  Read Menu_display_value_double
                  #if Menu_eeprom_preserve_use = True
                     If Menu_entry_flags(entry_index).menu_flag_eeprom_preserve = True Then
                        If Eeprom_initialized = Menu_eeprom_checksum Then
                           Readeeprom Menu_display_value_double , Eeprom_address
                        Else
                           Writeeeprom Menu_display_value_double , Eeprom_address
                        End If
                        Eeprom_address = Eeprom_address + 8
                     End If
                  #endif
                  Use_startvalue = True
               Else
                  Menu_entry_flags(entry_index).menu_flag_readonly = True
               End If
               #if Menu_debuglevel > 0
                  Print "Double Count: " ; Value_sub_index ; "   Start: " ; Menu_display_value_double ; "   Min: " ; Menu_value_min_double(value_sub_index) ; "   Max: " ; Menu_value_max_double(value_sub_index) ; "   Step: " ; Menu_value_step_double(value_sub_index)
               #endif
         #endif
         #if Menu_value_string_count > 0
            Case Menu_value_string
               Read Menu_tempbyte
               If Menu_tempbyte <> 0 Then
                  Read Menu_display_value_string
                  #if Menu_eeprom_preserve_use = True
                     If Menu_entry_flags(entry_index).menu_flag_eeprom_preserve = True Then
                        If Eeprom_initialized = Menu_eeprom_checksum Then
                           Readeeprom Menu_display_value_string , Eeprom_address
                        Else
                           Writeeeprom Menu_display_value_string , Eeprom_address
                        End If
                        Eeprom_address = Eeprom_address + Menu_value_string_width
                     End If
                  #else
                  #endif
                  Use_startvalue = True
               Else
                  Menu_entry_flags(entry_index).menu_flag_readonly = True
               End If
               #if Menu_debuglevel > 0
                  Print "String: " ; Menu_display_value_string
               #endif
         #endif
         End Select

         ' init variable
         If Use_startvalue = True Then
            Menu_data Writeto
            Use_startvalue = False
         End If

         #if Menu_debuglevel > 0
            Print Menu_value_type(value_index) ; " ";
            If Value_sub_index = 0 Then Print "Read only"
            If Menu_entry_flags(entry_index).menu_flag_eeprom_preserve = True Then Print "EEPROM"
         #endif
      Next

      #if Menu_eeprom_preserve_use = True
         ' EEPROM now contains proper values, write checksum
         If Eeprom_initialized <> Menu_eeprom_checksum Then
            Eeprom_initialized = Menu_eeprom_checksum
            Writeeeprom Eeprom_initialized , Menu_eeprom_start
         End If
      #endif
   End Sub

   Function Menu_searchentry_byvalue(byval Valueid As Byte) As Byte
      Local Foundid As Byte
      For Menu_searchentry_byvalue = 1 To Menu_entries_count
         If Menu_entry_type(menu_searchentry_byvalue) = Menu_value Then
            Foundid = Menu_entry_childid(menu_searchentry_byvalue)
            If Valueid = Foundid Then
               If Menu_value_type(foundid) = Menu_value_type(valueid) Then Exit For
            End If
         End If
      Next
   End Function
#endif

Macro Menu_check_update
   #if Menu_values_use = True
      If Menu_entry_type(menu_current_entry) = Menu_value Then
         If Menu_entry_flags(menu_current_entry).menu_flag_readonly = True Then
            Menu_redraw = True
         End If
      End If
   #endif
End Macro

Menu_runprog: