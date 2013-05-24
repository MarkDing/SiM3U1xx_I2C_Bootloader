//------------------------------------------------------------------------------
// Copyright (c) 2013 by Silicon Laboratories. 
// All rights reserved. This program and the accompanying materials
// are made available under the terms of the Silicon Laboratories End User 
// License Agreement which accompanies this distribution, and is available at
// http://developer.silabs.com/legal/version/v10/License_Agreement_v10.htm
// Original content and implementation provided by Silicon Laboratories.
//------------------------------------------------------------------------------

/*
 *  global.h
 *
 *    Revision: 1.0 (10 MAY 2013)
 *      Author: fbar / cmenke
 */

// Framework Defined Constants and User Configuration
#include "const.h"
#include "userconfig.h"
#include "factoryconfig.h"


// Framework Defined Type Definitions
#include <stdint.h>
#include "types.h"


// Global symbols
extern uint32_t trigger;
extern uint32_t flash_size;
extern uint32_t ram_size;


// Exported symbols from each module
#include "device.h"
#include "flctl.h"
#include "comm.h"
#include "filemgr.h"
#include "dfu.h"

