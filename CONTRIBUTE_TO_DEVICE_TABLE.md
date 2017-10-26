If you want to help me improve the DEVICES_AND_THEIR_KERNELS table:
- **What** information about your device do we need? 
    - MANUFACTURER, 
    - DEVICE_NAME, 
    - DEVICE_CODENAME, 
    - MODEL_NUMBER, 
    - SYSTEM_ON_CHIP_CODENAME = SOC, 
    - LINEAGEOS_LINUX_KERNEL_SRC = LOSK_SRC, 
    - LINEAGEOS_LINUX_KERNEL_VERSION = LOSK_VERS, 
    - MANUFACTURER_LINUX_KERNEL_SRC = MNFK_SRC, 
    - MANUFACTURER_LINUX_KERNEL_VERSION = MNFK_VERS,  
- **Where / how to find** ? 
    - with **Google search**
    - **on your device**: settings > About Phone <br/>
      => "Device model" tells you the DEVICE_NAME <br/>
      => the last few characters at "Android version" should tell you the DEVICE_CODENAME <br/>
      => the first few characters at "Baseband version" should tell you the SOC <br/>
      => the first numbers at "Kernel version" tell you the MANUFACTURER_KERNEL_VERSION = MNFK_VERS 
    - on the **LineageOS website**: <br/>
    - If you know the DEVICE_NAME: on the [Download website](https://download.lineageos.org/) <br/>
      => you can get the DEVICE_CODENAME
    - If you know the DEVICE_CODENAME: on the [CVE tracker](https://cve.lineageos.org/devices/)<br/>
      => you can get the MANUFACTURER + SOC + LOSK_SRC + LOSK_VERS + LINEAGEOS_DEVICE_REPO_URL => MODEL_NUMBER
    - If you know the DEVICE_CODENAME: in the [Wiki](https://wiki.lineageos.org/) - add the device codename at the end of the URL! <br/>
      => you can get the DEVICE_NAME + MODEL_NUMBER 
    - in **Wikipedia**: if you know the DEVICE_NAME, <br/>
      => you can get the MODEL_NUMBER
    - on the **Manufacturer** website: If you know the MODEL_NUMBER, MANUFACTURER and / or the DEVICE_CODENAME, <br/>
      => you can download the kernel source code for the specific device! 
      The kernel src code includes the MANUFACTURER_LINUX_KERNEL_VERSION = MNFK_VERS in its top-level **Makefile**. 
      To get the MANUFACTURER_KERNEL_SRC , **note** the URL from which the download started! <br/>
      - **Sony Xperia**:<br/>
        ``https://developer.sonymobile.com/downloads/xperia-open-source-archives/``<br>
        ``https://github.com/sonyxperiadev/device-sony-$DEVICE_CODENAME``
      - **Google**:<br/>
        ``https://android.googlesource.com/device/$MANUFACTURER/$DEVICE_CODENAME/``
      - **Motorola**:<br/>
        ``https://github.com/MotorolaMobilityLLC/kernel-msm/releases/``
      - **Samsung**:<br/>
        ``http://opensource.samsung.com/reception/receptionSub.do?method-sub&searchValue=$MODEL_NUMBER``

