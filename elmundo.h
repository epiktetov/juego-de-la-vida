/*
 * @(#)elmundo.h -- el Mundo (the World) header -- $Revision: $
 *
 * Viewer/editor/etc for Conway's Game of Life (in full color)
 * Copyright 2002 by Michael Epiktetov, epi@ieee.org
 *
 *-----------------------------------------------------------------------------
 *
 * $Log: $
 *
 */
#ifndef _EL_MUNDO_H_
#define _EL_MUNDO_H_

class elMundo;      // the world
class elObservador; // the observer (abstract class for iterations thru world)
class elVista;      // the view

class elMundo //---------------------------------------------------------------
{
public:  elMundo();
        ~elMundo();
  //
  // 'add' method should be optimized for case when cells added in order from
  // left to right and from top to bottom:
  //
  virtual void add   (int x, int y, int color) = 0;
  virtual void clear (int x, int y)            = 0;
  virtual void toggle(int x, int y, int color) = 0;

  virtual void setRules     (const char *ruleString) = 0;
  virtual void setColorRules(const char *ruleString) = 0;

  void paste      (const char *pattern, int *width = 0, int *height = 0);
  void pasteString(const char *pattern, int *width = 0, int *height = 0);

  int XvMin, XvMax, // position of the view frame (absolute coordinates)
      YvMin, YvMax; //
  virtual void setFrame(int xmin, int xmax, int ymin, int ymax);
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

                                                   //          0 all dead
  virtual int nextGeneration(elVista *ev = 0) = 0; // returns: 1 stable
};                                                 //          2 otherwise

elMundo *new_elMundoA(); // el Mundo type A (array) -- see elmundo-a.cpp

class elObservador //----------------------------------------------------------
{
public:
  virtual void observe(int x_abs, int y_abs, int clr) = 0;
};

#endif /* _EL_MUNDO_H_ */
