//------------------------------------------------------+----------------------
// juego de la vida     el Mundo (the World) header     | (c) Epi MG, 2002,2011
//------------------------------------------------------+----------------------
#ifndef EL_MUNDO_H_INCLUDED
#define EL_MUNDO_H_INCLUDED
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class elMatriz              // Rotation/mirror matrix, specifies transformation
{                           // as:  new_x = a*x + b*y
public:           int a, b; //      new_y = c*x + d*y
                  int c, d; //
  elMatriz(void)                       : a(1), b(0), c(0), d(1) { }
  elMatriz(int A, int B, int C, int D) : a(A), b(B), c(C), d(D) { }
};
extern elMatriz Id, Ro, Rc, Rx, Fx, Fy;
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class elMundo
{
public:    elMundo() { }    static void initRegExs(void);
  virtual ~elMundo() { }
  virtual void setColorRules(const char *ruleString) = 0;
  virtual void setRules     (QString     ruleString);
  virtual void setRules(int born_bit_mask, int stay_bit_mask) = 0;

  void pasteFile  (QString filename, int atX = 0, int atY = 0);
  void pasteString(QString contents, int atX = 0, int atY = 0,
                                elMatriz& M = Id, int o_color = elcDefault);
protected:
  void pasteStart(int atX = 0, int atY = 0); int cX0, cX, cY0, cY;
  void pasteLIFE_add(QString line);
  bool pasteRLEX_add(QString line, elMatriz& M = Id, int o_color = elcDefault);
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // 'add' method should be optimized for case when cells added in order from
  //     left to right and from top to bottom (since that's how it'll be used)
public:
  virtual void add    (int x, int y, int color) = 0;
  virtual void toggle (int x, int y, int color) = 0;
  virtual bool recolor(int x, int y, int color) = 0; // returns false if cannot
  virtual void clear  (int x, int y)            = 0; //   perform the operation
  virtual void clearTheWorld  (void)            = 0;
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  int XvMin, XvMax, // position of the view frame (absolute coordinates)
      YvMin, YvMax; //
  virtual void setFrame   (int  xmin, int  xmax, int  ymin, int  ymax);
  virtual void getFitFrame(int &xmin, int &xmax, int &ymin, int &ymax) = 0;
  virtual void show(elVista &ev);
  virtual void show(elVista &ev, int xmin, int xmax, int ymin, int ymax);
  //        ^
  // Although 'show' might be implemented using universal iterator described
  // below, method is virtual to allow optimization (it may be faster to show
  // cells in different order - that may not work for other observers, though)
  //
  virtual void iterate(elObservador &eo)                = 0;
  virtual void iterate(elObservador &eo, int colorMask) = 0;
  virtual void iterate(elObservador &eo, int xmin, int xmax,
                                         int ymin, int ymax) = 0;
  // Iterator to modify the world:
  //
  virtual void changeTheWorld(elObservador &eo)                       = 0;
  virtual int nextGeneration(elVista *ev = 0, elMundo *fromWorld = 0) = 0;
  //      ^
  // returns: 0 all dead            | not all implementations may properly
  //          1 stable, 2 otherwise | detect "stable" condition (return 2)
//
// just for fun (TODO: move to Lua script)
//
  void make4guns(const char *params);
  void pasteGun1(int x, int y, char Aj, const char *rle, elMatriz& M, int oc);
};
elMundo *new_elMundoA(); // el Mundo type A (array) -- see elmundo-a.cpp
//-----------------------------------------------------------------------------
class elObservador
{
public:
  virtual void observe(int x_abs, int y_abs, int clr) = 0;
  virtual int  recolor(int x_abs, int y_abs, int clr)
  {
    observe(x_abs, y_abs, clr); return clr; // to kill the cell, return elcDead
} };
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class elSalvador : public elObservador
{
  int X0, pX, pY, pC, pN; bool withColors;
  elMundo &world;             QString rle;
public:
  elSalvador(elMundo& mundoParaSalvar) : world(mundoParaSalvar) { }
  void save(bool conColores);
  virtual void flush (QString line);
  virtual void observe(int x, int y, int clr);
};
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class elRecolorator : public elObservador // recolors the observed word by the
{                                         // rule:
  int cR[elcMax];                         //       newColor = rcRules[oldColor]
  int xmin, xmax, ymin, ymax;
public:
  elRecolorator(const char *recolorRules, int x0=0,int x1=0,int y0=0,int y1=0);
  virtual void observe(int,   int,   int) { }
  virtual int  recolor(int x, int y, int clr);
};
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class elTerminator : public elObservador
{
  bool clearInside; int xmin, xmax, ymin, ymax;
public:
  elTerminator(bool claroInterior,   int x0,   int x1,   int y0,   int y1)
      : clearInside(claroInterior), xmin(x0), xmax(x1), ymin(y0), ymax(y1) { }
  virtual void observe(int,   int,   int) { }
  virtual int  recolor(int x, int y, int clr);
};
/*---------------------------------------------------------------------------*/
#endif                                                /* EL_MUNDO_H_INCLUDED */
