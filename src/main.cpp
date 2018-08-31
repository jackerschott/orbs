#include <gtk/gtk.h>

void on_window_main_destroy()
{
  gtk_main_quit();
}

void on_btn_render_clicked()
{
  g_print ("Render\n");
}

void on_btn_ring_clicked()
{
  g_print ("Create Ring\n");
}

int main(int argc, char *argv[])
{
  GtkBuilder *builder;
  GtkWidget *window;
  GtkWidget *btn_render;
  GtkWidget *btn_ring;
  gtk_init(&argc, &argv);
  builder = gtk_builder_new();
  gtk_builder_add_from_file (builder, "gui/window_main.glade", NULL);
  window = GTK_WIDGET(gtk_builder_get_object(builder, "window_main"));
  g_signal_connect(window, "destroy", G_CALLBACK(on_window_main_destroy), NULL);
  btn_render = GTK_WIDGET(gtk_builder_get_object(builder, "btn_render"));
  g_signal_connect(btn_render, "clicked", G_CALLBACK(on_btn_render_clicked), NULL);
  btn_ring = GTK_WIDGET(gtk_builder_get_object(builder, "btn_ring"));
  g_signal_connect(btn_ring, "clicked", G_CALLBACK(on_btn_ring_clicked), NULL);
  gtk_widget_show(window);
  gtk_main();
  return 0;
}
