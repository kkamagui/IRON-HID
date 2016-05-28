# IRON-HID: Create Your Own Bad USB Device

**You can find my presentation file under link**
[HITBSecConf 2016 Amsterdam](http://conference.hitb.org/hitbsecconf2016ams/sessions/iron-hid-create-your-own-bad-usb-device/)

Although embedded hardware hacking seems to be an out-of-date technique, it is still dangerous and effective.  I created a project called “IRON-HID” – a platform for security validation that can create various types of USB devices such as HID (Human Interface Device) type, mass-storage type, and vendor-specific type. The IRON-HID contains installable software that enables you to gain full control of the target machine. You can then perform whatever actions you want on the target device via an Android smartphone application (IRON-HID Commander application).

I designed the IRON-HID platform for pen-testing using open-source hardware. IRON-HID has many functions including:
– Small form-factor and communicates with an Android application.
– Emulate a CD-ROM, so it can install a trojan without a network connection
– Hook into a user’s keyboard event and send keystrokes to the attacker
– Perform screen captures of the target device
– Receive input from pen-tester via the Android application and send it to the target machine

I created “PowerShock” to show the potential uses of the “IRON-HID” platform. PowerShock looks like a portable charger and works as you would expect. You can charge your smartphone with it and recharge it with PC. However, when you plug it to your Android smartphone or PC, PowerShock will crack your device password or send important information from your PC to an attacker.  I will also show other examples of installing IRON-HID into existing hardware such as USB card readers (ReaderShock) and keyboards (KeyboardShock).

# How to build IRON-HID Hardware
Please see my presentation file. [IRON-HID Presentation File](http://conference.hitb.org/hitbsecconf2016ams/sessions/iron-hid-create-your-own-bad-usb-device/)

# How to Build IRON-HID Software
IRON-HID is composed of three kinds of software, a firmware for a custom device, a commander program for Android Smartphone, a test agent progam for POS systems and PCs.

## Build IRON-HID Firmware and Upload
You download Ubuntu 14.04 LTS 32bit DVD image from Ubuntu official website(http://www.ubuntu.com/download/desktop) . After downloading Ubuntu image and installing Ubuntu OS, you open a terminal program and type like this for installing AVR-GCC 4.8.2.

```sh
$>sudo apt-get install gcc-avr binutils-avr gdb-avr avr-libc dfu-programmer avrdude
```

You can download LUFA 100807 version from Dean Camera’s site(http://www.fourwalledcubicle.com/LUFA.php). After downloading the “LUFA 100807.zip” file, you open a terminal program and type like this for decompressing zip file.

```sh
$>unzip LUFA-100807.zip
```

The IRON-HID’s project consists of many directories. You copy “Firmware-Teensy” directory to the LUFA directory for building firmwares. And you build the firmware by typing like this.

```
$> make clean
$> make
```

If you succeed to build a firmware, you can find the "KeyboardFirmware.hex" file and upload it to Teensy. For uploading it, check the Teensy website, [Teensy Loader](https://www.pjrc.com/teensy/loader.html).

## Build a Test Agent Program and Build CD-ROM image

Under construction...

## Build a Commander Program for Android
The commander program is made in Android Studio. The source codes are in “Commander” directory and you can import the project by selecting "Open an existing Android Studio Project". After importing it, build “Commander.apk” file by selecting "Build" -> "Make Project" menu. You can install the apk file by selecting "Run" -> "Run Application" menu or sending the file to your smartphone via email.


# How to Use IRON-HID
Under construction...

# Others...
Under construction...
