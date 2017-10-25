### Notes: 
- *In the following, the sign "$" tells you that you should **replace the "$" and the following word** with the relevant information (it is a variable) in your issue or pull request!*
- **Sometimes**, kernels are **device-specific**; but most of the time, they are **only SYSTEM-ON-CHIP and MANUFACTURER-specific**. This is the reason why most of the time we **won't** name the patches after the **device**, but **only** after the **SOC and MANUFACTURER**, just like LineageOS: $MANUFACTURER_$SOC-CODENAME.patch

### Important: Before creating an issue, 
... look first in the file "DEVICES_AND_THEIR_KERNELS.md" for your device! 
  Example:<br/>
 ```
 MANUFACTURER: Motorola
 DEVICE_NAME: Moto G4 Play 
 DEVICE_CODENAME: harpia 
 MODEL_NUMBER: XT1600 
 SYSTEM_ON_CHIP_NAME = SOC_NAME: Qualcomm Snapdragon 410 
 SYSTEM_ON_CHIP_CODENAME = SOC: MSM8916
 LOSK_SRC = LINEAGEOS_LINUX_KERNEL_SRC: https://github.com/LineageOS/android_kernel_motorola_msm8916
 LOSK_VERS = LINEAGEOS_LINUX_KERNEL_VERSION: 3.10.49
 MNFK_SRC = MANUFACTURER_LINUX_KERNEL_SRC: https://github.com/MotorolaMobilityLLC/kernel-msm/releases/tag/MMI-MPI24.241-2.35-1 
 MNFK_VERS = MANUFACTURER_LINUX_KERNEL_VERSION: 3.10.49
 
 Therefore, we will call the patch for Motorola Moto G4 Play: motorola_msm8916.patch 
 It is also applicable for merlin, osprey, lux and surnia!
 ```
 
  - If your device **is already** on the DEVICES_AND_THEIR_KERNELS.md list, search... 
    1. for a patch in this repo that **matches the DEVICE_CODENAME** of your device 
    (according to the DEVICES_AND_THEIR_KERNELS list). 
        - If such a patch **is existing**, but **not working** (even after manually applying the patch!); <br>
          please create an issue with the following name:<br> 
          ``$DEVICE_CODENAME patch not working for $MODEL_NUMBER``. 
        - If **NO** such device specific patch is existing, search ...
    2. for a patch in this repo that **matches the MANUFACTURER and SOC** of your device 
    (according to the DEVICES_AND_THEIR_KERNELS list). 
        - If such a patch **is existing**, but **not working** (even after manually applying the patch!); <br>
          please create an issue with the following name:<br>
          ``$MANUFACTURER + $SOC patch not working for $DEVICE_CODENAME``. 
        - If **NO** such manufacturer and SoC-codename specific patch is existing, search ...
    3. for a patch in this repo that **matches the LINUX_KERNEL_VERSION** (MNFK_VERS or LOSK_VERS) of your device
    (according to the DEVICES_AND_THEIR_KERNELS list). 
        - If such a patch **is existing**, but **not working** (even after manually applying the patch!); <br>
          please create an issue with the following name:<br>
          ``kernel version $MNFK_VERS / $LOSK_VERS patch not working for $MANUFACTURER + $SOC / $DEVICE_CODENAME``.<br/>
        - If **NO** such patch is existing , please create an issue with the following name: <br/>
          ``MANUFACTURER + $SOC / $DEVICE_CODENAME patch not existing!`` . 
        - If you are more interested in a general-purpose patch for a newer linux kernel version, <br>
          and you would like to do the device-specific error-fixing yourself, please give the issue the following name:<br>
          ``general kernel version $MNFK_VERS / $LOSK_VERS patch not existing!``. 
  - If your device is **not yet** on the DEVICES_AND_THEIR_KERNELS.md list: 
    1. Do some research in the internet and on your device! Try to find out as much information as possible!
       - **What** information about your device do we need? 
           - MANUFACTURER, 
           - DEVICE_NAME, 
           - DEVICE_CODENAME, 
           - MODEL_NUMBER, 
           - SYSTEM_ON_CHIP_NAME, 
           - SYSTEM_ON_CHIP_CODENAME, 
           - LINEAGEOS_LINUX_KERNEL_SRC, 
           - LINEAGEOS_LINUX_KERNEL_VERSION, 
           - MANUFACTURER_LINUX_KERNEL_SRC, 
           - MANUFACTURER_LINUX_KERNEL_VERSION,  
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
               ```       
               https://android.googlesource.com/device/$MANUFACTURER/$DEVICE_CODENAME/
               ```
             - **Motorola**:<br/>
               ```
               https://github.com/MotorolaMobilityLLC/kernel-msm/releases/
               ```
             - **Samsung**:<br/>
               ```
               http://opensource.samsung.com/reception/receptionSub.do?method-sub&searchValue=$MODEL_NUMBER    
               ```    
    2. Make a **pull request**, that adds the collected information to the device_and_their_kernels.md file. <br/>
       Please give the pull request the following name: <br/>
       ``added device data for $DEVICE_CODENAME / $MODEL_NUMBER`` <br/>
       If you **don't know how to make a pull request**, add the information you found to your issue. 
    3. Please give the issue / pull request the following name:<br>
       ``$MANUFACTURER + $SOC + $MNFK_VERS / $LOSK_VERS + $DEVICE CODENAME patch missing!``

If you don't provide enough information about your device, I will close your issue without answering. You have been warned! 
   
