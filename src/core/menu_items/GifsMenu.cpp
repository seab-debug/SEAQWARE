#include "GifsMenu.h"
#include "core/display.h"
#include "core/sd_functions.h"
#include <algorithm>
#include <vector>

#if !defined(LITE_VERSION)
static const char *GIF_DIRS[] = {"/SEAQUI/gifs", "/gifs"};

// Each GIF plays at least this long (whole loops), then the next one starts.
static const unsigned long GIF_MIN_MS = 6000;
// Long GIFs move on after this, even mid-loop.
static const unsigned long GIF_MAX_MS = 20000;

static void listGifs(FS *fs, const char *dir, std::vector<String> &out) {
    if (!fs->exists(dir)) return;
    File root = fs->open(dir);
    if (!root || !root.isDirectory()) return;
    while (true) {
        bool isDir;
        String fullPath = root.getNextFileName(&isDir);
        if (fullPath == "") break;
        if (isDir) continue;
        String name = fullPath.substring(fullPath.lastIndexOf('/') + 1);
        if (name.startsWith(".")) continue;
        String lower = name;
        lower.toLowerCase();
        if (lower.startsWith("screensaver")) continue; // the screensaver GIF is not part of the playlist
        if (lower.endsWith(".gif")) out.push_back(fullPath);
    }
    root.close();
    std::sort(out.begin(), out.end());
}
#endif

// Plays every GIF one after another, full screen, no labels.
// Rotate = skip forward/back, Back = return to the main menu.
void GifsMenu::optionsMenu() {
#if !defined(LITE_VERSION)
    FS *fs = nullptr;
    std::vector<String> gifs;

    if (sdcardMounted) {
        for (const char *dir : GIF_DIRS) {
            listGifs(&SD, dir, gifs);
            if (!gifs.empty()) {
                fs = &SD;
                break;
            }
        }
    }
    if (gifs.empty()) {
        listGifs(&LittleFS, "/gifs", gifs);
        if (!gifs.empty()) fs = &LittleFS;
    }
    if (gifs.empty()) {
        displayWarning("No GIFs in SD:/SEAQUI/gifs", true);
        return;
    }

    const int total = gifs.size();
    int idx = 0;
    int failures = 0;
    bool exitPage = false;

    while (!exitPage) {
        tft.fillScreen(bruceConfig.bgColor);
        Gif gif;
        if (!gif.openGIF(fs, gifs[idx].c_str())) {
            log_e("Can't open GIF %s", gifs[idx].c_str());
            idx = (idx + 1) % total;
            if (++failures >= total) {
                displayError("Can't open GIFs", true);
                break;
            }
            continue;
        }
        failures = 0;

        int x = max(0, (tftWidth - gif.getCanvasWidth()) / 2);
        int y = max(0, (tftHeight - gif.getCanvasHeight()) / 2);
        unsigned long start = millis();

        while (true) {
            previousMillis = millis(); // watching GIFs counts as activity: don't dim or start the screensaver

            if (check(EscPress) || check(SelPress)) {
                exitPage = true;
                break;
            }
            if (check(NextPress)) {
                idx = (idx + 1) % total;
                break;
            }
            if (check(PrevPress)) {
                idx = (idx - 1 + total) % total;
                break;
            }

            int result = gif.playFrame(x, y); // returns 0 after the last frame, then loops
            unsigned long elapsed = millis() - start;
            if (result < 0) {
                log_e("GIF playFrame error: %d", gif.getLastError());
                idx = (idx + 1) % total;
                break;
            }
            if ((result == 0 && elapsed >= GIF_MIN_MS) || elapsed >= GIF_MAX_MS) {
                idx = (idx + 1) % total;
                break;
            }
        }
        // gif is closed and freed by its destructor at the end of this scope
    }
    tft.fillScreen(bruceConfig.bgColor);
#else
    displayWarning("GIFs not available in Lite", true);
#endif
}

void GifsMenu::drawIcon(float scale) {
    clearIconArea();
    uint16_t c = bruceConfig.priColor;
    int w = scale * 74;
    int h = scale * 48;
    int x = iconCenterX - w / 2;
    int y = iconCenterY - h / 2;

    // film frame
    tft.drawRoundRect(x, y, w, h, scale * 5, c);
    tft.drawRoundRect(x + 1, y + 1, w - 2, h - 2, scale * 4, c);

    // sprocket holes along the top and bottom
    int hole = max(2, (int)(scale * 4));
    int holes = 7;
    for (int i = 0; i < holes; i++) {
        int hx = x + scale * 7 + i * (w - scale * 14) / (holes - 1) - hole / 2;
        tft.fillRect(hx, y + scale * 4, hole, hole, c);
        tft.fillRect(hx, y + h - scale * 4 - hole, hole, hole, c);
    }

    // play triangle
    int t = scale * 11;
    tft.fillTriangle(
        iconCenterX - t / 2, iconCenterY - t, iconCenterX - t / 2, iconCenterY + t, iconCenterX + t, iconCenterY, c
    );
}
