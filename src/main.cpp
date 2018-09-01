#define COMPILE_GTK false
#if COMPILE_GTK

#include <gtk/gtk.h>
#include "../include/render.h"

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
  const uint nParticles = 10;
  const uint nColors = 5;
  double rs = 3000;
  double gr = 10 * rs;
  std::pair<color, double> *colorPalette = new std::pair<color, double>[nParticles];
  for (uint i = 0; i < nParticles; i++) {
    colorPalette[i] = { { (byte)(rand() % 256), (byte)(rand() % 256), (byte)(rand() % 256) }, (double)rand() / RAND_MAX };
  }
  initRender(rs, gr);
  createParticleRing(nParticles, 5 * rs, 0.0, 0.78, rs, 0.1, 0.1, nColors, colorPalette);
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

#else

#include "../include/render.h"

#define _USE_MATH_DEFINES

#include <iostream>
#include <fstream>
#include <math.h>
#include <random>
#include <libpng16/png.h>
#include <libpng16/pngconf.h>
#include <libpng16/pnglibconf.h>

int main(int argc, char *argv[]) {
  double rs = 1.0;
  double rg = 10 * rs;

  const uint nParticles = 1000;
  const uint nColors = 5;
  std::pair<color, double> *colorPalette = new std::pair<color, double>[nParticles];
  for (uint i = 0; i < nParticles; i++) {
    colorPalette[i] = { { (byte)(rand() % 256), (byte)(rand() % 256), (byte)(rand() % 256) }, 0.2 };
  }

  perspectiveCamera camera;
  camera.pos = { 9.0, -4.0, 8.0 };
  camera.lookDir = { -9.0, 4.0, -8.0 };
  camera.upDir = { 0.0, 0.0, 1.0 };
  camera.fov = 60.0;

  initRender(rs, rg);
  createParticleRing(nParticles, 5 * rs, { 0.0, -1.0, 1.0 }, 0.1 * rs, 0.1, nColors, colorPalette);
  setCamera(camera);
  renderConfig(1280, 720);
  byte* pixels = render();

  //int w = 1280;
  //int h = 720;
  //std::ofstream out;
  //out.open("E:\\Zwischenspeicher\\out");
  //for (int i = 0; i < w * h * bpp / 8; i++) {
  //  out << pixels[i];
  //}
  //out.close();

  return 0;
}

#endif
