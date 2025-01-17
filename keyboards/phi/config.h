// Copyright 2023 klefff (@klefff)
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

/*
 * Feature disable options
 *  These options are also useful to firmware size reduction.
 */

/* disable debug print */
//#define NO_DEBUG

/* disable print */
//#define NO_PRINT

/* disable action features */
//#define NO_ACTION_LAYER
//#define NO_ACTION_TAPPING
//#define NO_ACTION_ONESHOT

// Encoder
#define NUMBER_OF_ENCODERS 1

#define ENCODERS_PAD_A { D1 }
#define ENCODERS_PAD_B { D0 }

#define ENCODER_RESOLUTION 4

//#define DEBOUNCE 10


//#define ENCODER_DEFAULT_POS 0x3

//#define ENCODER_MAP_KEY_DELAY 10
