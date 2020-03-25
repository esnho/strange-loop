# install system dependencies
```
sudo apt-get install libavdevice-dev
```

# install OF addons
```
cd ~/openFrameworks/addons
git clone https://github.com/joshuajnoble/ofxWiringPi.git
wget https://github.com/jvcleave/ofxOMXPlayer/archive/0.10.0_V2.zip
unzip 0.10.0_V2.zip
mv ofxOMXPlayer-0.10.0_V2 ofxOMXPlayer
rm 0.10.0_V2.zip
```

# create target mount dir for USB key
sudo mkdir /media/pi/USBKey

# enable SPI support
sudo raspi-config

- Select "Interfacing Options"
- Select "SPI"
- Exit from config tool
- Reboot the device

# use with ofxMidi
if like me you want to use this toy with ofxMidi you should edit ofxOMXPlayer in order to compile with ofxMidi library:

https://github.com/jvcleave/ofxOMXPlayer/issues/142
