Convert your Android device into USB keyboard/mouse, control your PC from your Android device remotely, including BIOS/bootloader.


Installation
============

Nexus 7 2012 WiFi (Grouper), with Android 4.4.4
-----------------------------------------------

- Plug your device into PC using USB cable.
- Power off the device.
- Hold Volume Down button and Power button for 5 seconds, to enter fastboot mode.
- Copy appropriate fastboot executable from the directory `fastboot`.
- Launch command `fastboot oem unlock`
- Confirm unlock action by pressing Power button. This will factory reset your device.
- Copy *boot.img* from directory [nexus7-2012-wifi-grouper](nexus7-2012-wifi-grouper).
- Launch command `fastboot flash boot boot.img`.
- Reboot your device using Power button.
- Install and run USB Keyboard app.

LG G2
-----

- http://forum.xda-developers.com/showthread.php?t=2725023

Nexus 5
-------

- http://forum.xda-developers.com/showthread.php?t=2551441
- http://forum.xda-developers.com/showthread.php?t=2527130
- [Boot image for Android 5.0](nexus5-hammerhead-android-5.0)

Nexus 4
-------

- http://forum.xda-developers.com/showthread.php?t=2548872

Sony Ericsson phones
--------------------

- http://legacyxperia.github.io/

Motorola Moto G with Cyanogenmod
--------------------------------

- http://forum.xda-developers.com/showthread.php?t=2634745
- http://forum.xda-developers.com/showthread.php?t=2786336

Motorola Moto E with Cyanogenmod
--------------------------------

- http://forum.xda-developers.com/showthread.php?t=2931985

OnePlus One
-----------

- http://sourceforge.net/projects/namelessrom/files/bacon/ - it's ROM, not just a kernel

Galaxy S4
---------

- http://forum.xda-developers.com/showthread.php?t=2590246 - you have to enable  in the included STweaks app

Galaxy Note 2
-------------

- http://forum.xda-developers.com/showthread.php?t=2231374

Huawei Ideos X5
---------------

- http://forum.xda-developers.com/showthread.php?t=2616956

Sony Xperia Z3 and Z3 Compact
-----------------------------

- http://forum.xda-developers.com/showthread.php?t=2937173

Sony Xperia Z Ultra
-------------------

- http://forum.xda-developers.com/showthread.php?t=2926584

Xiaomi Redmi 1S
---------------

- http://forum.xda-developers.com/showthread.php?t=2998620

Other devices
-------------

- You will have to compile the kernel yourself.

Scripting
=========

There is a possibility to send keypresses in an automated way, using terminal emulator for Android or similar app.
This is done using [hid-gadget-test](hid-gadget-test/hid-gadget-test) utility.

First, copy this utility to your device.

	adb push hid-gadget-test/hid-gadget-test /data/local/tmp

You will need to set world-writable permissions on /dev/hidg0, or run hid-gadget-test from root shell.

	adb shell
	su
	chmod 666 /dev/hidg0 /dev/hidg1

To always have root shell, so you don't need to enter 'su' each time, run command

	adb root

Then, use hid-gadget-test to send keypresses.

	adb shell
	cd /data/local/tmp

	# Send letter 'a'
	echo a | ./hid-gadget-test /dev/hidg0 keyboard

You can also run this command without launching ADB shell, from shell script or .bat file.

	adb shell 'echo a | /data/local/tmp/hid-gadget-test /dev/hidg0 keyboard'

Advanced examples.

	# Send letter 'B'
	echo left-shift b | ./hid-gadget-test /dev/hidg0 keyboard

	# Send string 'abcdeZ'
	for C in a b c d e 'left-shift z' ; do echo "$C" ; sleep 0.1 ; done | ./hid-gadget-test /dev/hidg0 keyboard

	# You may combine several modifier keys
	echo left-ctrl left-shift enter | ./hid-gadget-test /dev/hidg0 keyboard

	# Try to guess what this command sends
	echo left-ctrl left-alt del | ./hid-gadget-test /dev/hidg0 keyboard

	# Bruteforce 4-digit PIN-code, that's a particularly popular script
	# that people keep asking me for. It executes for 42 hours.
	for a in 0 1 2 3 4 5 6 7 8 9; do
	for b in 0 1 2 3 4 5 6 7 8 9; do
	for c in 0 1 2 3 4 5 6 7 8 9; do
	for d in 0 1 2 3 4 5 6 7 8 9; do
	echo $a $b $c $d
	for C in $a $b $c $d enter ; do echo "$C" ; sleep 0.2 ; done | ./hid-gadget-test /dev/hidg0 keyboard
	sleep 15
	done
	done
	done
	done

	# Press right mouse button
	echo --b2 | ./hid-gadget-test /dev/hidg1 mouse

	# Hold left mouse button, drag 100 pixels to the right and 50 pixels up, then release
	echo --hold --b1 | ./hid-gadget-test /dev/hidg1 mouse
	echo --hold --b1 100 0 | ./hid-gadget-test /dev/hidg1 mouse
	echo --hold --b1 0 -50 | ./hid-gadget-test /dev/hidg1 mouse
	echo --b1 | ./hid-gadget-test /dev/hidg1 mouse

Here is [the list of keys that this utility supports](hid-gadget-test/jni/hid-gadget-test.c#L33)

If you need to crack a PIN code, but the target system loses keypresses (happens in MacOS BIOS),
there is a [handy app](send-pin-with-camera/) for that,
which uses camera to check if each keypress is recognized.

Compilation
===========

You have to run all following commands on Linux. Windows is not supported.

To compile USB Keyboard app, install Android SDK and NDK from site http://developer.android.com/ , and launch commands

	git clone https://github.com/pelya/commandergenius.git
	cd commandergenius
	git submodule update --init --recursive
	rm -f project/jni/application/src
	ln -s hid-pc-keyboard project/jni/application/src
	./changeAppSettings.sh -a
	android update project -p project

Add string `<uses-permission android:name="android.permission.ACCESS_SUPERUSER"/>` to file `project/AndroidManifest.xml`, and launch `./build.sh`

To compile kernel, launch commands

	git clone https://android.googlesource.com/platform/prebuilts/gcc/linux-x86/arm/arm-eabi-4.6
	git clone https://android.googlesource.com/kernel/tegra.git
	export PATH=`pwd`/arm-eabi-4.6/bin:$PATH
	export ARCH=arm
	export SUBARCH=arm
	export CROSS_COMPILE=arm-eabi-
	cd tegra
	git checkout android-tegra3-grouper-3.1-kitkat-mr1
	patch -p1 < ../kernel-3.1.patch
	make tegra3_android_defconfig
	make -j4

Use either [kernel-3.1.patch](kernel-3.1.patch) or [kernel-3.4.patch](kernel-3.4.patch), depending on your kernel version,

To compile *boot.img*, launch commands

	mkdir ~/bin
	export PATH=~/bin:$PATH
	curl http://commondatastorage.googleapis.com/git-repo-downloads/repo > ~/bin/repo
	chmod a+x ~/bin/repo
	mkdir aosp
	cd aosp
	repo init -u https://android.googlesource.com/platform/manifest -b android-4.4.2_r1
	repo sync
	cp -f ../tegra/arch/arm/boot/zImage device/asus/grouper/kernel
	patch -p1 < ../ueventd.patch
	make -j4 TARGET_PRODUCT=aosp_grouper TARGET_BUILD_VARIANT=userdebug

You then can find *boot.img* in directory `aosp/out/target/product/grouper`.

How it works
============

The custom kernel you have compiled with patch [kernel-3.1.patch](kernel-3.1.patch) or [kernel-3.4.patch](kernel-3.4.patch),
adds two new devices, /dev/hidg0 for keyboard, and /dev/hidg1 for mouse.

The patch [ueventd.patch](ueventd.patch) is only needed to set write permissions on these files -
if your device is rooted, USB Keyboard app will attempt to modify permissions on these files on start,
so you generally may skip this patch.

You can open these two files, using open() system call,
and write raw keyboard/mouse events there, using write() system call,
which will be sent through USB cable to your PC.

Keyboard event is an array of 8 byte length, first byte is a bitmask of currently pressed modifier keys:

	typedef enum {
		LCTRL = 0x1,
		LSHIFT = 0x2,
		LALT = 0x4,
		LSUPER = 0x8, // Windows key
		RCTRL = 0x10,
		RSHIFT = 0x20,
		RALT = 0x40,
		RSUPER = 0x80, // Windows key
	} ModifierKeys_t;

Remaining 7 bytes is a list of all other keys currently pressed, one byte for one key, or 0 if no key is pressed.
Consequently, the maximum amount of keys that may be pressed at the same time is 7, excluding modifier keys.

Professional or 'gamer' USB keyboards report several keyboard HID descriptors, which creates several keyboard devices in host PC,
to overcome that 7-key limit.

The scancode table for each key is available [in hid-gadget-test utility](hid-gadget-test/jni/hid-gadget-test.c#L33).
Extended keys, such as Play/Pause, are not supported, because they require modifying USB descriptor in kernel patch.

Mouse event is an array of 4 bytes, first byte is a bitmask of currently pressed mouse buttons:

	typedef enum {
		BUTTON_LEFT = 0x1,
		BUTTON_RIGHT = 0x2,
		BUTTON_MIDDLE = 0x4,
	} MouseButtons_t;

Remaining 3 bytes are X movement offset, Y movement offset, and mouse wheel offset, represented as signed integers.
Horizontal wheel is not supported yet.

See functions outputSendKeys() and outputSendMouse() inside file [input.cpp](remote-client/input.cpp)
for reference implementation.
