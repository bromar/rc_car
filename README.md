<h1 align="center">Smart RC-Car</h1>
&nbsp;

<h4><em>High Level Description</em></h4>

This invention is a transportation designed for your home. It is created to help you move or transport items around your house. It is manually controlled by a joystick to move it up, down, left, and right. 4 motors attached to it help it move around like a car. The vehicle also has a collision detection system infront of the car. It lets you know whether it is safe or not to continue moving forward. If there are no obstacles in the way, then the LED matrix will display a GO pattern. If an object is approaching ahead, it will give you a warning signal on the matrix. Finally, if there is an object directly infront of you, then it will give you a stop signal on the matrix.

&nbsp;
<h4><em>User Guide</em></h4>
* Move joystick up to move forward
* Move joystick down to go in reverse
* Move joystick left to move left
* Move joystick right to move right

&nbsp;

<h4><em>Components</em></h4>
* 2 ATMega32 microcontrollers
* Arduino Uno
* 8x8 RGB LED matrix
* 4 stepper motors
* 4 Shift Registers
* Joystick
* LCD screen
* Distance sensors

&nbsp;

<h4><em>MCU Pin Layout</em></h4>

<h6>AVR MCU 1</h6>
| Component | Pins
| ------------------- | :------------------------: |
| Data to 2nd AVR     | B4-B6                      |
| Input from Arduino  | D0-D7                      |
| Joystick            | A0, A1                     |
| Motors              | B0-B3, A4-A7, C0-C3, C4-C7 |

<h6>AVR MCU 2</h6>
| Components | Pins
| ------------------------ | :----------: |
| Input from Arduino, AVR1 | A0-A7, B0-B7 |
| LED Matrix               | C0-C2        |

<h6>Arduino Uno</h6>
| Component | Pins
| ------------------------------- | :---------: |
| Output to AVR2                  | Pins 2-9    |
| Echo(helps measure distance)    | Pin 10      |
| Trigger(helps measure distance) | Pin 11      |
| Onboard LED                     | Pin 13      |
