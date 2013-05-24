#pragma once

/////////////////////////////////////////////////////////////////////////////
// External Global Variables
/////////////////////////////////////////////////////////////////////////////

extern unsigned long _crctbl[];

/////////////////////////////////////////////////////////////////////////////
// Macros
/////////////////////////////////////////////////////////////////////////////

/***************************************************************************\
The updcrc macro (referred to here as _crc) is derived from an article
Copyright © 1986 by Stephen Satchell.
“Programmers may incorporate any or all code into their programs, giving
proper credit within the source. Publication of the source routines is
permitted so long as proper credit is given to Steven Satchell, Satchell
Evaluations, and Chuck Forsberg, Omen technology."
\***************************************************************************/
#define _crc(accum,delta)    (accum)=_crctbl[((accum)^(delta))&0xff]^((accum)>>8)
/***************************************************************************/
