#include "screensaver.h"
#include "core/display.h"
#include "core/settings.h"
#include "core/seaq_gifs.h"
#include <globals.h>

#if !defined(LITE_VERSION)
static const unsigned long SCREENSAVER_FALLBACK_IDLE_MS = 30000; // used when dimming is disabled
static const unsigned long SCREENSAVER_MAX_MS = 5UL * 60UL * 1000UL;

static bool findScreensaver(FS *&fs, const char *&path) {
    static const char *SD_PATHS[] = {"/SEAQUI/screensaver.gif", "/screensaver.gif"};
    if (sdcardMounted) {
        for (const char *p : SD_PATHS) {
            if (SD.exists(p)) {
                fs = &SD;
                path = p;
                return true;
            }
        }
    }
    if (LittleFS.exists("/screensaver.gif")) {
        fs = &LittleFS;
        path = "/screensaver.gif";
        return true;
    }
    return false;
}

static void clearButtons() {
    NextPress = false;
    PrevPress = false;
    UpPress = false;
    DownPress = false;
    SelPress = false;
    EscPress = false;
    AnyKeyPress = false;
    NextPagePress = false;
    PrevPagePress = false;
}
#endif

bool runScreensaverIfIdle() {
#if !defined(LITE_VERSION)
    if (isSleeping) return false;

    unsigned long idleMs =
        bruceConfig.dimmerSet > 0 ? (unsigned long)bruceConfig.dimmerSet * 1000UL : SCREENSAVER_FALLBACK_IDLE_MS;
    if (millis() - previousMillis < idleMs) return false;

    // A screensaver.gif on the SD card / LittleFS overrides the one built into the firmware.
    FS *fs = nullptr;
    const char *path = nullptr;
    Gif gif;
    if (findScreensaver(fs, path)) {
        if (!gif.openGIF(fs, path)) return false;
    } else {
        if (SEAQ_SCREENSAVER.data == nullptr) return false; // nothing to show: normal dimming behaviour
        if (!gif.openGIF(SEAQ_SCREENSAVER.data, (int)SEAQ_SCREENSAVER.size)) return false;
    }

    // Undo any dimming that already started, and keep the screen awake while it plays.
    previousMillis = millis();
    if (dimmer || isScreenOff) {
        dimmer = false;
        isScreenOff = false;
        getBrightness();
    }

    tft.fillScreen(bruceConfig.bgColor);
    int x = max(0, (tftWidth - gif.getCanvasWidth()) / 2);
    int y = max(0, (tftHeight - gif.getCanvasHeight()) / 2);
    unsigned long start = millis();

    while (true) {
        if (check(AnyKeyPress)) break;

        if (millis() - start > SCREENSAVER_MAX_MS) {
            // Hand over to the normal dim / screen-off timer, then wait for a key.
            previousMillis = millis() - idleMs;
            bool wentDark = false;
            while (!check(AnyKeyPress)) {
                if (dimmer || isScreenOff) wentDark = true;
                else if (wentDark) break; // the wake-up press is swallowed by the input handler
                vTaskDelay(pdMS_TO_TICKS(20));
            }
            break;
        }

        previousMillis = millis();
        if (gif.playFrame(x, y) < 0) {
            log_e("Screensaver GIF error: %d", gif.getLastError());
            break;
        }
    }

    // The key that woke the screen must not also act on the menu.
    vTaskDelay(pdMS_TO_TICKS(150));
    clearButtons();
    previousMillis = millis();
    if (dimmer || isScreenOff) {
        dimmer = false;
        isScreenOff = false;
        getBrightness();
    }
    tft.fillScreen(bruceConfig.bgColor);
    return true;
#else
    return false;
#endif
}
