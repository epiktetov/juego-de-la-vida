/*
 * @(#)elvista.cpp -- el Vista (the View) header -- $Revision: $
 *
 * Viewer/editor/etc for Conway's Game of Life (in full color)
 * Copyright 2002 by Michael Epiktetov, epi@ieee.org
 *
 *-----------------------------------------------------------------------------
 *
 * $Log: $
 *
 */

#ifndef _EL_VISTA_H_
#define _EL_VISTA_H_

#if 0 /* ndef __GDK_TYPES_H__ // file <gdk/gdktypes.h> */
  typedef struct _GdkWindow GdkDrawable;
  typedef struct _GdkEventConfigure GdkEventConfigure;
#endif
#include <gtk/gtkwidget.h>
#include "elmundo.h"

class elVista : public elObservador
{
public:
  elVista(elMundo *em, GtkWidget *gw);
  ~elVista();
  static int initColorTable(GdkDrawable *win); // return 0 if Ok

  int resize(int delta_mag, int x_fixed_vis, int y_fixed_vis);
  int resize(int delta_mag = 0); // with fixed center position
  void resize_to_fit();
  void center(int x_abs, int y_abs);

  int Xvis2abs(int x_vis), Yvis2abs(int y_vis),
      Xabs2vis(int x_abs), Yabs2vis(int y_abs);

  void clear(),  clear (int x_vis, int y_vis, int width, int height);
  void update(), update(int x_vis, int y_vis, int width, int height);

  void paint  (int x_vis, int y_vis, int color);
  void observe(int x_abs, int y_abs, int color); // from elObservador

private:
  elMundo   *world;
  GtkWidget *drawingArea; 

  int x_center, mag, xOffset, pixels_per_cell, cell_size,
      y_center,      yOffset, cells_per_pixel;
  void adjust_mag_parameters(int &width_abs, int &height_abs);
public:
  inline int get_mag() { // positive mag means pixels_per_cell > 1,
    return mag;          //     negative means cells_per_pixel > 1
  }
  inline int get_width () { return drawingArea->allocation.width;  }
  inline int get_height() { return drawingArea->allocation.height; }
};

#endif /* _EL_VISTA_H_ */

