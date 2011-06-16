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

//void pasteFile  (const char *pattern, int *width = 0, int *height = 0);
//void pasteString(const char *pattern, int *width = 0, int *height = 0);

  void pasteFile  (QString filename, int atX = 0, int atY = 0);
  void pasteString(QString contents, int atX = 0, int atY = 0,
                                elMatriz& M = Id, int o_color = elcDefault);
protected:
  void pasteStart(int atX = 0, int atY = 0);              int cX0, cX, cY0, cY;
  bool pasteRLEX_add(QString line, elMatriz& M = Id, int o_color = elcDefault);
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // 'add' method should be optimized for case when cells added in order from
  //     left to right and from top to bottom (since that's how it'll be used)
public:
  virtual void add   (int x, int y, int color) = 0;
  virtual void toggle(int x, int y, int color) = 0;
  virtual void clear (int x, int y)            = 0;
  virtual void clearTheWorld (void)            = 0;
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  int XvMin, XvMax, // position of the view frame (absolute coordinates)
      YvMin, YvMax; //
  virtual void setFrame   (int  xmin, int  xmax, int  ymin, int  ymax);
  virtual void getFitFrame(int &xmin, int &xmax, int &ymin, int &ymax) = 0;
  virtual void show(elVista &ev);
  virtual void show(elVista &ev, int xmin, int xmax, int ymin, int ymax);
  //
  // Although 'show' might be implemented using universal iterator described
  // below, method is virtual to allow optimization (it may be faster to show
  // cells in different order)
  //
  virtual void iterate(elObservador &eo)                = 0;
  virtual void iterate(elObservador &eo, int colorMask) = 0;
  virtual void iterate(elObservador &eo, int xmin, int xmax,
                                         int ymin, int ymax) = 0;
  // nextGen returns: 0 all dead
  //                  1 stable, 2 otherwise
  //
  virtual int nextGeneration(elVista *ev = 0) = 0;
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
};
/*---------------------------------------------------------------------------*/
#endif                                                /* EL_MUNDO_H_INCLUDED */
