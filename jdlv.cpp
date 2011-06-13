//------------------------------------------------------+----------------------
// juego de la vida      Startup and main controls      | (c) Epi MG, 2002,2011
//------------------------------------------------------+----------------------
#include <QApplication>
#include <QtGui>
#include "jdlv.h"
#include "elmundo.h" /* class elMundo, elObservador */
#include "elvista.h" /* class elVista               */
#include "version.h"

static elMundo  *theWorld;
static jdlvFrame *mainWin;
elMode eM;
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class jdlvStyle : public QProxyStyle
{
public:
  virtual int pixelMetric(PixelMetric m, const QStyleOption *opt = 0,
                                         const QWidget   *widget = 0) const
  { switch (m) {
    case QStyle::PM_ToolBarFrameWidth:  return 1;
    case QStyle::PM_ToolBarItemMargin:  return 0;
    case QStyle::PM_ToolBarItemSpacing: return 0;
    default:
      return QProxyStyle::pixelMetric(m, opt, widget);
  } }
};
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
int main(int argc, char *argv[])
{
  QApplication app(argc,argv); app.setStyle(new jdlvStyle);
  QCoreApplication::setOrganizationDomain("epiktetov.com");
  QCoreApplication::setOrganizationName("EpiMG");
  QCoreApplication::setApplicationName("micro7");
#ifdef Q_OS_MAC
  app.installEventFilter(new MacEvents);
#endif
  elMundo::initRegExs();
  elVista::initColors();
  theWorld = new_elMundoA(); if (argc == 2) theWorld->pasteFile(argv[1]);
  mainWin = new jdlvFrame(theWorld);
  mainWin->show();
  return app.exec();
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#ifdef Q_OS_MAC
bool MacEvents::eventFilter (QObject*, QEvent *ev)
{
  if (ev->type() == QEvent::FileOpen) {
    QString filename = static_cast<QFileOpenEvent*>(ev)->file();
    theWorld->clearTheWorld();
    theWorld->pasteFile(filename.cStr());
    mainWin->ShowTheWorld();
                                                    return true;
  } else                                            return false;
}
#endif
//-----------------------------------------------------------------------------
class permanentToolBar : public QToolBar
{
  bool event(QEvent *event)
  {
    if (event->type() == QEvent::Close) { event->ignore(); return true; }
    else return QToolBar::event(event);
  }
public: permanentToolBar(const QString &title, QWidget *parent = 0)
              : QToolBar(               title,          parent) { }
};
#ifdef Q_OS_MAC
# define jdlvFONTFACENAME "Menlo"
# define jdlvFONTSIZE  12
#else
# define jdlvFONTFACENAME "Consolas"
# define jdlvFONTSIZE  10
#endif
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
jdlvFrame::jdlvFrame(elMundo *world) : timerID(0), speed(3)
{
  QFont fixedFont(jdlvFONTFACENAME, jdlvFONTSIZE);
        fixedFont.setStyleHint(QFont::TypeWriter);
  vista = new elVista(this,world);
  vista->setMinimumSize(600, 360); setCentralWidget(vista);
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  QToolBar *bottom = new permanentToolBar("ctrl", this);
  bottom->setFloatable(false);
  bottom->setMovable  (false);
  bottom->setIconSize(QSize(28,22));
  modeView = new QAction(QIcon("buttons/eye.png"),   QString("view"),  this);
  modeRept = new QAction(QIcon("buttons/block.png"), QString("paint"), this);
  modeEdit = new QAction(QIcon("buttons/new1.png"),  QString("edit"),  this);
  connect(modeView, SIGNAL(triggered()), this, SLOT(SetModeView()));
  connect(modeRept, SIGNAL(triggered()), this, SLOT(SetModeRept()));
  connect(modeEdit, SIGNAL(triggered()), this, SLOT(SetModeEdit()));
  QActionGroup *modeGroup = new QActionGroup(this);
  modeView->setCheckable(true); modeView ->setChecked(true);
  modeRept->setCheckable(true);
  modeEdit->setCheckable(true);
  modeGroup->addAction(modeView); bottom->addAction(modeView);
  modeGroup->addAction(modeRept); bottom->addAction(modeRept);
  modeGroup->addAction(modeEdit); bottom->addAction(modeEdit);

  QLabel *magIcon = new QLabel;
  magIcon->setPixmap(QPixmap("buttons/zoom1.png"));
  magText = new QLabel(QString("+1"));
  magText->setFont(fixedFont);
  bottom->addWidget(magText);
  bottom->addWidget(magIcon);
  fitAll = new QAction(QIcon("buttons/full-size.png"), QString("fit"), this);
  connect(fitAll, SIGNAL(triggered()), this, SLOT(DoFitAll()));
  bottom->addAction(fitAll);
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  QMenu *play_menu = menuBar()->addMenu("Play");
  nextGen = new QAction(QIcon("buttons/step.png"), QString("step"), this);
  playStop = new QAction(QIcon("buttons/go-forward.png"), QString("go"), this);
  playDelay = new QLabel(QString(""));
  reLoad = new QAction(QIcon("buttons/reload2.png"), QString("reload"), this);
  connect(nextGen,  SIGNAL(triggered()), this, SLOT(DoNextGen()));
  connect(playStop, SIGNAL(triggered()), this, SLOT(DoPlayStop()));
  connect(reLoad,   SIGNAL(triggered()), this, SLOT(DoReload()));
  play_menu->addAction(nextGen);
  play_menu->addAction(playStop);
  play_menu->addAction(reLoad);  reLoad->setEnabled(false);
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  speedSlider = new QSlider(Qt::Horizontal, this);
  speedSlider->setMinimum(1); speedSlider->setValue(speed);
  speedSlider->setMaximum(4); speedSlider->setMaximumSize(50,22);
  speedSlider->setSingleStep(1);
  speedSlider->setTickInterval(1);
  speedSlider->setTickPosition(QSlider::TicksBelow);
  connect(speedSlider, SIGNAL(valueChanged(int)), this, SLOT(ChangeSpeed(int)));
  bottom->addAction(nextGen);
  bottom->addWidget(speedSlider);
  bottom->addAction(playStop);
  bottom->addWidget(playDelay);
  bottom->addAction(reLoad);
  addToolBar(Qt::BottomToolBarArea, bottom);
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void jdlvFrame::SetModeView () { eM = elModeView; }
void jdlvFrame::SetModeRept () { eM = elModeRept; }
void jdlvFrame::SetModeEdit () { eM = elModeEdit; }

void jdlvFrame::DoNextGen()    { vista->show_next_gen(); }
void jdlvFrame::DoFitAll ()    { vista->resize_to_fit(); }
void jdlvFrame::ShowTheWorld() { vista->resize_to_fit(); }
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
int speed2msec[] = { -1, 600, 150, 30, 0 };
quint64 lastTick;
void jdlvFrame::DoPlayStop()
{
  if (eM != elModePlay) {  genNo = 0;
      eM =  elModePlay;
    lastTick = pgtime();
    timerID = startTimer(speed2msec[speed]);
    playStop->setIcon(QIcon("buttons/stop.png"));
  }
  else {
                          eM = elModeView;
    killTimer(timerID);
    playStop->setIcon(QIcon("buttons/go-forward.png"));
  }
}
void jdlvFrame::ChangeSpeed (int newSpeed)
{
  speed = newSpeed; if (eM == elModePlay) { killTimer(timerID);
                       timerID = startTimer(speed2msec[speed]); }
}
void jdlvFrame::DoReload() { }
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void jdlvFrame::timerEvent(QTimerEvent *)
{
  quint64 tick_time = pgtime(); vista->show_next_gen(); genNo++;
  quint64 stop_time = pgtime();
  QString S;
  switch (speed) { // showing play speed differently for different selections
  case 1:          // (1,2: round to 1/100th of sec, 3,4: with msec precision)
  case 2:
    S.sprintf("%d,+%.2fs(%dms)", genNo, (qreal)(tick_time -  lastTick)/1000.0,
                                          (int)(stop_time - tick_time)); break;
  case 3:
    S.sprintf("%d,+%dms(%dms)", genNo, (int)(tick_time -  lastTick),
                                       (int)(stop_time - tick_time)); break;
  default:
    S.sprintf("%d,+%dms", genNo, (int)(tick_time -  lastTick));
  }
  playDelay->setText(S); lastTick = tick_time;
}
void jdlvFrame::UpdateMag() { QString S; S.sprintf("%+3d",vista->get_mag());
                                                        magText->setText(S); }
//-----------------------------------------------------------------------------
#include <time.h>
#ifdef Q_OS_WIN
#  include <windows.h>
#  define EPOCH_BIAS 116444736000000000LL
#  define TEN_MILLION          10000000LL
#  define TEN_THOUSAND            10000LL
typedef union {
  FILETIME         ft_struct;
  unsigned __int64 ft_scalar;
}
FILETIME_as_int64;

void usectime_p (struct timeval *tv)
{
  FILETIME_as_int64 ft;
  GetSystemTimeAsFileTime(&(ft.ft_struct));
  tv->tv_sec = (time_t)((ft.ft_scalar -   EPOCH_BIAS) /  TEN_MILLION);
  tv->tv_usec = (long)(((ft.ft_scalar / TEN_THOUSAND) % 1000) * 1000);
}
#else
# include <sys/time.h>
# define usectime_p(_stv) gettimeofday((_stv),0)
#endif
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
quint64 pgtime(void)    /* returns current time with "pretty good" precision */
{
  struct timeval tv;                   usectime_p(&tv);
  return (quint64)tv.tv_sec * 1000 + tv.tv_usec / 1000;
}
//-----------------------------------------------------------------------------
