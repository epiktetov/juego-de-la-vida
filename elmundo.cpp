//------------------------------------------------------+----------------------
// juego de la vida      el Mundo (the World) code      | (c) Epi MG, 2002,2011
//------------------------------------------------------+----------------------
#include <QFile>           // elMundo (the World) keeps the world of Life cells
#include <QTextStream>
#include "jdlv.h"
#include "elmundo.h"
#include "elvista.h" // (need to know that elVista is actually an elObservador)
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
elMatriz Id( 1, 0, 0, 1), // identical transformation (i.e. no transformation)
         Ro( 0, 1,-1, 0), // rotate clockwise
         Rc( 0,-1, 1, 0), // rotate counter-clockwise
         Rx(-1, 0, 0,-1), // rotate twice (180°)
         Fx(-1, 0, 0, 1), // flip (mirror) by X axis
         Fy( 1, 0, 0,-1); // flip (mirror) by Y axis
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
  QTextStream in(&file);                       setRules(0x8,0xC);
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
void elMundo::pasteString (QString contents, int atX,
                                             int atY, elMatriz& M, int o_color)
{
  pasteStart(atX, atY); pasteRLEX_add(contents, M, o_color);
}
void elMundo::pasteStart(int atX, int atY) { cX0 = cX = atX; cY0 = cY = atY; }
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool elMundo::pasteRLEX_add (QString line, elMatriz& M, int o_color)
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
      case 'b':  cX += M.a * cN; cY += M.b * cN; cN = 0; continue;
      case '$':  cX += M.c * cN; cY += M.d * cN; cN = 0;
        if (M.a) cX = cX0;
        else     cY = cY0; continue;
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
      default:  clr = o_color;
      }
      while (cN > 0) { add(cX, cY, clr); cX += M.a; cY += M.b; cN--; }
  } }                                  //                       ^
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
struct gunConfig {
  int x1, y1, x2, y2, x3, y3, x4, y4;
  const char *rle;             int p;
}
gunCfg[] =
{
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - 0 - p0030.lif - -
  { -80,-66, 66,-80, 80,66, -66,80,
    "27bo$26b4o$9boo14boobobo$bo7bobbo11b3obobbobboo$o3boo7bo11boobobo3boo$"
    "o5bo6bo12b4o$b5o7bo7bo5bo$9bobbo9bo$9boo9b3o!", 30 },
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - 1 - p0046.lif - -
  { -80,-64, 64,-80, 80,64, -64,80,
    "40bo$$34b4o5b6o$34boboo7b4o$34bo7boboo$oobboo4bobboo20boo5boo$ooboo6b"
    "oobo$4bobo6bo21boo5boo$5boo4b3o20bo7boboo$34boboo7b4o$5boo4b3o20b4o5b"
    "6o$4bobo6bo$ooboo6boobo$oobboo4bobboo16bo$32bo$30b3o!", 46 },
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - 2 - p0022.lif - -
  { -80,-70, 70,-80, 80,70, -70,80,
    "44bo$$18boo13boo$19bo6b3o3bobbo$19bobo5bo3bo$20boo4bo4bobboboo$27bobbo"
    "bob3o$26b3obobobbo$25boobobbo4bo4boo$31bo3bo5bobo$27bobbo3b3o6bo$23bo"
    "4boo13boo$24bo$oo7bo5boo5b3o$bo6b3o3b3o$bobo6bobboobboo$bboo4boobb5ob"
    "oo$10b3obobobbo9bo$7bobbobob3o10bobo$7boob5obboo4boo3boo$8boobboobbo6b"
    "obo$10b3o3b3o6bo$10boo5bo7boo7bo$35bo$o32b3o!", 22 },
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - 3 - p0023.lif - -
  { -90,-76, 76,-90, 90,76, -76,90,
    "39bo$boo27b3o$boo18boo5bo4bo11boo$28bo5bo10boo$13boo18bo$oo9boobbo15b"
    "oo$oo9b6o$11b4o16boo$24bo8bo$25bobbo5bo$23b3obbo4bo31boo$11b4o15b3o32b"
    "oo$oo9b6o$oo9boobbo$13boo26boo8b3o12boo$41boo8bo3bo10boo$boo48bo4bo$b"
    "oo49bo3bo$$52bo3bo$36bo14bo4bo$34bobo14bo3bo10boo$24bo10boo14b3o12boo$"
    "7boo15boo$7boo16boo$20boobboo39boo$40bobo22boo$41boo$41bo$20boobboo$7b"
    "oo16boo7boo$7boo15boo8boo11bo$24bo23bo$46b3o!", 23 },
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - 4 - p0024.lif - -
  { -80,-80, 80,-80, 80,80, -80,80,
    "23bobbo$21b6o$17boobo8bo$13boobobobob8obbo$11b3oboo3bobo7b3o$10bo4b3o"
    "bbo3bo3boo$11b3o3boob4obo3boboo$12bobo3bo5bo4bobbo4boobo$10bo8boboobb"
    "oobboo5boboobo$10b5ob4obo4b3o7bo4bo$15boo4bo4bob3obboobobooboo$12b5ob"
    "3o4booboo3bobobobobo$11bo5boo4booboboo5bo5bo$12b5o6boobo3bo3bobobooboo"
    "$ooboo9boobbo5bobo4bobb3obobo$boboboboboo3b3obo6bobbobo4b3obbo$obbo7bo"
    "6boo3b3o8boboboo$3obbo4boo11bo10bo$5b4obo17boo4boo$bboobo6bo14bo3bobb"
    "oo$bo4bo3bo16bo6boo$b3obo4bo16bo3bobbo$11bobbo3bo9boo4boboboo$b3obo4bo"
    "8boo3bo10b3obbo$bo4bo3bo7bo6bo8b3obobo$bboobo6bo10b3o8bobobooboo$5b4ob"
    "o24bo5bo$3obbo4boo21bobobobobo$obbo7bo9boo10boobobooboo$boboboboboo10b"
    "o8bo5bo4bo$ooboo17b3o6bo4boboobo$24bo4b3o5boobo!", 24 },
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - 5 - p0044.lif - -
  { -90,-76, 76,-90, 90,76, -76,90,
    "14bo$bboobb3obboboo5bo$bboobbooboboboobobobo3boo$4b3o6bo4bobboobbo$4bo"
    "17boo$4bo17b3o$$14b3o$9boo$9boo3bobo$15bo$29boo$29bo$bo11bo3bo9bobo$5b"
    "o6bo5bo8boo$o5bo$ooboobo6bo3bo13boo7bo$5bo8b3o14boo5b3o$37bo$37boo$$"
    "26boo$5bo8b3o8b4o3boo$ooboobo6bo3bo6bo3boo$o5bo17boobbo$5bo6bo5bo6b3o"
    "12bo$bo11bo3bo8bo11bobo$39boo$$15bo$9boo3bobo$9boo$14b3o13boo$30bo$4bo"
    "17b3o6b3o$4bo17boo9bo$4b3o6bo4bobboobbo$bboobbooboboboobobobo3boo$bboo"
    "bb3obboboo5bo$14bo!", 44 },
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - 6 - p0036.lif - -
  { -93,-96, 96,-93, 93,96, -96,93,
    "25bo$25b3o$28bo$27boo$$30bo$29b3o$28bobboo$28bo3bo$30bobo$39boo$27boob"
    "oo6bobbo$29bo6bobbobo$36bo3bo$36bo$28b3o8bo$29boo6boo$29boo4$35boo$27b"
    "oo6boo$26bo8b3o$29bo$25bo3bo$24bobobbo6bo25boo$24bobbo6booboo22bobbo$"
    "25boo35bobo$33bobo22boo3bo$33bo3bo6b4o3bobo5boboo$33boobbo5boobbobbo3b"
    "o6bo$11boo21b3o5boobbo4bo9bo$10bobbo21bo7bobbo6boo$10bobobbo28boo$11bo"
    "4bo48boo$16bo6boo3bo27boo6bobbo$12b3o6b3o4boo19bo9bo4bobboo$20bobbo5b"
    "oo6bo11bo6bo3bobbobboo$20bo7boo8bo9boobo5bobo3b4o4boo$20boo14b3o8bo3b"
    "oo18bobo$17boo27bobo24bo$9boo7bo27bobbo23boo$8boo5bobbo28boo$9boo4b3o"
    "6b3o$bboo6bo3boo6bo$bobo18bo4bo$bo21bobbobo17bo$oo23bobbo18bo$26boo17b"
    "3o!", 36 },
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - 7 - p0060.lif - -
  { -80,-64, 64,-80, 80,64, -64,80,
    "27bo$27b4o$11bo16b4o$10bobo5boo8bobbo5boo$3boo3boo3bo14b4o5boo$3boo3b"
    "oo3bo4boboboo3b4o$8boo3bo5boo3bobbo$10bobo10bo$11bo8bobbo$$26bobo$28bo"
    "$24bo$26bo$25bo$$11boo$11boo4bo$oo6boo6b5oboo$oo5b3o5bobboo4bo$8boo5b"
    "oo8bo12bo$11boo4bo7bo10bobo$11boo12bo11boo$24bo8boo$22boo9bobo$35bo$"
    "35boo!", 60 }
};
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void elMundo::pasteGun1 (int atX, int atY, char Aj,
                                   const char *rle, elMatriz& M, int o_color)
{ int N;
       if ('A' <= Aj && Aj <= 'U') N = Aj - 'A';
  else if ('V' <= Aj && Aj <= 'Z') N = Aj - 'B';
  else if ('a' <= Aj && Aj <= 'u') N = Aj - 'a';
  else if ('v' <= Aj && Aj <= 'z') N = Aj - 'b';
  else      fprintf(stderr, "bad Aj(%c)\n", Aj);
  atX += (N % 5) - 2;
  atY += (N / 5) - 2; pasteStart          (atX, atY);
                      pasteRLEX_add(rle, M, o_color);
};
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void elMundo::make4guns(const char *params)
{
  char c, sign[10] = "....:...."; srand(time(NULL));
  int N = 0, i;
  if (!params || strlen(params) < 9) params = "....:....";
  for (i = 0; i < 4; i++) {
    if (('A' <= (c = params[i]) && c <= 'Z') ||
        ('a' <=  c              && c <= 'z')) sign[i] = c;
    else {
      N = rand() % 52; sign[i] = (N < 26) ? (N+'A') : (N-26+'a');
  } }
  for (i = 0; i < 4; i++) {
    if (('0' <= (c = params[i+5]) && c <= '7')) sign[i+5] = c;
    else {
      N = rand() % 8; sign[i+5] = N+'0';
  } }
#define PASTE_1GUN(A,Z,k,x,y,M,color) \
  if (A <= (c = sign[k]) && c <= Z) { \
    i = sign[k+5] - '0';              \
    pasteGun1(gunCfg[i].x, gunCfg[i].y, c, gunCfg[i].rle, M, color); \
  }
  PASTE_1GUN('A','Z',0,x1,y1,Id,elcRed);
  PASTE_1GUN('A','Z',1,x2,y2,Ro,elcYellow);
  PASTE_1GUN('A','Z',2,x3,y3,Rx,elcGreen);
  PASTE_1GUN('A','Z',3,x4,y4,Rc,elcBlue);
  nextGeneration();
  PASTE_1GUN('a','z',0,x1,y1,Id,elcRed);
  PASTE_1GUN('a','z',1,x2,y2,Ro,elcYellow);
  PASTE_1GUN('a','z',2,x3,y3,Rx,elcGreen);
  PASTE_1GUN('a','z',3,x4,y4,Rc,elcBlue);
  fprintf(stderr, "4guns(%s,%d:%d:%d:%d)\n", sign, gunCfg[sign[5]-'0'].p,
                                                   gunCfg[sign[6]-'0'].p,
                                                   gunCfg[sign[7]-'0'].p,
                                                   gunCfg[sign[8]-'0'].p);
}
//-----------------------------------------------------------------------------
