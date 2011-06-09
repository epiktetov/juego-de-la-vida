static char const rcsid[] = "$Id: $";
/*
 * epiLife++ elVista (the View) is responsible for visual representation
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
#include <gdk/gdk.h>
#include <gtk/gtkwidget.h>
#include "elpp.h"
#include "elvista.h"
#include "elmundo.h"

elVista::elVista (elMundo *em, GtkWidget *gw)
{ 
  world = em;                drawingArea = gw;
  mag = pixels_per_cell = cells_per_pixel = 1;
  xOffset = yOffset = 0;
  x_center = y_center = 0;

  int width  = get_width(),  xmin = -width/2,
      height = get_height(), ymin = -height/2;

  world->setFrame(xmin, xmin+width-1, ymin, ymin+height-1);
  resize(0, -xmin, -ymin);
}
elVista::~elVista() {}
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
typedef struct { const char *name; // color name (just for diagnostic)
                 GdkColor   color; // definition of color (pixel/r/g/b)
                 GdkGC        *gc; // graphic context
} elColorTable;

static elColorTable visColors[] =
{
  { "Grey",        { 0, 0xAAAA, 0xAAAA, 0xAAAA }, 0 },
  { "Blue",        { 0, 0x0000, 0x0000, 0xFFFF }, 0 },
  { "Green",       { 0, 0x0000, 0xFFFF, 0x0000 }, 0 },
  { "Cyan",        { 0, 0x0000, 0xFFFF, 0xFFFF }, 0 },
  { "Red",         { 0, 0xFFFF, 0x0000, 0x0000 }, 0 },
  { "Magenta",     { 0, 0xFFFF, 0x0000, 0xFFFF }, 0 },
  { "Yellow",      { 0, 0xFFFF, 0xFFFF, 0x0000 }, 0 },
  { "White",       { 0, 0xFFFF, 0xFFFF, 0xFFFF }, 0 },

  { "DarkGrey",    { 0, 0x3333, 0x3333, 0x3333 }, 0 },
  { "DarkBlue",    { 0, 0x0000, 0x0000, 0x5555 }, 0 },
  { "DarkGreen",   { 0, 0x0000, 0x5555, 0x0000 }, 0 },
  { "DarkRed",     { 0, 0x0000, 0x5555, 0x5555 }, 0 },
  { "DarkCyan",    { 0, 0x5555, 0x0000, 0x0000 }, 0 },
  { "DarkMagenta", { 0, 0x5555, 0x0000, 0x5555 }, 0 },
  { "DarkYellow",  { 0, 0x5555, 0x5555, 0x0000 }, 0 },
  { "DarkWhite",   { 0, 0x5555, 0x5555, 0x5555 }, 0 },

  { "Background",  { 0, 0x0000, 0x0000, 0x2000 }, 0 },
  { "Grid",        { 0, 0x0000, 0x3000, 0x3000 }, 0 }
};

int elVista::initColorTable (GdkDrawable *win)
{
  GdkColormap *cmap = gdk_colormap_get_system();
  int i, error = 0;
  for (i = 0; i < TableSIZE(visColors); i++) {
    if (gdk_color_alloc(cmap, &visColors[i].color)) {
      visColors[i].gc = gdk_gc_new(win);
      gdk_gc_set_foreground(visColors[i].gc, &visColors[i].color);
    }
    else {
      fprintf(stderr, "ERROR: cannot allocate '%s' (%x;%d,%d,%d)\n",
              visColors[i].name,
              visColors[i].color.pixel, visColors[i].color.red,
              visColors[i].color.blue,  visColors[i].color.green); error++;
  } }
  return error;
}
/*---------------------------------------------------------------------------*/
static int magnifications[] = { -64, -32, -16, -8, -4, -2, -1,
                                                    1,  2,  3, 5, 8, 12, 20 };
static int getClosestMag (int target, int dir)
{                                      // dir > 0 means round up
  int i;                               // dir < 0 means round down

  if (dir > 0) {
    for (i = 0; i < TableSIZE(magnifications); i++)
      if (magnifications[i] >= target) return magnifications[i];
    //
    // Nothing appropriate found - return the maximum possible value:
    //
    return magnifications[TableSIZE(magnifications) - 1];
  }
  else {
    for (i = TableSIZE(magnifications); i--; )
      if (magnifications[i] <= target) return magnifications[i];
    /* else return minimum value: */   return magnifications[0];
} }
static int getRequiredMag (int abs, int vis)
{
  if (abs < vis) return vis/abs;
  else        return -abs/vis-1;
}
void elVista::adjust_mag_parameters (int &w_abs, int &h_abs)
{
  if (mag > 0) {
    pixels_per_cell = mag; w_abs = get_width()/mag;  h_abs = get_height()/mag;
    cells_per_pixel = 1; 
    xOffset = (get_width()  - w_abs*mag)/2; // distribute extra
    yOffset = (get_height() - h_abs*mag)/2; // pixels evenly
  }
  else {
    xOffset = yOffset = 0; // no unused pixels in this case
    pixels_per_cell = 1;
    cells_per_pixel = -mag; w_abs = -get_width() *mag;
                            h_abs = -get_height()*mag;
  }
  cell_size = (pixels_per_cell > 4) ? pixels_per_cell-1 : pixels_per_cell;
}
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
int elVista::resize (int delta_mag,
                     int x_fix_vis, int y_fix_vis) // fixed point
{
  int x_fix = Xvis2abs(x_fix_vis), w,
      y_fix = Yvis2abs(y_fix_vis), h, old_mag = mag;
#if 0
  printf("resize(%d;%d,%d) fix_abs=%d,%d", delta_mag, x_fix_vis,
                                                      y_fix_vis, x_fix, y_fix);
#endif
  mag = getClosestMag(mag+delta_mag, delta_mag);
  adjust_mag_parameters(w, h);

  int xmin = x_fix - cells_per_pixel*(x_fix_vis - xOffset)/pixels_per_cell,
      ymin = y_fix - cells_per_pixel*(y_fix_vis - yOffset)/pixels_per_cell;

  x_center = xmin + w/2;
  y_center = ymin + h/2;
#if 0
  printf(" x/ymin=%d/%d size=%dx%d\n", xmin, ymin, width, height);
#endif
  world->setFrame(xmin, xmin+w-1, ymin, ymin+h-1);

  return (mag == old_mag) ? 0 : delta_mag;
}
int elVista::resize (int delta_mag)
{ 
  int w, h, old_mag = mag;

  mag = getClosestMag(mag+delta_mag, delta_mag);
  adjust_mag_parameters(w, h);

  int xmin = x_center - w/2,
      ymin = y_center - h/2;

  world->setFrame(xmin, xmin+w-1, ymin, ymin+h-1);

  return (mag == old_mag) ? 0 : delta_mag;
}
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
void elVista::resize_to_fit (void)
{
  int xmin, xmax, ymin, ymax;      world->getFitFrame(xmin, xmax, ymin, ymax);
  int mag_by_w = getRequiredMag(xmax-xmin+1, get_width()),
      mag_by_h = getRequiredMag(ymax-ymin+1, get_height());

  center((xmin+xmax)/2, (ymin+ymax)/2);

  if (mag_by_w < mag_by_h) resize(mag_by_w-mag);
  else                     resize(mag_by_h-mag);
}
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
void elVista::center (int x_abs, int y_abs)
{
  x_center = x_abs;
  y_center = y_abs; resize();
}
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
inline int elVista::Xvis2abs (int x_vis) 
{ 
  return cells_per_pixel*(x_vis-xOffset)/pixels_per_cell + world->XvMin;
}
inline int elVista::Yvis2abs (int y_vis) 
{ 
  return cells_per_pixel*(y_vis-yOffset)/pixels_per_cell + world->YvMin;
}
inline int elVista::Xabs2vis (int x_abs) 
{ 
  return pixels_per_cell*(x_abs - world->XvMin)/cells_per_pixel + xOffset;
}
inline int elVista::Yabs2vis (int y_abs) 
{ 
  return pixels_per_cell*(y_abs - world->YvMin)/cells_per_pixel + yOffset;
}
/*---------------------------------------------------------------------------*/
void elVista::clear (void)
{
  clear(0, 0, drawingArea->allocation.width, drawingArea->allocation.height);
}
void elVista::clear (int x_vis, int y_vis, int width, int height)
{
  gdk_draw_rectangle(drawingArea->window,
            visColors[elvcBackground].gc, true, x_vis, y_vis,
                                                width, height);
  if (pixels_per_cell > 7) {
    int i, p;
    for (i = world->XvMin; i <= world->XvMax+1; i++) {
      p = Xabs2vis(i)-1;
      gdk_draw_line(drawingArea->window, visColors[elvcGrid].gc,
                    p, 0,
                    p, drawingArea->allocation.height-1);
    }
    for (i = world->YvMin; i <= world->YvMax+1; i++) {
      p = Yabs2vis(i)-1;
      gdk_draw_line(drawingArea->window, visColors[elvcGrid].gc,
                    0,                               p,
                    drawingArea->allocation.width-1, p);
  } }
}
void elVista::update (void)
{
  clear(); world->show(*this, world->XvMin, world->XvMax, 
                              world->YvMin, world->YvMax);
}
void elVista::update (int x_vis, int y_vis, int width, int height)
{
  clear(x_vis, y_vis, width, height);
  world->show(*this, Xvis2abs(x_vis), Xvis2abs(x_vis+width -1),
                     Yvis2abs(y_vis), Yvis2abs(y_vis+height-1));
}
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
void elVista::paint (int x_vis, int y_vis, int color)
{
  if (pixels_per_cell == 1) gdk_draw_point(drawingArea->window,
                                           visColors[color].gc, x_vis, y_vis);
  else
    gdk_draw_rectangle(drawingArea->window,
          visColors[color].gc, true, x_vis, y_vis, cell_size, cell_size);
}
void elVista::observe (int x_abs, int y_abs, int color)
{
  paint(Xabs2vis(x_abs), Yabs2vis(y_abs), color);
}

