# Robotic Arm Project (remote+joystick)

An Arduino-powered robotic arm that can be controlled using either a joystick or an IR remote.

The joystick gives manual control over the arm's movement, while the remote allows preset actions such as opening and closing the claw, resetting the arm, and running movement sequences. The project uses servo motors, an IR receiver, and joystick inputs to create a simple and interactive robotic arm system.

## Features

- Smooth joystick control for manual arm movement
- IR remote controls for preset movements
- Claw open, grip, and reset positions
- Servo-based base, arm, and claw control
- Arduino sketch ready to open in the Arduino IDE

## Hardware

- Arduino-compatible board
- 4 servo motors
- IR receiver
- IR remote
- Joystick module
- Robotic arm frame

## Files

- `RoboticArmRemoteJoystick/RoboticArmRemoteJoystick.ino` - Arduino sketch
- `media/IMG_4686.MOV` - project demo video

The demo video is a large file and should be uploaded with Git LFS or attached to a GitHub Release.

## Arduino Libraries

Install these libraries in the Arduino IDE before uploading the sketch:

- `Servo`
- `IRremote`

## GitHub Description

A robotic arm project controlled with both an IR remote and joystick using Arduino. The joystick allows smooth manual movement of the arm, while the remote can run preset movements and control the claw.
