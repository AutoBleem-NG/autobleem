# WiFi Setup

> **Note:** You can use a USB keyboard on the WiFi menu. This is particularly useful when entering your WiFi password and for paging down the timezone choices. Enter = X, Escape = O.

## Setup Steps

1. At the AutoBleem start screen, press **L1 + Square** on the controller. This will take you to the PlayStation Classic Hardware Information screen.

2. Press the **Select** button on the controller to enter WiFi Settings.

3. Navigate to SSID and press **Triangle** to scan for your SSID.
   - If your SSID is not discovered during the scan, manually enter your SSID by pressing **X** to select Edit SSID.

4. Navigate to Password and enter your WiFi password.

5. Select the appropriate Driver Mode for your WiFi dongle:
   - **wext** (default): Compatible with the 8188eu WiFi chipset (e.g., TP-Link TL-WN725N)
   - **nl80211**: For chipsets like rt5370 (e.g., Ralink 5370)

   It is recommended to try the nl80211 Driver Mode first and only use wext if nl80211 is not compatible. WiFi dongles using the rt5370 chipset tend to be more stable than the TL-WN725N.

6. Navigate to **Write Configuration/Restart Network** and press **X**. This will save your WiFi settings to the PSC.

7. Navigate to **Timezone** and press **X** to set your timezone.
   - Use Restart Network to reconnect if you lose connection.

Once setup, the AutoBleem kernel will automatically connect to WiFi during boot.

## Remote Access

| Service | Port | Notes |
|---------|------|-------|
| SSH (Putty) | 22 | - |
| FTP (WinSCP/FileZilla) | 21 | SFTP not supported |

**Credentials:**
- Username: `root`
- Password: `ab`
