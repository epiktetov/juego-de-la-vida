static char const rcsid[] = "$Id: $";
/*
 * epiLife++ elMundo (the World) keeps the world of Life cells
 *
 * Viewer/editor/etc for Conway's Game of Life (in full color)
 * Copyright 2002 by Michael Epiktetov, epi@ieee.org
 *
 *-----------------------------------------------------------------------------
 *
 * $Log: $
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "elpp.h"
#include "elmundo.h"
#include "elvista.h"

elMundo:: elMundo() { }
elMundo::~elMundo() { }

void elMundo::setFrame(int xmin, int xmax, int ymin, int ymax)
{
  XvMin = xmin; YvMin = ymin; 
  XvMax = xmax; YvMax = ymax;
}
void elMundo::show(elVista &ev) { iterate(ev); }
void elMundo::show(elVista &ev,
                   int xmin, int xmax, int ymin, int ymax)
{
  iterate(ev, xmin, xmax, ymin, ymax);
}
/*---------------------------------------------------------------------------*/

void elMundo::paste(const char *pattern, int *width, int *height)
{
  FILE *fp = fopen(pattern, "r");
  if (fp == NULL) pasteString(pattern, width, height);
  else {
    char line[100], *ptn = (char *)malloc(256);
    int size = 256, plen = 0, len;

    while (fgets(line, 80, fp)) {
      if ((len = strlen(line)) < 1 || line[0] == '#') continue;
      if (size < plen+len) ptn = (char *)realloc(ptn, size = size+size/2);
      strcpy(ptn+plen, line);                                 plen += len;
    }
    fclose(fp); pasteString(ptn, width, height);
    free(ptn);
} }
void elMundo::pasteString(const char *pattern, int *width, int *height)
{
  char rule[33]; const char *p = pattern, *p2;
  int x, y, clr, mult;
  switch (sscanf(pattern, "x = %d, y = %d, rule = %32s\n", &x, &y, &rule)) {
  case 3:
    setRules(rule); // FALL TROUGH
  case 2:
    if (width)  *width  = x;
    if (height) *height = y;
  case 1:
    p = strchr(pattern, '\n');
    p2 = strchr(pattern, ';');
    if (!p || (p2 && p > p2)) p = p2;
    if (!p || *(++p) == 0) return;
  }
  for (x = y = mult = 0; *p; p++) {
    if ('0' <= *p && *p <= '9') mult = mult*10 + (*p)-'0';
    else if (*p == '!') break;
    else if (*p != ' ' && *p != '\t' && *p != '\n') {
      if (mult == 0) mult = 1;
      switch (*p) {
      case 'b':        x += mult; mult = 0; continue;
      case '$': x = 0; y += mult; mult = 0; continue;
      //
      // Because 'b' is already taken, we have to use Spanish names for Blue
      //
      case 'r': clr = elcRed;     break;
      case 'g': clr = elcGreen;   break;  case 'v': clr = elcVerde; break;
                                          case 'a': clr = elcAzul;  break;
      case 'y': clr = elcYellow;  break;
      case 'c': clr = elcCyan;    break;
      case 'm': clr = elcMagenta; break;
      case 'k': clr = elcBlack;   break;  case 'n': clr = elcNegro; break;
      case 'w': clr = elcWhite;   break;  case 'x': clr = elcMax;   break;
      default:  clr = elcDefault; break;
      }
      while (mult) { add((x++), y, clr); mult--; } // make sure mult==0
  } }
}
