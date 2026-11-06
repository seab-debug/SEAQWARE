#include "GifsMenu.h"
#include "core/display.h"
#include "core/sd_functions.h"
#include <algorithm>
#include <vector>

#if !defined(LITE_VERSION)
static const char *GIF_DIRS[] = {"/SEAQUI/gifs", "/gifs"};

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
        if (lower.endsWith(".gif")) out.push_back(fullPath);
    }
    root.close();
    std::sort(out.begin(), out.end());
}

static String gifTitle(const String &path) {
    String name = path.substring(path.lastIndexOf('/') + 1);
    int dot = name.lastIndexOf('.');
    if (dot > 0) name = name.substring(0, dot);
    name.replace('_', ' ');
    name.toUpperCase();
    return name;
}

static void drawLabel(const String &title, int idx, int total, bool paused) {
    const int barH = 16;
    tft.fillRect(0, tftHeight - barH, tftWidth, barH, bruceConfig.bgColor);
    tft.drawFastHLine(0, tftHeight - barH, tftWidth, bruceConfig.priColor);
    tft.setTextSize(1);
    tft.setTextColor(bruceConfig.priColor, bruceConfig.bgColor);
    String left = (paused ? String("|| ") : String("")) + title;
    tft.drawString(left, 6, tftHeight - barH + 5);
    String right = String(idx + 1) + "/" + String(total);
    tft.drawRightString(right, tftWidth - 6, tftHeight - barH + 5, 1);
}
#endif

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
    bool exitPage = false;

    while (!exitPage) {
        tft.fillScreen(bruceConfig.bgColor);
        Gif gif;
        if (!gif.openGIF(fs, gifs[idx].c_str())) {
            displayError("Can't open " + gifTitle(gifs[idx]), true);
            idx = (idx + 1) % total;
            if (total == 1) break;
            continue;
        }

        int x = max(0, (tftWidth - gif.getCanvasWidth()) / 2);
        int y = max(0, (tftHeight - gif.getCanvasHeight()) / 2);
        String title = gifTitle(gifs[idx]);
        bool paused = false;

        // Show the name briefly, then clear it so the GIF plays full screen.
        drawLabel(title, idx, total, false);
        unsigned long labelUntil = millis() + 1500;

        while (true) {
            if (check(EscPress)) {
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
            if (check(SelPress)) {
                paused = !paused;
                drawLabel(title, idx, total, paused);
                labelUntil = paused ? 0 : millis() + 1200;
            }

            if (labelUntil && millis() > labelUntil) {
                // Clear the label and restart the GIF so no leftover pixels stay on screen.
                tft.fillScreen(bruceConfig.bgColor);
                gif.reset();
                labelUntil = 0;
            }

            if (paused) {
                vTaskDelay(20 / portTICK_PERIOD_MS);
                continue;
            }

            int result = gif.playFrame(x, y); // loops back to the first frame on its own
            if (result < 0) {
                log_e("GIF playFrame error: %d", gif.getLastError());
                gif.reset();
                vTaskDelay(20 / portTICK_PERIOD_MS);
            }
            if (labelUntil) drawLabel(title, idx, total, false); // keep the label on top while visible
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
