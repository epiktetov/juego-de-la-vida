//------------------------------------------------------+----------------------
// juego de la vida   el Mundo (the World type A) code  | (c) Epi MG, 2002,2011
//------------------------------------------------------+----------------------
#include <stdio.h>     // elMundo-A (the World type A) implemented with Arrays
#include <string.h>    // (the code was converted from epiLife.java applet 2.3
#include "jdlv.h"      //  copyright 1999,2002 by Michael Epiktetov == Epi MG)
#include "elmundo.h"
#include "elvista.h"

class elMundoA : public elMundo
{
  int Usize,  /* Current size of allocated memory                          */
      Ulen,   /* length of used memory (number of cell pairs)              */
              /*                  0yyy yyyy yyyy yyyy 0xxx xxxx xxxxx      */
     *Upos;   /* packed Position  .... .... .... .... .xxx xxxx xxxxx - X  */
              /* (of left cell):  .yyy yyyy yyyy yyyy .... .... ..... - Y  */
              /*                                                           */
  static const int maxpos = 0x7fffffff; /* Cell pairs in adjacent rows are */
  static const int  xmask =     0x7fff; /* shifted to form "brick wall"    */
  static const int  ymask = 0x7fff0000; /* pattern (see diagram below);    */
  static const int yshift = 16;         /* it results in smaller number of */
  static const int xone   =          1; /* neigbors for each pair (only 6) */
  static const int yone   =    0x10000; /*                                 */
  static const int zerox  =     0x4000; /* - Put (0,0) into center of the  */
  static const int zeroy  =     0x4000; /*   universe                      */

  static inline int pack(int x, int y)
  {
    return ((x + zerox) & xmask) + ((y + zeroy) << yshift);
  }
  static inline int X(int pos) { return  (pos & xmask)            - zerox; }
  static inline int Y(int pos) { return ((pos & ymask) >> yshift) - zeroy; }

  int *Uval; /*             Value: XXXR XGXB xxxr xgxb RGBr gbnn nnmm mmLR */
/*                                                                         */
/* Data for RIGHT cell in pair:    .... .... xxxr xgxb ...r gb.. ..mm mm.R */
/*   "cell's alive" flag           .... .... .... .... .... .... .... ...R */
/*   number of alive neighbors     .... .... .... .... .... .... ..mm mm.. */
/*   color genes                   .... .... .... .... ...r gb.. .... .... */
/*   sum of color genes (*)        .... .... xxxr xgxb .... .... .... .... */
/*                                                                         */
/* Data for LEFT cell in pair:     XXXR XGXB .... .... RGB. ..nn nn.. ..L. */
/*   "cell's alive" flag           .... .... .... .... .... .... .... ..L. */
/*   number of alive neighbors     .... .... .... .... .... ..nn nn.. .... */
/*   color genes                   .... .... .... .... RGB. .... .... .... */
/*   sum of color genes (*)        XXXR XGXB .... .... .... .... .... .... */
/*                                                                         */
/* (*) Because of overflow, we can't tell 4 blue neighbors from 1 green;   */
/*     but we don't have to - for new cell to be born, the position should */
/*     have exactly 3 "parents", which rules out the case of 4 blue cells. */
/*                                                                         */
/* Of course, such trick will not work in other variants of Life that have */
/* different born rules (for example, HighLife b36s23, LongLife b345s5, or */
/* Diamoeba b35678s5678); however, the algorithm of color inheritance must */
/* be different anyway.                                                    */

  int finalized; /* indicates whether the values are finalized (that is,   */
                 /* whether neighborhood information is up to date or not) */

  static const int cell_alive_mask = 0x3,
                   left_alive_mask = 0x2, right_alive_mask = 0x1;

  static const int rule_mask = 0x003ff; /* used to undex rules[] */
  int rules[rule_mask+1];
  static const int no_action            = 0; /* values selected to allow */
  static const int right_born           = 1; /* independent handling of  */
  static const int right_dies           = 2; /* left/right cell destiny: */
  static const int left_born            = 3; /*                          */
  static const int both_born            = 4; /* left_born = 10(3)        */
  static const int left_born_right_dies = 5; /* left_dies = 20(3)        */
  static const int left_dies            = 6; /*                          */
  static const int left_dies_right_born = 7; /* right_born = 01(3)       */
  static const int both_die             = 8; /* right_dies = 02(3)       */

public:
  void setRules(int born_bit_mask, int stay_bit_mask)
  {
    int i,k;
    for (i = 0; i <= 0x3fc; i += 4) { 
      rules[i+0] = no_action;
      rules[i+1] = right_dies; /* if not specifed explicitly, no actions */
      rules[i+2] = left_dies;  /* required for empty cell and all living */
      rules[i+3] = both_die;   /* cells are going to die (will clear the */
    }                          /* "death" bit for specified rules later) */
    for (k = 1; k < 9; k++) {
      if ((born_bit_mask & (1 << k)) == 0) continue;
      for (i = 0; i < 9; i++) {
        rules[(k<<6)+(i<<2)+0] += left_born;   /* cell will be born is empty */
        rules[(k<<6)+(i<<2)+1] += left_born;   /* position has exactly given */
        rules[(i<<6)+(k<<2)+0] += right_born;  /* number of alive neighbors  */
        rules[(i<<6)+(k<<2)+2] += right_born;  /* (anything else doesn't     */
    } }                                        /*                    matter) */
    for (k = 1; k < 9; k++) {
      if ((stay_bit_mask & (1 << k)) == 0) continue;
      for (i = 0; i < 9; i++) {
        rules[(k<<6)+(i<<2)+2] -= left_dies;   /* have to clear "death" bits */
        rules[(k<<6)+(i<<2)+3] -= left_dies;   /* (which were already set to */
        rules[(i<<6)+(k<<2)+1] -= right_dies;  /* all elements) for the rule */
        rules[(i<<6)+(k<<2)+3] -= right_dies;  /* explcitly specified in 'S' */
    } }                                        /* string of the rule set     */
  }
private:
  static int const left_rxgxb_mask   = 0x3f000000;  /* Birth of each cells   */
  static int const left_rxgxb_shift  =         24;  /* have to be processed  */
  static int const right_rxgxb_mask  =   0x3f0000;  /* independently anyway, */
  static int const right_rxgxb_shift =         16;  /* so we can use only 6  */
  int color_rules[0x3f+1];                          /* bit table             */
  static int const rgb_shift_left  = 13;  /* shifts required to put RGB into */
  static int const rgb_shift_right = 10;  /* place for left/right cells      */
  static int const colormask       =  7;
  static int const left_cell_mask  = 0xE002; /* RGB. .... .... ..L. */
  static int const right_cell_mask = 0x1C01; /* ...r gb.. .... ...R */
  static int const both_cells_mask = 0xFC03; /* RGBr gb.. .... ..LR */

  static inline int leftCOLOR (int val) { return (val >> rgb_shift_left) &7; }
  static inline int rightCOLOR(int val) { return (val >> rgb_shift_right)&7; }
  static inline int leftVALUE (int clr) { return (clr << rgb_shift_left) +2; }
  static inline int rightVALUE(int clr) { return (clr << rgb_shift_right)+1; }

  int               left_incr[8], /* what should be added to increment count */
    Uleft_incr[8], right_incr[8], /* for left/right/both cells (include both */
    Uright_incr[8], both_incr[8]; /* color and neighbor counters)            */

public:
  void setColorRules(const char *ruleString)
  {
    int r, g, b, i, color, clr_incr;

    for (r = 0; r < 4; r++) {
      for (g = 0; g < 4; g++) {
        for (b = 0; b < 4; b++) {
          i = (r << 4) + (g << 2) + b;
          color = ((r < 2) ? 0 : 4)  /* - color for the cell with 'r' red */
                + ((g < 2) ? 0 : 2)  /* neighbors, 'g' green and 'b' blue */
                + ((b < 2) ? 0 : 1); /* ones...                           */
          if (color == 7) color = 0; /* ... but white is forbidden        */
          color_rules[i] = color;
    } } }
    for (i = 0; i < 8; i++) {
      r = (i & 4);
      g = (i & 2);
      b = (i & 1); clr_incr = (r << 2) + (g << 1) + b;

      left_incr [i] = (clr_incr << left_rxgxb_shift)  + 0x40;
      right_incr[i] = (clr_incr << right_rxgxb_shift) +  0x4;
      both_incr [i] = left_incr[i] + right_incr[i];

      Uleft_incr [i] = right_incr[i] + leftVALUE(i);
      Uright_incr[i] = left_incr[i] + rightVALUE(i);
  } }
private:                         /*     +-------+-------+     */
  int Nsize, Nlen, *Npos, *Nval, /*     ! N(-1) ! N(+1) !     */
                                 /* +---+---+---+---+---+---+ */
      Wsize, Wlen, *Wpos, *Wval, /* !    Wr ! Ul Ur ! El    ! */
      Esize, Elen, *Epos, *Eval, /* +---+---+---+---+---+---+ */
                                 /*     ! S(-1) ! S(+1) !     */
      Ssize, Slen, *Spos, *Sval, /*     +-------+-------+     */
                                 /*                           */
      Msize, Mlen, *Mpos, *Mval; /* for keeping merge results */

  static int const Nshift = -xone-yone, NshiftPlus = +xone-yone,
                   Wshift = -2*xone,
                   Eshift = +2*xone,    NSshift    = +2*yone;

  static void adjustSize(int &Xsize, int *&Xpos, int *&Xval, int reqLen)
  {
    if (Xsize < reqLen) { 
      if (Xsize) { Xpos = (int *)realloc(Xpos, reqLen*sizeof(int));
                   Xval = (int *)realloc(Xval, reqLen*sizeof(int)); }
      else {
        Xpos = MALLOCxN(int, reqLen);
        Xval = MALLOCxN(int, reqLen); } Xsize = reqLen;
  } }
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
public: 
  elMundoA()
  {
    Usize = 64; Upos = MALLOCxN(int, Usize); Nsize = Nlen = 0;
    Ulen  =  0; Uval = MALLOCxN(int, Usize); Wsize = Wlen = 0;
                                             Esize = Elen = 0;
    setRules(0x8,0xC);                       Ssize = Slen = 0;
    setColorRules(NULL);   finalized = 0;    Msize = Mlen = 0;
  }
  ~elMundoA()
  {             if (Nsize) { free(Npos); free(Nval); }
    free(Upos); if (Wsize) { free(Wpos); free(Wval); }
    free(Uval); if (Esize) { free(Epos); free(Eval); }
                if (Ssize) { free(Spos); free(Sval); }
                if (Msize) { free(Mpos); free(Mval); }
  }
/*---------------------------------------------------------------------------*/
private:
  int start_pos, end_pos, offsetX,
                          offsetY;
public:
  void setFrame(int xmin, int xmax, int ymin, int ymax)
  {
    elMundo::setFrame(xmin, xmax, ymin, ymax);
    start_pos = pack(XvMin-1, YvMin);
    end_pos   = pack(XvMax+1, YvMax); // for optimization
  }
private:
  void paintCell(elVista *V, int pos, int clr)
  {
    if (start_pos <= pos && pos < end_pos) {
      int x = X(pos);
      if (XvMin <= x && x <= XvMax) V->observe(x, Y(pos), clr);
  } }
  void paintTrack(elVista *V, int pos, int clr)
  {
    if (start_pos <= pos && pos < end_pos) {
      int x = X(pos);
      if (XvMin <= x && x <= XvMax) V->observe(x, Y(pos), clr+elcvDead);
  } }
/*---------------------------------------------------------------------------*/
                    /* Update neighbor counters and generates N, W, E arrays */
  void passZero()   /* for initial configuration stored in Upos/val arrays   */
  {
    int i, clr, Uincr, /* what we should add to the value of orginal cell */
                NvalL, /* the value for N(-1) cell                        */
                NvalR; /* the value for N(+1) cell (see diagram above)    */

    adjustSize(Nsize, Npos, Nval, 2*Ulen+2);
    adjustSize(Wsize, Wpos, Wval,   Ulen+2);
    adjustSize(Esize, Epos, Eval,   Ulen+2);

    for (i = Nlen = Wlen = Elen = 0; i < Ulen; i++) {
      int val = Uval[i];
      if ((val & cell_alive_mask) != 0) {
        int pos = Upos[i];
        if ((val & left_alive_mask) != 0) {
          clr = leftCOLOR(val);
          NvalL = both_incr[clr]; Uincr = Wval[Wlen]   = right_incr[clr];
          NvalR = left_incr[clr];         Wpos[Wlen++] = pos + Wshift;
        }
        else Uincr = NvalL = NvalR = 0;

        if ((val & right_alive_mask) != 0) {
          clr = rightCOLOR(val);
          NvalL += right_incr[clr]; Uincr += (Eval[Elen]   = left_incr[clr]);
          NvalR += both_incr [clr];           Epos[Elen++] = pos + Eshift;
        }
        Uval[i] += Uincr; Npos[Nlen] = pos + Nshift;     Nval[Nlen++] = NvalL;
                          Npos[Nlen] = pos + NshiftPlus; Nval[Nlen++] = NvalR;
  } } }
                           /* Update cell status (i.e. kill some cells and */
  void passOne(elVista *V) /* give a birth to others) and generates E,N,W  */
  {                        /* arrays with increments for neighboring cells */

    int i, action, clr, Uincr, NvalL, NvalR;
    adjustSize(Nsize, Npos, Nval, 2*Ulen+2);
    adjustSize(Wsize, Wpos, Wval,   Ulen+2);
    adjustSize(Esize, Epos, Eval,   Ulen+2);

    for (i = Nlen = Wlen = Elen = 0; i < Ulen; i++) {
      int val = Uval[i];
      if ((action = rules[val & rule_mask]) != 0) {
        int pos = Upos[i];
        switch (action) {
        case left_born:
        case left_born_right_dies:
        case both_born:
          clr = color_rules[(val & left_rxgxb_mask) >> left_rxgxb_shift];
          Uincr = Uleft_incr[clr];        Wval[Wlen]   = right_incr[clr];
          NvalL = both_incr[clr];         Wpos[Wlen++] = pos + Wshift;
          NvalR = left_incr[clr];
          if (V) paintCell(V, pos, clr); break;

        case left_dies:
        case left_dies_right_born:
        case both_die:
          clr = leftCOLOR(val);
          Uincr = -Uleft_incr[clr]; Wval[Wlen]   = -right_incr[clr];
          NvalL = -both_incr[clr];  Wpos[Wlen++] = pos + Wshift;
          NvalR = -left_incr[clr];
          if (V) paintTrack(V, pos, clr); break;

        case right_born: /* in order to simplify algorithm, always set */
        case right_dies: /* Uincr & NvalL/R in processing of left cell */
        default:
          Uincr = NvalL = NvalR = 0;
        }
        switch (action) {
        case           right_born:
        case left_dies_right_born:
        case both_born:
          clr = color_rules[(val & right_rxgxb_mask) >> right_rxgxb_shift];
          Uincr += Uright_incr[clr];         Eval[Elen]   = left_incr[clr];
          NvalL += right_incr[clr];          Epos[Elen++] = pos + Eshift;
          NvalR += both_incr[clr];
          if (V) paintCell(V, pos+1, clr); break;

        case           right_dies:
        case left_born_right_dies:
        case both_die:
          clr = rightCOLOR(val);
          Uincr -= Uright_incr[clr];  Eval[Elen]   = -left_incr[clr];
          NvalL -= right_incr[clr];   Epos[Elen++] = pos + Eshift;
          NvalR -= both_incr[clr];
          if (V) paintTrack(V, pos+1, clr); break;
        }
        Uval[i] += Uincr; Npos[Nlen] = pos + Nshift;     Nval[Nlen++] = NvalL;
                          Npos[Nlen] = pos + NshiftPlus; Nval[Nlen++] = NvalR;
  } } }
                 /* Merge same-position cells in N array and copy them */
  void passTwo() /* (with appropriate shift) to S array                */
  {
    int i = 0, pos,                      /* to remove end-of-array check */
        j = 0, val; Npos[Nlen] = maxpos; /*<--        in the nested loop */

    adjustSize(Ssize, Spos, Sval, Nlen+2);

    while (i < Nlen) {
      pos = Npos[i];
      val = Nval[i++]; while (Npos[i] == pos) val += Nval[i++];
      Npos[j] = pos;
      Spos[j] = pos + NSshift;
      Nval[j] = Sval[j] = val; j++;
    }
    Nlen = Slen = j;
  }
  int merge(int *S1p, int *S1v, int S1len,
            int *S2p, int *S2v, int S2len, int *Dp, int *Dv)
  {
    int i1 = 0, p1,
        i2 = 0, p2, n = 0;

    S1p[S1len] = maxpos; /* to reduce number of operation in the loop */
    S2p[S2len] = maxpos; /* make sure parsing will end simultaneously */

    p1 = S1p[0]; /* another small optimization: in theory, indexation */
    p2 = S2p[0]; /* takes some time.. not sure it's true in Java, tho */

    while (i1 <= S1len) {
           if (p1 < p2) { Dp[n] = p1; Dv[n++] = S1v[i1++]; p1 = S1p[i1]; }
      else if (p1 > p2) { Dp[n] = p2; Dv[n++] = S2v[i2++]; p2 = S2p[i2]; }
      else {
        Dp[n] = p1; Dv[n++] = S1v[i1++] + S2v[i2++]; p1 = S1p[i1];
                                                     p2 = S2p[i2];
    } }
    return n-1; /* have to remove garbage we inserted */
  }
  int merge_n_clear(int *S1p, int *S1v, int S1len,
                    int *S2p, int *S2v, int S2len, int *Dp, int *Dv)
  {
    int i1 = 0, p1,             /* the same as previous function, except */
        i2 = 0, p2, n = 0, val; /* it clears all totally dead cells i.e. */
                                /* those with no alive neighbors from Dx */

    S1p[S1len] = maxpos; p1 = S1p[0]; S1v[S1len] = 1;
    S2p[S2len] = maxpos; p2 = S2p[0]; S2v[S2len] = 1;

    while (i1 <= S1len) {
      if (p1 < p2) {
        if ((val = S1v[i1++]) != 0) { Dp[n] = p1; Dv[n++] = val; }
        p1 = S1p[i1];
      }
      else if (p1 > p2) { 
        if ((val = S2v[i2++]) != 0) { Dp[n] = p2; Dv[n++] = val; }
        p2 = S2p[i2];
      }
      else {
        if ((val = S1v[i1++] + S2v[i2++]) != 0) { Dp[n] = p1; Dv[n++] = val; }
        p1 = S1p[i1];     p2 = S2p[i2];
    } }
    return n-1;
  }
  void passThree() /* Merges all arrays back to U */
  {
    adjustSize(Msize, Mpos, Mval, Nlen+Wlen+2);
    Mlen = merge(Npos, Nval, Nlen,
                 Wpos, Wval, Wlen, Mpos, Mval);

    adjustSize(Nsize, Npos, Nval, Mlen+Elen+2);
    Nlen = merge(Mpos, Mval, Mlen,
                 Epos, Eval, Elen, Npos, Nval);

    adjustSize(Msize, Mpos, Mval, Nlen+Slen+2);
    Mlen = merge(Npos, Nval, Nlen,
                 Spos, Sval, Slen, Mpos, Mval);

    adjustSize(Nsize, Npos, Nval, Mlen+Ulen+2);
    Nlen = merge_n_clear(Mpos, Mval, Mlen,
                         Upos, Uval, Ulen, Npos, Nval);
    int *tmp, len, size;
                                          /*- Do not need the contents of N, */
    tmp = Npos; Npos = Upos; Upos = tmp;  /* but it's more effective to swap */
    tmp = Nval; Nval = Uval; Uval = tmp;  /* arrays than to re-allocate them */
    len = Nlen; Nlen = Ulen; Ulen = len;  /* later                           */
    size = Nsize; Nsize = Usize; Usize = size;
  }
                   /* Un-finalaze the universe (i.e. remove all neighborhood */
  void passZilch() /* counters that may confuse passZero function later)     */
  {               
    int i, j;
    for (i = j = 0; i < Ulen; i++) {
      int val = Uval[i] & both_cells_mask;
      if (val) {
        Upos[j] = Upos[i];
        Uval[j++] = val; /* we're not adding cells so don't need extra space */
    } }
    Ulen = j;
  }
/*---------------------------------------------------------------------------*/
public:                           /* Calculate new generation, with updating */
  int nextGeneration(elVista *V)  /* the universe using given Vista (actual  */
  {                               /* update is made during the first pass)   */
    if (!finalized) {
      passZero(); passTwo(); passThree(); finalized = 1;
#if 0
      V->update(); return 2;
#endif
    }
    passOne(V);   passTwo(); passThree();
    return 2;
  }
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
private:
  enum cell_location {
    new_insert = 1,
    new_at_end = 2,
    update_one = 3
  };
  cell_location find_cell_location(int x, int y, int &where, bool &is_left)
  {
    int last_pos, pos = pack(x, y),
                    i = (pos & xone+yone);
    is_left =  (i == 0 || i == xone+yone);

    if (finalized) { passZilch();   finalized = 0; }
    if (Ulen == 0) { where = 0; return new_at_end; }
    else {
      if (!is_left) pos--;
      if (pos > Upos[Ulen-1]) { where = Ulen; return new_at_end; }
      else {
        for (i = Ulen; i--;) {
          if (pos == (last_pos = Upos[i])) { where = i;   return update_one; }
          if (pos >   last_pos)            { where = i+1; return new_insert; }
        }
        where = 0;
        return new_insert;
  } } }
public:
  void add(int x, int y, int clr)
  {
    bool is_left;
    int n;
    if (Usize < Ulen+1) /* make sure we have room for extra cell */
      adjustSize(Usize, Upos, Uval, Usize+Usize/2);

    switch (find_cell_location(x, y, n, is_left)) {
    case new_insert:
      for (int i = Ulen; i > n; i--) { Upos[i] = Upos[i-1];
                                       Uval[i] = Uval[i-1]; } /* FALL THRU */
    case new_at_end:
      Upos[n] = pack(x - 1+is_left, y);
      Uval[n] = is_left ? leftVALUE(clr) : rightVALUE(clr); Ulen++;
      break;
    case update_one:
      if (is_left) Uval[n] = (Uval[n] & ~left_cell_mask)| leftVALUE(clr);
      else         Uval[n] = (Uval[n] &~right_cell_mask)|rightVALUE(clr);
  } }
  void toggle(int x, int y, int clr)
  {
    bool is_left;
    int n;
    if (Usize < Ulen+1) /* make sure we have room for extra cell */
      adjustSize(Usize, Upos, Uval, Usize+Usize/2);

    switch (find_cell_location(x, y, n, is_left)) {
    case new_insert:
      for (int i = Ulen; i > n; i--) { Upos[i] = Upos[i-1];
                                       Uval[i] = Uval[i-1]; } /* FALL THRU */
    case new_at_end:
      Upos[n] = pack(x - 1+is_left, y);
      Uval[n] = is_left ? leftVALUE(clr) : rightVALUE(clr); Ulen++;
      break;
    case update_one:
      if (is_left) {
        if (Uval[n] & left_alive_mask) {
          if (leftCOLOR(Uval[n]) == clr) Uval[n] &= ~left_cell_mask;
          else Uval[n] = (Uval[n] & ~left_cell_mask)|leftVALUE(clr);
        }
        else Uval[n] |= leftVALUE(clr);
      }
      else {
        if (Uval[n] & right_alive_mask) {
          if (rightCOLOR(Uval[n]) == clr) Uval[n] &= ~right_cell_mask;
          else Uval[n] = (Uval[n] & ~right_cell_mask)|rightVALUE(clr);
        }
        else Uval[n] |= rightVALUE(clr);
  } } }
  void clear(int x, int y)
  {
    bool is_left;
    int n;
    if (find_cell_location(x, y, n, is_left) == update_one) {
      Uval[n] &= is_left ? ~left_cell_mask : ~right_cell_mask;
      if (Uval[n] == 0) {
        for (int i = n+1; i < Ulen; i++) { Upos[i-1] = Upos[i];
                                           Uval[i-1] = Uval[i]; }
        Ulen--;
  } } }
  void clearTheWorld(void)
  {
    Ulen = 0;
  }
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  void iterate(elObservador &eo)
  {
    for (int i = 0; i < Ulen; i++) {
      int val = Uval[i], x = X(Upos[i]),
                         y = Y(Upos[i]);

      if ((val & left_alive_mask)  != 0) eo.observe(x,   y, leftCOLOR(val));
      if ((val & right_alive_mask) != 0) eo.observe(x+1, y, rightCOLOR(val));
  } }
  void iterate(elObservador &eo, int colorMask)
  {
    for (int i = 0; i < Ulen; i++) {
      int val = Uval[i], x = X(Upos[i]),
                         y = Y(Upos[i]), clr;

      if ((val & left_alive_mask) != 0 && 
         ((clr = leftCOLOR(val)) & colorMask)) eo.observe(x, y, clr);

      if ((val & right_alive_mask) != 0 &&
         ((clr = rightCOLOR(val)) & colorMask)) eo.observe(x+1, y, clr);
  } }
  void iterate(elObservador &eo, int xmin, int xmax, int ymin, int ymax)
  {
    for (int i = 0; i < Ulen; i++) {
      int y = Y(Upos[i]);
      if (ymin <= y && y <= ymax) {
        int val = Uval[i], x = X(Upos[i]);
        if ((val & left_alive_mask)
                 && xmin <= x && x <= xmax) eo.observe(x, y, leftCOLOR(val));
        x++;
        if ((val & right_alive_mask)
                 && xmin <= x && x <= xmax) eo.observe(x, y, rightCOLOR(val));
  } } }
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  void getFitFrame(int &xmin, int &xmax, int &ymin, int &ymax)
  {
    if (Ulen) {
      xmin = xmax = X(Upos[0]);
      ymin =        Y(Upos[0]);
      for (int i = 1; i < Ulen; i++) {
        int x = X(Upos[i]);
        if (x < xmin) xmin = x;
        if (x > xmax) xmax = x;
      }
      ymax = Y(Upos[Ulen-1]);
    }
    else {
      xmin = XvMin; xmax = XvMax; // when the world is empty, preserve status
      ymin = YvMin; ymax = YvMax; // quo (so the screen will not flicker)
  } }
};
elMundo *new_elMundoA() { return new elMundoA(); }
//-----------------------------------------------------------------------------
