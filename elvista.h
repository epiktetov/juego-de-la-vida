//------------------------------------------------------+----------------------
// juego de la vida      el Vista (the View) header     | (c) Epi MG, 2002,2011
//------------------------------------------------------+----------------------
#ifndef EL_VISTA_H_INCLUDED
#define EL_VISTA_H_INCLUDED

#include "elmundo.h"
#include <QWidget>

class elVista : public QWidget, public elObservador
{
  jdlvFrame  *dad;              // father frame
  elMundo  *world;              // the world we're observing
  QPainter    *pt; //bool ptNext; // painter & "paint next gen" flag
  QPixmap *pixmap;
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
public:    elVista(jdlvFrame *padre, elMundo *mirandoAlMundo);
  virtual ~elVista();
  static void initColors(void);
         void observe(int x_abs, int y_abs, int color); // from elObservador
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  void makePixmap(int xmin, int ymin, int w, int h);
  void show_next_gen();
  int Xvis2abs(int x_vis), Yvis2abs(int y_vis),
      Xabs2vis(int x_abs), Yabs2vis(int y_abs);
protected:
  int x_center, mag, xOffset, pixels_per_cell, cell_size,
      y_center,      yOffset, cells_per_pixel;
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  void adjust_mag_params         (int  &width_abs, int &height_abs);
  void resizeVista(int delta_mag, int x_fixed_vis, int y_fixed_vis);
  void resizeVista(int delta_mag = 0); // with fixed center position
public:
  inline int get_mag() { // positive mag means pixels_per_cell > 1,
    return mag;          //     negative means cells_per_pixel > 1
  }
  void resize_to_fit();
  void resizeEvent(QResizeEvent *ev);
  void paintEvent (QPaintEvent  *ev);
//protected:
//  void paint(int x_vis, int y_vis, int color);
//  void clearRect (QRect rect);
//  void updateRect(QRect rect);
//public:
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  void mousePressEvent(QMouseEvent *ev);
  void wheelEvent     (QWheelEvent *ev);
};
/*---------------------------------------------------------------------------*/
#endif                                                /* EL_VISTA_H_INCLUDED */
