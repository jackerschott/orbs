#define COMPILE_GTK true
#if COMPILE_GTK

#include <gtk/gtk.h>
#include "render.h"

GtkWidget *window_main;
GtkAdjustment *adj_rg;
GtkAdjustment *adj_rs;
GtkWidget *btn_render;
GtkAdjustment *adj_nParticles;
GtkAdjustment *adj_rr;
GtkAdjustment *adj_rtheta;
GtkAdjustment *adj_rphi;
GtkAdjustment *adj_rdr;
GtkAdjustment *adj_rdtheta;
GtkAdjustment *adj_rdphi;
GtkWidget *btn_rcolors;
GtkWidget *btn_ring;

GtkWidget *window_colorPalette;
GtkWidget *cbt_select;
GtkWidget *btn_delete;
GtkAdjustment *adj_prob;
GtkWidget *ccwidget_color;
GtkWidget *btn_save;
GtkWidget *btn_close;

const uint nColors = 1;
std::pair<color, double> *colorPalette = new std::pair<color, double>[nColors];

void on_window_main_destroy() {
  g_print("Exit\n");
  gtk_main_quit();
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

void on_btn_render_clicked() {
  g_print("Render\n");
}

void on_btn_rcolors_clicked() {
  gtk_widget_show(window_colorPalette);
}

void on_btn_ring_clicked() {
  g_print("Creating Ring...\n");
  uint nParticles = (uint)gtk_adjustment_get_value(adj_nParticles);
  double rr = gtk_adjustment_get_value(adj_rr);
  double rtheta = gtk_adjustment_get_value(adj_rtheta);
  double rphi = gtk_adjustment_get_value(adj_rphi);
  double rdr = gtk_adjustment_get_value(adj_rdr);
  double rdtheta = gtk_adjustment_get_value(adj_rdtheta);
  double rdphi = gtk_adjustment_get_value(adj_rdphi);
  for (uint i = 0; i < nColors; i++) {
    colorPalette[i] = { { (byte)(rand() % 256), (byte)(rand() % 256), (byte)(rand() % 256) }, 0.2 };
  }
  //createParticleRing(nParticles, rr, rtheta, rphi, rdr, rdtheta, rdphi, nColors, colorPalette);
  g_print("done creating Ring\n");
}

void on_cbt_select_changed() {
  uint active = gtk_combo_box_get_active(GTK_COMBO_BOX(cbt_select));
  if (active == 0) { //new color
    //append text, change array, add new index, check delete-btn sensitive
  }
  else {
    //load color and prob, check delete-btn sensitive
  }
}

void on_btn_save_clicked() {
  GdkRGBA _gcolor;
  color _color;
  gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(ccwidget_color), &_gcolor);
  _color.r = (byte)255*_gcolor.red;
  _color.g = (byte)255*_gcolor.red;
  _color.b = (byte)255*_gcolor.red;
  
  uint active = gtk_combo_box_get_active(GTK_COMBO_BOX(cbt_select));
  colorPalette[active].first = _color;
  colorPalette[active].second = gtk_adjustment_get_value(adj_prob);
}

void on_btn_close_clicked() {
  //get save here, change delete-event and delete save-btn
  gtk_widget_hide(window_colorPalette);
}

int main(int argc, char *argv[]) {
  GtkBuilder *builder;
  GError *err = NULL;
  gtk_init(&argc, &argv);
  builder = gtk_builder_new();
  gtk_builder_add_from_file(builder, "gui/window_main.glade", &err);
  gtk_builder_add_from_file(builder, "gui/window_colorPalette.glade", &err);
  
  window_main = GTK_WIDGET(gtk_builder_get_object(builder, "window_main"));
  adj_rg = GTK_ADJUSTMENT(gtk_builder_get_object(builder, "adj_rg"));
  adj_rs = GTK_ADJUSTMENT(gtk_builder_get_object(builder, "adj_rs"));
  btn_render = GTK_WIDGET(gtk_builder_get_object(builder, "btn_render"));
  adj_nParticles = GTK_ADJUSTMENT(gtk_builder_get_object(builder, "adj_nParticles"));
  adj_rr = GTK_ADJUSTMENT(gtk_builder_get_object(builder, "adj_rr"));
  adj_rtheta = GTK_ADJUSTMENT(gtk_builder_get_object(builder, "adj_rtheta"));
  adj_rphi = GTK_ADJUSTMENT(gtk_builder_get_object(builder, "adj_rphi"));
  adj_rdr = GTK_ADJUSTMENT(gtk_builder_get_object(builder, "adj_rdr"));
  adj_rdtheta = GTK_ADJUSTMENT(gtk_builder_get_object(builder, "adj_rdtheta"));
  adj_rdphi = GTK_ADJUSTMENT(gtk_builder_get_object(builder, "adj_rdphi"));
  btn_rcolors = GTK_WIDGET(gtk_builder_get_object(builder, "btn_rcolors"));
  btn_ring = GTK_WIDGET(gtk_builder_get_object(builder, "btn_ring"));
  g_signal_connect(window_main, "destroy", G_CALLBACK(on_window_main_destroy), NULL);
  g_signal_connect(adj_rg, "value-changed", G_CALLBACK(on_adj_rx_changed), NULL);
  g_signal_connect(adj_rs, "value-changed", G_CALLBACK(on_adj_rx_changed), NULL);
  g_signal_connect(adj_rr, "value-changed", G_CALLBACK(on_adj_rx_changed), NULL);
  g_signal_connect(btn_render, "clicked", G_CALLBACK(on_btn_render_clicked), NULL);
  g_signal_connect(btn_rcolors, "clicked", G_CALLBACK(on_btn_rcolors_clicked), NULL);
  g_signal_connect(btn_ring, "clicked", G_CALLBACK(on_btn_ring_clicked), NULL);
  
  window_colorPalette = GTK_WIDGET(gtk_builder_get_object(builder, "window_colorPalette"));
  cbt_select = GTK_WIDGET(gtk_builder_get_object(builder, "cbt_select"));
  btn_delete = GTK_WIDGET(gtk_builder_get_object(builder, "btn_delete"));
  adj_prob = GTK_ADJUSTMENT(gtk_builder_get_object(builder, "adj_prob"));
  ccwidget_color = GTK_WIDGET(gtk_builder_get_object(builder, "ccwidget_color"));
  btn_save = GTK_WIDGET(gtk_builder_get_object(builder, "btn_save"));
  btn_close = GTK_WIDGET(gtk_builder_get_object(builder, "btn_close"));
  g_signal_connect(window_colorPalette, "delete-event", G_CALLBACK(gtk_widget_hide_on_delete), NULL);
  g_signal_connect(cbt_select, "changed", G_CALLBACK(on_cbt_select_changed), NULL);
  //g_signal_connect(btn_delete, "clicked", G_CALLBACK(on_btn_delete_clicked), NULL);
  //g_signal_connect(adj_prob, "value-changed", G_CALLBACK(on_adj_prob_changed), NULL);
  g_signal_connect(btn_save, "clicked", G_CALLBACK(on_btn_save_clicked), NULL);
  g_signal_connect(btn_close, "clicked", G_CALLBACK(on_btn_close_clicked), NULL);
  gtk_window_set_transient_for(GTK_WINDOW(window_colorPalette), GTK_WINDOW(window_main));
  
  gtk_widget_show(window_main);
  gtk_main();
  return 0;
}

#else

#include "render.h"

#define _USE_MATH_DEFINES

#include <iostream>
#include <fstream>
#include <math.h>
#include <random>


int main(int argc, char *argv[]) {
  double rs = 1.0;
  double gr = 10.0 * rs;

  const uint nParticles = 1000;
  const uint nColors = 5;
  std::pair<color, double> *colorPalette = new std::pair<color, double>[nParticles];
  for (uint i = 0; i < nColors; i++) {
    colorPalette[i] = { { (byte)(rand() % 256), (byte)(rand() % 256), (byte)(rand() % 256) }, 0.2 };
  }

  perspectiveCamera camera;
  camera.pos = { 15.0, -30.0, 15.0 };
  camera.lookDir = { -1.0, 2.0, -1.0 };
  camera.upDir = { 0.0, 0.0, 1.0 };
  camera.fov = 60.0;

  initRender(rs, rg);
  createParticleRing(nParticles, 5.0 * rs, { 0.0, -1.0, 1.0 }, 0.1 * rs, 0.1, nColors, colorPalette);
  setCamera(camera);
  renderConfig(1280, 720);
  byte* pixels = render();

  int w = 1280;
  int h = 720;
  std::ofstream out;
  out.open("E:\\Zwischenspeicher\\out");
  for (int i = 0; i < w * h * bpp / 8; i++) {
    out << pixels[i];
  }
  out.close();

  return 0;
}

#endif
