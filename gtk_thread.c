#include <gtk/gtk.h>



typedef struct
{
	GtkWidget  *window;
	guint       progress_id;
} WorkerData;

WorkerData *wd = NULL;
GtkWidget *progress_bar;
gdouble fraction = 0.0;

static gboolean
worker_finish_in_idle (gpointer data)
{
	WorkerData *wd = data;
	if(wd){
		/* we're done, stop updating the progress bar */
		g_source_remove (wd->progress_id);
		/* and destroy everything */
		g_free (wd);
  	}
  
  
  	return FALSE; /* stop running */
}

static gpointer
worker (gpointer data)
{
  	WorkerData *wd = data;
	
	/* hard work here */
	while (fraction < 1){
			g_usleep (1000000);
			gfloat temp = gtk_progress_bar_get_fraction (GTK_PROGRESS_BAR (progress_bar));
			fraction = (fraction < temp) ? fraction : temp;

			fraction += 0.1;
			//gdk_threads_add_idle ();

			gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (progress_bar), fraction);
	}
	
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
static gboolean
fill (gpointer   user_data)
{
    
	GtkWidget *progress_bar = user_data;

	/*Ensures that the fraction stays below 1.0*/
	if (fraction < 1.0) 
		return TRUE;
	
	return FALSE;
}

static void
button_clicked_handler (GtkWidget  *button,
                        gpointer    data)
{
    fraction = 0.0;
    
    GThread    *thread;
    if (wd){
        g_source_remove(wd->progress_id);
    }
    wd = (WorkerData*)g_malloc (sizeof (WorkerData));
    wd->window = data;
        
    /* run the time-consuming operation in a separate thread */
    thread = g_thread_new ("worker", worker, wd);
    g_thread_unref (thread);
    
}

int
main (int     argc,
      char  **argv)
{
    GtkWidget  *window;
    GtkWidget  *button;
    GtkWidget  *box;

    

    gtk_init (&argc, &argv);
    
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size (GTK_WINDOW (window), 500, 500);
    g_signal_connect (G_OBJECT (window), "destroy",
                        G_CALLBACK (gtk_main_quit), NULL);
    
    button = gtk_button_new_from_stock (GTK_STOCK_OK);
    

	/*Create a progressbar and add it to the window*/
	progress_bar = gtk_progress_bar_new ();

    g_signal_connect (G_OBJECT (button), "clicked",
                    G_CALLBACK (button_clicked_handler), window);

    box = gtk_box_new(TRUE, 10);
    gtk_box_pack_start(GTK_BOX(box), button, 0 ,0, 1);
	gtk_box_pack_start(GTK_BOX(box), progress_bar, 0 ,0, 1);
	gtk_container_add (GTK_CONTAINER (window), box);

    gtk_widget_show_all (window);
    
    gtk_main ();
    
    return 0;
}