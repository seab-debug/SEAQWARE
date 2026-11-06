#ifndef __GIFS_MENU_H__
#define __GIFS_MENU_H__

#include <MenuItemInterface.h>

// Main-menu page that plays GIFs full screen.
// Looks in SD:/SEAQUI/gifs, then SD:/gifs, then LittleFS:/gifs.
// Controls: next/prev = change GIF, select = pause/resume, esc = back to the main menu.
class GifsMenu : public MenuItemInterface {
public:
    GifsMenu() : MenuItemInterface("GIFs") {}

    void optionsMenu(void);
    void drawIcon(float scale);
    bool hasTheme() { return bruceConfig.theme.gifs; }
    const String &themePath() override { return bruceConfig.theme.paths.gifs; }
};

#endif
