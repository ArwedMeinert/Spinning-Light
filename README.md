# Spinning Light

___  
This project combines dynamic RGB lighting with a home trainer to enhance your workout experience. The lamp pairs with a power trainer, mapping your power output to its LEDs. The displayed color corresponds to different zones based on your set FTP value. During sprints, the LEDs flash red and yellow to motivate you in your "pain cave."  

All parameters—such as LED brightness, FTP value, and the power trainer’s Bluetooth address—can be adjusted via the GUI. Under **Advanced Settings**, you can modify the zone percentages, LED filter constants, and the number of pixels.  
___  
On the main screen, sensor data for humidity and temperature is displayed. The second page shows the current power output, while the third page displays the Bluetooth address and the power trainer's name.

---

# Controller and Code

The lamp is built around an ESP32 NodeMCU Mini to keep the design compact. The firmware was developed using the Arduino IDE with the ESP32 board definitions (select Node32s). The encoder’s built-in push button is handled without an interrupt, which improves performance and reduces false button detections. A physical pull-up resistor is used, so the internal pull-up is disabled on that pin.

## Connecting Bluetooth

Bluetooth Low Energy (BTLE) is used for connectivity, with the necessary library included in the sketch. Ideally, the device writes the trainer’s address to the EEPROM for automatic reconnection on subsequent startups. However, while the ESP can reconnect automatically after a trainer is initially connected, it does not do so after a restart and must search for the trainer again. Once connected, the ESP subscribes to the power service of the trainer and receives its power data, which is then mapped to LED colors.

---

# Casing

The casing is designed for modularity. Openings for the display, encoder, temperature sensor, and USB port can be finalized after printing. This also allows you to hardcode the Bluetooth address if you prefer not to use the GUI.

<img src="https://github.com/user-attachments/assets/75ef979d-03b7-4561-b3dc-d7930409f4f4" alt="Exploded view" style="width:25%;">

---

# PCB

<img src="https://github.com/user-attachments/assets/e6889604-5c36-4e03-aae6-2c7883edb1f4" alt="IMG_20250216_102324_218" style="width:20%;">

The PCB is designed to work with DuPont connectors for all external components. The ESP is mounted on the back to leave space for the casing lid. The ESP's USB-C port is positioned to allow external access if the plastic cover is removed. The board can be powered either via USB or through the 5V connector located in the left opening. The encoder is mounted directly on the PCB and uses three pull-up resistors—two for rotation detection and one for the push button.

Not all supports need to be screwed in; they primarily help stabilize the push button and prevent the PCB from bending under pressure.

<img src="https://github.com/user-attachments/assets/20a13f36-a476-45df-8c34-f55ceb221f56" alt="IMG_20250216_104605_619" style="width:20%;">

---

# Assembly

The temperature sensor is secured to the left side of the casing. To ensure a proper fit, a section of the lid must be removed to allow the sensor's PCB to protrude. The sensor is mounted using M2 screws, fastened from the outside with a nut on the opposite side. This step should be completed first.

Next, insert the screws into the standoff holes. Use at least four screws to secure the PCB; the remaining standoffs primarily support the encoder’s push button. Once the PCB is fixed within the casing, connect the 5V supply if necessary—or use the USB port for power.

<img src="https://github.com/user-attachments/assets/612b4093-2728-4690-a1e3-c02f1faa55dd" alt="IMG_20250216_113616_389" style="width:20%;">

Prepare the lid by removing the sections designated for I/O components (the square for the screen and the circle for the encoder). The OLED screen should be equipped with a DuPont connector and then screwed in place. It doesn't matter whether the screen is mounted on the lid or underneath it; however, if it is mounted below, ensure it is properly supported to avoid damage.

Finally, attach the LED profile to the top, ensuring that the cable is routed into the casing. Depending on the LEDs used, prepare the cables according to the PCB's silkscreen guidelines. Once the lid is modified and the connectors are attached, carefully install the lid to avoid interfering with the sensor and encoder. Finally, secure the lid with screws, and your setup should be operational.

<img src="https://github.com/user-attachments/assets/46d06167-4708-4c2c-b793-650a5338f7ea" alt="IMG_20250216_135642_520" style="width:20%;">

## Bill Of Material (BOM)
# Bill of Materials (BoM)

<details>
  <summary>Click to expand</summary>

| #  | Reference         | Qty | Value                 | Footprint                                                         | DNP |
|----|------------------|-----|----------------------|-----------------------------------------------------------------|-----|
| 1  | J0               | 1   | Power                | Connector_PinHeader_2.54mm:PinHeader_1x02_P2.54mm_Vertical      |     |
| 2  | J1               | 1   | Neopixel             | Connector_PinHeader_2.54mm:PinHeader_1x03_P2.54mm_Horizontal    |     |
| 3  | J2               | 1   | Temperature Sensor   | Connector_PinHeader_2.54mm:PinHeader_1x03_P2.54mm_Horizontal    |     |
| 4  | J3               | 1   | OLED                 | Connector_PinHeader_2.54mm:PinHeader_1x04_P2.54mm_Horizontal    |     |
| 5  | J4               | 1   | Reset                | Connector_PinHeader_2.54mm:PinHeader_1x02_P2.54mm_Vertical      |     |
| 6  | J5               | 1   | Reserve IO23 5VCC    | Connector_PinHeader_2.54mm:PinHeader_1x03_P2.54mm_Horizontal    |     |
| 7  | J6               | 1   | Reserve IO25 5VCC    | Connector_PinHeader_2.54mm:PinHeader_1x03_P2.54mm_Horizontal    |     |
| 8  | J7               | 1   | Reserve IO09 3V3     | Connector_PinHeader_2.54mm:PinHeader_1x03_P2.54mm_Horizontal    |     |
| 9  | J8               | 1   | Reserve IO12 3V3     | Connector_PinHeader_2.54mm:PinHeader_1x03_P2.54mm_Horizontal    |     |
| 10 | J9               | 1   | GND                  | Connector_PinHeader_2.54mm:PinHeader_1x03_P2.54mm_Horizontal    |     |
| 11 | J10              | 1   | 3V3                  | Connector_PinHeader_2.54mm:PinHeader_1x03_P2.54mm_Horizontal    |     |
| 12 | J11              | 1   | 5VCC                 | Connector_PinHeader_2.54mm:PinHeader_1x03_P2.54mm_Horizontal    |     |
| 13 | R1, R2, R3       | 3   | 10k                  | Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P7.62mm_Horizontal   |     |
| 14 | SW1              | 1   | RotaryEncoder_Switch | Rotary_Encoder:RotaryEncoder_Alps_EC11E-Switch_Vertical_H20mm   |     |
| 15 | U1               | 1   | esp32-wemos-d1-mini  | ESP32Mini:esp32-wemos-d1-mini                                   |     |
| 16 | U2               | 1   | I2C IIC BiDirektional 5V~3.3V   | Level Shifter:4xLevel Shifter                                  |     |
| 17 | WS2812 Neopixel  | 1   | WS2812B LED Strip    | LED Strip (1m)                                                 |     |
| 18 | Diffusion Profile| 1   | Aluminium (1m)       | LED Diffuser Aluminum Profile                                  |     |
| 19 | DHT2302 Sensor   | 1   | DHT2302              | Temperature & Humidity Sensor                                  |     |
| 20 | OLED Display     | 1   | 0.96\" I2C OLED       | 128x64 OLED Display Module                                    |     |

</details>

### Notes:
- **WS2812 Neopixel**: Addressable LED strip (1m).
- **Aluminium diffusion profile**: 1-meter LED diffuser.
- **DHT2302**: Temperature and humidity sensor.
- **OLED display**: 0.96\" I2C screen (128x64 resolution).


---

# Usage

Upon startup, the LEDs display random colors. Pressing the encoder takes you to the menu, where you can adjust the FTP, connect the power trainer, and change the brightness (default is 50%). As you adjust the brightness, a preview gradient is shown—from the Z1 color on the first LED to the Z6 color on the last LED.

Within the menu, pressing the back button returns you to the main screen, which displays the current temperature and humidity, power output, or connected device (selectable via the encoder). A symbol in the top right corner indicates whether the power trainer is connected.

Pressing the button again returns you to the main menu. The final option in the menu is the **Advanced Settings** tab. Here, you can adjust the zone percentages (for example, changing the Z1 zone from 60% of FTP to 70% of FTP), the number of LEDs (default is 144), and the filter constants. The filter provides a gradual transition between LED colors using a low-pass algorithm. The filter constant determines the weight of the most recent power reading; for example, a constant of 0.05 means the new color is calculated as 95% of the previous color and 5% of the new value. Since the LEDs refresh multiple times per second, this results in a smooth color transition.

Similarly, the spark constant controls the number of sparks triggered when exceeding zone Z7. A higher spark constant produces more sparks during sprints, while a lower constant produces fewer sparks. The final option in the Advanced Settings tab resets the configuration data to its default values. Once you change a setting and return to the main screen, the settings are saved to the EEPROM and loaded at each startup—so you only need to configure your FTP, trainer address, brightness, and other settings once.

---

# Limitations and Improvements

- **Bluetooth Channels:**  
  ~~The lamp only works with trainers that support at least two Bluetooth channels.~~ It is only possible to connect the lamp and Zwift via bluetooth if the power trainer supports two bluetooth cahannels. It is however possible to connect Zwift via ANT+ and the lamp via BTLE (virtual shifting does not work in this case). I have tested the bluetooth connection with the Wahoo KickrCore and the Zwift Hub. Since the Wahoo KickrCore works, I suspect it will also work with other Wahoo power trainers, although I have not tested them. I welcome any additional information on this topic. See the table below.

- **Automatic Connection Issue:**  
  For some reason, upon startup the microcontroller does not automatically connect to the power trainer, even though the saved address appears to be correct. Manual connection works without issue, and the device reconnects if the power trainer loses power. Despite saving the address in the configuration data, automatic reconnection does not function as expected. I have tried scanning and multiple connection attempts at startup without success. The current workaround is to avoid powering off the lamp (as it consumes minimal power). Any advice on resolving this issue would be greatly appreciated.

- **UI Responsiveness:**  
  At times, the user interface may feel somewhat sluggish, particularly when the encoder is turned rapidly. I suspect this is primarily due to the inexpensive encoder, though other factors may contribute. Turning the encoder slowly generally results in smooth operation.

---

# Known Compatible Power Trainers

| Works           | Does Not Work     |
|-----------------|-------------------|
| Wahoo KickrCore | Zwift Hub/Hub One |


