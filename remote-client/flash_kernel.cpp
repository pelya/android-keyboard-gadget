/*
 * Copyright (C) 2015 Sergii Pylypenko
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include "gfx.h"
#include "gui.h"
#include "input.h"
#include "scancodes.h"
#include "flash_kernel.h"

static struct supportedDevices_t
{
	const char * device;
	const char * download;
	const char * checksum;
	const char * flash;
	const char * clear_tmp;
}
supportedDevices[] =
{
	{
		"$APPDIR/busybox [ \"`getprop ro.product.device`\" = grouper -a \"`getprop ro.build.version.release`\" = 5.1.1 ] && echo Matched || $APPDIR/busybox [ \"`getprop ro.product.device`\" = nakasi -a \"`getprop ro.build.version.release`\" = 5.1.1 ] && echo Matched",
		"$APPDIR/wget --no-check-certificate -O boot.img 'https://github.com/pelya/android-keyboard-gadget/blob/b08d8fa3f0476b638dcb4fbf7ec0f7558bf838e3/nexus7-2012-wifi-grouper/boot.img?raw=true' && echo Successful",
		"echo 'ddab9eb735e3b96596407062afae69f829ba8139  boot.img' | $APPDIR/busybox sha1sum -c",
		"echo \"$APPDIR/busybox dd if=boot.img of=/dev/block/platform/sdhci-tegra.3/by-name/LNX && echo Successful\" | su",
		"rm boot.img"
	},
	{
		"$APPDIR/busybox [ \"`getprop ro.product.device`\" = grouper -a \"`getprop ro.build.version.release`\" = 5.0.2 ] && echo Matched || $APPDIR/busybox [ \"`getprop ro.product.device`\" = nakasi -a \"`getprop ro.build.version.release`\" = 5.0.2 ] && echo Matched",
		"$APPDIR/wget --no-check-certificate -O boot.img 'https://github.com/pelya/android-keyboard-gadget/blob/e44670bdcbc8d6a6939e083fcefec067f11094a8/nexus7-2012-wifi-grouper/boot.img?raw=true' && echo Successful",
		"echo 'bb164ba3a76a6d2921414414a26c0498b7ce29de  boot.img' | $APPDIR/busybox sha1sum -c",
		"echo \"$APPDIR/busybox dd if=boot.img of=/dev/block/platform/sdhci-tegra.3/by-name/LNX && echo Successful\" | su",
		"rm boot.img"
	},
	{
		"$APPDIR/busybox [ \"`getprop ro.product.device`\" = grouper -a \"`getprop ro.build.version.release`\" = 4.4.4 ] && echo Matched || $APPDIR/busybox [ \"`getprop ro.product.device`\" = nakasi -a \"`getprop ro.build.version.release`\" = 4.4.4 ] && echo Matched",
		"$APPDIR/wget --no-check-certificate -O boot.img 'https://github.com/pelya/android-keyboard-gadget/blob/bfcefabcd60829866c23ad03ea43fd3166462468/nexus7-2012-wifi-grouper/boot.img?raw=true' && echo Successful",
		"echo '1b57049e0823f632f8c69bbde8f9dd632cad7e7b  boot.img' | $APPDIR/busybox sha1sum -c",
		"echo \"$APPDIR/busybox dd if=boot.img of=/dev/block/platform/sdhci-tegra.3/by-name/LNX && echo Successful\" | su",
		"rm boot.img"
	},
	{ NULL, NULL, NULL, NULL, NULL }
};

static int executeCommand(const char * cmd, const char *search)
{
	int success = 0;
	printf("executeCommand > %s", cmd);
	char buf[512] = "";
	FILE *ff = popen(cmd, "r");
	while( fgets(buf, sizeof(buf), ff) )
	{
		printf("executeCommand >> %s", buf);
		addDialogText(buf);
		mainLoop(true);
		if (strstr(buf, search) != NULL)
			success = 1;
	}
	pclose(ff);
	return success;
}

static int flashCustomKernelDialog(struct supportedDevices_t & dev)
{
	createDialog();
	addDialogText("You will need custom kernel to use this app.");
	addDialogText("Do you wish to download and flash custom kernel?");
	addDialogText("You will need root on your device.");
	addDialogText("If you don't have root, please follow this link:");
	addDialogText("");
	addDialogText("");
	addDialogUrlButton("https://github.com/pelya/android-keyboard-gadget");
	addDialogYesNoButtons();

	int result = 0;
	while( !getDialogResult(&result) )
		mainLoop(true);

	if( result == 0 )
		exit(0);

	createDialog();
	addDialogText("Downloading package...");
	mainLoop(true);
	if( !executeCommand(dev.download, "Successful") )
		showErrorMessage("Cannot download kernel, please check your network connectivity");

	addDialogText("Validating package...");
	mainLoop(true);
	if( !executeCommand(dev.checksum, "boot.img: OK") )
		showErrorMessage("Downloaded package was corrupted, please re-download it");

	createDialog();
	addDialogText("Custom kernel will be flashed to your device.");
	addDialogText("This kernel is EXPERIMENTAL, and comes WTIH NO WARRANTY.");
	addDialogText("Your device may become unstable, and reboot at random.");
	addDialogText("You will lose root, please root your device again.");
	addDialogText("");
	addDialogText("Do you wish to flash custom kernel to your device?");
	addDialogYesNoButtons();
	while( !getDialogResult(&result) )
		mainLoop(true);

	if( result == 0 )
		exit(0);

	result = executeCommand(dev.flash, "Successful");
	executeCommand(dev.clear_tmp, "-");
	if( result )
		showErrorMessage("Flashing kernel succeeded.\nPlease restart your device now.");
	else
		showErrorMessage("Flashing kernel failed.\nDo you have root installed on your device?");

	return 1;
}

int flashCustomKernel()
{
	int d;
	for( d = 0; supportedDevices[d].device; d++ )
		if( executeCommand(supportedDevices[d].device, "Matched") )
			return flashCustomKernelDialog(supportedDevices[d]);

	createDialog();
	addDialogText("You will need a custom kernel to use this app.");
	addDialogText("Prebuilt kernel is available only for Nexus 7 2012 WiFi");
	addDialogText("with Android 4.4.4 or 5.0.2 or 5.1.1.");
	addDialogText("You will need to compile and install the kernel yourself for other devices.");
	addDialogText("Compilation instructions are available here:");
	addDialogText("");
	addDialogUrlButton("https://github.com/pelya/android-keyboard-gadget");
	while( true )
		mainLoop(true);

	return 0;
}
