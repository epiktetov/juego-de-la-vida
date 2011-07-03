//------------------------------------------------------+----------------------
// juego de la vida       el Vista (the View) code      | (c) Epi MG, 2002,2011
//------------------------------------------------------+----------------------
#include <QtGui> // elVista (the View) is responsible for visual representation

#include "jdlv.h"
#include "elvista.h"
#include "elmundo.h"

QIcon *enBlancoIco, *enRojoIco, *enCastanoIco, *enVerdeIco, *enAzulIco;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
static  QColor visColors[elvcMax]; /* indexed by elVisColor enum, see jdlv.h */
void elVista::initColors(void)
{
  visColors[ 0].setHsvF(  0.0/360,0.0,  0.0); // ... black
  visColors[ 8].setHsvF(200.0/360,0.333,1.0); // black ghost
  visColors[ 1].setHsvF(  0.0/360,1.0,  0.6); // ..R red
  visColors[ 9].setHsvF(  0.0/360,0.16, 1.0); // ..R red
  visColors[ 2].setHsvF(120.0/360,0.8,  0.5); // .G. green
  visColors[10].setHsvF(120.0/360,0.25, 1.0); // .G. green
  visColors[ 3].setHsvF( 40.0/360,1.0,  0.4); // .GR yellow
  visColors[11].setHsvF( 40.0/360,0.25, 0.9); // .GR yellow
  visColors[ 4].setHsvF(220.0/360,1.0,  0.6); // B.. blue
  visColors[12].setHsvF(220.0/360,0.25, 1.0); // B.. blue
  visColors[ 5].setHsvF(300.0/360,1.0,  0.4); // B.R magenta
  visColors[13].setHsvF(300.0/360,0.16, 1.0); // B.R magenta
  visColors[ 6].setHsvF(180.0/360,1.0,  0.4); // BG. cyan
  visColors[14].setHsvF(180.0/360,0.25, 0.9); // BG. cyan
  visColors[ 7].setHsvF(  0.0/360,1.0,  1.0); // xxx not used
  visColors[15].setHsvF(  0.0/360,1.0,  1.0); // xxx not used
  visColors[elvcGrid] = QColor("white");
  visColors[elvcBackground].setHsvF(200.0/360,0.2,1.0);
  visColors[elvcInfoText]  .setHsvF(200.0/360,0.8,0.8);
  visColors[elvcRectBorder].setHsvF(240.0/360,0.8,0.8);
  visColors[elvcRectFill]  .setHsvF(240.0/360,0.2,1.0);
  visColors[elvcPermSelect].setHsvF(220.0/360,0.3,0.9);
//
  QPixmap blank("buttons/empty1.png");
  QRect colorBlock(5,5,11,11);
  QPainter dc(&blank);                   enBlancoIco  = new QIcon(blank);
  dc.fillRect(colorBlock, visColors[1]); enRojoIco    = new QIcon(blank);
  dc.fillRect(colorBlock, visColors[2]); enVerdeIco   = new QIcon(blank);
  dc.fillRect(colorBlock, visColors[3]); enCastanoIco = new QIcon(blank);
  dc.fillRect(colorBlock, visColors[4]); enAzulIco    = new QIcon(blank);
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
elVista::elVista (jdlvFrame *father, elMundo *lookingAtWorld)
                       : dad(father),   world(lookingAtWorld),
     pt(0), pixmap(NULL), isMoving(false), isSelecting(false)
{ 
  setAttribute(Qt::WA_OpaquePaintEvent);
  mag = pixels_per_cell = cells_per_pixel = 1;
  xOffset = yOffset = 0;
  x_center = y_center = 0;
  QPalette palette(visColors[elvcBackground]);
  palette.setColor(QPalette::WindowText, visColors[elvcInfoText]);
  setPalette(palette);
}
void elVista::lookAtThis (elMundo *lookAtMe)
{
  lookAtMe->setFrame(world->XvMin, world->XvMax, world->YvMin, world->YvMax);
  world = lookAtMe; // do not use updateTheWorld - new world not populated yet
}
elVista::~elVista() { if (pixmap) delete pixmap; }
//-----------------------------------------------------------------------------
void elVista::observe (int x_abs, int y_abs, int color)
{
  int x_vis = Xabs2vis(x_abs), y_vis = Yabs2vis(y_abs);
  if (pt) {
    QBrush brush(visColors[color], Qt::SolidPattern);
    pt->setBrush(brush);
    if (draw_halo && color < elcDead) {
      QPen pen(visColors[color+8], 1.0);
      pt->setPen(pen);
      pt->drawRect(x_vis-1, y_vis-1, cell_size+2, cell_size+2);
    }
    else { QPen pen(brush, 1.0);
           pt->setPen(pen);
           pt->drawRect(x_vis, y_vis, cell_size, cell_size); }
} }
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void elVista::show_next_gen (elMundo *orig) // if orig == NULL, 'world' is used
{
  QPainter dc(pixmap);               pt = &dc;
  world->nextGeneration(this, orig); pt = NULL; update();
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void elVista::makePixmap (int xmin, int ymin, int w, int h)
{
  world->setFrame(xmin, xmin+w-1, ymin, ymin+h-1); // sets vis/abs conversion
  QRect rect = this->rect();
  if (pixmap) delete pixmap; pixmap = new QPixmap(rect.size());
  QPainter dc(pixmap);
  QBrush  brush (visColors[elvcBackground], Qt::SolidPattern);
  dc.setBrush(brush); dc.setPen(Qt::NoPen); dc.drawRect(rect);
  QRect selection;
  if (!absSelection.isEmpty()) {
    selection = rectAbs2vis(absSelection);
    brush = QBrush(visColors[elvcPermSelect], Qt::SolidPattern);
    dc.setBrush(brush);                  dc.drawRect(selection);
  }
  if (pixels_per_cell > 7) {
    dc.setPen(QPen( QColor(visColors[elvcGrid]) ));
    int i, p;
    for (i = world->XvMin; i <= world->XvMax+1; i++) {
      p = Xabs2vis(i)-1;
      dc.drawLine(p, rect.top(), p, rect.bottom());
    }
    for (i = world->YvMin; i <= world->YvMax+1; i++) {
      p = Yabs2vis(i)-1;
      dc.drawLine(rect.left(), p, rect.right(), p);
  } }
  if (!selection.isEmpty()) {
    QPen pen(QColor(visColors[elvcRectBorder]),2); dc.setBrush(Qt::NoBrush);
    dc.setPen(pen);
         if (pixels_per_cell  > 7) selection.adjust(-2,-2,+1,+1);
    else if (pixels_per_cell == 5) selection.adjust(-1,-1, 0, 0);
    dc.drawRect(selection);
  }                                                   pt = &dc;
  world->show(*this, xmin, xmin+w-1, ymin, ymin+h-1); pt = NULL;
  dad->UpdateMag();
  timeInfo.clear();
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
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
QRect elVista::rectAbs2vis (QRect abs)
{
  return QRect(QPoint(Xabs2vis(abs.left() )  , Yabs2vis(abs.top()   )  ),
               QPoint(Xabs2vis(abs.right())-1, Yabs2vis(abs.bottom())-1) );
}
QRect elVista::rectVis2abs (QRect visRect)
{
  return QRect(QPoint(Xvis2abs(visRect.left() + pixels_per_cell - 1),
                      Yvis2abs(visRect.top()  + pixels_per_cell - 1)      ),
               QPoint(Xvis2abs(visRect.right()), Yvis2abs(visRect.bottom()) ));
}
QRect elVista::rectFrom2points(QPoint a, QPoint b)
{
  int xmin, xmax, ymin, ymax;
  if (a.x() < b.x()) { xmin = a.x(); xmax = b.x(); }
  else               { xmin = b.x(); xmax = a.x(); }
  if (a.y() < b.y()) { ymin = a.y(); ymax = b.y(); }
  else               { ymin = b.y(); ymax = a.y(); }
  return QRect(QPoint(xmin,ymin), QPoint(xmax,ymax));
}
//-----------------------------------------------------------------------------
static int magnifications[] = { -64, -32, -16, -8, -4, -2, -1,
                                                    1,  2,  3, 5, 8, 12, 20 };
int elVista::find_closest_mag (int delta)
{
  int target = mag + delta;
  int i;
  if (delta < 0) {
    if (mag == 1) target = delta - 1;
    for (i = TableSIZE(magnifications); i--; )
      if (magnifications[i] <= target) return magnifications[i];
    /* else return minimum value: */   return magnifications[0];
  }
  else {
    if (mag == -1) target = delta + 1;
    for (i = 0; i < TableSIZE(magnifications); i++)
      if (magnifications[i] >= target) return magnifications[i];
//  else
      return magnifications[TableSIZE(magnifications)-1];
} }
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
  draw_halo = (pixels_per_cell == 5);
  cell_size = (pixels_per_cell >  4) ? pixels_per_cell-2 :
              (pixels_per_cell >  1) ? pixels_per_cell-1 : 1;
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void elVista::resizeVista (int delta_mag,
                           int x_fix_vis, int y_fix_vis) // fixed point
{
  int w, h, x_fix = Xvis2abs(x_fix_vis), y_fix = Yvis2abs(y_fix_vis);
  mag = find_closest_mag(delta_mag);               adjust_mag_params(w, h);
  int xmin = x_fix - cells_per_pixel*(x_fix_vis - xOffset)/pixels_per_cell,
      ymin = y_fix - cells_per_pixel*(y_fix_vis - yOffset)/pixels_per_cell;
  x_center = xmin + w/2;
  y_center = ymin + h/2; makePixmap(xmin, ymin, w, h);
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void elVista::resizeVista (int delta_mag)
{ 
  int w, h;
  mag = find_closest_mag(delta_mag); adjust_mag_params(w, h);
  int xmin = x_center - w/2,
      ymin = y_center - h/2; makePixmap(xmin, ymin, w, h);
}
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
inline int getRequiredMag (int abs, int vis) { if (abs < vis) return vis/abs;
                                               else        return -abs/vis-1; }
void elVista::resize_to_fit (void)
{
  int                xmin, xmax, ymin, ymax;
  world->getFitFrame(xmin, xmax, ymin, ymax);
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
  QPainter dc(this);       dc.drawPixmap        (0,0,*pixmap);
  if (!timeInfo.isEmpty()) dc.drawText(2,height()-2,timeInfo);
  if (!visPreSelect.isEmpty()) {
    QBrush B(visColors[elvcRectFill], Qt::SolidPattern); dc.setBrush(B);
    QPen pen(visColors[elvcRectBorder], 1.0);            dc.setPen(pen);
    dc.setOpacity(0.5);
    dc.drawRect(visPreSelect);
} }
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void elVista::mousePressEvent (QMouseEvent *ev)
{
  switch (ev->type()) {
  case QEvent::MouseButtonPress:
    cpLast = QPoint(x_center, y_center);
    mpLast = ev->pos();
#ifdef Q_OS_MAC                          // On Mac Qt always reports:
    if (ev->button() == Qt::RightButton) //   Ctrl+LeftButton == RightButton
#else                                    // make it the same on other platforms
    if (ev->button() == Qt::RightButton ||
       (ev->modifiers() & Qt::ControlModifier))
#endif
    { QRect vis = rectAbs2vis(absSelection);
      if (!absSelection.isEmpty() && vis.contains(ev->pos())) {
        if (ev->x() < vis.x() + vis.width()/2)   mpLast.setX(vis.right() +1);
        else                                     mpLast.setX(vis.left()    );
        if (ev->y() < vis.y() + vis.height()/2)  mpLast.setY(vis.bottom()+1);
        else                                     mpLast.setY(vis.top()     );
        visPreSelect = rectFrom2points(ev->pos(),mpLast);
      }
      absSelection.setSize(QSize(0,0)); updateTheWorld();
      isSelecting = true;     setCursor(Qt::CrossCursor);
    }
    else if (dad->changesAllowed()) {
      int absX = Xvis2abs(ev->x()),
          absY = Yvis2abs(ev->y());
      if (ev->modifiers() & Qt::ShiftModifier)
                world->toggle (absX, absY, dad->getCurrentColor());
      else if (!world->recolor(absX, absY, dad->getCurrentColor())) break;
      ev->accept();
      updateTheWorld(); dad->notifyOfChange();
    }
    break;
  case QEvent::MouseButtonDblClick:
    x_center = Xvis2abs(ev->x());
    y_center = Yvis2abs(ev->y()); ev->accept(); updateTheWorld();
  default:
    break;
} }
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void elVista::mouseMoveEvent (QMouseEvent *ev)
{
  if (isSelecting) { visPreSelect = rectFrom2points(mpLast, ev->pos());
                                                              update(); }
  else {
    x_center = cpLast.x()+cells_per_pixel*(mpLast.x()-ev->x())/pixels_per_cell;
    y_center = cpLast.y()+cells_per_pixel*(mpLast.y()-ev->y())/pixels_per_cell;
    ev->accept();
    if (! isMoving) setCursor(Qt::ClosedHandCursor);
          isMoving = true;         updateTheWorld();
} }
void elVista::mouseReleaseEvent (QMouseEvent*)
{
  if (isMoving)      unsetCursor(); isMoving    = false;
  if (isSelecting) { unsetCursor(); isSelecting = false;
    absSelection = rectVis2abs(visPreSelect);
    updateTheWorld();
    visPreSelect.setSize(QSize(0,0));
} }
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void elVista::wheelEvent (QWheelEvent *ev)
{
  if (ev->modifiers() & (Qt::ALT|Qt::META|Qt::CTRL))
       { resizeVista(ev->delta() > 0 ? +1 : -1, ev->x(), ev->y()); update(); }
  else {
    int delta = ev->delta()/2;
    if ((ev->modifiers() & Qt::SHIFT) || ev->orientation() == Qt::Horizontal)
         x_center = Xvis2abs(Xabs2vis(x_center)-delta);
    else y_center = Yvis2abs(Yabs2vis(y_center)-delta); updateTheWorld();
} }
//-----------------------------------------------------------------------------
