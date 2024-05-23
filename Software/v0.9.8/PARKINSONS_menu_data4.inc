$nocompile

Const Lcd_width = 16
Const Lcd_bar_style = 1
Const Lcd_bit_display_off = "Off"
Const Lcd_bit_display_on = "On"

Const Menu_entries_count = 23
Const Menu_default_entry = 2

Const Menu_values_use = True
Const Menu_values_edit_use = True
Const Menu_eeprom_preserve_use = True
Const Menu_values_count = 11
Const Menu_value_bit_count = 4
Const Menu_value_byte_count = 2
Const Menu_value_word_count = 2
Const Menu_value_integer_count = 0
Const Menu_value_dword_count = 0
Const Menu_value_long_count = 0
Const Menu_value_single_count = 1
Const Menu_value_double_count = 0
Const Menu_value_string_count = 2

Const Menu_eeprom_checksum = 177
Const Menu_eeprom_size = 15

Goto Menu_data_jumpover

Menu_entries:
   ' Type, Text ID, Before, Next, Child
   Data Menu_exit, 0, 23, 2, 23                             ' 1, {127} Exit
   Data Menu_function, 1, 1, 3, 2                           ' 2, Start Therapy
   Data Menu_function, 2, 2, 4, 3                           ' 3, Stop Therapy
   Data Menu_link, 3, 3, 8, 5                               ' 4, Device Settings
   Data Menu_value, 7, 7, 6, 1                              ' 5, Autostart
   Data Menu_function, 8, 5, 7, 9                           ' 6, Load Defaults
   Data Menu_link, 9, 6, 5, 4                               ' 7, {127} Back
   Data Menu_link, 4, 4, 14, 9                              ' 8, Therapy Setng
   Data Menu_value, 10, 13, 10, 2                           ' 9, SessionMinutes
   Data Menu_value, 11, 9, 11, 3                            ' 10, Intensity
   Data Menu_value, 12, 10, 12, 4                           ' 11, Jitter
   Data Menu_value, 13, 11, 13, 5                           ' 12, Hand Mirror
   Data Menu_link, 9, 12, 9, 8                              ' 13, {127} Back
   Data Menu_link, 5, 8, 22, 15                             ' 14, Advanced
   Data Menu_value, 14, 21, 16, 6                           ' 15, Drive Voltage
   Data Menu_value, 15, 15, 17, 7                           ' 16, LRA Frequency
   Data Menu_value, 16, 16, 18, 8                           ' 17, ERM/LRA Status, read only
   Data Menu_value, 17, 17, 19, 9                           ' 18, Set LRA
   Data Menu_value, 18, 18, 20, 10                          ' 19, OL/CL Status, read only
   Data Menu_value, 19, 19, 21, 11                          ' 20, Set Open Loop
   Data Menu_link, 9, 20, 15, 14                            ' 21, {127} Back
   Data Menu_function, 6, 14, 23, 4                         ' 22, About
   Data Menu_exit, 0, 22, 1, 0                              ' 23, {127} Exit

Menu_string_constants:
   Data "{127} Exit"                                        ' 0
   Data "Start Therapy"                                     ' 1
   Data "Stop Therapy"                                      ' 2
   Data "Device Settings"                                   ' 3
   Data "Therapy Setng"                                     ' 4
   Data "Advanced"                                          ' 5
   Data "About"                                             ' 6
   Data "Autostart"                                         ' 7
   Data "Load Defaults"                                     ' 8
   Data "{127} Back"                                        ' 9
   Data "SessionMinutes"                                    ' 10
   Data "Intensity"                                         ' 11
   Data "Jitter"                                            ' 12
   Data "Hand Mirror"                                       ' 13
   Data "Drive Voltage"                                     ' 14
   Data "LRA Frequency"                                     ' 15
   Data "ERM/LRA Status"                                    ' 16
   Data "Set LRA"                                           ' 17
   Data "OL/CL Status"                                      ' 18
   Data "Set Open Loop"                                     ' 19

Menu_values:
   ' Type, Value Child, Step/ReadOnly (if 0), Min, Max, [Init value]
   Data Menu_value_bit + Menu_value_flag_eeprom_preserve, 1, True, True   ' 1, Autostart
   Data Menu_value_byte + Menu_value_flag_eeprom_preserve, 1, 5, 5, 240, 90   ' 2, SessionMinutes
   Data Menu_value_byte + Menu_value_flag_eeprom_preserve, 2, 10, 10, 100, 100   ' 3, Intensity
   Data Menu_value_word + Menu_value_flag_eeprom_preserve, 1, 5%, 0%, 625%, 625%   ' 4, Jitter
   Data Menu_value_bit + Menu_value_flag_eeprom_preserve, 2, True, True   ' 5, Hand Mirror
   Data Menu_value_single + Menu_value_flag_eeprom_preserve, 1, .1!, 0!, 5!, 1.5!   ' 6, Drive Voltage
   Data Menu_value_word + Menu_value_flag_eeprom_preserve, 2, 5%, 80%, 300%, 250%   ' 7, LRA Frequency
   Data Menu_value_string, 1, 0                             ' 8, ERM/LRA Status, read only
   Data Menu_value_bit + Menu_value_flag_eeprom_preserve, 3, True, True   ' 9, Set LRA
   Data Menu_value_string, 2, 0                             ' 10, OL/CL Status, read only
   Data Menu_value_bit + Menu_value_flag_eeprom_preserve, 4, True, True   ' 11, Set Open Loop

Macro Menu_varpointers
   Menu_value_varpointer(1) = Varptr(AutoStart)             ' Autostart, entry: 5, value: 1
   Menu_value_varpointer(2) = Varptr(Treatment_time)        ' SessionMinutes, entry: 9, value: 1
   Menu_value_varpointer(3) = Varptr(Vib_Intensity)         ' Intensity, entry: 10, value: 2
   Menu_value_varpointer(4) = Varptr(JitterSetting)         ' Jitter, entry: 11, value: 1
   Menu_value_varpointer(5) = Varptr(HandMirror)            ' Hand Mirror, entry: 12, value: 2
   Menu_value_varpointer(6) = Varptr(Drive_Voltage)         ' Drive Voltage, entry: 15, value: 1
   Menu_value_varpointer(7) = Varptr(LRA_Frequency)         ' LRA Frequency, entry: 16, value: 2
   Menu_value_varpointer(8) = Varptr(ERM_LRA_Setting)       ' ERM/LRA Status, entry: 17, value: 1
   Menu_value_varpointer(9) = Varptr(Is_LRA)                ' Set LRA, entry: 18, value: 3
   Menu_value_varpointer(10) = Varptr(OL_CL_Setting)        ' OL/CL Status, entry: 19, value: 2
   Menu_value_varpointer(11) = Varptr(Is_OL)                ' Set Open Loop, entry: 20, value: 4
End Macro

Menu_string_characters:
   ' Table of characters used in string editing
   Data 254                                              ' start of table, needed

   Data "ABCDEFGHIJKLMNOPQRSTUVWXYZ 0123456789"

   ' special characters with function, these are needed
   Data Menu_character_backspace                         ' backspace
   Data Menu_character_finish                            ' finish string editing
   Data 254                                              ' end of table

Menu_data_jumpover:

'(
###DESIGNER###
1f8b08000000000004007d93cb72da301486f79af13b78867d2209cc65c9b5496aea1653d276e351
40504d884c659909d3e99375d147ea2b54371b04a10b6c8fcfafeffce7f8e7efef3fa81d001480fe
60381a4fdedddd3fbc8fa71f928f9f66e9fcf3e2f1cbd76f2144b8d98ada9d6e2f000957bff53a00
3800d01c9c525e8e5f990cc04f843bbf42fb8c9ba68e6d7d52f2a564b93a9b4a226438ff4e05d91d
2a4af35295ef8e228b6a5951ccf873004674cf96344ca9948c6f8a004446d33dd5b8f35ac43701e8
59c31ea6bfda13bea42bf5de02f085e1fe535e4ad31ddaa9de9ed71a8c6c7941b62555474b99177a
de009837032d9c0b5db2572d48ad6042b60535fb5498f6b9893827ab7044d7a4dccac28ca2541dab
1a90e573e5c53eb7cc227a9e97941685424d192f252d2a4307a94a3d787445897ca15c6692bdd0da
949a0ab7a0bd1b34821efb9e4bca0b260f1e16c19abb604fd989a89a153a11b211d060e4811f9894
5438ea632ed4776ae3a8a2daaa8b404d854e13557b40d863de11be0aa74c885c5cfb2c5a52298e2b
d0a8e6b595776db2901f8091607b1a2ef2ad241beabaa5caec564f7f530f6264592d3b0ea21437c8
c4c1c0db1e3c9ef5c389a03f4aca97076f4738aa37af44d989c8a1bb7ae1d036e83878c7838f67d3
5bdd40a55396555c5229cca66b8e5265ba43fd0d6cd36a19ddb304ca5089af2dfdbec84cd5a17b8e
e1a738896f87f1dba6dc39774be26c189ffb42eeff8fe185b164477918e7f9ee3ff692f8185de448
e85a2074b5d1688c9389ba06e01f534198df68050000
###DESIGNER_END###
')
