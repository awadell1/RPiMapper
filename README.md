# Objective
The purpose of the RPiMapper was build a mobile robotics platform for mapping an environment using an array of ultrasonic sensors. Mapping is done using an occupancy grid method based on the algorithm detailed in [Probabilistic Robotics](http://www.probabilistic-robotics.org/). By using a 360 degree array of 8 sonars sensors updated at 4Hz, the RPiMapper is designed to be able to quickly map an environment. Localization while mapping was done by tracking the rotation of the wheel spokes with limited success and future iterations should focus on improving localization.

# Introduction
The RPiMapper consists of three main systems:
* Arduino Nano: Handles Motor control timing, sonar measurements and odometry measurements
* The Raspberry Pi 3: Relays control commands and sensor measurements between the host computer and the Arduino via a web-socket
* The Host Computer: Runs the MATLAB code that handles updating the occupancy grid and sets robot velocity commands

This division of labor works to play to the strengths of each system, the Arduino generates the timing pulse required to control the motors, and precisely measures the sonar pulse width. The RPi3 then communicates with the Arduino using I2C and relays measurements over a web-socket to the host computer. To prevent timing issues on the Arduino, RPi3 receives raw measurements from the Arduino, which it then converts into SI units before sending the data along to the host computer. Similarly wheel speeds are given by the Host in m/s are converted into a percentage of max wheel speed before sending to the Arduino.
Finally, the Host Computer uses a MATLAB script to update the occupancy grid based on the current sensor readings and estimated pose, and presents a GUI for setting the robot's forward and angular velocity. 