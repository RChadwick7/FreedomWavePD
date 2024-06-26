# FreedomWavePD
### An open source version of the equipment used by researchers to treat Parkinson's Disease.
![alt text](Pictures/FreedomWave.jpg)

- UPDATE 05-18-2024 The menu is now implemented in the firmware.
- UPDATE 02-19-2024 The PCB file should now be good. You can now see the assembly video here: https://youtu.be/leJTp28FIpU . The PCB used in the video had a few small mistakes that were corrected in the posted PCB and Gerbers, as well as the links to ordering a PCB.

**** If you don't know what this is, see this video: https://youtu.be/YEEwbxFT4Bc?feature=shared

Like many others, one morning I was watching the news and saw a story about Dr. Peter Tass at Stanford who was using vibrating gloves to treat Parkinson's Disease with amazing results. Realizing it might be years before they become available, I decided to make my own. What should have been a month of work stretched out to 8 months as I have very limited spare time. 

The theory of these gloves is relatively simple. You attach a device that either vibrates or physically pushes a small pin into the skin (NOT breaking the skin) to each finger, minus the thumbs (8 in total). These are activated in specific random patterns. The vibration triggers nerves in the fingers and these nerves go up to the brain. When these nerves are triggered in certain patterns, they can, in simple terms, help the brain straighten itself out. Look at Dr. Tass's work for much more detail.

The design goals were to make everything as flexible and adjustable as possible. While the results seem incredible, the researchers don't necessarily know the best settings yet. The gloves I constructed are bulky and awkward to put on but have the advantage of being able to put the vibrator anywhere on the fingertip. The circuit uses 8 DRV2605L haptic driver ic's but has room for 10 DRV2605L's in case there may be a benefit to using the thumbs. While these are more expensive than simple transistor drivers, they will easily drive both ERM and LRA vibrators. If there's demand, I'll design a daughterboard that will let this drive Piezo actuators.

Another design goal was to make this as easy as possible to make. I originally had hoped this could be done without soldering, but this just doesn't seem likely. My intention is that you will be able to order a PCB and all components, solder mostly modules to the PCB, and make the gloves. I have a video on YouTube showing step by step how to make the control box.


What are ERM and LRA motors? ERM stands for Eccentric Rotating Mass. Basically, they are motors that spin, and have an unbalanced weight on the output. When you add electricity, the motors spin, and they vibrate. LRA work a little differently. Linear Resonant Actuators have a magnet on a spring and a coil. If you have seen how loudspeakers work, these work on a very similar principle. When you add voltage, the coil generates a magnetic field and either repulses or attracts the magnet. If you reverse the polarity, the magnet is either attracted or repulsed, based on the polarity. If you keep alternating the polarity, the magnet will keep moving back and forth. While you can alternate the current at any frequency you want, these tend to resonate at specific frequencies and work best when you drive them at their resonant frequency.

The current state of the project is that everything should be functional. It implements all the patterns properly (I spent a LOT of time making sure randomness and jitter were spot on). The menu allows you to configure the control box to work with your specific gloves (ERM, LRA, Other) as well as treatment time, jitter, hand mirror, and others. More polish and features will be added as needed but everything is functional.

The project is divided into two parts: the gloves themselves, and the controller that runs and controls the gloves. While I don't currently have directions for making them, there are pictures here that should be enough to figure it out. You can also design your own gloves, or use some else's design. I will be mostly focusing on the controller, which is the harder and more critical part of the project. I'm currently looking into using audio transducers with a 3D Printed housing. Most files are already here but I will post more details as I get them.

The code is written in BASCOM BASIC, available at https://www.mcselec.com/ . It's not free but if you want to easily program microcontrollers it's worth every penny. I purchased a license over 20 years ago and I can't recommend it enough.

The schematic is in the Hardware folder. It looks complicated but it's mainly just wires. You could make this point-to-point but making a PCB is much easier. Be aware the schematic includes a lot of stuff you may not need. Two output connectors are added for the haptic actuators. You only need one. Also, I added driver IC's for thumbs. This isn't implemented. It's just there in case thumb actuators turn out to be useful in the future.

DISCLAIMER!!!!!!! I am NOT a Doctor. I don't make any claims that these gloves will do anything, including vibrate.
