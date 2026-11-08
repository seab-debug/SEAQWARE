#ifndef __GIFS_MENU_H__
#define __GIFS_MENU_H__

#include <MenuItemInterface.h>

// GIFs page: as soon as it is highlighted in the main menu, GIFs play full screen
// (built-in SEAQWARE GIFs, then extras from SD:/SEAQUI/gifs, SD:/gifs or LittleFS:/gifs).
// Rotate = move to the next/previous menu item, press = skip to the next GIF.
class GifsMenu : public MenuItemInterface {
public:
    GifsMenu() : MenuItemInterface("GIFs") {}

    void optionsMenu(void);
    void drawIcon(float scale);
    bool playsOnHover() override { return true; }
    void onHover() override;

private:
    void play(bool fromMenuHover);
    int _idx = 0; // remembers where the playlist was when you scroll back
    bool hasTheme() { return bruceConfig.theme.gifs; }
    const String &themePath() override { return bruceConfig.theme.paths.gifs; }
};

#endif
