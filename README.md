# Spinning Light
___
This project aims to bring gaming RGB and and spinning bikes to your home trainer. The lamp pairs with the Power trainer and maps the power to its LEDs. The color corresponds to the zones based on the set FTP value. When sprinting the LEDs show this by randomly turning red and yellow to add motivaition to you pain cave. 
All parameters such as the brightness of the LEDs, the FTP value and the power trainer BT adress can be changed via the GUI. Under Advanced settings, the parameters on the zone percentages, the filter constants for the LEDs and the amount of pixels can be changed. 
___
On the main screen, the sensor data for humidity and temperatre is displayed, on the second page the current power and on the third is the bluetooth adress and the power trainers name displayed. 
# Controller and Code
The light uses an ESP32 NodeMCU mini to not take up to much space on the PLC. The programming was done using Arduino IDE with the ESP32 Boards installed (select Node32s). The push button included in the encoder is used without an interrupt. This somehow helps to improve performance and helps prevent wrongly detected button inputs. Since a real pull up resistor is used, the internal pull up on this pin is disabled. 
## Connecting Bluetooth
The connection is done using BTLE with the respective libary included in this sketch. Ideally, it should write the adress to the EEPROM to save for later startups and connect automatically. However, this does not work as intended. Once a power trainer is connected, the ESP is able to reconnect automatically. But when the ESP is restarted, the trainer needs to be searched again. 
Once connected, the ESP subscribes to the Power Service ot the trainer and thereby gets the power the trainer detects. The color is displayed on the LED strip while being mapped to the respective color.

# Casing 
The casing is designed with modularity in mind. All holes such as the display, the encoder, the temperature sensor and the USB port can be opened after the printing is complete. So if you want to hardcode the BT adress and dont want to use the GUI that is possible. 
![Exploded view](https://github.com/user-attachments/assets/75ef979d-03b7-4561-b3dc-d7930409f4f4){width=50%}


# PCB
![IMG_20250216_102324_218](https://github.com/user-attachments/assets/e6889604-5c36-4e03-aae6-2c7883edb1f4){width=50%}
The PCB is designed to work with Dupond connectors for all external parts. The ESP is mounted on the back to leave space for the casings lid. The USB-C port of the ESP is placed in a way to allow access from the outside if the plastic is removed. It can be powered either via USB or via the 5V connector that can be placed in the hole on the left. The encoder is placed directly on the PCB and it uses three pull up resistors (two for the rotation of the encoder, one for the push button on the encoder). All external connectors are 
The supports do not all need to be screwed in, since they mostly support the push button to prevent the PCB from bending when pushing. 
![IMG_20250216_104605_619](https://github.com/user-attachments/assets/20a13f36-a476-45df-8c34-f55ceb221f56){width=50%}
#Assembly
The Temperature sensor can be screwed on the left side of the casing. To make it fit, the respectable part of the lid needs to be removed to allow space for the PCB of the sensor to stick out. The sensor is mounted using M2 screws. It should be screwed on from the outside and needs to be fixed using a nut on the other side. This needs to be done first.
Next, the threads can be placed in the holes of the standoffs. At minimum, four screws should be used to fix the PCB. THe other standoffs are mainly for the support of the encoder push button. The PCB can be fixed in the casing and the 5V supply can be connected if it should be used. Otherwise, the power can be supplied via the USB port. 
![IMG_20250216_113616_389](https://github.com/user-attachments/assets/612b4093-2728-4690-a1e3-c02f1faa55dd){width=50%}

Next, the lid needs to be prepared. The parts of the lid where the IO should be placed needs to be removed (the square for the screen and the circle for the encoder). The OLED screen needs to be prepared with a dupond connector and can be srewed in place. If the screen is fixed on the lid or below does not mapper, if it is placed below however, one needs to make shure the screen does not break. 
Finally, the profile with the LEDs needs to be attatched on the top with the cable sticking into the casing. Depending on the LEDs that are used, the Cables need to be prepared according to the PCBs silkscreen. When the lid is prepared and the connectores have been attatched, it can be placed on the lid. When a temperature sensor is used, this needs to be done carefully to not break any parts. It can be tricky to place it correctly because of the sensor and the encoder interfering with other parts. Once done, screw the lid down and everything should work. 
![IMG_20250216_135642_520](https://github.com/user-attachments/assets/46d06167-4708-4c2c-b793-650a5338f7ea){width=50%}
#Usage
At startup, the LEDs light up in random colours. When pressing down the encoder, you are directed to the menu. Here, you can change the FTP, connect the powertrainer and change the brightness (default is 50%). When changing the brightness, you get a preview with the Z1 color on the first LED and the Z6 color at the top and all other colors in between. 
In the menu, pressing back brings you to the main screen with either the current temperature and humidity, the power or the current connected device (can be changed with the encoder). On the top right, the symbol indicates if the powertrainer is connected or not. 
You can go back to the main menu by pressing the button down. The last point in this menu is the Advanced Settings tab. When navigating there and pressing down the encoder, you can change the percentage of the zones (e.G. changing the Z1 percentage from 60% of the FTP to 70% of the FTP). You can also change the amount of LEDs that you use (default is 144) and the constants of the filter. The filter is used to achive a gradual change in the LED color. It is implemented as a classical low-pass filter. The filter constant defines the percentage of the new colour of the most recent power reading (e.G when using 0.05, the new colour is obtained by taking 95% of the old colour and 5% of the new color based on the power setting). Since the LEDs refresh multiple times per second this gives a smooth effect in color change.
The spark constant simmilarly defines the amount of sparks when exeeding Z7. A higher spark constant gives more sparks when sprinting and a lower gives less. 
The last point in the advanced settings tab resets the config data to the default settings. 
Once a setting has been changed and the main page (where you see the power, temperature or device adress) is accessed, the settings are saved to the eeprom and are accessed at every start up. So you only need to set your FTP, trainer adress (in theory), brightness, etc. once.
#Limitation/Improvements
- The light only works when your trainer has at least two bluetooth channels. I got to test the Wahoo KickrCore and the Zwift Hub. Since the Wahoo KickrCore works I suspect it works with other Wahoo power trainers as well, but I have not tested it. But I am happy for Information on this topic. The following table can be updated.
| Works           | Does not Work     |
|-----------------|-------------------|
| Wahoo KickrCore | Zwift Hub/Hub One |
- For some reason, at startup of the microcontroller it does not connect automatically to the power trainer even if the adress should work. When connecting manually, it connects without a problem and reconnects when the power trainer loses power. Since the only thing used for connecting is the adress and that is saved to the config data, I have no idea why it does not work. I have tried to scan at startup or connect multiple times, but nothing helps. So the workaround is to just dont power off the lamp (it does not consume a lot of power). For advice I would be very thankful.
- Sometimes, the UI feels a bit janky. Espeacially when turniing the encoder fast. I suspect that the main reason is the cheap encoder, but I could be wrong. Turning the encoder knob slowly usually works.

