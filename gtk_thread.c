#include <gtk/gtk.h>
#include <stdlib.h>
typedef struct
{
	GtkWidget  *window;
	guint       progress_id;
} WorkerData;

WorkerData *wd = NULL;
GtkWidget *progress_bar;
GThread *thread = NULL;
gdouble fraction = 0.0;
int run = 0;

static gboolean
worker_finish_in_idle (gpointer data)
{
	WorkerData *w = data;
	if(w){
		/* we're done, stop updating the progress bar */
		g_source_remove (w->progress_id);
	}
	return FALSE; /* stop running */
}

static gpointer
worker (gpointer data)
{
	WorkerData *wd = data;
	
	while (fraction < 1 && run == 1){
		g_usleep (1e6);//pause for one second
		gfloat temp = gtk_progress_bar_get_fraction (GTK_PROGRESS_BAR (progress_bar));
		fraction = (fraction < temp) ? fraction : temp;
		fraction += 0.1;
		gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (progress_bar), fraction);
	}
	
	/* we finished working, do something back in the main thread */
	g_idle_add (worker_finish_in_idle, wd);
	
	return NULL;
}

static void
button_clicked_handler (GtkWidget  *button,
                        gpointer    data)
{
	GThread    *thread;
	if (wd){
		run = 0;
		g_usleep(1e6);
		fraction = 0;
		gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (progress_bar), fraction);
		wd = NULL;
		thread = NULL;
	}

	fraction = 0.0;
	run = 1;

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
	GtkWidget  *button1, *button2;
	GtkWidget  *box;

	gtk_init (&argc, &argv);
    
	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size (GTK_WINDOW (window), 500, 500);
	g_signal_connect (G_OBJECT (window), "destroy",
						G_CALLBACK (gtk_main_quit), NULL);
    
	button1 = gtk_button_new_with_label ("Start");

	/*Create a progressbar and add it to the window*/
	progress_bar = gtk_progress_bar_new ();

	g_signal_connect (G_OBJECT (button1), "clicked",
						G_CALLBACK (button_clicked_handler), window);

	box = gtk_box_new(TRUE, 10);
	gtk_box_pack_start(GTK_BOX(box), button1, 0 ,0, 1);
	gtk_box_pack_start(GTK_BOX(box), progress_bar, 0 ,0, 1);
	gtk_container_add (GTK_CONTAINER (window), box);

	gtk_widget_show_all (window);
    
	gtk_main ();
    
	return 0;
}
