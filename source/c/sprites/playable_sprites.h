#include "source/c/library/bank_helpers.h"
#define PRG_BANK_PLAYER_SPRITE 2

ZEROPAGE_EXTERN(int, catapultXPosition);
ZEROPAGE_EXTERN(int, playerXPosition);
ZEROPAGE_EXTERN(int, playerYPosition);
ZEROPAGE_EXTERN(int, playerXVelocity);
ZEROPAGE_EXTERN(int, playerYVelocity);
ZEROPAGE_EXTERN(int, nextPlayerXPosition);
ZEROPAGE_EXTERN(int, nextPlayerYPosition);
ZEROPAGE_EXTERN(unsigned char, playerInvulnerabilityTime);
ZEROPAGE_EXTERN(unsigned char, playerControlsLockTime);
ZEROPAGE_EXTERN(unsigned char, playerDirection);