# AutoBleem Kernel Installation

> **Warning:** Following these steps will modify your console's firmware and void your warranty. This is not required to use AutoBleem. Only do this if you want OTG, NTFS, exFAT, WiFi, Ethernet, and experimental Bluetooth support.

## Prerequisites

The AutoBleem Kernel can only be installed on a PlayStation Classic that is in its original condition. If you have installed BleemSync or Project Eris, you must first uninstall them. See [Removing BleemSync](bleemsync-removal.md).

## Installation Steps

1. Format your USB drive to FAT32 and label it `SONY`

2. Install a full AutoBleem USB package that includes the Apps set and
   Kernel Installer (ABFlashKit)

3. Ensure your PSC is unmodified or has been restored to original condition

4. Remove your USB drive from your PC and insert it into the **player 2 controller port** on the front of the PSC

5. Make sure a controller is connected to the player 1 port

6. Connect the power cable to the PSC and wait until the power light turns orange

7. Press the power button. The power light will blink orange then green repeatedly and will eventually stop blinking and remain constant green

8. At the AutoBleem start screen, press **Start** on the controller

9. Once in AutoBleem, press **Select** until the Apps set is shown. Navigate to **Kernel Installer (ABFlashKit)**. If it is not listed, the USB package is incomplete for kernel installation

10. Start Kernel Installer (ABFlashKit) by pressing **X**

11. Read the ABFlashKit notes that appear, then press **X**
    - If you get an error that the PSC is not in its original condition, see [Removing BleemSync](bleemsync-removal.md) and try again

12. When the process is complete, your PSC will boot normally. You now have restored your backup kernel or vanilla PSC kernel

13. To install the AutoBleem kernel, navigate to the AutoBleem Apps menu and start Kernel Installer (ABFlashKit) again (steps 8-11)

14. Press **X** to flash the AutoBleem kernel. Once complete, the screen will go black

15. Remove and reinsert the power cable. Wait for the power light to turn orange, then press the power button

Congratulations! You have installed AutoBleem and the AutoBleem kernel.

## Kernel Features

- OTG support
- NTFS and exFAT filesystem support
- WiFi (all Linux drivers) with auto-connect on boot
- Ethernet (all Linux drivers)
- Experimental Bluetooth support
- RNDIS support
