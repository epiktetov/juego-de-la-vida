//------------------------------------------------------+----------------------
// juego de la vida      Startup and main controls      | (c) Epi MG, 2002-2017
//------------------------------------------------------+----------------------
#include <QApplication>
#include <QFileDialog>
#include <QtGui>
#include <QMenuBar>
#include <QToolBar>
#include <QProxyStyle>
#include "jdlv.h"
#include "elmundo.h" /* class elMundo, elObservador */
#include "elvista.h" /* class elVista               */
#include "version.h"

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
  QApplication app(argc,argv); app.setStyle(new jdlvStyle); srand(time(NULL));
  QCoreApplication::setOrganizationDomain("epiktetov.com");
  QCoreApplication::setOrganizationName("EpiMG");  elMundo::initRegExs();
  QCoreApplication::setApplicationName("jdlv");    elVista::initColors();
#ifdef Q_OS_MAC
  app.installEventFilter(new MacEvents);
#elif defined(UNIX)
  app.setWindowIcon(QIcon(":/jdlv48.png"));
#endif
  const char *filename = NULL, // TODO: move option4g to Lua script
             *option4g = NULL;
  if (argc > 1) {
    if (strcmp(argv[1],"-4g") == 0) option4g = argv[2] ? argv[2] : "*";
                               else filename = argv[1];
  }
  mainWin = new jdlvFrame(filename);
  if (option4g) {
    mainWin->getPrimeWorld()->make4guns(option4g);
    mainWin->getCurrentVista()->resize_to_fit();
  }
  return app.exec();
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
jdlvFrame::jdlvFrame (const char *fileToLoad) : nextWorld(NULL),
  isChanged(false),
    eM(elModeView), genNo(0), curColor(0), speed(3), timerID(0)
{
  QFont fixedFont(jdlvFONTFACENAME, jdlvFONTSIZE);
        fixedFont.setStyleHint(QFont::TypeWriter);
  primeWorld = new_elMundoA();
  vista = new elVista (this, primeWorld); setCentralWidget(vista);
  vista->setMinimumSize(720, 480);
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  QToolBar *bottom = new permanentToolBar("ctrl", this);
  bottom->setFloatable(false);
  bottom->setMovable  (false);
  bottom->setIconSize(QSize(28,25));
  showTimeCB = new QAction(QIcon(":/time.png"), QString("time"), this);
  showInfoPB = new QAction(QIcon(":/info.png"), QString("info"), this);
  connect(showTimeCB, SIGNAL(triggered(bool)), this, SLOT(ToggleTime(bool)));
  connect(showInfoPB, SIGNAL(triggered()),     this, SLOT(ShowInfo()));
  bottom->addAction(showTimeCB);
  bottom->addAction(showInfoPB);         showInfoPB->setEnabled(false); //+
  bottom->addWidget(new QLabel(" "));
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  QActionGroup *modeGroup = new QActionGroup(this);
  modeEdit = new QAction(QIcon(":/pen-in-box.png"),QString("Edit (X)"),  this);
  modeView = new QAction(QIcon(":/eye-half.png"),  QString("View (Z)"),  this);
  setColor = new QAction(QIcon(":/empty1.png"),    QString("color"),     this);
  connect(modeView, SIGNAL(triggered()), this, SLOT(SetModeView()));
  connect(modeEdit, SIGNAL(triggered()), this, SLOT(SetModeEdit()));
  connect(setColor, SIGNAL(triggered()), this, SLOT(PopupColorMenu()));
  showTimeCB->setCheckable(true);
  modeEdit  ->setCheckable(true); modeEdit->setShortcut(QKeySequence("X"));
  modeView  ->setCheckable(true); modeView->setShortcut(QKeySequence("Z"));
  modeView  ->setChecked  (true);
  modeGroup->addAction(modeView); bottom->addAction(modeView);
  modeGroup->addAction(modeEdit); bottom->addAction(modeEdit);
                                  bottom->addAction(setColor);
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  QMenu *file_menu = menuBar()->addMenu("File");
  openFile =  new QAction(QIcon(":/book.png"), QString("Open.."), this);
  reLoad = new QAction(QIcon(":/reload1.png"), QString("reload"), this);
  connect(openFile, SIGNAL(triggered()), this, SLOT(OpenFile()));
  connect(reLoad,   SIGNAL(triggered()), this, SLOT(DoReload()));
  file_menu->addAction(openFile);
  file_menu->addAction(reLoad);        reLoad->setEnabled(false);
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  QMenu *edit_menu = menuBar()->addMenu("Edit");
  deleteSelected = new QAction(QString("delete"), this);
  deleteSelected->setShortcut(QKeySequence("Del"));
  cropSelected   = new QAction(QString("crop"), this);
  pasteClipboard =
            new QAction(QIcon(":/win-paste.png"),   QString("paste"),    this);
  copyCLR = new QAction(QIcon(":/export-blue.png"), QString("copy"),     this);
  copyBnW = new QAction(QIcon(":/export-mono.png"), QString("copy b/w"), this);
  newWin = new QAction(QIcon(":/windows1.png"),   QString("new window"), this);
  bottom->addAction(pasteClipboard);     pasteClipboard->setEnabled(false); //+
  bottom->addAction(copyCLR); copyCLR->setShortcut(QKeySequence("Ctrl+C"));
  bottom->addAction(copyBnW);
  bottom->addAction(newWin);       newWin->setEnabled(false); //+
  connect(deleteSelected, SIGNAL(triggered()), this, SLOT(DeleteSelected()));
  connect(  cropSelected, SIGNAL(triggered()), this, SLOT(  CropSelected()));
  connect(pasteClipboard, SIGNAL(triggered()), this, SLOT(PasteClipboard()));
  connect(copyCLR, SIGNAL(triggered()), this, SLOT(CopyCLR()));
  connect(copyBnW, SIGNAL(triggered()), this, SLOT(CopyBnW()));
  connect(newWin, SIGNAL(triggered()), this, SLOT(NewWindow()));
  edit_menu->addAction(deleteSelected);
  edit_menu->addAction(  cropSelected); edit_menu->addSeparator();
  edit_menu->addAction(pasteClipboard); edit_menu->addAction(copyCLR);
                                        edit_menu->addAction(copyBnW);
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  colorMenu = menuBar()->addMenu("Color");
#define ADD_colorMenu(ico,text,shortcut) \
  colorMenu->addAction(*ico,text)->setShortcut(QKeySequence(shortcut))
  ADD_colorMenu(enBlancoIco,            "blanco",     "B");
  ADD_colorMenu(enRojoIco,              "rojo (red)", "R");
  ADD_colorMenu(enCastanoIco, Utf8("castaño (brown)"),"C");
  ADD_colorMenu(enVerdeIco,          "verde (green)", "V");
  ADD_colorMenu(enAzulIco,             "azul (blue)", "A");
  colorMenu->addSeparator();
  colorMenu->addAction("random Bicolor")->setShortcut(QKeySequence("Ctrl+B"));
  colorMenu->addAction("random Recolor")->setShortcut(QKeySequence("Ctrl+R"));
  colorMenu->addAction("Un-color all")  ->setShortcut(QKeySequence("Ctrl+U"));
  connect(colorMenu, SIGNAL(triggered(QAction*)),
               this, SLOT(SelectColor(QAction*)));
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  QLabel *magIcon = new QLabel; magIcon->setPixmap(QPixmap(":/zoom3.png"));
  magText = new QLabel(QString("+1"));
  magText->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
  magText->setFont(fixedFont);
  magText->setMinimumSize(QSize(30,25));     bottom->addWidget(magText);
                                             bottom->addWidget(magIcon);
  fitView = new QAction(QIcon(":/full-size.png"), QString("fit"), this);
  connect(fitView, SIGNAL(triggered()), this, SLOT(DoFitView()));
  bottom->addAction(fitView);
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  QMenu *play_menu = menuBar()->addMenu("Play");
  playGen = new QLabel(QString("0"));
  playGen->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
  playGen->setMinimumSize(QSize(66,25));
  prevGen =  new QAction(QIcon(":/step-back.png"),    QString("back"), this);
  nextGen =  new QAction(QIcon(":/go-forward.png") ,  QString("step"), this);
  playStop = new QAction(QIcon(":/fast-forward.png"), QString("go!"),  this);
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
  bottom->addAction(nextGen);
  bottom->addWidget(speedSlider);
  bottom->addAction(playStop);
  addToolBar(Qt::BottomToolBarArea, bottom);
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  if (fileToLoad) LoadTheWorld(fileToLoad);
  else                       SetWinTitle();
  show(); raise();
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void jdlvFrame::LoadTheWorld (QString filename)
{
  if (isChanged) reLoad->setEnabled(false); isChanged = false;
  if (eM & elModePlay) DoPlayStop(); worldFilename = filename;
  genNo = 0;  playGen->setText("0");
  primeWorld->clearTheWorld();
  primeWorld->pasteFile(filename.cStr()); vista->lookAtThis(primeWorld);
  SetWinTitle();                          vista->resize_to_fit();
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void jdlvFrame::SetWinTitle()
{
  QString title = QString("juego de la vida %1").arg(jdlvVERSION);
  QString worldName = worldFilename;
  if (! worldName.isEmpty()) {
    if (worldName.length() > 32) worldName = Utf8("…") + worldName.right(31);
                                       title.append(Utf8(" • ") + worldName);
  }
  setWindowTitle(title);
}
//-----------------------------------------------------------------------------
void jdlvFrame::ToggleTime(bool on) { if (!on) vista->show_time_info(""); }
void jdlvFrame::ShowInfo() {}
void jdlvFrame::SetModeView()
{
  if (eM == elModePlay) DoPlayStop(); eM = elModeView;
}
void jdlvFrame::SetModeEdit()
{
  if (eM == elModePlay) DoPlayStop();
                        DoPrevGen();
  eM = elModeEdit;
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void jdlvFrame::SelectColor (QAction *act) // called from menu action
{                                          //
  if (act->text().startsWith("random")) {
    elRecolorator rc(act->text().indexOf("Bi") > 0 ? "%%%%%%%" : "*******");
    primeWorld->changeTheWorld(rc);
         vista->updateTheWorld  (); notifyOfChange();
  }
  else { char cl = act->text().at(0).unicode();
    switch (cl) {
    case 'b': curColor = elcDefault; cl = 'o'; break; // -- "blanco"
    case 'r': curColor = elcRojo;              break;
    case 'v': curColor = elcVerde;             break;
    case 'a': curColor = elcAzul;              break;
    case 'x': curColor = elcCianico;           break;
    case 'c': curColor = elcCastano;           break;
    case 'z': curColor = elcMagenta;           break;
    case 'U':
      { elRecolorator uc("ooooooo"); // everything -> default color ("blanco")
        primeWorld->changeTheWorld(uc);
             vista->updateTheWorld  (); notifyOfChange(); return; }
    }
    setColor->setIcon (act->icon());
    QRect S = vista->getSelection();
    if (eM == elModeEdit && !S.isEmpty()) {   QString rule(elcMax, QChar(cl));
      elRecolorator rc(rule.cStr(), S.left(), S.right(), S.top(), S.bottom());
      primeWorld->changeTheWorld(rc);
           vista->updateTheWorld  (); notifyOfChange();
} } }
void jdlvFrame::PopupColorMenu() { colorMenu->popup(QCursor::pos()); }
//-----------------------------------------------------------------------------
void jdlvFrame::OpenFile()
{
  QString name = QFileDialog::getOpenFileName(this, "Open", worldFilename);
  if (!name.isEmpty()) { LoadTheWorld(worldFilename = name); SetWinTitle(); }
}
void jdlvFrame::DoReload() { if (!worldFilename.isEmpty())
                                               LoadTheWorld(worldFilename); }
void jdlvFrame::notifyOfChange (void)
{
  if (!isChanged) reLoad->setEnabled(true); isChanged = true;
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void jdlvFrame::DoTermination(bool inside)
{
  QRect S = vista->getSelection();
  if (!S.isEmpty()) {
    elTerminator tm(inside, S.left(), S.right(), S.top(), S.bottom());
    primeWorld->changeTheWorld(tm);
         vista->updateTheWorld  (); notifyOfChange();
} }
void jdlvFrame::DeleteSelected() { DoTermination(true);  }
void jdlvFrame::CropSelected  () { DoTermination(false); }
//-----------------------------------------------------------------------------
class ClipSaver : public elSalvador
{
  static QClipboard  *theClipboard;
  static QClipboard::Mode clipMode; QString clipContents;
public:
  ClipSaver (elMundo& mundoParaSalvar) : elSalvador(mundoParaSalvar)
  {
    if (!theClipboard) { theClipboard = QApplication::clipboard();
      clipMode = theClipboard->supportsSelection() ? QClipboard::Selection
                                                   : QClipboard::Clipboard;
  } }
  virtual void flush (QString line) { clipContents += line + "\n"; }
  virtual ~ClipSaver() {
    if (!clipContents.isEmpty()) theClipboard->setText(clipContents, clipMode);
  }
};
QClipboard *ClipSaver::theClipboard = NULL;
QClipboard::Mode ClipSaver::clipMode;
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void jdlvFrame::PasteClipboard() { }
void jdlvFrame::CopyBnW()
{
  elMundo *world = (genNo && nextWorld) ? nextWorld : primeWorld;
  ClipSaver saver(*world);                     saver.save(false);
}
void jdlvFrame::CopyCLR()
{
  elMundo *world = (genNo && nextWorld) ? nextWorld : primeWorld;
  ClipSaver saver(*world);                      saver.save(true);
}
void jdlvFrame::NewWindow() { }
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void jdlvFrame::DoFitView() { vista->resize_to_fit(); }
void jdlvFrame::UpdateMag() { QString S; S.sprintf("%+3d",vista->get_mag());
                                                        magText->setText(S); }
//-----------------------------------------------------------------------------
void jdlvFrame::PreparePlay()
{
  if (genNo && nextWorld) return;
  else {
    if (eM == elModeEdit) modeView->setChecked(true); eM = elModeView;
    if (nextWorld) nextWorld->clearTheWorld();
    else           nextWorld = new_elMundoA(); vista->lookAtThis(nextWorld);
    prevGen->setEnabled(true);
} }
void jdlvFrame::DoPrevGen() // until full undo is implemented, "prev gen" means
{                           // reverting back to the (untouched) primeWorld
  if (genNo && nextWorld) {
    if (eM & elModePlay) DoPlayStop();         genNo = 0;
    vista->lookAtThis(primeWorld); playGen->setText("0");
    vista->updateTheWorld();       prevGen->setEnabled(false);
} }
void jdlvFrame::DoNextGen() { PreparePlay(); timerEvent(NULL); }
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
int speed2msec[] = { -1, 600, 150, 30, 0 };
quint64 lastTick;
void jdlvFrame::DoPlayStop()
{
  if (eM == elModePlay) {
      eM =  elModeView;         killTimer(timerID);
    playStop->setIcon(QIcon(":/fast-forward.png")); playStop->setText("go!");
  }
  else {  PreparePlay(); eM = elModePlay;
    lastTick = pgtime();
    timerID = startTimer(speed2msec[speed]);
    playStop->setIcon (QIcon(":/step.png")); playStop->setText("stop");
} }
void jdlvFrame::ChangeSpeed (int newSpeed)
{
  speed = newSpeed; if (eM & elModePlay) {  killTimer(timerID);
                       timerID = startTimer(speed2msec[speed]); }
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void jdlvFrame::timerEvent(QTimerEvent *)
{
  elMundo *origWorld = genNo ? nextWorld : primeWorld;
  quint64 tick_time = pgtime(); vista->show_next_gen(origWorld); genNo++;
  quint64 stop_time = pgtime();
#ifdef Q_OS_LINUX
  QApplication::syncX(); // only for X11 (does nothing on other platforms)
#endif
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
