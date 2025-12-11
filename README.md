# ![](\mainpage) PlatformSoftKeyboardControl 
Control a 2 Degree of Freedom Platform using a soft sensing sheet.

This README is also used as the main page for the Doxygen-generated documentation.
The remainder of this file contains project details, diagrams, and usage
information.
# PlatformSoftKeyboardControl 

Project Goal:

Control a 2 Degree of Freedom Platform using a custom piezoresistive sensing sheet.
## Mechanical Considerations
The balancing table is controlled by two motors connected to the table's surface via a strut with two ball joints that control rotation about perpendicular axes.

<img width="663" height="512" alt="Lever Arm Powered By Motor" src="https://github.com/user-attachments/assets/a2d62a46-fd9c-42a5-a989-7851e44c7777" />

## Software Design
### Task Diagram

<img width="781" height="659" alt="ESP32 Code Task Diagram" src="https://github.com/user-attachments/assets/38f003e1-e04e-4a36-8adb-b4d4d479dcad" />

### Webserver Task 
The ESP32 hosts its own wifi on which it hosts a webpage on 192.168.5.1 that communicates the recorded data in csv forma to either the user or an external program that interprets it. The functionality of this task is based on an example by ![A. Sinha](https://github.com/hippyaki/WebServers-on-ESP32-Codes).

<img width="458" height="314" alt="Webserver Task State Diagram" src="https://github.com/user-attachments/assets/f6c1ada7-053a-4696-8f9d-5a1bf2a299a8" />

### Motor Control Task 
The motor control task awaits confirmation that the IMU has initialized and then begins operating the motor PIDs according to the setpoint stored in the xBar and yBar shares.

<img width="1010" height="451" alt="Motor Control State Diagram" src="https://github.com/user-attachments/assets/63f04a26-7069-487f-a202-9b285c1271b7" />

### Material Reading Task 
In order to perform EIT analysis, a series of voltage differences needs to be measured. For one complete measurement, one electrode is grounded while its neighboring electrode is supplied with a current while the remaining electrodes are used to measure the voltage differences. This is then repeated for each electrode. This task is responsible for taking those individual datapoints and reporting the resulting 208 values to the webpage task to be published in csv format.

<img width="840" height="629" alt="Material Reading State Diagram" src="https://github.com/user-attachments/assets/ca44845a-5584-47f4-b5b2-e5d67d461c8b" />

### PyEIT interpretation
A python script using the ![pyEIT module](https://github.com/eitcom/pyEIT) is able to read the data hosted by the webpage and analyze any changes in resistivity of the material. The resulting centroid is then passed back to the webpage using url arguments.

## Other Code Used
- Liu, et al:
+ - https://github.com/eitcom/pyEIT 
- bryanduxbury:
+ - https://github.com/bryanduxbury/adc128d818_driver 
- yuskegoto:
+ - https://github.com/yuskegoto/PCA9956 
- spluttflob:
+ - PrintStream Library https://github.com/spluttflob/Arduino-PrintStream.git 
+ - ME507 Support Library https://github.com/spluttflob/ME507-Support.git
- madhephaestus:
+ - https://github.com/madhephaestus/ESP32Encoder 
