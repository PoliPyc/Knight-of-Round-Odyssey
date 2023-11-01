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
    if (controllerState & PAD_SELECT) {
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
            if ((char)((playerXPosition + PLAYER_POSITION_TILE_MASK_MIDDLE) & PLAYER_POSITION_TILE_MASK) > (char)(PLAYER_POSITION_TILE_MASK_MIDDLE)) {
                playerXVelocity = 0-PLAYER_VELOCITY_NUDGE;
            } else if ((char)((playerXPosition + PLAYER_POSITION_TILE_MASK_MIDDLE) & PLAYER_POSITION_TILE_MASK) < (char)(PLAYER_POSITION_TILE_MASK_MIDDLE)) {
                playerXVelocity = PLAYER_VELOCITY_NUDGE;
            } // If equal, do nothing. That's our goal.
        }

        if (playerXVelocity != 0 && playerYVelocity == 0 && (controllerState & (PAD_UP | PAD_DOWN)) == 0) {
            if ((char)((playerYPosition + PLAYER_POSITION_TILE_MASK_MIDDLE + PLAYER_POSITION_TILE_MASK_EXTRA_NUDGE) & PLAYER_POSITION_TILE_MASK) > (char)(PLAYER_POSITION_TILE_MASK_MIDDLE)) {
                playerYVelocity = 0-PLAYER_VELOCITY_NUDGE;
            } else if ((char)((playerYPosition + PLAYER_POSITION_TILE_MASK_MIDDLE + PLAYER_POSITION_TILE_MASK_EXTRA_NUDGE) & PLAYER_POSITION_TILE_MASK) < (char)(PLAYER_POSITION_TILE_MASK_MIDDLE)) {
                playerYVelocity = PLAYER_VELOCITY_NUDGE;
            } // If equal, do nothing. That's our goal.
        }
    #endif

    nextPlayerXPosition = playerXPosition + playerXVelocity;
    nextPlayerYPosition = playerYPosition + playerYVelocity;

}

void do_catapult_movement(void) {

    // This will knock out the player's speed if they hit anything.
    test_player_tile_collision();
    // If the new player position hit any sprites, we'll find that out and knock it out here.
    handle_player_sprite_collision();

    playerXPosition += playerXVelocity;
    playerYPosition += playerYVelocity;


    rawXPosition = (playerXPosition >> PLAYER_POSITION_SHIFT);
    rawYPosition = (playerYPosition >> PLAYER_POSITION_SHIFT);
    // The X Position has to wrap to allow us to snap to the grid. This makes us shift when you wrap around to 255, down to 240-ish
    if (rawXPosition > (SCREEN_EDGE_RIGHT+4) && rawXPosition < SCREEN_EDGE_LEFT_WRAPPED) {
        // We use sprite direction to determine which direction to scroll in, so be sure this is set properly.
        if (playerInvulnerabilityTime) {
            playerXPosition -= playerXVelocity;
            rawXPosition = (playerXPosition >> PLAYER_POSITION_SHIFT);
        } else {
            playerDirection = SPRITE_DIRECTION_LEFT;
            gameState = GAME_STATE_SCREEN_SCROLL;
            playerOverworldPosition--;
        }
    } else if (rawXPosition > SCREEN_EDGE_RIGHT && rawXPosition < (SCREEN_EDGE_RIGHT+4)) {
        if (playerInvulnerabilityTime) {
            playerXPosition -= playerXVelocity;
            rawXPosition = (playerXPosition >> PLAYER_POSITION_SHIFT);
        } else {
            playerDirection = SPRITE_DIRECTION_RIGHT;
            gameState = GAME_STATE_SCREEN_SCROLL;
            playerOverworldPosition++;
        }
    } else if (rawYPosition > SCREEN_EDGE_BOTTOM) {
        if (playerInvulnerabilityTime) {
            playerYPosition -= playerYVelocity;
            rawYPosition = (playerYPosition >> PLAYER_POSITION_SHIFT);
        } else {
            playerDirection = SPRITE_DIRECTION_DOWN;
            gameState = GAME_STATE_SCREEN_SCROLL;
            playerOverworldPosition += 8;
        }
    } else if (rawYPosition < SCREEN_EDGE_TOP) {
        if (playerInvulnerabilityTime) {
            playerYPosition -= playerYVelocity;
            rawYPosition = (playerYPosition >> PLAYER_POSITION_SHIFT);
        } else {
            playerDirection = SPRITE_DIRECTION_UP;
            gameState = GAME_STATE_SCREEN_SCROLL;
            playerOverworldPosition -= 8;
        }
    }
}