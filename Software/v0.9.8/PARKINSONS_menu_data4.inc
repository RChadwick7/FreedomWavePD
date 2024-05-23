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
   Data Menu_value_byte + Menu_value_flag_eeprom_preserve, 1, 5, 5, 150, 90   ' 2, SessionMinutes
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
1f8b08000000000004007d93cb72da301486f79ad13b78867d2209cc65c9b5496aea1653d276e351
40504d884c659909d3e99375d147ea2b54371b0ca10b6c8fcfafeffce7f7e1efef3fb80d0186a03f
188ec6937777f70fefa3e987f8e3a75932ffbc78fcf2f55b803069b6c276a7db832016fab75e4340
2040f6e0948962fcca15043f31e9fc0adc3369da3a71f54921968a67fa6ca2a854c1fc3b93747728
29cd4b55b63b8a1caae5441117cf108cd89e2f599030a5b8d8e4108456d33dd5f8f346243610f49c
e11aa6bfda53b1642bfdde01c885e1fe535628db1db9a9de9ed7190c5d7941b705d3470b95e5665e
08ec9b8111cea529b9ab11244e30a1db9cd93c35a67d6e22cae82a18b1352db62ab7a36855c7a906
74f95c7a71cf2d1b44afe6256179ae51532e0ac5f2d2d041e9520f1d5d31aa5e9850a9e22fac3215
9a7890bd138bc6a8c6be178a899cab430d8b51c55df0a7f44454ce8abc08bb1530605c033f70a598
f4d4c74ceaefd42661497555bf021515794d58e680498d7947c52a9872293379edb31849a9384660
50cd6b9177dd66e1fa028c24dfb360916d15dd30df2dd166b766fa9b6a102b4b2bd97110adb8c176
1d2cbc5d8347b37e3091ec47c1c4f250cb888455f25a949e883cba6b0247ae41c7c33b35f87836bd
350df476aaa25c9744499b74c5d1aad474a8be816b5a86d13ddb401568f1b5d0eff3d4563dbae719
f52d8ea3db61f4b6297fcedfe2281d46e7beb0ffff1374612cde31114459b6fb8fbd383aae2ef624
7c6d214cb5d1688ce389be42f00f29dcbe0b68050000
###DESIGNER_END###
')
