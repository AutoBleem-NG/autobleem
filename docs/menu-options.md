# AutoBleem Menu Options

This document describes the menu options visible in AutoBleem-NG.

## Start Screen

### Main Controls

| Control | Option | What it does |
| --- | --- | --- |
| Start | Start EvolutionUI | Opens the carousel launcher. |
| Cross | Scan | Scans for games. |
| Square | RetroArch | Starts RetroArch. If RetroArch is missing, AutoBleem-NG shows a confirmation prompt before continuing. |
| Triangle | About | Opens the About screen. |
| Select | AutoBleem settings | Opens the AutoBleem settings menu. |
| L1 held + Cross | Memory card manager | Opens the custom memory card list. |
| L1 held + Circle | Game manager | Opens the USB game manager. |
| L2 held + R2 | Power off | Shuts down the system. |

## Launcher Carousel

### Game View Controls

| Control | Option | What it does |
| --- | --- | --- |
| Left / Right | Browse games | Moves to the previous or next game. Holding a direction scrolls faster after a delay. |
| Down | Options row | Opens the lower options row for the selected game or list. |
| Cross | Play | Starts the selected item. PS1 games normally use PCSX unless a setting routes them through RetroArch. RetroArch entries start RetroArch. Apps show a confirmation/readme screen first. |
| Square | Play with RetroArch | Starts the selected PS1 game through RetroArch when RetroArch is installed. |
| Triangle | Button guide | Opens the control guide. In resume-slot mode, deletes the selected resume point after confirmation. |
| Circle | Exit | Leaves the launcher carousel. |
| Select | Next set | Cycles through PS1, RetroArch, Lightgun, and Apps sets. The Lightgun set is skipped when there are no lightgun games. |
| L2 + Select | Choose subset | Opens the PS1 game-directory selector in the PS1 set, or the playlist selector in the RetroArch set. |
| Start | Random game | Selects a random game in the current list. |
| L1 / R1 | Previous / next first letter | Jumps to the first game starting with the previous or next title letter. Holding continues jumping. |
| L2 + R2 | Power off | Shuts down the system. |

### Sets and Subsets

| Set or submenu | Entries | What it displays |
| --- | --- | --- |
| PS1 set | All Games | USB games plus internal games when internal games are enabled. |
| PS1 set | Internal Games | Internal console games only. |
| PS1 set | USB Games and subdirectories | Games from the USB games folder and discovered subdirectories. |
| PS1 set | Favorite Games | PS1 games marked as favorite. |
| PS1 set | Game History | Recently played PS1 games. |
| RetroArch set | Playlists | Games from the selected RetroArch playlist. |
| Lightgun set | Lightgun Games | Games marked as lightgun games. |
| Apps set | Apps | Installed apps with app metadata. |

## Options Row

| Option | Availability | What it opens or does |
| --- | --- | --- |
| Settings | Always available | Opens AutoBleem settings. |
| Game | Available for selected games | Opens the game editor. |
| Memory Card | PS1 games only | Opens the two-card memory card editor for the selected game. |
| Resume | PS1 games with active AutoBleem resume slots | Opens resume-slot selection. RetroArch resume points are not supported here. |

## AutoBleem Settings

### Controls

| Control | What it does |
| --- | --- |
| Up / Down | Selects a setting. |
| Left / Right | Moves to the previous or next value. Most settings stop at the first or last value; Font wraps around. |
| L1 / R1 | Jumps backward or forward through the selected setting's value list. |
| L2 / R2 | Selects the first or last value. |
| Start | Randomizes Theme, Font, or Music only. |
| Cross | Saves settings and exits. |
| Circle | Cancels changes and exits. |

### Settings

| Setting | Values | What it does |
| --- | --- | --- |
| AutoBleem Theme | Installed themes | Changes the active UI theme immediately. |
| Use Font from Theme | Off/on | Controls whether the selected theme font is used. |
| Font | Theme default plus installed fonts | Overrides the UI body font. |
| Show Internal Games | Off/on | Shows or hides the internal PlayStation Classic games in PS1 lists. |
| Cover Style | Installed cover frames | Selects the frame shown around game covers. |
| Music | Theme music or installed music tracks | Selects the background music. |
| Background Music | Off/on | Enables or disables background music. |
| Widescreen | Off/on | Uses widescreen display settings for supported launch paths. |
| GFX Filter | Off/on | Changes the graphics filtering behavior used by supported launch paths. |
| Update RA Config | Off/on | Allows AutoBleem-NG to update RetroArch settings when launching games through RetroArch. |
| Play all PSX games with RA | Off/on | Forces PS1 games to start through RetroArch from the carousel. |
| Showing Timeout (0 for no timeout) | 0 through 20 | Controls notification timeout seconds. 0 disables the timeout. |
| Language | Installed languages | Changes the UI language immediately. |

## PS1 Game Editor

### Controls

| Control | What it does |
| --- | --- |
| Up / Down | Selects a game setting. |
| Left / Right | Decreases/disables or increases/enables the selected setting. Numeric settings stop at their minimum and maximum values. |
| Triangle | Renames the game. |
| Square | Changes the selected game's memory card. |
| Start | Creates and assigns a shared custom memory card for games currently using the internal card. |
| Circle | Exits the editor. |

### Options

| Option | Values | What it does |
| --- | --- | --- |
| Favorite | Off/on | Adds or removes the game from the Favorite Games list. |
| Lightgun Game | Off/on | Marks the game as a lightgun game. Enabling this also enables Play using RA; disabling it turns Play using RA off. |
| Play using RA | Off/on | Starts this PS1 game through RetroArch. This cannot be changed while Lightgun Game is enabled. |
| Lock data | Off/on | Prevents scanner automation from overwriting game metadata for USB games. |
| High res | Off/on | Enables or disables enhanced GPU resolution. |
| SpeedHack | Off/on | Enables or disables the enhanced GPU speedhack. |
| Scanlines | Off/on | Enables or disables scanline overlay behavior. |
| Scanline Level | 0 through 100 | Sets scanline opacity level. |
| Clock | 0 through 100 | Sets the emulated PSX clock. |
| Frameskip | 0 through 3 | Sets frameskip. |
| Plugin | Built-in GPU or PEOPS GPU | Selects the GPU plugin for USB games. |
| Spu Interpolation | 0 through 3 | Sets audio interpolation behavior. |

## RetroArch Game Editor

| Option | What it does |
| --- | --- |
| Lightgun Game | Adds or removes the selected RetroArch/foreign game from the lightgun game list. |
| Circle | Exits the editor. |

## Game Manager

| Control | Option | What it does |
| --- | --- | --- |
| Up / Down | Select game | Moves through sorted USB games. |
| L1 / R1 | Page | Pages through the game list. |
| Cross | Select | Opens the PS1 game editor for the selected USB game. |
| Square | Delete Game | Confirms and deletes the selected game. A rescan is forced after a successful deletion. |
| Triangle | Flush covers | Confirms and deletes cover PNG files, then forces a rescan and closes the manager. |
| Circle | Close | Closes the manager. If changes were made, a rescan is forced. |

## Custom Memory Card List

| Control | Option | What it does |
| --- | --- | --- |
| Up / Down | Select card | Moves through custom memory cards. |
| L1 / R1 | Page | Pages through the card list. |
| Cross | Rename | Renames the selected card unless the new name is empty, reserved, or already used. |
| Square | New Card | Creates a custom memory card unless the name is empty or reserved. |
| Triangle | Delete | Confirms and deletes the selected custom memory card. |
| Circle | Go back | Closes the list. |

## Select Memory Card Dialog

| Mode | Entries | What selection means |
| --- | --- | --- |
| Custom list | Internal card plus custom card names | Used by game settings to choose the game's mapped memory card. |
| Manager list | Configured card slots plus custom cards | Used by the two-card editor to replace the right-side card. |

Controls are Up/Down to move, L1/R1 to page, Cross to select, and Circle to
cancel.

## Two-Card Memory Card Editor

| Control | Option | What it does |
| --- | --- | --- |
| D-pad | Move slot cursor | Moves around two 15-slot cards. Left/right wraps between the left and right card grids. |
| Start | Select Right Card | Saves pending changes if accepted, then opens the memory-card selector for the right-side card. |
| Select | Defragment Card | Rebuilds the selected card by compacting saves when possible. |
| Cross | Reload Cards | Offers to save pending changes, then reloads both card files from disk. |
| Triangle | Delete | Deletes the selected save when the cursor is on a save-start slot. Linked blocks and free slots are ignored. |
| Square | Copy | Copies the selected save-start block from the current card to the other card if enough empty slots are available. |
| Circle | Go back | Offers to save pending changes, then exits. |

## Resume Slots

| Control | What it does |
| --- | --- |
| Left / Right | Selects one of four resume slots. |
| Cross | Loads the selected active resume point, or saves to the selected slot when saving a resume point. |
| Triangle | Deletes the selected active resume point after confirmation. |
| Circle | Cancels resume-slot mode. |

## App Start Screen

| Control | What it does |
| --- | --- |
| Cross | Confirms and starts the selected app. |
| Circle | Cancels app start and returns to the launcher. |
