# Hardware Description

The microcontroller for this is an Arduino Nano v3.0. The output pins go to a TCA9548A, which allows programming the DRV2605L driver IC's individually (Since the driver chips all have the same address). The I2C bus also goes to the LCD. 8 or 10 lines from PortD go to each DRV2605L to allow triggering without the overhead of using I2C. The PCB also has room for LED's and a connector so these outputs can be observed. The outputs of the DRV2605L's go directly to the ERM/LRA vibrators. The DRV2605L can work with a 4 Ohm load.

There are pads at each driver module for a surface mount capacitor across the power input. A capacitor may be needed here, depending on your application.

There are pads at each driver module for a surface mount TVS diode. If you are driving a high inductance load, such as actuators made with relays, you might want a TVS diode to protect the driver modules from back EMF.
