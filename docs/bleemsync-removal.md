# Removing BleemSync and Restoring Kernel Backup

The AutoBleem Kernel can only be installed on a PlayStation Classic that is in its original condition. If you have installed BleemSync or Project Eris, you must first uninstall them.

## If You Have BleemSync/Project Eris Installed

1. Find the backup `LBOOT.EPB` file that BleemSync (or Project Eris) made during installation

2. Follow the MMC uninstall guide at: https://modmyclassic.com/?s=uninstall+project+eris

### Multiple BleemSync Versions

If you installed multiple versions of BleemSync on top of one another, uninstall them in reverse order.

**Example:** If you installed BleemSync 1.1, then later installed BleemSync 1.2 on top:
1. Uninstall BleemSync 1.2 using its backup (restores to 1.1)
2. Uninstall BleemSync 1.1 using its backup (restores to original)

## If You Lost Your BleemSync Backup File

1. Find someone with a PlayStation Classic in original condition
2. Ask them to install the AutoBleem Kernel (this creates a backup of the original kernel)
3. Use that backup with AutoBleem to restore your PSC

> **Warning:** DO NOT USE AUTOBLEEM TO RESTORE A BLEEMSYNC BACKUP! This will brick your PSC!

## Restoring a Backup Made with AutoBleem

> **Important:** When restoring to original condition, you MUST format the USB stick as FAT32, as it will boot as an original stock PSC that only supports FAT32.

1. During kernel installation, AutoBleem makes a backup of your kernel
2. Place the `LBOOT.EPB` file created by AutoBleem on the root of your USB drive
3. Boot AutoBleem on your PSC
4. Navigate to Apps and run Kernel Installer (ABFlashKit)
5. Press **Triangle** to restore the `LBOOT.EPB` file to the PSC
6. If you want to install the AutoBleem Kernel afterwards, press **X**
