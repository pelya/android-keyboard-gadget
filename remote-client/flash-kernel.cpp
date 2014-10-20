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
#include "flash-kernel.h"

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
		"$APPDIR/busybox [ \"`getprop ro.product.device`\" = grouper -a \"`getprop ro.build.version.release`\" = 4.4.4 ]",
		"$APPDIR/wget --no-check-certificate -O boot.img 'https://github.com/pelya/android-keyboard-gadget/blob/master/nexus7-2012-wifi-grouper/boot.img?raw=true'",
		"echo 'a58769a63cd2338cdb9f6432e0891dc2901af73d  boot.img' | $APPDIR/busybox sha1sum -c",
		"echo \"$APPDIR/busybox dd if=boot.img of=/dev/block/platform/sdhci-tegra.3/by-name/LNX\" | su",
		"rm boot.img"
	},
	{ NULL, NULL, NULL, NULL, NULL }
};

static int executeCommand(const char * cmd)
{
	printf("executeCommand > %s", cmd);
	char buf[512] = "";
	FILE *ff = popen(cmd, "r");
	while( fgets(buf, sizeof(buf), ff) )
	{
		printf("executeCommand >> %s", buf);
		addDialogText(buf);
		mainLoop();
	}
	if( pclose(ff) != 0 )
		return 0;
	return 1;
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
		mainLoop();

	if( result == 0 )
		exit(0);

	createDialog();
	addDialogText("Downloading package...");
	mainLoop();
	if( !executeCommand(dev.download) )
		showErrorMessage("Cannot download kernel, please check your network connectivity");

	addDialogText("Validating package...");
	mainLoop();
	if( !executeCommand(dev.checksum) )
		showErrorMessage("Downloaded package was corrupted, please re-download it");

	createDialog();
	addDialogText("Custom kernel will be flashed to your device.");
	addDialogText("This kernel is EXPERIMENTAL, and comes WTIH NO WARRANTY.");
	addDialogText("Your device may become unstable, and reboot at random.");
	addDialogText("");
	addDialogText("Do you wish to flash custom kernel to your device?");
	addDialogYesNoButtons();
	while( !getDialogResult(&result) )
		mainLoop();

	if( result == 0 )
		exit(0);

	result = executeCommand(dev.flash);
	executeCommand(dev.clear_tmp);
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
		if( executeCommand(supportedDevices[d].device) )
			return flashCustomKernelDialog(supportedDevices[d]);

	createDialog();
	addDialogText("You will need a custom kernel to use this app.");
	addDialogText("Prebuilt kernel is available only for Nexus 7 2012 WiFi with Android 4.4.4.");
	addDialogText("You will need to compile and install the kernel yourself for other devices.");
	addDialogText("Compilation instructions are available here:");
	addDialogText("");
	addDialogText("");
	addDialogUrlButton("https://github.com/pelya/android-keyboard-gadget");
	while( true )
		mainLoop();

	return 0;
}
