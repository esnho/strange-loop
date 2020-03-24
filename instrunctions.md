# install system dependencies
```
sudo apt-get install libavformat-dev
sudo apt-get install libavdevice-dev
```

# install OF libraries
```
cd ~/openFrameworks/addons
git clone https://github.com/joshuajnoble/ofxWiringPi.git
wget https://github.com/jvcleave/ofxOMXPlayer/archive/0.10.0_V2.zip
unzip 0.10.0_V2.zip
rm 0.10.0_V2.zip
```

# create target mount dir for USB key
sudo mkdir /media/pi/USBKey

# enable SPI support
sudo raspi-config

- Select "Interfacing Options"
- Select "SPI"
- After enabling it reboot the device

sudo reboot now
