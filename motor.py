# ECE5725: Lab 3
# Alex Wadell, alw224
# Sid Nanda, sn435
# 2017/4/9

import RPi.GPIO as GPIO

# Set up GPIO
#GPIO.setmode(GPIO.BCM)

class Motor:
  def __init__(self, pin, name, fwdPin, stopPin, revPin):
    # Setup Motor GPIO
    self.mPin = pin
    self.name = name
    GPIO.setup(pin, GPIO.OUT)
    
    # Start PWM
    self.pwm = GPIO.PWM(pin, 50)
    self.pwm.start(0)
    
    # Motor Kill Button
    self.live = True

    # Set Contol Pins
    self.fwdPin = fwdPin
    self.stopPin = stopPin
    self.revPin = revPin
    self.currentDir = 0
    self.counter = [0,0,0]

  def setSpeed(self, dir):
    if self.live and not(self.currentDir == dir):
      if dir is -1:
        s = 6.5 #pwmRev
        self.currentDir = dir
        self.counter[0]+=1
      elif dir is 1:
        s = 8.5 #pwmFwd
        self.currentDir = dir
        self.counter[2]+=1
      else:
        self.currentDir = dir
        s = 0.0 #pwmStop
        self.counter[1]+=1
      
      self.pwm.ChangeDutyCycle(s)

  def setLive(self, state):
    if state:
      self.live = True
      newDir = self.currentDir
      self.currentDir = 0
      self.setSpeed(newDir)
    else:     
      oldDir = self.currentDir
      self.setSpeed(0)
      self.currentDir = oldDir
      self.live = False

  def checkPin(self, pin):
    if (pin == self.fwdPin):
      self.setSpeed(1)
    elif (pin == self.stopPin):
      self.setSpeed(0)
    elif (pin == self.revPin):
      self.setSpeed(-1)