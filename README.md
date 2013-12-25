Convert your Android device into USB keyboard/mouse, control your PC from your Android device remotely, including BIOS/bootloader.


Installation
============

You will need Nexus 7 2012 WiFi (Grouper), with Android 4.4.2 installed.

- Plug your device into PC using USB cable.
- Power off the device.
- Hold Volume Down button and Power button for 5 seconds, to enter fastboot mode.
- Copy appropriate fastboot executable from the directory `fastboot`.
- Launch command `fastboot oem unlock`
- Confirm unlock action by pressing Power button. This will factory reset your device.
- Copy *boot.img* from directory `nexus7-2012-wifi-grouper`.
- Launch command `fastboot flash boot boot.img`.
- Reboot your device using Power button.
- Install and run USB Keyboard app.

Compilation
===========

You have to run all following commands on Linux. Windows is not supported.

To compile USB Keyboard app, install Android SDK and NDK from site http://developer.android.com/ ,
go to http://github.com/ and create an account there, and launch commands

	git clone git@github.com:pelya/commandergenius.git
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
	patch -p1 < ../kernel-tegra.patch
	make tegra3_android_defconfig
	make -j4

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
