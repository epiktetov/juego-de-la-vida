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
  visColors[elvcGrid] = QColor("white");
  visColors[elvcBackground].setHsvF(200.0/360,0.2,1.0);
  visColors[elvcInfoText]  .setHsvF(200.0/360,0.8,0.8);
//
  QPixmap blank("buttons/empty1.png");
  QRect colorBlock(5,5,11,11);
  QPainter dc(&blank);                   enBlancoIco  = new QIcon(blank);
  dc.fillRect(colorBlock, visColors[4]); enRojoIco    = new QIcon(blank);
  dc.fillRect(colorBlock, visColors[6]); enCastanoIco = new QIcon(blank);
  dc.fillRect(colorBlock, visColors[2]); enVerdeIco   = new QIcon(blank);
  dc.fillRect(colorBlock, visColors[1]); enAzulIco    = new QIcon(blank);
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
elVista::elVista (jdlvFrame *father, elMundo *lookingAtWorld)
                       : dad(father),   world(lookingAtWorld),
                         pt(0), pixmap(NULL), isMoving(false)
{ 
  setAttribute(Qt::WA_OpaquePaintEvent);
//+  setAttribute(Qt::WA_PaintOnScreen);
//   setAttribute(Qt::WA_NoSystemBackground);

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
    pt->setBrush(brush);        QPen pen(brush, 1.0);
    pt->setPen(pen);
    pt->drawRect(x_vis, y_vis, cell_size, cell_size);
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
//+
//fprintf(stderr, "pixmap:x=%d,y=%d,%dx%d,mag=%d\n", xmin, ymin, w, h, mag);
//-
  QRect rect = this->rect();  if (pixmap) delete pixmap;
  pixmap = new QPixmap(rect.size());
  world->setFrame(xmin, xmin+w-1, ymin, ymin+h-1);
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
  } }                                                 pt = &dc;
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
  QPainter dc(this);       dc.drawPixmap        (0,0,*pixmap);
  if (!timeInfo.isEmpty()) dc.drawText(2,height()-2,timeInfo);
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void elVista::mousePressEvent (QMouseEvent *ev)
{
  switch (ev->type()) {
  case QEvent::MouseButtonPress: mpLast = ev->pos();
    if (dad->changesAllowed()) {
      int absX = Xvis2abs(ev->x()),
          absY = Yvis2abs(ev->y());
      if (ev-> modifiers() & Qt::ShiftModifier)
                world->toggle (absX, absY, dad->getCurrentColor());
      else if (!world->recolor(absX, absY, dad->getCurrentColor())) break;
      ev->accept();                              updateTheWorld();
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
  int dX = ev->x() - mpLast.x();
  int dY = ev->y() - mpLast.y();
  x_center = Xvis2abs(Xabs2vis(x_center)-dX);
  y_center = Yvis2abs(Yabs2vis(y_center)-dY); mpLast = ev->pos();
//+
  fprintf(stderr, "dx=(%d,%d),center=(%d,%d)\n", dX, dY,
                            x_center, y_center);
//-
  ev->accept();
  if (! isMoving) setCursor(Qt::ClosedHandCursor);
        isMoving = true;         updateTheWorld();
}
void elVista::mouseReleaseEvent (QMouseEvent*)
{
  if (isMoving) unsetCursor(); isMoving = false;
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void elVista::wheelEvent (QWheelEvent *ev)
{
  if (ev->modifiers() & (Qt::MetaModifier|Qt::ControlModifier)) {
    resizeVista(ev->delta() > 0 ? +1 : -1, ev->x(), ev->y());
    update();
  }
  else {
    int delta = ev->delta()/2;
    if (ev->orientation() == Qt::Vertical)
         y_center = Yvis2abs(Yabs2vis(y_center)-delta);
    else x_center = Xvis2abs(Xabs2vis(x_center)-delta); updateTheWorld();
} }
//-----------------------------------------------------------------------------
