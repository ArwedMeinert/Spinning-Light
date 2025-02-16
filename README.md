# Spinning Light
___
This project aims to bring gaming RGB and and spinning bikes to your home trainer. The lamp pairs with the Power trainer and maps the power to its LEDs. The color corresponds to the zones based on the set FTP value. When sprinting the LEDs show this by randomly turning red and yellow to add motivaition to you pain cave. 
All parameters such as the brightness of the LEDs, the FTP value and the power trainer BT adress can be changed via the GUI. Under Advanced settings, the parameters on the zone percentages, the filter constants for the LEDs and the amount of pixels can be changed. 
___
On the main screen, the sensor data for humidity and temperatre is displayed, on the second page the current power and on the third is the bluetooth adress and the power trainers name displayed. 
# Controller and Code
The light uses an ESP32 NodeMCU mini to not take up to much space on the PLC. The programming was done using Arduino IDE with the ESP32 Boards installed. The push button included in the encoder is used as an interrupt pin. 
## Connecting Bluetooth
The connection is done using BTLE with the respective libary included in this sketch. Ideally, it should write the adress to the EEPROM to save for later startups and connect automatically. However, this does not work as intended. Once a power trainer is connected, the ESP is able to reconnect automatically. But when the ESP is restarted, the trainer needs to be searched again. 
Once connected, the ESP subscribes to the Power Service ot the trainer and thereby gets the power the trainer detects. The color is displayed on the LED strip while being mapped to the respective color.

# Casing 
The casing is designed with modularity in mind. All holes such as the display, the encoder, the temperature sensor and the USB port can be opened after the printing is complete. So if you want to hardcode the BT adress and dont want to use the GUI that is possible. 
![Exploded view](https://github.com/user-attachments/assets/75ef979d-03b7-4561-b3dc-d7930409f4f4)

# PCB
The PCB is designed to work with Dupond connectors for all external parts. The ESP is mounted on the back to leave space for the casings lid. 
