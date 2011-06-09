/*
 * @(#)elpp.h -- epiLife++ main header -- $Revision: $
 *
 * Viewer/editor/etc for Conway's Game of Life (in full color)
 * Copyright 2002 by Michael Epiktetov, epi@ieee.org
 *
 *-----------------------------------------------------------------------------
 *
 * $Log: $
 *
 */
#ifndef _EpiLifePP_H_
#define _EpiLifePP_H_

#include <stdlib.h>
#define MALLOCx1(_type)    (_type *)malloc(    sizeof(_type))
#define MALLOCxN(_type, N) (_type *)malloc(N * sizeof(_type))

#define TableSIZE(_X) (sizeof(_X)/sizeof(*(_X)))

typedef enum { elcBlack   = 0, elcNegro    = 0, elcDefault  = 0,
               elcBlue    = 1, elcAzul     = 1, elcSelected = 1,
               elcGreen   = 2, elcVerde    = 2,
               elcRed     = 4, elcRojo     = 4,
               elcCyan    = 3, elcCianico  = 3,
               elcMagenta = 5,
               elcYellow  = 6, elcAmarillo = 6,
               elcBrown   = 6, elcMarron   = 6,
               elcGray    = 7, elcGris     = 7,
               elcWhite   = 7, elcBlanco   = 7, elcMax     = 7,
                                                elcDead    = 8
} elColor;

typedef enum {
  elvcGrey    = 0, elvcDarkGrey    =  8, elcvDead = 8,
  elvcBlue    = 1, elvcDarkBlue    =  9,
  elvcGreen   = 2, elvcDarkGreen   = 10, elvcBackground = 16,
  elvcRed     = 4, elvcDarkRed     = 12, elvcGrid       = 17,
  elvcCyan    = 3, elvcDarkCyan    = 11,
  elvcMagenta = 5, elvcDarkMagenta = 13,
  elvcYellow  = 6, elvcDarkYellow  = 14,
  elvcWhite   = 7, elvcDarkWhite   = 15, elvcMax = 18
}
elVisColor;

#endif /* _EpiLifePP_H_ */

