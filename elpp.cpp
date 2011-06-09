static char const rcsid[] = "$Id: $";
/*
 * epiLife++ startup and main controls
 *
 * Viewer/editor/etc for Conway's Game of Life (in full color)
 * Copyright 2002 by Michael Epiktetov, epi@ieee.org
 *
 *-----------------------------------------------------------------------------
 *
 * $Log: $
 *
 */
#include <stdio.h>
#include <gtk/gtk.h>
#include <sys/time.h>
#include "elpp.h"
#include "elmundo.h" /* class elMundo, elObservador */
#include "elvista.h" /* class elVista               */

static GtkWidget *mainWin = NULL,
             *drawingArea = NULL;

static elMundo *M;
static elVista *V;


/*---------------------------------------------------------------------------*/
unsigned long inline timeDiff_msec (struct timeval then, struct timeval now)
{
  return 1000*(now.tv_sec - then.tv_sec) + (now.tv_usec - then.tv_usec)/1000;
}
/*---------------------------------------------------------------------------*/
GtkLabel *egtk_box_add_label (GtkBox *box, const char *text)
{
  GtkLabel *label = GTK_LABEL(gtk_label_new(text));
  gtk_widget_show(GTK_WIDGET(label));
  gtk_box_pack_start(box, GTK_WIDGET(label), /* expand:  */ FALSE,
                                             /* fill:    */ FALSE,
                                             /* padding: */ 0);
  return label;
}
void egtk_button_set_text (GtkButton *button, const char *new_text)
{
  GtkLabel *label = GTK_LABEL(GTK_BIN(button)->child);
  gtk_label_set_text(label, new_text);
}
GtkButton *egtk_box_add_button (GtkBox *box, 
                          const char *label, GtkSignalFunc action_when_clicked)
{
  GtkButton *button = GTK_BUTTON(gtk_button_new_with_label(label));
  gtk_widget_show(GTK_WIDGET(button));
  gtk_box_pack_start(box, GTK_WIDGET(button), FALSE, FALSE, 0);
  gtk_signal_connect(GTK_OBJECT(button), "clicked", action_when_clicked, 0);
  return button;
}
GtkToggleButton *egtk_box_add_toggle_button (GtkBox *box,
            const char *label, bool pressed, GtkSignalFunc action_when_toggled)
{
  GtkToggleButton *button = GTK_TOGGLE_BUTTON(
                 gtk_toggle_button_new_with_label(label));
  if (pressed) gtk_toggle_button_set_active(button, TRUE);
  gtk_widget_show(GTK_WIDGET(button));
  gtk_box_pack_start(box, GTK_WIDGET(button), FALSE, FALSE, 0);
  gtk_signal_connect_after(GTK_OBJECT(button), "toggled", 
                                       action_when_toggled, 0);
  return button;
}
GtkRadioButton *egtk_box_add_radio_button (GtkBox *box, GtkRadioButton *prev,
         const char *label, bool selected, GtkSignalFunc action_when_toggled)
{
  GtkRadioButton *button = GTK_RADIO_BUTTON(
     prev ? gtk_radio_button_new_with_label_from_widget(prev, label)
          : gtk_radio_button_new_with_label            (NULL, label));

  if (selected) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), TRUE);
  gtk_widget_show(GTK_WIDGET(button));
  gtk_box_pack_start(box, GTK_WIDGET(button), FALSE, FALSE, 0);
  gtk_signal_connect_after(GTK_OBJECT(button), "toggled", 
                                       action_when_toggled, 0);
  return button;
}
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
typedef struct { const char *text;
                 int   val1, val2; } eGtkOption;

GtkOptionMenu *egtk_box_add_option_menu (GtkBox *box, eGtkOption *optList,
                                  int N, GtkSignalFunc action_when_selected)
{
  GtkOptionMenu *optMenu = GTK_OPTION_MENU(gtk_option_menu_new());
  GtkWidget *menu = gtk_menu_new(), *item;     int i, iActive = 0;

  for (i = 0; optList->text != NULL; optList++) {
    if (optList->text[0] == '_') item = gtk_menu_item_new(); // separator
    else item = gtk_menu_item_new_with_label(optList->text);
    gtk_widget_show(item);
    gtk_menu_append(GTK_MENU(menu), item);
    gtk_signal_connect(GTK_OBJECT(item), "activate", 
                               action_when_selected, optList);
  }
  gtk_option_menu_set_menu(optMenu, menu);
  gtk_option_menu_set_history(optMenu, N);
  gtk_widget_show(GTK_WIDGET(menu)); 
  gtk_widget_show(GTK_WIDGET(optMenu));
  gtk_box_pack_start(box, GTK_WIDGET(optMenu), FALSE, FALSE, 0);
  return optMenu;
}
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
GtkWidget *egtk_box_add_display (GtkBox *box, const char *name,
                                              const char *initial_text)
{
  GtkWidget *label = gtk_label_new(name);
  gtk_widget_show(label);
  gtk_box_pack_start(box, label, FALSE, FALSE, 0);

  GtkWidget *display = gtk_label_new(initial_text);
  gtk_widget_show(display);
  gtk_box_pack_start(box, display, FALSE, FALSE, 0);
  return display;
}
void egtk_display_set_text (GtkWidget *display, const char *new_text)
{
  gtk_label_set_text(GTK_LABEL(display), new_text);
}
void egtk_display_set_number (GtkWidget *display, int N, const char *format)
{
  char buffer[16];    sprintf(buffer, format, N);
  gtk_label_set_text(GTK_LABEL(display), buffer);
}
/*---------------------------------------------------------------------------*/

GtkButton      *nextButton, *playButton, *fitButton;
GtkRadioButton *editButton, *zoomButton;

GtkWidget *magDisplay;

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
typedef enum {
  emEditing = 0,
  emZooming = 1
}
EditMode; EditMode eM = emZooming;

static void edit_or_zoom_button_toggled()
{
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(editButton))) {
    if (eM == emEditing) return;
    eM = emEditing;
  }
  else {
    if (eM == emZooming) return;
    eM = emZooming;
} }
static void fit_button_clicked()
{
  V->resize_to_fit();
  V->update();
  egtk_display_set_number(magDisplay, V->get_mag(), "%3d ");
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
typedef enum {
  pmStartup = 0,
  pmPaused  = 1,
  pmPlaying = 2,
  pmEditing = 3
}
PlayMode; PlayMode pM = pmStartup;

guint32 play_interval_msec = 50;
guint32 play_N_at_once     =  1;
gint timer_id = 0;

eGtkOption play_speed[] =
{
  { "fast",   50, 20 },
  { " 10 ",   50, 10 },
  { "  5 ",   50,  5 },
  { "  2 ",   50,  2 },
  { "  1 ",   50,  1 }, // default (4)
  { "____",   50,  1 },
  { " -1 ",  100,  1 },
  { " -2 ",  200,  1 },
  { " -3 ",  333,  1 },
  { "slow",  500,  1 }, { 0 }
};

#ifdef MEASURE_TIME
  struct timeval timeStartNextGen, timeStopNextGen;
  unsigned long minGenTime, maxGenTime, sumGenTime;
  unsigned long minGtkTime, maxGtkTime, sumGtkTime; int numSamples;
#endif

static gint timeout_callback(gpointer data_not_used)
{
#ifdef MEASURE_TIME
  struct timeval prevStartTime; prevStartTime = timeStartNextGen;
  unsigned long dt;
  gettimeofday(&timeStartNextGen, 0);   
#endif

  for (int i = 0; i < play_N_at_once; i++) M->nextGeneration(V);

#ifdef MEASURE_TIME
  gettimeofday(&timeStopNextGen, 0);
  dt = timeDiff_msec(timeStartNextGen, timeStopNextGen);
  if (dt < minGenTime) minGenTime = dt;
  if (dt > maxGenTime) maxGenTime = dt; sumGenTime += dt; 
  if (numSamples) {
    dt = timeDiff_msec(prevStartTime, timeStartNextGen);
    if (dt < minGtkTime) minGtkTime = dt;
    if (dt > maxGtkTime) maxGtkTime = dt; sumGtkTime += dt;
  }
  numGenerations++;
#endif
  return TRUE; // continue
}
static void next_button_clicked() { M->nextGeneration(V); }
static void play_button_clicked()
{
  if (pM != pmPlaying) {
    timer_id = gtk_timeout_add(play_interval_msec, timeout_callback,0);
                                                   timeout_callback(0);
#ifdef MEASURE_TIME
    minGenTime = minGtkTime = 2147483647;
    maxGenTime = maxGtkTime = 0;
    sumGenTime = sumGtkTime = 0;
    numSamples = 0;
#endif
    pM = pmPlaying;
    egtk_button_set_text(playButton, "Stop");
  }
  else {
    gtk_timeout_remove(timer_id);
#ifdef MEASURE_TIME
    if (numSamples) {
      printf("GenTime: %d..(%d)..%d\n", minGenTime, sumGenTime/numSamples,
                                        maxGenTime);
      if (numSamples > 1)
        printf("GtkTime: %d..(%d)..%d\n", minGtkTime,
               sumGtkTime/(numSamples-1), maxGtkTime);
    }
#endif
    pM = pmPaused;
    egtk_button_set_text(playButton, "Play");
} }
static void play_speed_changed (GtkOptionMenu *menu, eGtkOption *opt)
{
  play_interval_msec = opt->val1;
  play_N_at_once     = opt->val2;
  if (pM == pmPlaying) {
    gtk_timeout_remove(timer_id);
    timer_id = gtk_timeout_add(play_interval_msec, timeout_callback,0);
                                                   timeout_callback(0);
} }

GtkBox *createControlBar(void)
{
  GtkBox *hbox = GTK_BOX(gtk_hbox_new(/* homogeneous: */ FALSE,
                                      /* spacing:     */ 0));

  editButton = egtk_box_add_radio_button(hbox, NULL,       "Edit",
                            (eM == emEditing), edit_or_zoom_button_toggled);
  zoomButton = egtk_box_add_radio_button(hbox, editButton, "Zoom",
                            (eM == emZooming), edit_or_zoom_button_toggled);

  fitButton = egtk_box_add_button(hbox, "Fit ",  fit_button_clicked);
  magDisplay = egtk_box_add_display(hbox, " mag:", "  1 ");

  nextButton = egtk_box_add_button(hbox, "Next", next_button_clicked);
  playButton = egtk_box_add_button(hbox, "Play", play_button_clicked);

  egtk_box_add_label(hbox, " speed:");
  egtk_box_add_option_menu(hbox, 
                     play_speed, 4, GTK_SIGNAL_FUNC(play_speed_changed));

  gtk_widget_show(GTK_WIDGET(hbox));
  return hbox;
}
/*---------------------------------------------------------------------------*/

static void expose_event (GtkWidget *vista, GdkEventExpose *event)
{
#if 0
  printf("expose %d[%d]x%d[%d]\n", event->area.x, event->area.width,
                                   event->area.y, event->area.height);
#endif
  V->update(event->area.x,     event->area.y,
            event->area.width, event->area.height);
}
static void motion_event (GtkWidget *vista, GdkEventMotion *event)
{
#if 0
  printf("motion x=%f,y=%f\n", event->x, event->y);
#endif
}
static void button_event (GtkWidget *vista, GdkEventButton *event)
{
  int x_vis = (int)event->x, // 'visual' coordinates of the cursor
      y_vis = (int)event->y; // fraction part are not used (always zero)
#if 0
  printf("button[%d] %s x=%f,y=%f state=%x\n", event->button,
         (event->type == GDK_BUTTON_PRESS)   ? "press" :
         (event->type == GDK_2BUTTON_PRESS)  ? "2xPress" :
         (event->type == GDK_3BUTTON_PRESS)  ? "3xPress" :
         (event->type == GDK_BUTTON_RELEASE) ? "release" : "??",
         event->x, event->y, event->state);
#endif
  int N;
  switch (event->button) {
  case 1:
    if (event->type == GDK_BUTTON_PRESS &&
        (event->state & GDK_MOD1_MASK)) {
      M->toggle(V->Xvis2abs(x_vis), V->Yvis2abs(y_vis), 2);
      V->update();
    }
    else if (event->type == GDK_2BUTTON_PRESS) {
      V->center(V->Xvis2abs(x_vis), V->Yvis2abs(y_vis));
      V->update();
    }
    break;
  case 3:
    if (event->type == GDK_BUTTON_PRESS) {
      M->toggle(V->Xvis2abs(x_vis), V->Yvis2abs(y_vis), 2);
      V->update();
    }
    break;
  case 4: N = +1; goto resize_it;
  case 5: N = -1;      resize_it:
    if (event->type == GDK_BUTTON_PRESS && V->resize(N, x_vis, y_vis)) {
      V->update();
      egtk_display_set_number(magDisplay, V->get_mag(), "%3d ");
    }
    break;
  default:
    break;
  }
}
static void config_event (GtkWidget *vista, GdkEventConfigure *event)

{
#if 0
  printf("config %d[%d]x%d[%d]\n", event->x, event->width,
                                   event->y, event->height);
#endif
  if (V) V->resize();
}
GtkWidget *createDrawingArea (int min_width, int min_height)
{
  GtkWidget *vista = gtk_drawing_area_new();
  gtk_drawing_area_size(GTK_DRAWING_AREA(vista), min_width, min_height);
  gtk_widget_show(vista);
  gtk_widget_set_events(vista, GDK_EXPOSURE_MASK
   + GDK_POINTER_MOTION_MASK + GDK_POINTER_MOTION_HINT_MASK
   + GDK_BUTTON_PRESS_MASK   + GDK_BUTTON_RELEASE_MASK
   + GDK_STRUCTURE_MASK /* <-"configure-event" */ );

  gtk_signal_connect(GTK_OBJECT(vista), "expose-event",
         GTK_SIGNAL_FUNC(expose_event), NULL);
  gtk_signal_connect(GTK_OBJECT(vista), "motion-notify-event",
         GTK_SIGNAL_FUNC(motion_event), NULL);
  gtk_signal_connect(GTK_OBJECT(vista), "button-press-event",
         GTK_SIGNAL_FUNC(button_event), NULL);
  gtk_signal_connect(GTK_OBJECT(vista), "button-release-event",
         GTK_SIGNAL_FUNC(button_event), NULL);
  gtk_signal_connect(GTK_OBJECT(vista), "configure-event",
         GTK_SIGNAL_FUNC(config_event), NULL);
  return vista;
}

/*---------------------------------------------------------------------------*/

void gtki_start (int *pargc, char **pargv[])
{
  gtk_init(pargc, pargv);

  mainWin = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_widget_set_name(mainWin, "epiLife++");

  gtk_signal_connect(GTK_OBJECT(mainWin), "destroy",
               GTK_SIGNAL_FUNC(gtk_exit), NULL);

  GtkWidget *vbox = gtk_vbox_new(FALSE, 0);
  gtk_container_add(GTK_CONTAINER(mainWin), vbox);
  gtk_widget_show(vbox);

  drawingArea = createDrawingArea(640, 400);
  gtk_box_pack_start(GTK_BOX(vbox), drawingArea, TRUE, TRUE, 0);

  GtkWidget *ctrlBox = GTK_WIDGET(createControlBar());
  gtk_box_pack_start(GTK_BOX(vbox), ctrlBox, FALSE, FALSE, 0);

  gtk_widget_show(mainWin);
  elVista::initColorTable(mainWin->window);
}
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

int main (int argc, char *argv[])
{
  gtki_start(&argc, &argv);

  M = new_elMundoA();  
  V = new elVista(M, drawingArea);

  if (argc == 2) { int w = 100, h = 100; M->paste(argv[1], &w, &h);
                                         V->resize_to_fit();
         egtk_display_set_number(magDisplay, V->get_mag(), "%3d ");
  }
  gtk_main(); return 0;
}

