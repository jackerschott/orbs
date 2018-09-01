#define COMPILE_GTK true
#if COMPILE_GTK

#include <gtk/gtk.h>
#include "../include/render.h"

GtkWidget *sbtn_rs;
GtkWidget *sbtn_rg;
GtkWidget *sbtn_nParticles;
GtkWidget *sbtn_rr;
GtkWidget *sbtn_rtheta;
GtkWidget *sbtn_rphi;
GtkWidget *sbtn_rdr;
GtkWidget *sbtn_rdtheta;
GtkWidget *sbtn_rdphi;
GtkAdjustment *adj_rg;
GtkAdjustment *adj_rs;
GtkAdjustment *adj_rr;

void on_window_main_destroy() {
  g_print("Exit\n");
  gtk_main_quit();
}

void on_btn_render_clicked() {
  g_print("Render\n");
}

void on_btn_ring_clicked() {
  g_print("Creating Ring...\n");
  uint nParticles = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(sbtn_nParticles));
  double rr = gtk_spin_button_get_value(GTK_SPIN_BUTTON(sbtn_rr));
  double rdr = gtk_spin_button_get_value(GTK_SPIN_BUTTON(sbtn_rdr));
  double rtheta = gtk_spin_button_get_value(GTK_SPIN_BUTTON(sbtn_rr));
  double rdtheta = gtk_spin_button_get_value(GTK_SPIN_BUTTON(sbtn_rdr));
  double rphi = gtk_spin_button_get_value(GTK_SPIN_BUTTON(sbtn_rr));
  double rdphi = gtk_spin_button_get_value(GTK_SPIN_BUTTON(sbtn_rdr));
  const uint nColors = 5;
  std::pair<color, double> *colorPalette = new std::pair<color, double>[nParticles];
  for (uint i = 0; i < nColors; i++) {
    colorPalette[i] = { { (byte)(rand() % 256), (byte)(rand() % 256), (byte)(rand() % 256) }, 0.2 };
  }
  createParticleRing(nParticles, rr, rtheta, rphi, rdr, rdtheta, rdphi, nColors, colorPalette);
  g_print("done creating Ring\n");
}

void on_adj_rx_changed() {
  double rg_value = gtk_adjustment_get_value(adj_rg);
  double rs_value = gtk_adjustment_get_value(adj_rs);
  double rr_value = gtk_adjustment_get_value(adj_rr);
  gtk_adjustment_set_upper(adj_rs, rg_value);
  gtk_adjustment_set_upper(adj_rr, rg_value);
  gtk_adjustment_set_lower(adj_rg, rs_value);
  gtk_adjustment_set_lower(adj_rr, rs_value);
  if (rr_value < rs_value) {
    gtk_adjustment_set_value(adj_rr, rs_value);
  }
  if (rr_value > rg_value) {
    gtk_adjustment_set_value(adj_rr, rg_value);
  }
}

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
  sbtn_rs = GTK_WIDGET(gtk_builder_get_object(builder, "sbtn_rs"));
  sbtn_rg = GTK_WIDGET(gtk_builder_get_object(builder, "sbtn_rg"));
  sbtn_nParticles = GTK_WIDGET(gtk_builder_get_object(builder, "sbtn_nParticles"));
  sbtn_rr = GTK_WIDGET(gtk_builder_get_object(builder, "sbtn_rr"));
  sbtn_rdr = GTK_WIDGET(gtk_builder_get_object(builder, "sbtn_rdr"));
  sbtn_rtheta = GTK_WIDGET(gtk_builder_get_object(builder, "sbtn_rtheta"));
  sbtn_rdtheta = GTK_WIDGET(gtk_builder_get_object(builder, "sbtn_rdtheta"));
  sbtn_rphi = GTK_WIDGET(gtk_builder_get_object(builder, "sbtn_rphi"));
  sbtn_rdphi = GTK_WIDGET(gtk_builder_get_object(builder, "sbtn_rdphi"));
  adj_rg = GTK_ADJUSTMENT(gtk_builder_get_object(builder, "adj_rg"));
  adj_rs = GTK_ADJUSTMENT(gtk_builder_get_object(builder, "adj_rs"));
  adj_rr = GTK_ADJUSTMENT(gtk_builder_get_object(builder, "adj_rr"));
  g_signal_connect(adj_rg, "value-changed", G_CALLBACK(on_adj_rx_changed), NULL);
  g_signal_connect(adj_rs, "value-changed", G_CALLBACK(on_adj_rx_changed), NULL);
  g_signal_connect(adj_rr, "value-changed", G_CALLBACK(on_adj_rx_changed), NULL);
  gtk_widget_show(window);
  gtk_main();
  return 0;
}

#else

#include "../include/render.h"

#define _USE_MATH_DEFINES

#include <iostream>
#include <math.h>
#include <random>

int main(int argc, char *argv[]) {
  double rs = 1.0;
  double rg = 10.0;

  const uint nParticles = 10;
  const uint nColors = 5;
  std::pair<color, double> *colorPalette = new std::pair<color, double>[nParticles];
  for (uint i = 0; i < nColors; i++) {
    colorPalette[i] = { { (byte)(rand() % 256), (byte)(rand() % 256), (byte)(rand() % 256) }, 0.2 };
  }

  initRender(rs, rg);
  createParticleRing(nParticles, 5 * rs, 0.0, M_PI_4, rs, 0.1, 0.1, nColors, colorPalette);

  std::cin.get();
  return 0;
}

#endif
