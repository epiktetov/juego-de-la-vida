//------------------------------------------------------+----------------------
// juego de la vida      Самый главный header файл      | (c) Epi MG, 2002,2011
//------------------------------------------------------+----------------------
#ifndef JDLV_H_INCLUDED // Viewer/editor/etc for Conway's Game of Life (+color)
#define JDLV_H_INCLUDED // by Michael Epiktetov (М.Г.Эпиктетов), epi@ieee.org
#include <stdlib.h>
#include <QMainWindow>
class QLabel;
class QSlider;
class elMundo;      // the world
class elObservador; // the observer (abstract class for iterations thru world)
class elVista;      // the view (double-inherited from QWidget & elObservador)
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
typedef enum {
  elModeView = 1,
  elModeEdit = 2,
  elModePlay = 4
}
elMode;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
typedef enum { elcBlack   = 0, elcNegro   = 0, elcDefault   =  0,
               elcRed     = 1, elcRojo    = 1,
               elcGreen   = 2, elcVerde   = 2, elcRandomRed = 40,
               elcBlue    = 4, elcAzul    = 4, elcRandomAny = 42,
               elcYellow  = 3, elcCastano = 3,
               elcMagenta = 5,
               elcCyan    = 6, elcCianico = 6, elcMax = 7, elcDead = 8
} elColor;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
typedef enum {
  elvcBlack   = 0, elvcOffBlack   =  8, elcvDead = 8,
  elvcRed     = 1, elvcOffRed     =  9,
  elvcGreen   = 2, elvcOffGreen   = 10, elvcBackground = 16,
  elvcYellow  = 3, elvcOffYellow  = 11, elvcGrid       = 17,
  elvcBlue    = 4, elvcOffBlue    = 12, elvcInfoText   = 18,
  elvcMagenta = 5, elvcOffMagenta = 13,
  elvcCyan    = 6, elvcOffCyan    = 14,
  elvcWhite   = 7, elvcOffWhite   = 15, elvcMax = 19
}
elVisColor;
/*---------------------------------------------------------------------------*/
class jdlvFrame : public QMainWindow // QDialog
{
  Q_OBJECT                 elMundo *primeWorld, *nextWorld;
  QString   worldFilename; elVista *vista;  bool isChanged;
  int eM, genNo, curColor;
  int speed, timerID;

  QAction *openFile, *reLoad;
  QAction *showTimeCB, *showInfoPB, *modeView, *modeEdit, *setColor,
                                    *pasteClip, *saveBnW, *saveCLR;
  QSlider *speedSlider;
  QMenu *colorMenu;
  QLabel *magText, *playGen;
  QAction *newWin;
  QAction  *fitView,  *prevGen, *nextGen, *playStop;

public slots:
  void OpenFile();
  void DoReload(); void ToggleTime(bool on);
  void ShowInfo(); void PopupColorMenu(); void SelectColor(QAction *choice);
  void PasteClip();
  void SaveBnW();
  void SaveCLR();
  void NewWindow();

  void SetModeView(); void DoFitView();  void ChangeSpeed(int newSpeed);
  void SetModeEdit(); void DoPrevGen();
                      void DoNextGen();
                      void DoPlayStop(); void timerEvent(QTimerEvent *ev);
  void UpdateMag();

public:    jdlvFrame(const char *fileToLoad);
  virtual ~jdlvFrame() { }
  void LoadTheWorld(QString filename);
protected:
  void SetWinTitle();
  void PreparePlay();
public:
  bool changesAllowed() const { return (eM == elModeEdit && genNo == 0); }
  void notifyOfChange(void);
  int      getCurrentColor(void) const { return   curColor; }
  elVista *getCurrentVista(void) const { return      vista; }
  elMundo *getPrimeWorld  (void) const { return primeWorld; }
};
#ifdef Q_OS_MAC
class MacEvents : public QObject { Q_OBJECT
                  protected: bool eventFilter(QObject *obj, QEvent *event); };
#endif
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
#define cStr()  toLocal8Bit().data() /* to pass QString value to legacy code */
#define uStr()       toUtf8().data() /* the same, but convert value to UTF-8 */
#define Utf8(x) QString::fromUtf8(x) /* just a shorthand to save some space  */

#define MALLOCxN(_type, _N) (_type *)malloc(_N * sizeof(_type))
#define TableSIZE(_X)           (int)(sizeof(_X)/sizeof(*(_X)))
#define pgt_1SEC 1000
quint64 pgtime(void);   /* returns current time with "pretty good" precision */
/*---------------------------------------------------------------------------*/
#endif                                                    /* JDLV_H_INCLUDED */
