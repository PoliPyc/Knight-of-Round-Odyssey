#include "source/c/neslib.h"
#include "source/c/menus/title.h"
#include "source/c/globals.h"
#include "source/c/configuration/game_states.h"
#include "source/c/configuration/system_constants.h"
#include "source/c/menus/text_helpers.h"
#include "graphics/palettes/palettes.config.h"
#include "source/c/configuration/game_info.h"

CODE_BANK(PRG_BANK_TITLE);

void draw_title_screen(void) {
    ppu_off();
    pal_bg(titlePalette);
    pal_spr(titlePalette);

    set_chr_bank_0(CHR_BANK_MENU);
    set_chr_bank_1(CHR_BANK_MENU);
    clear_screen();
    oam_clear();

    
    put_str(NTADR_A(3, 5), gameName);
    
    put_str(NTADR_A(2, 26), gameAuthorContact);
    
    put_str(NTADR_A(2, 28), "Copyright");
    put_str(NTADR_A(12, 28), currentYear);
    put_str(NTADR_A(17, 28), gameAuthor);

    put_str(NTADR_A(10, 16), "Press Start!");
    ppu_on_all();

    gameState = GAME_STATE_TITLE_INPUT;
}

void handle_title_input(void) {
    if (pad_trigger(0) & PAD_START) {
        gameState = GAME_STATE_POST_TITLE;
    }
}