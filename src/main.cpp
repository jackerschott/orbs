#include <gtk/gtk.h>

void on_window_main_destroy() {
   gtk_main_quit();
}

void on_btn_render_clicked() {
   g_print("Render\n");
}

GtkWidget *sbtn_rr;
GtkWidget *sbtn_rdr;
GtkWidget *sbtn_rtheta;
GtkWidget *sbtn_rdtheta;
GtkWidget *sbtn_rphi;
GtkWidget *sbtn_rdphi;

void on_btn_ring_clicked() {
  double rr = gtk_spin_button_get_value(GTK_SPIN_BUTTON(sbtn_rr));
  double rdr = gtk_spin_button_get_value(GTK_SPIN_BUTTON(sbtn_rdr));
  double rtheta = gtk_spin_button_get_value(GTK_SPIN_BUTTON(sbtn_rr));
  double rdtheta = gtk_spin_button_get_value(GTK_SPIN_BUTTON(sbtn_rdr));
  double rphi = gtk_spin_button_get_value(GTK_SPIN_BUTTON(sbtn_rr));
  double rdphi = gtk_spin_button_get_value(GTK_SPIN_BUTTON(sbtn_rdr));
   g_print("Create Ring\n");
}

//gulong g_signal_connect_data(gpointer	instance, const gchar	*detailed_signal, GCallback	c_handler, gpointer	data, GClosureNotify destroy_data, GConnectFlags connect_flags) {
//	return 0;
//}
//
//GTypeInstance* g_type_check_instance_cast(GTypeInstance *instance, GType iface_type) {
//	return 0;
//}
//
//void g_print(const gchar *format, ...) G_GNUC_PRINTF(1, 2) {
//
//}

int main(int argc, char *argv[]) {
  GtkBuilder *builder;
  GtkWidget *window;
  GtkWidget *btn_render;
  GtkWidget *btn_ring;
	GError *err = NULL;
	gtk_init(&argc, &argv);
	builder = gtk_builder_new();
	gtk_builder_add_from_file(builder, "gui/window_main.glade", &err);
	if (err != NULL) {
		fprintf(stderr, "Error adding build from file. Error: %s\n", err->message);
		g_error_free(err);
		return 1;
	}
	window = GTK_WIDGET(gtk_builder_get_object(builder, "window_main"));
  g_signal_connect(window, "destroy", G_CALLBACK(on_window_main_destroy), NULL);
  btn_render = GTK_WIDGET(gtk_builder_get_object(builder, "btn_render"));
  g_signal_connect(btn_render, "clicked", G_CALLBACK(on_btn_render_clicked), NULL);
  btn_ring = GTK_WIDGET(gtk_builder_get_object(builder, "btn_ring"));
  g_signal_connect(btn_ring, "clicked", G_CALLBACK(on_btn_ring_clicked), NULL);
  sbtn_rr = GTK_WIDGET(gtk_builder_get_object(builder, "sbtn_rr"));
  sbtn_rdr = GTK_WIDGET(gtk_builder_get_object(builder, "sbtn_rdr"));
  sbtn_rtheta = GTK_WIDGET(gtk_builder_get_object(builder, "sbtn_rtheta"));
  sbtn_rdtheta = GTK_WIDGET(gtk_builder_get_object(builder, "sbtn_rdtheta"));
  sbtn_rphi = GTK_WIDGET(gtk_builder_get_object(builder, "sbtn_rphi"));
  sbtn_rdphi = GTK_WIDGET(gtk_builder_get_object(builder, "sbtn_rdphi"));
  gtk_widget_show(window);
  gtk_main();
  return 0;
}
