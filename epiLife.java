/*
 * @(#)epiLife.java version 2.3 -- 30-May-2002
 *
 * Java applet for Conway's Game of Life (colorized)
 * Copyright 1999,2002 by Michael Epiktetov, epi@ieee.org
 *
 * This code was inspired by (and partially based on) LifeApp applet
 * (Copyright 1996-97 by Paul B. Callahan, callahanp@acm.org); algorithm
 * is quite different (and color genes were added to living cells), but
 * some ideas remains the same (see details below).
 *
 * ------------------------------------------------------------------------
 *
 * Author(s) grant(s) you ("Licensee") a non-exclusive, royalty free, 
 * license to use, modify and redistribute this software in source and 
 * binary code form, provided that (i) this copyright notice and license 
 * appear on all copies of the software; and (ii) Licensee does not utilize 
 * the software in a manner which is disparaging to the Author(s).
 *
 * This software is provided "AS IS," without a warranty of any kind.
 * ALL EXPRESS OR IMPLIED CONDITIONS, REPRESENTATIONS AND WARRANTIES,
 * INCLUDING ANY IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE OR NON-INFRINGEMENT, ARE HEREBY EXCLUDED.
 *   AUTOR(S) AND ITS LICENSORS SHALL NOT BE LIABLE FOR ANY DAMAGES
 * SUFFERED BY LICENSEE AS A RESULT OF USING, MODIFYING OR DISTRIBUTING
 * THE SOFTWARE OR ITS DERIVATIVES. IN NO EVENT WILL AUTOR(S) OR ITS
 * LICENSORS BE LIABLE FOR ANY LOST REVENUE, PROFIT OR DATA, OR FOR
 * DIRECT, INDIRECT, SPECIAL, CONSEQUENTIAL, INCIDENTAL OR PUNITIVE
 * DAMAGES, HOWEVER CAUSED AND REGARDLESS OF THE THEORY OF LIABILITY,
 * ARISING OUT OF THE USE OF OR INABILITY TO USE SOFTWARE, EVEN IF
 * AUTHOR(S) HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
 *
 * This software is not designed or intended for use in on-line control of
 * aircraft, air traffic, aircraft navigation or aircraft communications;
 * or in the design, construction, operation or maintenance of any nuclear
 * facility. Licensee represents and warrants that it will not use or
 * redistribute the Software for such purposes.
 *
 * ------------------------------------------------------------------------
 *
 * Version history:
 *
 * 1.x - was an attempt to eliminate all merging arrays from Callahan's
 *       code; the idea was to keep single copy of the universe (with all
 *       neighbor counter precalculated) in L2 list. However, the result
 *       was not satisfactory - it was found that Java is not well suited
 *       for working with pointers.. in fact, it has no real pointers at
 *       all. So, this version was abandoned even before fully debugged.
 *
 * 2.1 - This version still keeps neighbor counter for each cell (because
 *       of added color feature, one integer is not enough for the call
 *       anyway, and two integers per cell have plenty of rooms), but uses
 *       arrays (like original Callahan's version) for keeping cells. It's
 *       not possible to insert new values in the middle, so merging arrays
 *       are still used, but, unlike Callahan's version, only for changed
 *       cells. That was supposed to improve performance... it did, but
 *       even new algorithm worked a bit slower the its monochrome ancestor
 *       (because it have to keep twice as much information). -- 1999/09/11
 *
 * 2.2 - After squeezing two cells into one integer (remember, we had
 *       "plenty of room" there - no more), performance was significantly
 *       improved. It still isn't the fastest applet for Game of Life, but
 *       fastest doesn't have so many colors! -- 1999/09/27
 *
 * 2.3 - Implemented reading initial patter from ECL file (which is just an
 *       expansion of wide-used RLE format, with addition of cell's colors);
 *       finally found how to get array length in Java - so Xsize variables
 *       are gone (and the code is slightly restuctured) -- 2002/05/30
 */

import java.applet.Applet;
import java.awt.*;
import java.awt.event.*;
import java.awt.image.ImageObserver;
import java.awt.image.PixelGrabber;
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.net.URL;
import java.util.Random;

class epiLifeEngine
{
  int Ulen,   /* Length of used memory (number of cell pairs)              */
              /*                  0yyy yyyy yyyy yyyy 0xxx xxxx xxxxx      */
      Upos[]; /* packed Position  .... .... .... .... .xxx xxxx xxxxx - X  */
              /* (of left cell):  .yyy yyyy yyyy yyyy .... .... ..... - Y  */
              /*                                                           */
  final static int maxpos = 0x7fffffff; /* Cell pairs in adjacent rows are */
  final static int  xmask =     0x7fff; /* shifted to form "brick wall"    */
  final static int  ymask = 0x7fff0000; /* pattern (see diagram below);    */
  final static int yshift = 16;         /* it results in smaller number of */
  final static int xone   =          1; /* neigbors for each pair (only 6) */
  final static int yone   =    0x10000; /*                                 */
  final static int zerox  =     0x4000; /* - Put (0,0) into center of the  */
  final static int zeroy  =     0x4000; /*   universe                      */

  public static int pack(int x, int y)
  {
    return ((x + zerox) & xmask) + ((y + zeroy) << yshift);
  }
  static int X(int pos) { return  (pos & xmask)            - zerox; }
  static int Y(int pos) { return ((pos & ymask) >> yshift) - zeroy; }

  int Uval[]; /*            Value: XXXR XGXB xxxr xgxb RGBr gbnn nnmm mmLR */
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

  public final static int cell_alive_mask = 0x3,
                          left_alive_mask = 0x2, right_alive_mask = 0x1;

  final static int rule_mask = 0x003ff; /* used to undex rules[] */
private static int rules[];
  final static int no_action            = 0; /* values selected to allow */
  final static int right_born           = 1; /* independent handling of  */
  final static int right_dies           = 2; /* left/right cell destiny: */
  final static int left_born            = 3; /*                          */
  final static int both_born            = 4; /* left_born = 10(3)        */
  final static int left_born_right_dies = 5; /* left_dies = 20(3)        */
  final static int left_dies            = 6; /*                          */
  final static int left_dies_right_born = 7; /* right_born = 01(3)       */
  final static int both_die             = 8; /* right_dies = 02(3)       */

  static void initRules(String ruleString)
  {
    rules = new int[rule_mask+1];
    int i, j, k;

    for (i = 0; i <= 0x3fc; i += 4) { 
      rules[i+0] = no_action;  /* if not specifed explicitly, no action  */
      rules[i+1] = right_dies; /* required for empty cell and all living */
      rules[i+2] = left_dies;  /* cells are going to die (it's a harsh   */
      rules[i+3] = both_die;   /* world, baby :)                         */
    }
    if (ruleString.charAt(0) != 'b') ruleString = "b3s23";

    for (j = 1; j < ruleString.length() && ruleString.charAt(j) != 's'; j++) {
      k = ruleString.charAt(j) - '0';
      if (k < 1 || k > 8) {
        System.err.println("Incorrect k="+k+" in '"+ruleString+"'"); break;
      }
      for (i = 0; i < 9; i++) {               /* cell should be born if it */
        rules[(k<<6)+(i<<2)+0] += left_born;  /* has exactly given number  */
        rules[(k<<6)+(i<<2)+1] += left_born;  /* of alive neighbors (state */
        rules[(i<<6)+(k<<2)+0] += right_born; /* and neighbors of its peer */
        rules[(i<<6)+(k<<2)+2] += right_born; /* don't count at all)       */
    } }
    for (j++; j < ruleString.length() && ruleString.charAt(j) != 's'; j++) {
      k = ruleString.charAt(j) - '0';
      if (k < 1 || k > 8) {
        System.err.println("Incorrect k="+k+" in '"+ruleString+"'"); break;
      }
      for (i = 0; i < 9; i++) {               /* have to clear "death" bit  */
        rules[(k<<6)+(i<<2)+2] -= left_dies;  /* (which was already set to  */
        rules[(k<<6)+(i<<2)+3] -= left_dies;  /* all elements) for those    */
        rules[(i<<6)+(k<<2)+1] -= right_dies; /* combination that should be */
        rules[(i<<6)+(k<<2)+3] -= right_dies; /* preserved                  */
    } }
  }

  final static int left_rxgxb_mask   = 0x3f000000; /* Birth of each cells   */
  final static int left_rxgxb_shift  =         24; /* have to be processed  */
  final static int right_rxgxb_mask  =   0x3f0000; /* independently anyway, */
  final static int right_rxgxb_shift =         16; /* so we can use only 6  */
private static int color_rules[];                  /* bit table             */
  final static int rgb_shift_left  = 13; /* shifts required to put RGB into */
  final static int rgb_shift_right = 10; /* place for left/right cells, and */
private static int left_incr[],  /* what should be added to increment count */
    Uleft_incr[],  right_incr[], /* for left/right/both cells (include both */
    Uright_incr[], both_incr[];  /* color and neighbor counters)            */

  static void initColorRules(String ruleString)
  {
    int r, g, b;  color_rules = new int[0x3f+1];
    int i, color; left_incr  = new int[num_colors+1];
    int clr_incr; right_incr = new int[num_colors+1];
                  both_incr  = new int[num_colors+1];
    Uleft_incr = new int[num_colors+1];
    Uright_incr = new int[num_colors+1];

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

      Uleft_incr [i] = right_incr[i] + (i<<rgb_shift_left) + left_alive_mask;
      Uright_incr[i] = left_incr[i] + (i<<rgb_shift_right) + right_alive_mask;
    }
  }

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  public final static int num_colors       =   7;
  public final static int colormask        = 0x7;
  public final static int display_colors[] =
  {
    0x000000, 0x0000af, 0x007f00, 0x006f9f, 0xaf0000, 0x7f007f, 0x7f7f00
  };
                            /*     +-------+-------+     */
  int Nlen, Npos[], Nval[], /*     ! N(-1) ! N(+1) !     */
                            /* +---+---+---+---+---+---+ */
      Wlen, Wpos[], Wval[], /* !    Wr ! Ul Ur ! El    ! */
      Elen, Epos[], Eval[], /* +---+---+---+---+---+---+ */
                            /*     ! S(-1) ! S(+1) !     */
      Slen, Spos[], Sval[], /*     +-------+-------+     */
                            /*                           */
      Mlen, Mpos[], Mval[]; /* for keeping merge results */

  final static int Nshift = -xone-yone, NshiftPlus = +xone-yone,
                   Wshift = -2*xone,
                   Eshift = +2*xone,    NSshift    = +2*yone;

  static int[] realloc(int X[], int new_len)
  {
    int i, old_len = X.length, Y[] = new int[new_len];
    for (i = 0; i < old_len; i++) Y[i] = X[i];
    return Y;
  }
  Random randGenerator; /* used for random_recolor (pattern loading) */
  String status;
  int statuspos = 0, generation;

  public epiLifeEngine()
  {
    Upos = new int[64]; Npos = new int[64]; Wpos = new int[64];
    Uval = new int[64]; Nval = new int[64]; Wval = new int[64];
    Epos = new int[64]; Spos = new int[64]; Mpos = new int[64];
    Eval = new int[64]; Sval = new int[64]; Mval = new int[64];

    initRules("b3s23"); generation = 0;
    initColorRules("");
    randGenerator = new Random(); status = new String("");

    offsetX = offsetY = 0;
    cellsize = boxsize = 2;
  }

  void NWEsize() /* check/adjust size of N, W, E arrays */
  {
    if (Npos.length < 2*Ulen+2) { Npos = new int[2*Ulen+2];
                                  Nval = new int[2*Ulen+2]; }

    if (Wpos.length < Ulen+2) { Wpos = new int[Ulen+2];
                                Wval = new int[Ulen+2]; }
    if (Epos.length < Ulen+2) { Epos = new int[Ulen+2];
                                Eval = new int[Ulen+2]; }
  }
                  /* Updates neighbor counters and generates N, W, E arrays */
  void passZero() /* for initial configuration stored in Upos/val arrays    */
  {
    int i, clr, Uincr, /* what we should add to the value of orginal cell */
                NvalL, /* the value for N(-1) cell                        */
                NvalR; /* the value for N(+1) cell (see diagram above)    */
    NWEsize();

    for (i = Nlen = Wlen = Elen = 0; i < Ulen; i++) {
      int val = Uval[i];
      if ((val & cell_alive_mask) != 0) {
        int pos = Upos[i];
        if ((val & left_alive_mask) != 0) {
          clr = (val >> rgb_shift_left) & colormask;
          NvalL = both_incr[clr];
          NvalR = left_incr[clr]; Uincr = Wval[Wlen]   = right_incr[clr];
                                          Wpos[Wlen++] = pos + Wshift;
        }
        else Uincr = NvalL = NvalR = 0;

        if ((val & right_alive_mask) != 0) {
          clr = (val >> rgb_shift_right) & colormask;
          NvalL += right_incr[clr];
          NvalR += both_incr [clr]; Uincr += (Eval[Elen]   = left_incr[clr]);
                                              Epos[Elen++] = pos + Eshift;
        }
        Uval[i] += Uincr; Npos[Nlen] = pos + Nshift;     Nval[Nlen++] = NvalL;
                          Npos[Nlen] = pos + NshiftPlus; Nval[Nlen++] = NvalR;
    } }
  }
                           /* Updates cell status (i.e. kill some cells and */
  void passOne(Graphics g) /* give a birth to others) and generates E, N, W */
  {                        /* arrays with increments for neighboring cells  */

    int i, action, clr, Uincr, NvalL, NvalR;
    NWEsize();

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
          paintCell(g, pos, clr); break;

        case left_dies:
        case left_dies_right_born:
        case both_die:
          clr = (val >> rgb_shift_left) & colormask;
          Uincr = -Uleft_incr[clr]; Wval[Wlen]   = -right_incr[clr];
          NvalL = -both_incr[clr];  Wpos[Wlen++] = pos + Wshift;
          NvalR = -left_incr[clr];
          paintTrack(g, pos, clr); break;

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
          paintCell(g, pos+1, clr); break;

        case           right_dies:
        case left_born_right_dies:
        case both_die:
          clr = (val >> rgb_shift_right) & colormask;
          Uincr -= Uright_incr[clr];  Eval[Elen]   = -left_incr[clr];
          NvalL -= right_incr[clr];   Epos[Elen++] = pos + Eshift;
          NvalR -= both_incr[clr];
          paintTrack(g, pos+1, clr); break;
        }
        Uval[i] += Uincr; Npos[Nlen] = pos + Nshift;     Nval[Nlen++] = NvalL;
                          Npos[Nlen] = pos + NshiftPlus; Nval[Nlen++] = NvalR;
    } }
  }
                 /* Merges same-position cells in N array and copy them */
  void passTwo() /* (with appropriate shift) to S array                 */
  {
    if (Spos.length < Nlen+2) { Spos = new int[Nlen+2]; 
                                Sval = new int[Nlen+2]; }
    int i = 0, pos,
        j = 0, val; Npos[Nlen] = maxpos; /* to remove end-of-array check */
                                         /* in the nested loop           */
    while (i < Nlen) {
      pos = Npos[i];
      val = Nval[i++]; while (Npos[i] == pos) val += Nval[i++];
      Npos[j] = pos;
      Spos[j] = pos + NSshift;
      Nval[j] = Sval[j] = val; j++;
    }
    Nlen = Slen = j;
  }

  static int merge(int S1p[], int S1v[], int S1len,
                   int S2p[], int S2v[], int S2len, int Dp[], int Dv[])
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

  static int merge_n_clear(               /* the same as previous function, */
         int S1p[], int S1v[], int S1len, /* except clears all totally dead */
         int S2p[], int S2v[], int S2len, /* cells (i.e. dead with no alive */
         int Dp [], int Dv [])            /* neighbors) from the result     */
  {
    int i1 = 0, p1,
        i2 = 0, p2, n = 0, val;

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
    if (Mpos.length < Nlen+Wlen+2) { Mpos = new int[Nlen+Wlen+2]; 
                                     Mval = new int[Nlen+Wlen+2]; }
    Mlen = merge(Npos, Nval, Nlen,
                 Wpos, Wval, Wlen, Mpos, Mval);

    if (Npos.length < Mlen+Elen+2) { Npos = new int[Mlen+Elen+2]; 
                                     Nval = new int[Mlen+Elen+2]; }
    Nlen = merge(Mpos, Mval, Mlen,
                 Epos, Eval, Elen, Npos, Nval);

    if (Mpos.length < Nlen+Slen+2) { Mpos = new int[Nlen+Slen+2]; 
                                     Mval = new int[Nlen+Slen+2]; }
    Mlen = merge(Npos, Nval, Nlen,
                 Spos, Sval, Slen, Mpos, Mval);

    if (Npos.length < Mlen+Ulen+2) { Npos = new int[Mlen+Ulen+2];
                                     Nval = new int[Mlen+Ulen+2]; }
    Nlen = merge_n_clear(Mpos, Mval, Mlen,
                         Upos, Uval, Ulen, Npos, Nval);
    int tmp[], len;     
                                         /*- Do not need the contents of N, */
    tmp = Npos; Npos = Upos; Upos = tmp; /* but it's more effective to swap */
    tmp = Nval; Nval = Uval; Uval = tmp; /* arrays than to re-allocate them */
    len = Nlen; Nlen = Ulen; Ulen = len; /* later                           */
  }

  public void nextGeneration(Graphics g) /* Calculate new generation, with */
  {                                      /* updating of the universe using */
    generation++;                        /* given Graphics (actual update  */
    passOne(g); passTwo(); passThree();  /* is made during the first pass) */
  }

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  int statheight = 32;
  int d_width;
  int d_height;

  int xmin, sizeX, offsetX,
      ymin, sizeY, offsetY, start_pos, end_pos;

  int cellsize, csize_ix; /* Current cell size and its index in the following */
                          /* table of valid sizes (set of valid sizes limited */
  static int sizes[] = {  /* to improve interface, algorithms do not depend   */
    1, 2, 3, 5, 8, 12, 20 /* on sizes in any way)                             */
  };
  static int max_sz_ix = 7;

  int boxsize;            /* Like the previous one, boxsize is not claculated */
                          /* but extracted from the table: if we have limited */
  static int bsizes[] = { /* set of sizes, why not simplify computer's life?  */
    1, 2, 3, 4, 7, 11, 19 /* Note that for small values boxsize == cellsize   */
  };

  Color border, bckgnd,       /* border/background colors  */
           true_bckgnd;       /* true background color     */
  static Color live_colors[], /* colors for painting cells */
               trax_colors[]; /* ... and cell tracks       */
  static boolean showtracks = true;
  public void setShowTrack(boolean showtrax) { showtracks = showtrax; }

  int recolor_black = 0; /* convert all blacks into given color */
  public final static int random_recolor = -1; /* ... or random */

  public void setDisplaySize(int width, int height, int footer)
  {
    d_width  = width;
    d_height = height - (statheight = footer);
  }
  public void setColors(Color tbg_color, Color bg_color, 
                        Color brd_color, int rblack)
  {
    bckgnd = bg_color; true_bckgnd = tbg_color;
    border = brd_color; recolor_black = rblack;

    int i, r = bckgnd.getRed(),   rt = (int)(r*0.8),
           g = bckgnd.getGreen(), gt = (int)(g*0.8),
           b = bckgnd.getBlue(),  bt = (int)(b*0.8);

    live_colors = new Color[num_colors];
    for (i = 0; i < num_colors; i++)
      live_colors[i] = new Color(display_colors[i]);

    trax_colors = new Color[num_colors];
    for (i = 1; i < num_colors; i++) {
      trax_colors[i] = new Color(((i & 4) == 0) ? rt : r,
                                 ((i & 2) == 0) ? gt : g,
                                 ((i & 1) == 0) ? bt : b);
    }
    trax_colors[0] = new Color((int)(r*0.95),  // Exception:
                               (int)(g*0.95),  //  make gray color a bit
                               (int)(b*0.95)); //  brighter (for better look)
  }

  public void updateStatus(Graphics g, String text)
  {
    if (statuspos == 0)
      statuspos = d_width - g.getFontMetrics()
                             .stringWidth("Generation: 2147483648");
    else {
      g.setColor(true_bckgnd); /* clear previous status */
      g.drawString(status, statuspos, d_height+15);
    }
    status = text;       g.setColor(Color.black);
    g.drawString(status, statuspos, d_height+15);
  }
  public void updateStatus(Graphics g) { updateStatus(g, "Generation: "
                                                         + generation); }
  public void updatePos(Graphics g, int x, int y)
  {
    if (x < d_width && y < d_height) {
      offsetX += x/cellsize - sizeX/2;
      offsetY += y/cellsize - sizeY/2; paintIt(g);
    }
  }
  public void reCenter()
  {
    if (Ulen > 0) { int pos = Upos[Ulen/2]; offsetX = X(pos);
                                            offsetY = Y(pos); }
  }

  void paintCell(Graphics g, int pos, int clr)
  {
    int x;
    if (start_pos <= pos                        && pos < end_pos &&
        0         <= (x = (pos & xmask) - xmin) && x   < sizeX) {

      g.setColor(live_colors[clr]);
      g.fillRect(4+cellsize*x, 
                 4+cellsize*(((pos & ymask) >> yshift) - ymin), boxsize, 
                                                                boxsize);
  } }
  void paintTrack(Graphics g, int pos, int clr)
  {
    int x;
    if (start_pos <= pos                        && pos < end_pos &&
        0         <= (x = (pos & xmask) - xmin) && x   < sizeX) {

      g.setColor(showtracks ? trax_colors[clr] : bckgnd);
      g.fillRect(4+cellsize*x, 
                 4+cellsize*(((pos & ymask) >> yshift) - ymin), boxsize, 
                                                                boxsize);
  } }

  public void paintIt(Graphics g)
  {
    g.clearRect(0, 0, d_width, d_height + statheight);
    g.setColor(bckgnd); g.fillRect(0, 0, d_width, d_height);

    g.setColor(border.darker());   g.drawRect(0, 0, d_width-4, d_height-4);
    g.setColor(border.brighter()); g.drawRect(3, 3, d_width-4, d_height-4);
    g.setColor(border.darker());   g.drawRect(1, 1, d_width-4, d_height-4);
    g.setColor(border.brighter()); g.drawRect(2, 2, d_width-4, d_height-4);

    sizeX = (d_width  - 8) / cellsize; xmin = offsetX - sizeX/2;
    sizeY = (d_height - 8) / cellsize; ymin = offsetY - sizeY/2;

    start_pos = pack(xmin-1,     ymin);
    end_pos   = pack(xmin+sizeX, ymin+sizeY-1);   xmin += zerox;
                                                  ymin += zeroy;
    for (int i = 0; i < Ulen; i++) {
      if ((Uval[i] & left_alive_mask) != 0)
        paintCell(g, Upos[i], (Uval[i] >> rgb_shift_left) & colormask);
      if ((Uval[i] & right_alive_mask) != 0)
        paintCell(g, Upos[i]+1, (Uval[i] >> rgb_shift_right) & colormask);
    }
    updateStatus(g);
  }

  public void zoom(int incr)
  {
    csize_ix += incr; if (csize_ix <           0) csize_ix =           0;
                      if (csize_ix > max_sz_ix-1) csize_ix = max_sz_ix-1;
    cellsize = sizes[csize_ix];
    boxsize = bsizes[csize_ix];
  }

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  public void clearAll_n_setCellsize(int w, int h)
  {
    Ulen = 0; 
                               cellsize = d_width/w;
    if (cellsize > d_height/h) cellsize = d_height/h;
    if (cellsize < 1)          cellsize = boxsize = 1;
    else { 
      for (csize_ix = max_sz_ix; csize_ix-- > 0; )
        if (sizes[csize_ix] <= cellsize) {
          cellsize = sizes[csize_ix];
          boxsize = bsizes[csize_ix]; break;
    }   }
    offsetX = offsetY = 0;
  }
  public void addCell(int x, int y, int rgb)
  {
    int pos = pack(x, y); /* pack position into internal format */
    if (rgb == 0) {
      if (recolor_black == random_recolor)
           rgb = (randGenerator.nextInt()&0x7fffffff)%(num_colors-1)+1;
      else rgb = recolor_black;
    }
    if (Upos.length < Ulen+1) {
      Upos = realloc(Upos, Upos.length + Upos.length/2);
      Uval = realloc(Uval, Upos.length); /* Upos.length is updated already */
    }
    switch (pos & (xone+yone)) {
    case 0:
    case xone+yone:
      Upos[Ulen] = pos;
      Uval[Ulen++] = (rgb << rgb_shift_left) + left_alive_mask;
      break;
               /* rule of thumb: if LS bits of X and Y are different, */
    case xone: /* this is right cell of pair, try to update existing  */
    case yone:
      int val = (rgb << rgb_shift_right) + right_alive_mask;
      pos--;
      if (Ulen > 0 && Upos[Ulen-1] == pos) Uval[Ulen-1] += val;
      else {          Upos[Ulen]   =  pos; Uval[Ulen++]  = val; }
    }
  }
  public void finishEditing() { passZero ();
                                passTwo  ();
                                passThree(); generation = 0; }
}

/*---------------------------------------------------------------------------*/

public class epiLife 
  extends Applet
  implements Runnable, ActionListener, ItemListener, MouseListener
{
  epiLifeEngine U; /* U stands for Universe */
  String pattern, ptn_name;

  boolean inited = false, 
          running = false, need_repaint = false;

  Thread runner = null; int bystep = 0;
                        int speed = 50;

  Button gobutton, stepbutton, backbutton, cntrbutton, zinbutton, zoutbutton;
  Checkbox showchkbox;
  static final int bottomheight = 32;

  public epiLife  () { U = new epiLifeEngine(); }
  public void init()
  {
    ptn_name = getParameter("pattern");
    if (ptn_name == null) pattern = "x=7,y=3;bo$3bo$oobb3o!"; /* acorn */

    String cname; Color bgcolor = ((cname = getParameter("bkgnd")) != null)
                                  ? convertColor(cname) : getBackground();
    int new_black = 0;
    Color borderc;
    if (bgcolor.equals(Color.white)) borderc = Color.gray;
    else {
      setBackground(bgcolor); /* buttons don't look good */
      borderc = bgcolor;      /*  with white background  */
    }
    if ((cname = getParameter("black")) != null) {
      if (cname.equalsIgnoreCase("random")) new_black = U.random_recolor;
      else {
        new_black = getRGBfromDisplay(convertColor(cname).getRGB());
        if (new_black == U.colormask) new_black = U.num_colors-1;
    } }
    U.setColors(getBackground(), bgcolor, borderc, new_black);
    U.setDisplaySize(getSize().width, getSize().height, bottomheight);

    setLayout(new FlowLayout(FlowLayout.LEFT, 0, 
                             getSize().height-bottomheight));

    add(gobutton   = new Button("-Go-")); gobutton.addActionListener(this);
    add(stepbutton = new Button("Step")); stepbutton.addActionListener(this);
    add(backbutton = new Button("Back")); backbutton.addActionListener(this);

    add(showchkbox = new Checkbox("Tracks", true));
        showchkbox.addItemListener(this);

    add(cntrbutton = new Button("Cntr")); cntrbutton.addActionListener(this);
    add(zoutbutton = new Button("(-)"));  zoutbutton.addActionListener(this);
    add(zinbutton = new Button("(+)"));   zinbutton.addActionListener(this);

    addMouseListener(this);
  }
  public void destroy() { removeMouseListener(this); }
  public void start() 
  {
    runner = new Thread(this); runner.start();
  }
  public synchronized void stop() { runner = null; }

  public void run() 
  {
    Thread me = Thread.currentThread();
    if (!inited) { 
      if (ptn_name != null) loadNew(ptn_name);
      else             setFromString(pattern);

      if (U.cellsize <= 1) zoutbutton.setEnabled(false);
      U.paintIt(getGraphics());           inited = true;
    }
    while (runner == me) {
      try { Thread.sleep(speed); } catch (InterruptedException e) { }
      repaint();
    }
  }
  public void paint (Graphics g) { U.paintIt(g); }
  public void update(Graphics g) 
  {
    if (running || (bystep--) > 0) {
      U.nextGeneration(g);
      if (need_repaint) { U.paintIt(g); need_repaint = false; }
      else                U.updateStatus(g);
    }
  }

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  void loadNew(String pname)
  {
    if (pname.endsWith("!")) setFromString(pname);
    else 
    if (pname.endsWith(".gif") || pname.endsWith(".png")) {
      //
      // Only GIF and PNG are supported for image files (and only lower-case)
      //
      Image img = getImage(getCodeBase(), pname);

      while (runner != null && !loadFromImage(img, this)) {
        try { Thread.sleep(200); }
        catch (InterruptedException e) { }
      }
      return;
    } 
    try {
      String codebase = getCodeBase().toString();
      if (!codebase.endsWith("/")) codebase += "/";
      String line;
      StringBuffer ptn = new StringBuffer();
      URL url = new URL(codebase+pname);
      BufferedReader in = new BufferedReader(
                          new InputStreamReader(url.openStream()));
      while (true) {
        if ((line = in.readLine()) == null) { ptn.append('!'); break; }
        else {
          ptn.append(line).append('\n');
          if (line.endsWith("!")) break;
      } }
      pattern = ptn.toString();
    }
    catch(Exception e) {
      U.updateStatus(getGraphics(), e.toString()); return;
    }
    setFromString(pattern);
  }

  public void setFromString(String ptn)
  {
    int i, ptnLen = ptn.length();

    if (ptn.startsWith("#")) {
      for (i = 0; i < ptnLen && ptn.charAt(i) == '#'; i++)
        if ((i = ptn.indexOf('\n', i)) < 0) return;

      ptn = ptn.substring(i); ptnLen = ptn.length();
    }
    int x = 0, w = getIntValueOf(ptn, "x"),
        y = 0, h = getIntValueOf(ptn, "y"), mult = 0, clr;

    if (w <= 0 || h <= 0 || (i = ptn.indexOf('\n')) < 0) return;
    U.clearAll_n_setCellsize(w, h);

    while ((++i) < ptnLen) {
      char ch = ptn.charAt(i);
      if ('0' <= ch && ch <= '9') mult = mult*10 + ch-'0';
      else if (ch == '!') break;
      else if (ch != ' ' && ch != ';' && ch != '\n') {
        if (mult == 0) mult = 1;
        switch (ch) {
        case 'b':        x += mult; mult = 0; continue;
        case '$': x = 0; y += mult; mult = 0; continue;
        //
        // Because 'b' is already taken, we have to use Spanish names for
        // prime colors and XYZ for mixed ones:
        //
        //   Rojo (red), Verde (green), Azul (blue)
        //
        //   X cyan (think of cyanide ;)
        //   Y yellow/brown
        //   Z magenta
        //
        // All unknown symbols default to black color
        //
        case 'r': clr = 4; break;
        case 'v': clr = 2; break;
        case 'a': clr = 1; break;
        case 'x': clr = 5; break;
        case 'y': clr = 6; break;
        case 'z': clr = 3; break;
        default:  clr = 0; break;
        }
        while (mult > 0) { U.addCell((x++)-w/2, y-h/2, clr); mult--; }
    } }
    U.finishEditing();
  }
  String getValueOf(String str, String name)
  {
    int i, len = str.length(), i0 = -1, iend = len-1;

    if ((i = str.indexOf( name )) < 0 ||
        (i = str.indexOf('=', i)) < 0) return null;

    while ((++i) < len && str.charAt(i) == ' ');
    while (i < len) {
      char ch = str.charAt(i);
      if (ch == ' ') continue;      if (i0 < 0)   i0   = i;
      if (ch == ',' || ch == ';' || ch == '\n') { iend = i; break; }  
      i++;
    }
    if (i0 < 0) return null;
    else        return str.substring(i0, iend);
  }
  int getIntValueOf(String str, String name)
  {
    String val = getValueOf(str, name);
    if (val == null) return -1;
    else             return Integer.parseInt(val);
  }

  public boolean loadFromImage(Image img, ImageObserver imo)
  {
    int i,j, w = img.getWidth(imo),
        rgb, h = img.getHeight(imo); if (w <= 0 || h <= 0) return false;

    int[] pixels = new int[w*h];
    U.clearAll_n_setCellsize(w, h);

    PixelGrabber pg = new PixelGrabber(img, 0, 0, w, h, pixels, 0, w);
    try {
      pg.grabPixels();
    } 
    catch (InterruptedException e) { return false; }

    for (i = 0; i < h; i++) {
      for (j = 0; j < w; j++) {
        rgb = getRGBfromDisplay(pixels[i*w + j]);
        if (rgb != U.colormask) U.addCell(j-w/2, i-h/2, rgb);
    } }
    U.finishEditing(); return true;
  }
  public static int getRGBfromDisplay(int pixel) /* ... from display color */
  {
    return ((pixel & 0x010000) >> 14) + /* R -> 4 */
           ((pixel & 0x000100) >>  7) + /* G -> 2 */
            (pixel & 0x000001);         /* B -> 1 */
  }
  static Color convertColor(String name)
  {
    if (name.equalsIgnoreCase("white"))     return Color.white;
    if (name.equalsIgnoreCase("lightGray")) return Color.lightGray;
    if (name.equalsIgnoreCase("gray"))      return Color.gray;
    if (name.equalsIgnoreCase("darkGray"))  return Color.darkGray;
    if (name.equalsIgnoreCase("black"))     return Color.black;
    if (name.equalsIgnoreCase("red"))       return Color.red;
    if (name.equalsIgnoreCase("pink"))      return Color.pink;
    if (name.equalsIgnoreCase("orange"))    return Color.orange;
    if (name.equalsIgnoreCase("yellow"))    return Color.yellow;
    if (name.equalsIgnoreCase("green"))     return Color.green;
    if (name.equalsIgnoreCase("magenta"))   return Color.magenta;
    if (name.equalsIgnoreCase("cyan"))      return Color.cyan;
    if (name.equalsIgnoreCase("blue"))      return Color.blue;

    if (name.charAt(0) == '#') return Color.decode("0x"+name.substring(1));
    return new Color(255, 255, 0); /* bold yellow as indication of error */
  }

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  public void actionPerformed(ActionEvent e) 
  {
    Object src = e.getSource();

    if (src == gobutton) {
      if (running) { running = false; gobutton.setLabel("-Go-"); }
      else         { running =  true; gobutton.setLabel("Stop"); }
    }
    else if (src == stepbutton) {
      bystep = 1;
      if (running) { running = false; gobutton.setLabel("-Go-"); }
    }
    else if (src == backbutton) {
      stop();
      inited = false; zinbutton.setEnabled(true);
                      zoutbutton.setEnabled(true);
      start();
      U.paintIt(getGraphics());
    }
    else if (src == cntrbutton) {
      U.reCenter();
      U.paintIt(getGraphics());
    }
    else if (src == zinbutton) {
      U.zoom(+1);                      zoutbutton.setEnabled(true);
      if (U.csize_ix >= U.max_sz_ix-1) zinbutton.setEnabled(false);
      U.paintIt(getGraphics());
    }
    else if (src == zoutbutton) {
      U.zoom(-1);         zinbutton.setEnabled(true);
      if (U.csize_ix < 1) zoutbutton.setEnabled(false);
      U.paintIt(getGraphics());
    } 
  }

  public void itemStateChanged(ItemEvent e) 
  {
    Object src = e.getSource();
    boolean selected = (e.getStateChange() == ItemEvent.SELECTED);

    if (src == showchkbox) {
      U.setShowTrack(selected);
      U.paintIt(getGraphics());
    }
  }

  public synchronized void mouseClicked(MouseEvent ev)
  {
    U.updatePos(getGraphics(), ev.getX(), ev.getY());
  }
  public void mousePressed (MouseEvent e) {}
  public void mouseReleased(MouseEvent e) {}
  public void mouseEntered (MouseEvent e) {}
  public void mouseExited  (MouseEvent e) {}

  public String getAppletInfo() 
  {
    return "Author: Michael Epiktetov (epi@ieee.org)\nTitle: epiLife\nColorized Conway's Game of Life";
  }
  public String[][] getParameterInfo() 
  {
    String parm_info[][] = 
    {
      { "pattern", "string", "pattern (inline of file name)"    },
      { "bkgnd",   "string", "background color"                 },
      { "black",   "string", "repaint black to: random/{color}" },
    };
    return parm_info;
  }
}

