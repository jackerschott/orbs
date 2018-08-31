#include <gtk/gtk.h>

void on_window_main_destroy()
{
  gtk_main_quit();
}

void on_button_render_clicked()
{
  g_print ("Render\n");
}

void on_button_ring_clicked()
{
  g_print ("Create Ring\n");
}

int main(int argc, char *argv[])
{
  GtkBuilder *builder;
  GtkWidget *window;
  GtkWidget *button_render;
  GtkWidget *button_ring;
  gtk_init(&argc, &argv);
  builder = gtk_builder_new();
  gtk_builder_add_from_file (builder, "gui/window_main.glade", NULL);
  window = GTK_WIDGET(gtk_builder_get_object(builder, "window_main"));
  g_signal_connect(window, "destroy", G_CALLBACK(on_window_main_destroy), NULL);
  button_render = GTK_WIDGET(gtk_builder_get_object(builder, "button_render"));
  g_signal_connect(button_render, "clicked", G_CALLBACK(on_button_render_clicked), NULL);
  button_ring = GTK_WIDGET(gtk_builder_get_object(builder, "button_ring"));
  g_signal_connect(button_ring, "clicked", G_CALLBACK(on_button_ring_clicked), NULL);
  gtk_widget_show(window);
  gtk_main();
  return 0;
}
