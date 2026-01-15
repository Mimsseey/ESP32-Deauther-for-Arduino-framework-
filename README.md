# ESP32-Deauther - _ tesa-klebeband revival _            
Original program for ESP32 with the ability to deauthenticate stations/clients connected to WiFi networks
# DISCLAIMER
- This tool has been made for educational and testing purposes only. Any misuse or illegal activities conducted with the tool are strictly prohibited. The author is **not** responsible for any criminal liability and legal consequences arising from the use of the tool, which is done at your own risk. **Always consult with your Country context-specific Law **
## Building & Compilation
- Building is done using the ArduinoIDE with ESP Core. Please use `tesa-klebeband/ESP32-Deauther` for the version of this tool compatible with PlatformIO and VsCode.
### Requirements
- ESP32 dev enviroment for Arduino-ide 
- A patched platform.txt in Arduino installation directory, typically in: `C:\Users\<YourUsername>\AppData\Local\Arduino15\packages\esp32\hardware\esp32\<version>\boards.txt` to allow linker flags to bypass ESP WiFi library `raw_frame_sanity_check` built-in security feature 
### Patching platform.txt
Make sure you to replace `{version}` with the currently installed version of the ESP32 dev enviroment (for example 2.0.9). 
Open the file in your favourite text editor and add:
* **-zmuldefs** to **compiler.c.elf.libs.esp32**
* **-w** to **build.extra_flags.esp32**

Note: ** If you are using other board from Espressif, add latter mentioned flags to appropriate board **

This will allow the c compiler to tolerate multiple definitions, which are used to bypass the deauthentication frame check in the ESP32's firmware. After this you can hit upload in the ArduinoIDE
## Using ESP32-Deauther
The ESP32 hosts a WiFi network with the name of `ESP32-Deauther` and a password of `lozinka32`. Connect to this network and type the IP of your ESP32 (typically **192.168.4.1**) into a webbrowser of a choice. You will see the following options:
* Rescan networks: Rescan and detect all WiFi networks in your area. After a successful scan, the networks are listed in the above table.
* Launch Deauth-Attack: Deauthenticates all clients connected to a network. Enter the network number from the table at the top and a reason code from the table at the bottom of the page. After that click the button and your ESP32's LED will flash when it deauthenticates a station.
* Deauth all networks: Launches a Deauth-Attack on all networks and stations with a specific reason. In order to stop this, you have to reset your ESP32 (no other way to code this since the ESP32 rapidly changes through all network channels and has to disable its AP)
* Stop Deauth-Attack: Does what it says (lol)
## License
All files within this repo are released under the GNU GPL V3 License as per the LICENSE file stored in the root of this repo.
<img width="1080" height="2340" alt="Screenshot_20250828_011122_Firefox" src="https://github.com/user-attachments/assets/c958af8b-6493-4ab6-abaa-239b75a9cb60" />

<img width="1080" height="2340" alt="Screenshot_20250831_165528_Samsung Internet" src="https://github.com/user-attachments/assets/efcb2195-33ea-4432-a735-1820862a0d03" />
