#include "source/c/library/bank_helpers.h"

// Some useful global variables
ZEROPAGE_DEF(int, catapultXPosition);
ZEROPAGE_DEF(int, playerXPosition);
ZEROPAGE_DEF(int, playerYPosition);
ZEROPAGE_DEF(int, playerXVelocity);
ZEROPAGE_DEF(int, playerYVelocity);
ZEROPAGE_DEF(int, nextPlayerXPosition);
ZEROPAGE_DEF(int, nextPlayerYPosition);
ZEROPAGE_DEF(unsigned char, playerControlsLockTime);
ZEROPAGE_DEF(unsigned char, playerInvulnerabilityTime);
ZEROPAGE_DEF(unsigned char, playerDirection);