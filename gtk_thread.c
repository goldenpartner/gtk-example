#include <gtk/gtk.h>


typedef struct
{
  GtkWidget  *window;
  guint       progress_id;
} WorkerData;


static gboolean
worker_finish_in_idle (gpointer data)
{
  WorkerData *wd = data;
  
  /* we're done, stop updating the progress bar */
  g_source_remove (wd->progress_id);
  /* and destroy everything */
  gtk_widget_destroy (wd->window);
  g_free (wd);
  
  return FALSE; /* stop running */
}

static gpointer
worker (gpointer data)
{
  WorkerData *wd = data;
  
  /* hard work here */
  g_usleep (5000000);
  
  /* we finished working, do something back in the main thread */
  g_idle_add (worker_finish_in_idle, wd);
  
  return NULL;
}

static gboolean
update_progress_in_timeout (gpointer pbar)
{
  gtk_progress_bar_pulse (pbar);
  
  return TRUE; /* keep running */
}

static void
button_clicked_handler (GtkWidget  *button,
                        gpointer    data)
{
  WorkerData *wd;
  GThread    *thread;
  GtkWidget  *pbar;
  
  wd = g_malloc (sizeof *wd);
  
  wd->window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  
  pbar = gtk_progress_bar_new ();
  gtk_container_add (GTK_CONTAINER (wd->window), pbar);
  
  gtk_widget_show_all (wd->window);
  
  /* add a timeout that will update the progress bar every 100ms */
  wd->progress_id = g_timeout_add (100, update_progress_in_timeout, pbar);
  
  /* run the time-consuming operation in a separate thread */
  thread = g_thread_new ("worker", worker, wd);
  g_thread_unref (thread);
}

static gboolean
fill (gpointer   user_data)
{
	GtkWidget *progress_bar = user_data;

	/*Get the current progress*/
	gdouble fraction;
	fraction = gtk_progress_bar_get_fraction (GTK_PROGRESS_BAR (progress_bar));

	/*Increase the bar by 10% each time this function is called*/
	fraction += 0.05;

	/*Fill in the bar with the new fraction*/
	gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (progress_bar), fraction);

	/*Ensures that the fraction stays below 1.0*/
	if (fraction < 1.0) 
		return TRUE;
	
	return FALSE;
}


int
main (int     argc,
      char  **argv)
{
  GtkWidget  *window;
  GtkWidget  *button;
  GtkWidget *progress_bar, *box;

  gdouble fraction = 0.0;

  gtk_init (&argc, &argv);
  
  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_default_size (GTK_WINDOW (window), 500, 500);
  g_signal_connect (G_OBJECT (window), "destroy",
                    G_CALLBACK (gtk_main_quit), NULL);
  
  button = gtk_button_new_from_stock (GTK_STOCK_OK);
  g_signal_connect (G_OBJECT (button), "clicked",
                    G_CALLBACK (button_clicked_handler), NULL);



  box = gtk_box_new(TRUE, 10);
    gtk_box_pack_start(GTK_BOX(box), button, 0 ,0, 1);
	gtk_container_add (GTK_CONTAINER (window), box);
	
	/*Create a progressbar and add it to the window*/
	progress_bar = gtk_progress_bar_new ();
	gtk_box_pack_start(GTK_BOX(box), progress_bar, 0 ,0, 1);
	/*Fill in the given fraction of the bar. Has to be between 0.0-1.0 inclusive*/
	gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (progress_bar), fraction);

	/*Use the created fill function every 500 milliseconds*/
	g_timeout_add (500, fill, GTK_PROGRESS_BAR (progress_bar));
  
  gtk_widget_show_all (window);
  
  gtk_main ();
  
  return 0;
}