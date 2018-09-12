#define COMPILE_UI true
#if COMPILE_UI

#include <cmath>
#include <thread>
#include <gtk/gtk.h>

#include "render.hpp"

GtkWidget *window_main;
GtkImage *img_main;
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
GtkWidget *btn_close;

uint w = 920;
uint h = 690;
byte activeColor = 1;
uint nColors = 1;
probColor *colorPalette = new probColor[nColors];

bool useHardwAcc = false;
bool on_cbt_select_changed_codeCall = false;

void on_key_press(GtkWidget *widget, GdkEventKey *event);
void on_key_release(GtkWidget *widget, GdkEventKey *event);

void on_adj_rx_changed();

void on_btn_render_clicked();
void on_btn_rcolors_clicked();
void on_btn_ring_clicked();

void on_cbt_select_changed();
void on_btn_close_clicked();

void on_window_main_destroy();


void save_color(byte index);
void load_color(byte index);

void renderFrame();


int main(int argc, char *argv[]) {
  GtkBuilder *builder;
  GError *err = NULL;
  gtk_init(&argc, &argv);
  builder = gtk_builder_new();
  gtk_builder_add_from_file(builder, "gui/window_main.glade", &err);
  gtk_builder_add_from_file(builder, "gui/window_colorPalette.glade", &err);

  window_main = GTK_WIDGET(gtk_builder_get_object(builder, "window_main"));
  img_main = GTK_IMAGE(gtk_builder_get_object(builder, "img_main"));
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
  g_signal_connect(window_main, "key-press-event", G_CALLBACK(on_key_press), NULL);
  g_signal_connect(window_main, "key-release-event", G_CALLBACK(on_key_release), NULL);
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
  btn_close = GTK_WIDGET(gtk_builder_get_object(builder, "btn_close"));
  gtk_window_set_transient_for(GTK_WINDOW(window_colorPalette), GTK_WINDOW(window_main));
  g_signal_connect(window_colorPalette, "delete-event", G_CALLBACK(on_btn_close_clicked), NULL);
  g_signal_connect(cbt_select, "changed", G_CALLBACK(on_cbt_select_changed), NULL);
  //g_signal_connect(btn_delete, "clicked", G_CALLBACK(on_btn_delete_clicked), NULL);
  //g_signal_connect(adj_prob, "value-changed", G_CALLBACK(on_adj_prob_changed), NULL);
  g_signal_connect(btn_close, "clicked", G_CALLBACK(on_btn_close_clicked), NULL);

  render::init(1.0, 10.0);
  render::initHardwAcc(cl::Platform::getDefault(), cl::Device::getDefault());
  render::clearParticleRings();
  renderFrame();

  gtk_widget_show(window_main);
  gtk_main();
  return 0;
}

void on_key_press(GtkWidget *widget, GdkEventKey *event) {

  // g_print(gdk_keyval_name(event->keyval));

  switch (event->keyval) {
  case GDK_KEY_w: case GDK_KEY_Up:
    g_print("forward\n");
    break;
  case GDK_KEY_a: case GDK_KEY_Left:
    g_print("left\n");
    break;
  case GDK_KEY_s: case GDK_KEY_Down:
    g_print("backward\n");
    break;
  case GDK_KEY_d: case GDK_KEY_Right:
    g_print("right\n");
    break;
  case GDK_KEY_Delete: {
    render::clearParticleRings();
    renderFrame();
  }
  default:
    g_print("else\n");
  }
}
void on_key_release(GtkWidget *widget, GdkEventKey *event) {
  switch (event->keyval) {
  case GDK_KEY_w: case GDK_KEY_Up:
    g_print("stop forward\n");
    break;
  case GDK_KEY_a: case GDK_KEY_Left:
    g_print("stop left\n");
    break;
  case GDK_KEY_s: case GDK_KEY_Down:
    g_print("stop backward\n");
    break;
  case GDK_KEY_d: case GDK_KEY_Right:
    g_print("stop right\n");
    break;
  default:
    g_print("released else\n");
  }
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

void on_btn_rcolors_clicked() {
  gtk_widget_show(window_colorPalette);
}
void on_cbt_select_changed() {
  if (on_cbt_select_changed_codeCall)
    return;
  on_cbt_select_changed_codeCall = true;

  save_color(activeColor);
  activeColor = gtk_combo_box_get_active(GTK_COMBO_BOX(cbt_select));
  if (activeColor == 0) {
    probColor *newColorPalette = new probColor[nColors + 1];
    for (uint i = 0; i < nColors; i++) {
      newColorPalette[i] = colorPalette[i];
    }
    newColorPalette[nColors] = { 1.0, 1.0, 1.0, 0.0 }; //random here
    delete[] colorPalette;
    colorPalette = newColorPalette;
    activeColor = ++nColors;
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(cbt_select), ("Color " + std::to_string(activeColor)).c_str());
    gtk_combo_box_set_active(GTK_COMBO_BOX(cbt_select), activeColor);
    load_color(activeColor);
  }
  else {
    load_color(activeColor);
  }

  on_cbt_select_changed_codeCall = false;
}
void on_btn_close_clicked() {
  save_color(activeColor);
  gtk_widget_hide(window_colorPalette);
}

void on_btn_render_clicked() {
  //char* btn_lbl;
  //if (rendering) {
  //  gtk_button_set_label(GTK_BUTTON(btn_render), "Render");
  //  rendering = false;
  //}
  //else {
  //  gtk_button_set_label(GTK_BUTTON(btn_render), "Reset");
  //  rendering = true;
  //}
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

  perspectiveCamera camera;
  camera.pos = { 30.0, 0.0, 0.0 };
  camera.lookDir = { -1.0, 0.0, 0.0 };
  camera.upDir = { 0.0, 0.0, 1.0 };
  camera.fov = 60.0;
  render::setCamera(camera);
  vector rn = { sin(rtheta) * cos(rphi), sin(rtheta) * sin(rphi), cos(rtheta) };
  render::createParticleRing(nParticles, rr, rn, rdr, rdtheta, rdphi, 5, colorPalette);

  renderFrame();
}

void on_window_main_destroy() {
  g_print("Exit\n");
  gtk_main_quit();
}


void save_color(byte index) {
  index--;
  GdkRGBA gcolor;
  gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(ccwidget_color), &gcolor);
  colorPalette[index].r = gcolor.red;
  colorPalette[index].g = gcolor.green;
  colorPalette[index].b = gcolor.blue;
  colorPalette[index].p = gtk_adjustment_get_value(adj_prob);
}
void load_color(byte index) {
  index--;
  GdkRGBA gcolor;
  gcolor.red = colorPalette[index].r;
  gcolor.green = colorPalette[index].g;
  gcolor.blue = colorPalette[index].b;
  gcolor.alpha = 1.0;
  gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(ccwidget_color), &gcolor);
  gtk_adjustment_set_value(adj_prob, colorPalette[index].p);
}

void renderFrame() {
  render::config(w, h, false);
  render::render();
  
  std::thread waiter([] () {
    while (render::isRendering()) {};
    g_print("Waiting succeded!\n");
    GBytes* gPixels = g_bytes_new(render::getImageData(), render::sPixels);
    GdkPixbuf* pixbuf = gdk_pixbuf_new_from_bytes(gPixels, GDK_COLORSPACE_RGB, false, 8, w, h, w * render::bpp / 8);
    g_print("Data gathered.\n");
    gtk_image_set_from_pixbuf(img_main, pixbuf);
  });
  g_print("Waiting...\n");
  waiter.detach();
}

#else

#include "randutils.hpp"

#define _USE_MATH_DEFINES

#include <cmath>
#include <iostream>
#include <fstream>
#include <random>
#include <climits>

#include "tmain.hpp"

typedef unsigned int dword;
typedef unsigned long long qword;

namespace render {
  extern uint sPixels;
}

int main(int argc, char *argv[]) {
  uint w = 1920;
  uint h = 1080;
  double rs = 1.0;
  double rg = 10.0 * rs;

  const uint nParticles = 5000;

  perspectiveCamera camera;
  camera.pos = { 30.0, 0.0, 0.0 };
  camera.lookDir = { -1.0, 0.0, 0.0 };
  camera.upDir = { 0.0, 0.0, 1.0 };
  camera.fov = 60.0;

  probColor *colorPalette = new probColor[1];
  for (uint i = 0; i < 1; i++) {
    colorPalette[i] = { 1.0, 1.0, 1.0, 1.0 };
  }
  render::init(rs, rg);
  render::initHardwAcc(cl::Platform::getDefault(), cl::Device::getDefault());
  render::createParticleRing(nParticles, 5.0 * rs, { 1.0, -1.0, 1.0 }, 0.1 * rs, 0.1, 0.1, 1, colorPalette);
  render::setCamera(camera);

#if false
  uint nFrames = 100;
  render::config(w, h, false);
  std::chrono::high_resolution_clock::time_point tc1 = std::chrono::high_resolution_clock::now();
  for (uint i = 0; i < nFrames; i++) {
    render::render();
    while (render::isRendering()) {};
  }
  std::chrono::high_resolution_clock::time_point tc2 = std::chrono::high_resolution_clock::now();
  double cRenderTime = std::chrono::duration_cast<std::chrono::nanoseconds>(tc2 - tc1).count() * 1.0e-9;
  double cFrameRate = nFrames / cRenderTime;
  std::cout << "CPU frame rate: " << cFrameRate << " fps" << std::endl;

  render::config(w, h, true);
  std::chrono::high_resolution_clock::time_point tg1 = std::chrono::high_resolution_clock::now();
  for (uint i = 0; i < nFrames; i++) {
    render::render();
    while (render::isRendering()) {};
  }
  std::chrono::high_resolution_clock::time_point tg2 = std::chrono::high_resolution_clock::now();
  double gRenderTime = std::chrono::duration_cast<std::chrono::nanoseconds>(tg2 - tg1).count() * 1.0e-9;
  double gFrameRate = nFrames / gRenderTime;
  std::cout << "GPU frame rate: " << gFrameRate << " fps" << std::endl;

  std::cin.get();
#elif false
  uint nFrames = 100;
  uint nTests = 1;
  double *frameRates = new double[nTests];
  for (int i = 0; i < nTests; i++) {
    render::config(w, h, false);
    std::chrono::high_resolution_clock::time_point tg1 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < nFrames; i++) {
      render::render();
      while (render::isRendering()) {};
    }
    std::chrono::high_resolution_clock::time_point tg2 = std::chrono::high_resolution_clock::now();
    double gRenderTime = std::chrono::duration_cast<std::chrono::nanoseconds>(tg2 - tg1).count() * 1.0e-9;
    frameRates[i] = nFrames / gRenderTime;
    std::cout << "CPU frame rate: " << frameRates[i] << " fps, " << i << std::endl;
  }

  double frameRateMean = 0.0;
  double frameRateStdErrMean = 0.0;
  for (int i = 0; i < nTests; i++) {
    frameRateMean += frameRates[i];
  }
  frameRateMean /= nTests;
  for (int i = 0; i < nTests; i++) {
    frameRateStdErrMean += (frameRates[i] - frameRateMean) * (frameRates[i] - frameRateMean);
  }
  frameRateStdErrMean = sqrt(frameRateStdErrMean / (nTests - 1));

  std::cout << std::endl;
  std::cout << "CPU mean frame rate:\t\t\t\t\t" << frameRateMean << " fps" << std::endl;
  std::cout << "CPU standard deviation of frame rate mean value:\t" << frameRateStdErrMean << " fps" << std::endl;

  delete[] frameRates;
  std::cin.get();
#else
  render::config(w, h, false);
  render::render();
  std::thread waiter([]() {
    while (render::isRendering()) {};
    std::this_thread::sleep_for(std::chrono::milliseconds(30000));
    std::cout << "Waiting succeded!\n";
  });

  std::cout << "before" << std::endl;
  waiter.detach();
  std::cout << "after" << std::endl;

  while (true)
  {

  }

  byte* pixels = render::getImageData();
  for (uint i = 0; i < render::sPixels; i++) {
    if (pixels[i] != 0)
      std::cout << i << "\t" << (int)pixels[i] << std::endl;
  }
  render::close();
#endif

  return 0;
}

#endif
