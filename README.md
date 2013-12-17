android-gadget-hid-remote-keyboard
==================================

Convert your Android device into USB keyboard/mouse, control your PC from your Android device remotely, including BIOS/bootloader.


Installation
============

You will need Nexus 7 2012 WiFi, with Android 4.4.2 installed.

- Plug your device into PC using USB cable.
- Power off the device.
- Hold Volume Down button and Power button for 5 seconds, to enter fastboot mode.
- Copy appropriate fastboot executable from the directory fastboot.
- Launch command `fastboot oem unlock`
- Confirm unlock action by pressing Power button. This will factory reset your device.
- Copy boot.img from directory nexus7-2012-wifi-grouper
- Launch command `fastboot flash boot boot.img`
- Reboot your device using Power button.

Compilation
===========

You have to run all following commands on Linux. Windows is not supported.

To compile hid-gadget-test, install Android NDK, and launch commands

	cd hid-gadget-test
	ndk-build

To compile kernel, launch commands

	git clone https://android.googlesource.com/platform/prebuilts/gcc/linux-x86/arm/arm-eabi-4.6
	git clone https://android.googlesource.com/kernel/tegra.git
	export PATH=`pwd`/arm-eabi-4.6/bin:$PATH
	export ARCH=arm
	export SUBARCH=arm
	export CROSS_COMPILE=arm-eabi-
	cd tegra
	git checkout android-tegra3-grouper-3.1-kitkat-mr1
	patch -p1 < ../kernel-tegra.patch
	make tegra3_android_defconfig
	make -j4

To compile boot.img, launch commands

	mkdir ~/bin
	export PATH=~/bin:$PATH
	curl http://commondatastorage.googleapis.com/git-repo-downloads/repo > ~/bin/repo
	chmod a+x ~/bin/repo
	mkdir aosp
	cd aosp
	repo init -u https://android.googlesource.com/platform/manifest -b android-4.4.2_r1
	repo sync
	cp -f ../tegra/arch/arm/boot/zImage device/asus/grouper/kernel
	make -j4 TARGET_PRODUCT=aosp_grouper TARGET_BUILD_VARIANT=userdebug

You then can find boot.img in directory aosp/out/target/product/grouper

To test keyboard and mouse input, flash boot.img to your device as described above, and launch commands

	adb push hid-gadget-test/hid-gadget-test /data/local/tmp
	adb shell
	cd /data/local/tmp
	./hid-gadget-test /dev/hidg0 keyboard
	./hid-gadget-test /dev/hidg1 mouse

Enter symbols and commands - they will be sent back to your PC as keypresses.
For mouse movement, enter two numbers separated by space, they should not be between -127 and 127.
