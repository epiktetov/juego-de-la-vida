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
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class jdlvStyle : public QProxyStyle
{
public:
  virtual int pixelMetric(PixelMetric m, const QStyleOption *opt = 0,
                                         const QWidget   *widget = 0) const
  { switch (m) {
    case QStyle::PM_ToolBarFrameWidth:  return 0;
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
  QCoreApplication::setApplicationName("jdlv");
#ifdef Q_OS_MAC
  app.installEventFilter(new MacEvents);
#elif defined(UNIX)
  app.setWindowIcon(QIcon(":/jdlv48.png"));
#endif
  elMundo::initRegExs(); theWorld = new_elMundoA();
  elVista::initColors();
  if (argc > 1) {
    if (strcmp(argv[1],"-4g") == 0) theWorld->make4guns(argv[2]);
    else                            theWorld->pasteFile(argv[1]);
  }
  mainWin = new  jdlvFrame(theWorld);
  mainWin->show(); return app.exec();
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#ifdef Q_OS_MAC
bool MacEvents::eventFilter (QObject*, QEvent *ev)
{
  if (ev->type() == QEvent::FileOpen) {
    mainWin->LoadTheWorld(static_cast<QFileOpenEvent*>(ev)->file());
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
jdlvFrame::jdlvFrame(elMundo *world) : eM(elModeView), genNo(0),
                          showTime(false), timerID(0), speed(3)
{
  QFont fixedFont(jdlvFONTFACENAME, jdlvFONTSIZE); SetWinTitle();
        fixedFont.setStyleHint(QFont::TypeWriter);
  vista = new elVista(this,world);
  vista->setMinimumSize(720, 480); setCentralWidget(vista);
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  QToolBar *bottom = new permanentToolBar("ctrl", this);
  bottom->setFloatable(false);
  bottom->setMovable  (false);
  bottom->setIconSize(QSize(28,25));
  showTimeCB = new QAction(QIcon("buttons/time1.png"), QString("time"), this);
  showInfoPB = new QAction(QIcon("buttons/info.png"), QString("info"), this);
  connect(showTimeCB, SIGNAL(triggered(bool)), this, SLOT(ToggleTime(bool)));
  connect(showInfoPB, SIGNAL(triggered()), this, SLOT(ShowInfo()));
  bottom->addAction(showTimeCB);
  bottom->addAction(showInfoPB);          showInfoPB->setEnabled(false); //+
  bottom->addWidget(new QLabel(" "));
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  QActionGroup *modeGroup = new QActionGroup(this);
  modeView = new QAction(QIcon("buttons/eye-half.png"),   QString("View"),  this);
  modeEdit = new QAction(QIcon("buttons/pen-in-box-2.png"),  QString("Edit"),  this);
  setColor = new QAction(QIcon("buttons/empty1.png"), QString("color"), this);
  connect(modeView, SIGNAL(triggered()), this, SLOT(SetModeView()));
  connect(modeEdit, SIGNAL(triggered()), this, SLOT(SetModeEdit()));
  connect(setColor, SIGNAL(triggered()), this, SLOT(PopupColorMenu()));
  showTimeCB->setCheckable(true);
  modeView  ->setCheckable(true); modeView ->setChecked(true);
  modeEdit  ->setCheckable(true);
  modeView->setShortcut(QKeySequence("F1"));
  modeEdit->setShortcut(QKeySequence("F2"));
  modeGroup->addAction(modeView); bottom->addAction(modeView);
  modeGroup->addAction(modeEdit); bottom->addAction(modeEdit);
                                  bottom->addAction(setColor);
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  QMenu *file_menu = menuBar()->addMenu("File");
  openFile = new QAction(QIcon("buttons/book.png"), QString("Open.."), this);
  reLoad = new QAction(QIcon("buttons/reload1.png"), QString("reload"), this);
  connect(reLoad,   SIGNAL(triggered()), this, SLOT(DoReload()));
  file_menu->addAction(openFile);
  file_menu->addAction(reLoad);    reLoad->setEnabled(false);
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  QMenu *edit_menu = menuBar()->addMenu("Edit");
//  pasteClip = new QAction(QIcon("buttons/new2.png"), QString("paste"), this);
  pasteClip = new QAction(QIcon("buttons/win-menu.png"), QString("paste"), this);
  saveCLR = new QAction(QIcon("buttons/transfers-blue.png"), QString("save"), this);
  saveBnW = new QAction(QIcon("buttons/transfers.png"), QString("save b&w"), this);
  newWin = new QAction(QIcon("buttons/windows1.png"), QString("new window"), this);
  bottom->addAction(pasteClip);     pasteClip->setEnabled(false); //+
  bottom->addAction(saveCLR);
  bottom->addAction(saveBnW);
  bottom->addAction(newWin);       newWin->setEnabled(false); //+
  connect(pasteClip, SIGNAL(triggered()), this, SLOT(PasteClip()));
  connect(saveCLR, SIGNAL(triggered()), this, SLOT(SaveCLR()));
  connect(saveBnW, SIGNAL(triggered()), this, SLOT(SaveBnW()));
  connect(newWin, SIGNAL(triggered()), this, SLOT(NewWindow()));
  edit_menu->addAction(pasteClip);
  edit_menu->addAction(saveCLR);
  edit_menu->addAction(saveBnW);
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  QString yellow = Utf8("castaño");
  colorMenu = menuBar()->addMenu("Color");
  colorMenu->addAction(*enBlancoIco, "blanco")->setShortcut(QKeySequence("b"));
  colorMenu->addAction(*enRojoIco,     "rojo")->setShortcut(QKeySequence("r"));
  colorMenu->addAction(*enCastanoIco,  yellow)->setShortcut(QKeySequence("c"));
  colorMenu->addAction(*enVerdeIco,   "verde")->setShortcut(QKeySequence("v"));
  colorMenu->addAction(*enAzulIco,     "azul")->setShortcut(QKeySequence("a"));
  connect(colorMenu, SIGNAL(triggered(QAction*)),
               this, SLOT(SelectColor(QAction*)));
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  QLabel *magIcon = new QLabel;
  magIcon->setPixmap(QPixmap("buttons/zoom3.png"));
  magText = new QLabel(QString("+1"));
  magText->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
  magText->setFont(fixedFont);
  magText->setMinimumSize(QSize(30,25));
  bottom->addWidget(magText);
  bottom->addWidget(magIcon);
  fitView = new QAction(QIcon("buttons/full-size.png"), QString("fit"), this);
  connect(fitView, SIGNAL(triggered()), this, SLOT(DoFitView()));
  bottom->addAction(fitView);
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  QMenu *play_menu = menuBar()->addMenu("Play");
  playGen = new QLabel(QString("0"));
  playGen->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
  playGen->setMinimumSize(QSize(66,25));
  prevGen = new QAction(QIcon("buttons/step-back.png"), QString("back"), this);
  nextGen = new QAction(QIcon("buttons/go-forward.png"), QString("step"), this);
  playStop = new QAction(QIcon("buttons/fast-forward.png"), QString("go!"), this);
  connect(prevGen,  SIGNAL(triggered()), this, SLOT(DoPrevGen()));
  connect(nextGen,  SIGNAL(triggered()), this, SLOT(DoNextGen()));
  connect(playStop, SIGNAL(triggered()), this, SLOT(DoPlayStop()));
  play_menu->addAction(nextGen);
  play_menu->addAction(playStop);
  play_menu->addAction(prevGen); prevGen->setEnabled(false);
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  speedSlider = new QSlider(Qt::Horizontal, this);
  speedSlider->setMinimum(1); speedSlider->setValue(speed);
  speedSlider->setMaximum(4); speedSlider->setMaximumSize(50,22);
  speedSlider->setSingleStep(1);
  speedSlider->setTickInterval(1);
  speedSlider->setTickPosition(QSlider::TicksBelow);
  connect(speedSlider, SIGNAL(valueChanged(int)), this, SLOT(ChangeSpeed(int)));
  bottom->addWidget(playGen);
  bottom->addAction(prevGen);
  bottom->addAction(nextGen);
  bottom->addWidget(speedSlider);
  bottom->addAction(playStop);
  addToolBar(Qt::BottomToolBarArea, bottom);
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void jdlvFrame::LoadTheWorld(QString filename)
{
  theWorld->clearTheWorld();
  theWorld->pasteFile(filename.cStr()); vista->resize_to_fit();
  genNo = 0;     playGen->setText("0");
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void jdlvFrame::SetWinTitle()
{
  setWindowTitle(QString("juego de la vida %1").arg(jdlvVERSION));
}
//-----------------------------------------------------------------------------
void jdlvFrame::ToggleTime(bool on) { if (!on) vista->show_time_info(""); }
void jdlvFrame::ShowInfo() {}
void jdlvFrame::SetModeView() { eM = (eM & elModePlay) | elModeView; }
void jdlvFrame::SetModeEdit() { eM = (eM & elModePlay) | elModeEdit; }
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void jdlvFrame::PopupColorMenu()          { colorMenu->popup(QCursor::pos()); }
void jdlvFrame::SelectColor(QAction *act) { setColor->setIcon(act->icon());   }
void jdlvFrame::PasteClip() {}
void jdlvFrame::SaveBnW()
{
  elSalvador saver(*theWorld);
  saver.save(false);
}
void jdlvFrame::SaveCLR()
{
  elSalvador saver(*theWorld);
  saver.save(true);
}
void jdlvFrame::NewWindow() {}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void jdlvFrame::DoPrevGen() { }
void jdlvFrame::DoNextGen() { timerEvent(NULL);       }
//+{ vista->show_next_gen(); }
void jdlvFrame::DoFitView() { vista->resize_to_fit(); }
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
int speed2msec[] = { -1, 600, 150, 30, 0 };
quint64 lastTick;
void jdlvFrame::DoPlayStop()
{
  if (eM & elModePlay) {
    eM &= ~elModePlay;                killTimer(timerID);
    playStop->setIcon(QIcon("buttons/fast-forward.png"));
  }
  else {
      eM |= elModePlay;
    lastTick = pgtime();
    timerID = startTimer(speed2msec[speed]);
    playStop->setIcon(QIcon("buttons/step.png"));
  }
}
void jdlvFrame::ChangeSpeed (int newSpeed)
{
  speed = newSpeed; if (eM & elModePlay) {  killTimer(timerID);
                       timerID = startTimer(speed2msec[speed]); }
}
void jdlvFrame::DoReload() { }
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void jdlvFrame::UpdateMag() { QString S; S.sprintf("%+3d",vista->get_mag());
                                                        magText->setText(S); }
void jdlvFrame::timerEvent(QTimerEvent *)
{
  quint64 tick_time = pgtime(); vista->show_next_gen(); genNo++;
  quint64 stop_time = pgtime();
  QString S = QString::number(genNo);
  if (S.length() > 3) S.insert(-3,',');
  if (S.length() > 7) S.insert(-7,','); playGen->setText(S);
  if (showTimeCB->isChecked()) {
    switch (speed) { // showing play speed differently for different selections
    case 1:          // 1,2: round to 1/100th of sec ~ 3,4: with msec precision
    case 2:
      S.sprintf("+%.2fs(%dms)", (qreal)(tick_time -  lastTick)/1000.0,
                                  (int)(stop_time - tick_time)); break;
    case 3:
      S.sprintf("+%dms(%dms)", (int)(tick_time -  lastTick),
                               (int)(stop_time - tick_time)); break;
    default:
      S.sprintf("+%dms", (int)(tick_time -  lastTick));
    }
    vista->show_time_info(S); lastTick = tick_time;
} }
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
