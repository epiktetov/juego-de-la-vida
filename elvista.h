//------------------------------------------------------+----------------------
// juego de la vida      el Vista (the View) header     | (c) Epi MG, 2002,2011
//------------------------------------------------------+----------------------
#ifndef EL_VISTA_H_INCLUDED
#define EL_VISTA_H_INCLUDED

#include "elmundo.h"
#include <QWidget>

class elVista : public QWidget, public elObservador
{
  jdlvFrame   *dad; // father frame
  elMundo   *world; // the world we're observing
  QPainter     *pt; // painter used to draw on pixmap, used by observe()
  QPixmap  *pixmap; // how the wolrd looks (effectively triple buffering)
  QString timeInfo; // time info string to show in the corner
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
public:    elVista(jdlvFrame *padre, elMundo *mirandoAlMundo);
  virtual ~elVista();
  void lookAtThis(elMundo *mirandoAlMundo);
  static void initColors(void);
         void observe(int x_abs, int y_abs, int color); // from elObservador
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  void makePixmap(int xmin, int ymin, int w, int h);
  void show_next_gen (elMundo *startingFrom = NULL);
  int Xvis2abs(int x_vis), Yvis2abs(int y_vis),
      Xabs2vis(int x_abs), Yabs2vis(int y_abs);
protected:
  int x_center, mag, xOffset, pixels_per_cell,      cell_size,
      y_center,      yOffset, cells_per_pixel; bool draw_halo;
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  int find_closest_mag(int delta_mag);
  void adjust_mag_params         (int  &width_abs, int &height_abs);
  void resizeVista(int delta_mag, int x_fixed_vis, int y_fixed_vis);
  void resizeVista(int delta_mag = 0); // with fixed center position
public:
  inline int get_mag() { // positive mag means pixels_per_cell > 1,
    return mag;          //     negative means cells_per_pixel > 1
  }
  void updateTheWorld() { resizeVista(); update(); }
  void resize_to_fit();
  void resizeEvent(QResizeEvent *ev);
  void paintEvent (QPaintEvent  *ev);
  void show_time_info (QString info) { timeInfo = info; }
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
protected:
  void mousePressEvent  (QMouseEvent *ev); QPoint mpLast; // mouse press (vis)
  void mouseMoveEvent   (QMouseEvent *ev); QPoint cpLast; // last center (abs)
  void mouseReleaseEvent(QMouseEvent *ev); bool isMoving;
  void wheelEvent       (QWheelEvent *ev); bool isSelecting;
         QRect rectAbs2vis         (QRect visRect);
         QRect rectVis2abs         (QRect visRect);
  static QRect rectFrom2points(QPoint a, QPoint b);
public:
  QRect getSelection(void) const { return  absSelection; }
protected:
  QRect visPreSelect; // selection in process (right-dragging), vista coords
  QRect absSelection; // block has been selected, absolute (world) coordinates
};
extern QIcon *enBlancoIco, *enRojoIco, *enCastanoIco, *enVerdeIco, *enAzulIco;
/*---------------------------------------------------------------------------*/
#endif                                                /* EL_VISTA_H_INCLUDED */
