#include "source/c/neslib.h"
#include "source/c/sprites/playable_sprites.h"
#include "source/c/sprites/catapult.h"
#include "source/c/library/bank_helpers.h"
#include "source/c/globals.h"
#include "source/c/map/map.h"
#include "source/c/configuration/game_states.h"
#include "source/c/configuration/system_constants.h"
#include "source/c/sprites/collision.h"
#include "source/c/sprites/sprite_definitions.h"
#include "source/c/sprites/map_sprites.h"
#include "source/c/menus/error.h"
#include "source/c/graphics/hud.h"
#include "source/c/graphics/game_text.h"
#include "source/c/sprites/map_sprites.h"

CODE_BANK(PRG_BANK_CATAPULT_SPRITE);

// Huge pile of temporary variables
#define rawXPosition tempChar1
#define rawYPosition tempChar2
#define rawTileId tempChar3

void update_catapult_sprite(void) {
    // Calculate the position of the player itself, then use these variables to build it up with 4 8x8 NES sprites.
    rawXPosition = (catapultXPosition >> PLAYER_POSITION_SHIFT);
    rawYPosition = (playerYPosition >> PLAYER_POSITION_SHIFT);
    rawTileId = CATAPULT_SPRITE_TILE_ID;

    if (playerXVelocity != 0 || playerYVelocity != 0) {
        // Does some math with the current NES frame to add either 2 or 0 to the tile id, animating the sprite.
        rawTileId += ((frameCount >> SPRITE_ANIMATION_SPEED_DIVISOR) & 0x01) << 1;
    }

    // Clamp the player's sprite X Position to 0 to make sure we don't loop over, even if technically we have.
    if (rawXPosition > (SCREEN_EDGE_RIGHT + 4)) {
        rawXPosition = 0;
    }
    
    if (gameState == GAME_STATE_CATAPULT) {
        oam_spr(rawXPosition, rawYPosition, rawTileId, 0x02, CATAPULT_SPRITE_INDEX);
        oam_spr(rawXPosition + NES_SPRITE_WIDTH, rawYPosition, rawTileId + 1, 0x02, CATAPULT_SPRITE_INDEX+4);
        oam_spr(rawXPosition, rawYPosition + NES_SPRITE_HEIGHT, rawTileId + 16, 0x02, CATAPULT_SPRITE_INDEX+8);
        oam_spr(rawXPosition + NES_SPRITE_WIDTH, rawYPosition + NES_SPRITE_HEIGHT, rawTileId + 17, 0x02, CATAPULT_SPRITE_INDEX+12);
    }
    else {
        //recolor to grey
        oam_spr(rawXPosition, rawYPosition, rawTileId, 0x00, CATAPULT_SPRITE_INDEX);
        oam_spr(rawXPosition + NES_SPRITE_WIDTH, rawYPosition, rawTileId + 1, 0x00, CATAPULT_SPRITE_INDEX+4);
        oam_spr(rawXPosition, rawYPosition + NES_SPRITE_HEIGHT, rawTileId + 16, 0x00, CATAPULT_SPRITE_INDEX+8);
        oam_spr(rawXPosition + NES_SPRITE_WIDTH, rawYPosition + NES_SPRITE_HEIGHT, rawTileId + 17, 0x00, CATAPULT_SPRITE_INDEX+12);
    }
    

}

void prepare_catapult_movement(void) {
    // Using a variable, so we can change the velocity based on pressing a button, having a special item,
    // or whatever you like!
    int maxVelocity = PLAYER_MAX_VELOCITY;
    lastControllerState = controllerState;
    controllerState = pad_poll(0);

    // If Start is pressed now, and was not pressed before...
    if (controllerState & PAD_START && !(lastControllerState & PAD_START)) {
        gameState = GAME_STATE_PAUSED;
        return;
    }
    if (controllerState & PAD_SELECT && !(lastControllerState & PAD_SELECT)) {
        gameState = GAME_STATE_BISHOP;
        return;
    }
  
    if (controllerState & PAD_RIGHT && playerXVelocity >= (0 - PLAYER_VELOCITY_NUDGE)) {
        // If you're moving right, and you're not at max, speed up.
        if (playerXVelocity < maxVelocity) {
            playerXVelocity += PLAYER_VELOCITY_ACCEL;
        // If you're over max somehow, we'll slow you down a little.
        } else if (playerXVelocity > maxVelocity) {
            playerXVelocity -= PLAYER_VELOCITY_ACCEL;
        }
    } else if (controllerState & PAD_LEFT && playerXVelocity <= (0 + PLAYER_VELOCITY_NUDGE)) {
        // If you're moving left, and you're not at max, speed up.
        if (ABS(playerXVelocity) < maxVelocity) {
            playerXVelocity -= PLAYER_VELOCITY_ACCEL;
        // If you're over max, slow you down a little...
        } else if (ABS(playerXVelocity) > maxVelocity) { 
            playerXVelocity += PLAYER_VELOCITY_ACCEL;
        }
    } else if (playerXVelocity != 0) {
        // Not pressing anything? Let's slow you back down...
        if (playerXVelocity > 0) {
            playerXVelocity -= PLAYER_VELOCITY_ACCEL;
        } else {
            playerXVelocity += PLAYER_VELOCITY_ACCEL;
        }
    }
    
    // Now, slowly adjust to the grid as possible, if the player isn't pressing a direction. 
    #if PLAYER_MOVEMENT_STYLE == MOVEMENT_STYLE_GRID
        if (playerYVelocity != 0 && playerXVelocity == 0 && (controllerState & (PAD_LEFT | PAD_RIGHT)) == 0) {
            if ((char)((catapultXPosition + PLAYER_POSITION_TILE_MASK_MIDDLE) & PLAYER_POSITION_TILE_MASK) > (char)(PLAYER_POSITION_TILE_MASK_MIDDLE)) {
                playerXVelocity = 0-PLAYER_VELOCITY_NUDGE;
            } else if ((char)((catapultXPosition + PLAYER_POSITION_TILE_MASK_MIDDLE) & PLAYER_POSITION_TILE_MASK) < (char)(PLAYER_POSITION_TILE_MASK_MIDDLE)) {
                playerXVelocity = PLAYER_VELOCITY_NUDGE;
            } // If equal, do nothing. That's our goal.
        }
    #endif
}

void do_catapult_movement(void) {

    // This will knock out the player's speed if they hit anything.
    // test_player_tile_collision();
    // If the new player position hit any sprites, we'll find that out and knock it out here.
    // handle_player_sprite_collision();

    catapultXPosition += playerXVelocity;


    rawXPosition = (catapultXPosition >> PLAYER_POSITION_SHIFT);
    rawYPosition = (playerYPosition >> PLAYER_POSITION_SHIFT);
    // The X Position has to wrap to allow us to snap to the grid. This makes us shift when you wrap around to 255, down to 240-ish
    if (rawXPosition > (SCREEN_EDGE_RIGHT+4) && rawXPosition < SCREEN_EDGE_LEFT_WRAPPED) {
        // We use sprite direction to determine which direction to scroll in, so be sure this is set properly.
        if (playerInvulnerabilityTime) {
            catapultXPosition -= playerXVelocity;
            rawXPosition = (catapultXPosition >> PLAYER_POSITION_SHIFT);
        } else {
            playerDirection = SPRITE_DIRECTION_LEFT;
            gameState = GAME_STATE_SCREEN_SCROLL;
            playerOverworldPosition--;
        }
    } else if (rawXPosition > SCREEN_EDGE_RIGHT && rawXPosition < (SCREEN_EDGE_RIGHT+4)) {
        if (playerInvulnerabilityTime) {
            catapultXPosition -= playerXVelocity;
            rawXPosition = (catapultXPosition >> PLAYER_POSITION_SHIFT);
        } else {
            playerDirection = SPRITE_DIRECTION_RIGHT;
            gameState = GAME_STATE_SCREEN_SCROLL;
            playerOverworldPosition++;
        }
    }
}