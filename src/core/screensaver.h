#ifndef __SCREENSAVER_H__
#define __SCREENSAVER_H__

// Idle screensaver: when the device sits in a menu with no input, play a GIF full screen.
// Uses SD:/SEAQUI/screensaver.gif, SD:/screensaver.gif or LittleFS:/screensaver.gif if present,
// otherwise the sleeping-serpent GIF built into the firmware.
// Starts after the "Dim time" setting (or 30 s if dimming is disabled).
// Any button stops it. After 5 minutes it hands over to the normal dim/screen-off.
// Returns true if the screensaver ran (the caller should redraw its screen).
bool runScreensaverIfIdle();

#endif
