//------------------------------------------------------+----------------------
// juego de la vida      el Mundo (the World) code      | (c) Epi MG, 2002,2011
//------------------------------------------------------+----------------------
#include <QFile>           // elMundo (the World) keeps the world of Life cells
#include <QTextStream>
#include "jdlv.h"
#include "elmundo.h"
#include "elvista.h" // (need to know that elVista is actually an elObservador)
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static QRegExp reRLEstart, reBrule("B(\\d+)", Qt::CaseInsensitive),
                           reSrule("S(\\d+)", Qt::CaseInsensitive);

void elMundo::initRegExs(void)
{
  QString RLEst = Utf8("x…=…(\\d+),…y…=…(\\d+)(?:,…rule…=…([bs/0-9]+))?");
  RLEst.replace(Utf8("…"), "\\s*");
  reRLEstart.setCaseSensitivity(Qt::CaseInsensitive);
  reRLEstart.setPattern(RLEst);
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void elMundo::setRules (QString ruleString)
{
  QString::const_iterator it; int B_bitmask = 0, S_bitmask = 0;
  QString digits;
  if (reBrule.indexIn(ruleString) >= 0) {
    digits = reBrule.cap(1);
    for (it = digits.constBegin(); it != digits.constEnd(); it++)
      B_bitmask |= (1 << it->digitValue());
  }
  if (reSrule.indexIn(ruleString) >= 0) {
    digits = reSrule.cap(1);
    for (it = digits.constBegin(); it != digits.constEnd(); it++)
      S_bitmask |= (1 << it->digitValue());
  }
  fprintf(stderr,"Rules(%s=+%x:%x)\n", ruleString.cStr(), B_bitmask,S_bitmask);
  setRules(B_bitmask, S_bitmask);
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void elMundo::pasteFile (QString filename, int atX, int atY)
{
  QFile file(filename);
  if (! file.open(QIODevice::ReadOnly | QIODevice::Text)) return;
  QTextStream in(&file);                    bool started = false;
  while (! in.atEnd()) {
    QString line = in.readLine();
    if (line.startsWith('#')) continue; // TODO: keep comments somewhere
    if (reRLEstart.exactMatch(line)) {
      if (!reRLEstart.cap(3).isEmpty()) setRules(reRLEstart.cap(3).cStr());
      pasteStart(atX, atY);
      while (!in.atEnd())
        if (pasteRLEX_add(in.readLine())) break; break;
    }
    // TODO: other types
    else
      fprintf(stderr, "?:%s\n", line.cStr());
  }
  file.close();
}
void elMundo::pasteString (QString contents, int atX, int atY)
{
  pasteStart(atX, atY); pasteRLEX_add(contents);
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void elMundo::pasteStart(int atX, int atY) { cX0 = cX = atX; cY = atY; }
bool elMundo::pasteRLEX_add (QString line)
{
  int clr, cN = 0;
  for (QString::const_iterator it  = line.constBegin();
                               it != line.constEnd(); it++) {
    QChar c = *it;
         if (c.isDigit()) cN = 10*cN + c.digitValue();
    else if (c == '!')                    return true;
    else if (c == ' ' || c == '\t'
                      || c == '\n') continue;
    else {
      if (cN == 0) cN = 1;
      switch (c.unicode()) {
      case 'b':           cX += cN; cN = 0; continue;
      case '$': cX = cX0; cY += cN; cN = 0; continue;
      //
      // Because 'b' is already taken, we have to use Spanish name for Blue...
      // so adding a few other names for consistency:
      //
      case 'k': clr = elcBlack;   break;  case 'n': clr = elcNegro;   break;
      case 'g': clr = elcGreen;   break;  case 'v': clr = elcVerde;   break;
                                          case 'r': clr = elcRojo;    break;
                                          case 'a': clr = elcAzul;    break;
      case 'x': clr = elcCyan;    break;  case 'c': clr = elcCianico; break;
      case 'y': clr = elcYellow;  break;
      case 'z': clr = elcMagenta; break;
      case 'o':
      default:  clr = elcDefault; break;
      }
      while (cN > 0) { add((cX++), cY, clr); cN--; }
  } }                                  //     ^
  return false;                        // don't replace that with "while(cN--)"
}                                      // as "cN = 0" is required loop invarint
//-----------------------------------------------------------------------------
void elMundo::setFrame(int xmin, int xmax, int ymin, int ymax)
{
  XvMin = xmin; YvMin = ymin; 
  XvMax = xmax; YvMax = ymax;
}
void elMundo::show(elVista &ev) { iterate(ev); }
void elMundo::show(elVista &ev,
                   int xmin, int xmax, int ymin, int ymax)
{
  iterate(ev, xmin, xmax, ymin, ymax);
}
//-----------------------------------------------------------------------------
