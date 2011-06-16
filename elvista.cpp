//------------------------------------------------------+----------------------
// juego de la vida       el Vista (the View) code      | (c) Epi MG, 2002,2011
//------------------------------------------------------+----------------------
#include <QtGui> // elVista (the View) is responsible for visual representation

#include "jdlv.h"
#include "elvista.h"
#include "elmundo.h"
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
elVista::elVista (jdlvFrame *father, elMundo *lookingAtWorld)
                       : dad(father),   world(lookingAtWorld),
                           pt(0), pixmap(NULL) //ptNext(false)
{ 
  setAttribute(Qt::WA_OpaquePaintEvent);
//  setAttribute(Qt::WA_PaintOnScreen);
//  setAttribute(Qt::WA_NoSystemBackground);

  mag = pixels_per_cell = cells_per_pixel = 1;
  xOffset = yOffset = 0;
  x_center = y_center = 0;
}
elVista::~elVista() { if (pixmap) delete pixmap; }
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
static  QColor visColors[elvcMax]; /* indexed by elVisColor enum, see jdlv.h */
void elVista::initColors(void)
{
  visColors[ 0].setHsvF(  0.0/360,0.0,  0.0); // ... black
  visColors[ 8].setHsvF(200.0/360,0.333,1.0); // dead black cell
  visColors[ 1].setHsvF(220.0/360,1.0,  0.6); // ..B blue
  visColors[ 9].setHsvF(220.0/360,0.25, 1.0); // ..B blue dead
  visColors[ 2].setHsvF(120.0/360,0.8,  0.5); // .G. green
  visColors[10].setHsvF(120.0/360,0.25, 1.0); // .G. green
  visColors[ 3].setHsvF(180.0/360,1.0,  0.4); // .GB cyan
  visColors[11].setHsvF(180.0/360,0.25, 0.9); // .GB cyan
  visColors[ 4].setHsvF(  0.0/360,1.0,  0.6); // R.. red
  visColors[12].setHsvF(  0.0/360,0.16, 1.0); // R.. red
  visColors[ 5].setHsvF(300.0/360,1.0,  0.4); // R.B magenta
  visColors[13].setHsvF(300.0/360,0.16, 1.0); // R.B magenta
  visColors[ 6].setHsvF( 40.0/360,1.0,  0.4); // RG. yellow
  visColors[14].setHsvF( 40.0/360,0.25, 0.9); // RG. yellow
  visColors[ 7].setHsvF(  0.0/360,1.0,  1.0); // xxx not used
  visColors[15].setHsvF(  0.0/360,1.0,  1.0); // xxx not used
  visColors[elvcBackground].setHsvF(200.0/360,0.2,1.0);
  visColors[elvcGrid] = QColor("white");
}
//-----------------------------------------------------------------------------
void elVista::observe (int x_abs, int y_abs, int color)
{
  int x_vis = Xabs2vis(x_abs), y_vis = Yabs2vis(y_abs);
  if (pt) {
    QBrush brush(visColors[color], Qt::SolidPattern);
    pt->setBrush(brush);        QPen pen(brush, 1.0);
    pt->setPen(pen);
    pt->drawRect(x_vis, y_vis, cell_size, cell_size);
} }
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void elVista::show_next_gen()
{
  QPainter dc(pixmap);         pt = &dc;
  world->nextGeneration(this); pt = NULL; update();
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void elVista::makePixmap (int xmin, int ymin, int w, int h)
{
//+
//fprintf(stderr, "pixmap:x=%d,y=%d,%dx%d,mag=%d\n", xmin, ymin, w, h, mag);
//-
  QRect rect = this->rect();  if (pixmap) delete pixmap;
  pixmap = new QPixmap(rect.size());
  QPainter dc(pixmap);                                  dc.setPen(Qt::NoPen);
  QBrush B(visColors[elvcBackground],Qt::SolidPattern); dc.setBrush(B);
  dc.drawRect(rect);
  if (pixels_per_cell > 7) {
    dc.setPen(QPen(QColor(visColors[elvcGrid])));
    int i, p;
    for (i = world->XvMin; i <= world->XvMax+1; i++) {
      p = Xabs2vis(i)-1;
      dc.drawLine(p, rect.top(), p, rect.bottom());
    }
    for (i = world->YvMin; i <= world->YvMax+1; i++) {
      p = Yabs2vis(i)-1;
      dc.drawLine(rect.left(), p, rect.right(), p);
  } }
  world->setFrame   (xmin, xmin+w-1, ymin, ymin+h-1); pt = &dc;
  world->show(*this, xmin, xmin+w-1, ymin, ymin+h-1); pt = NULL;
  dad->UpdateMag();
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
int elVista::Xvis2abs (int x_vis)
{ 
  return cells_per_pixel*(x_vis-xOffset)/pixels_per_cell + world->XvMin;
}
int elVista::Yvis2abs (int y_vis)
{ 
  return cells_per_pixel*(y_vis-yOffset)/pixels_per_cell + world->YvMin;
}
int elVista::Xabs2vis (int x_abs)
{ 
  return pixels_per_cell*(x_abs - world->XvMin)/cells_per_pixel + xOffset;
}
int elVista::Yabs2vis (int y_abs)
{ 
  return pixels_per_cell*(y_abs - world->YvMin)/cells_per_pixel + yOffset;
}
//-----------------------------------------------------------------------------
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
    return magnifications[TableSIZE(magnifications)-1];
  }
  else {
    for (i = TableSIZE(magnifications); i--; )
      if (magnifications[i] <= target) return magnifications[i];
    /* else return minimum value: */   return magnifications[0];
} }
inline int getRequiredMag (int abs, int vis)
{
  if (abs < vis) return vis/abs;
  else        return -abs/vis-1;
}
void elVista::adjust_mag_params (int &w_abs, int &h_abs)
{
  if (mag > 0) {
    pixels_per_cell = mag; w_abs = width()/mag;  h_abs = height()/mag;
    cells_per_pixel = 1; 
    xOffset = (width()  - w_abs*mag)/2; // distribute extra
    yOffset = (height() - h_abs*mag)/2; // pixels evenly
  }
  else {
    xOffset = yOffset = 0; // no unused pixels in this case
    pixels_per_cell = 1;
    cells_per_pixel = -mag; w_abs = -width() *mag;
                            h_abs = -height()*mag;
  }
  cell_size = (pixels_per_cell >  4) ? pixels_per_cell-2 :
              (pixels_per_cell >  1) ? pixels_per_cell-1 : 1;
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void elVista::resizeVista (int delta_mag,
                           int x_fix_vis, int y_fix_vis) // fixed point
{
  int w, h, x_fix = Xvis2abs(x_fix_vis), y_fix = Yvis2abs(y_fix_vis);
  mag = getClosestMag(mag+delta_mag, delta_mag);   adjust_mag_params(w, h);
  int xmin = x_fix - cells_per_pixel*(x_fix_vis - xOffset)/pixels_per_cell,
      ymin = y_fix - cells_per_pixel*(y_fix_vis - yOffset)/pixels_per_cell;
  x_center = xmin + w/2;
  y_center = ymin + h/2; makePixmap(xmin, ymin, w, h);
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void elVista::resizeVista (int delta_mag)
{ 
  int w, h;
  mag = getClosestMag(mag+delta_mag, delta_mag); adjust_mag_params(w, h);
  int xmin = x_center - w/2,
      ymin = y_center - h/2; makePixmap(xmin, ymin, w, h);
}
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
void elVista::resize_to_fit (void)
{
  int                xmin, xmax, ymin, ymax;
  world->getFitFrame(xmin, xmax, ymin, ymax);
//+
//fprintf(stderr, "World:x=%d-%d,y=%d-%d(size:%dx%d)\n",
//                      xmin, xmax, ymin, ymax, width(), height());
//-
  int mag_by_w = getRequiredMag(xmax-xmin+1, width()),
      mag_by_h = getRequiredMag(ymax-ymin+1, height());
  if (mag_by_w > 1) mag_by_w = getRequiredMag(5*(xmax-xmin+1)/4, width());
  if (mag_by_h > 1) mag_by_h = getRequiredMag(5*(ymax-ymin+1)/4, height());
  if (mag_by_w > 8) mag_by_w = 8;                 x_center = (xmin+xmax)/2;
  if (mag_by_h > 8) mag_by_h = 8;                 y_center = (ymin+ymax)/2;
  if (mag_by_w < mag_by_h) resizeVista(mag_by_w - mag);
  else                     resizeVista(mag_by_h - mag);  update();
}
void elVista::resizeEvent (QResizeEvent*) { resizeVista(); }
//-----------------------------------------------------------------------------
void elVista::paintEvent (QPaintEvent*)
{
  QPainter dc(this); dc.drawPixmap(0,0,*pixmap);
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void elVista::mousePressEvent (QMouseEvent *ev)
{
  switch (ev->type()) {
  case QEvent::MouseButtonPress: break;
  case QEvent::MouseButtonDblClick:
    x_center = Xvis2abs(ev->x());
    y_center = Yvis2abs(ev->y()); resizeVista(); update(); ev->accept();
  default:
    break;
} }
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void elVista::wheelEvent (QWheelEvent *ev)
{
  if (ev->modifiers() & (Qt::MetaModifier|Qt::ControlModifier)) {
    resizeVista(ev->delta() > 0 ? +1 : -1, ev->x(), ev->y());
    update();
  }
  else {
    int delta = -ev->delta()/2;
    if (ev->orientation() == Qt::Vertical)
         y_center = Yvis2abs(Yabs2vis(y_center)+delta);
    else x_center = Xvis2abs(Xabs2vis(x_center)+delta);
    resizeVista();                            update();
} }
//-----------------------------------------------------------------------------
