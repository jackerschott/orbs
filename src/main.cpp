#define TEST_WITHOUT_GTK false
#if !TEST_WITHOUT_GTK

#include <cmath>
#include <fstream>
#include <gtk/gtk.h>
#include <GL/glew.h>
#ifdef _WIN32
#include <GL/wglew.h>
#endif
#ifdef __unix__
#include <GL/glxew.h>
#endif
#include <iostream>
#include <thread>
#include <gtk/gtk.h>

#include "res.hpp"
#include "render.hpp"

#include "trender.hpp" // test render file up until gtk migration

// MainWindow ui elements
GtkWidget *window_main;
GtkMenuBar *menu_bar;
GtkMenuItem *mi_userPref;
GtkWidget *gla_out;
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

// ColorPaletteWindow ui elements
GtkWidget* window_colorPalette;
GtkWidget* cbt_select;
GtkWidget* btn_new;
GtkWidget* btn_delete;
GtkAdjustment* adj_prob;
GtkWidget* ccwidget_color;
GtkWidget* btn_close;

// User preferences window ui elements
GtkWidget* window_userPreferences;
GtkButton* btnSaveUserPreferences;
GtkStack* stackDevices;
GtkComboBoxText* cbt_cp_devices;
GtkComboBoxText* cbt_gp_devices;

bool is_on_btn_new_clicked_user_call = true;
bool is_on_cbt_select_changed_user_call = true;

// Variables for render specifications
uint w = 920;
uint h = 690;
byte activeColor = 0;
uint nColors = 1;
color* colorPalette = new color[nColors];
bool useHardwAcc = true;

GdkPixbuf* framePixBuf;
std::thread renderWaiter;

cl::Platform clPlatform;
cl::Device clDevice;

// Function declerations for event and helper functions
void on_gla_render(GtkGLArea *glArea, GdkGLContext *glContext);
void on_mi_userPref_activate();
void on_key_press(GtkWidget* widget, GdkEventKey* event);
void on_key_release(GtkWidget* widget, GdkEventKey* event);
void on_adj_rx_changed();
void on_btn_rcolors_clicked();
void on_btn_render_clicked();
void on_btn_ring_clicked();
void on_window_main_destroy();
void on_btn_new_clicked();
void on_cbt_select_changed();
void on_btn_close_clicked();
void save_color(byte index);
void load_color(byte index);
void on_btnSaveUserPreferences_clicked();

// Entry point
int main(int argc, char* argv[]) {
  GtkBuilder* builder;
  GError* err = NULL;
  gtk_init(&argc, &argv);
  builder = gtk_builder_new();
  gtk_builder_add_from_file(builder, GIT_FOLDER_PATH "gui/window_main.glade", &err);
  gtk_builder_add_from_file(builder, GIT_FOLDER_PATH "gui/window_colorPalette.glade", &err);
  gtk_builder_add_from_file(builder, GIT_FOLDER_PATH "gui/window_userPreferences.glade", &err);

  window_main = GTK_WIDGET(gtk_builder_get_object(builder, "window_main"));
  gla_out = GTK_WIDGET(gtk_builder_get_object(builder, "gla_out"));
  mi_userPref = GTK_MENU_ITEM(gtk_builder_get_object(builder, "mi_userPref"));
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
  g_signal_connect(gla_out, "render", G_CALLBACK(on_gla_render), NULL);
  g_signal_connect(mi_userPref, "activate", G_CALLBACK(on_mi_userPref_activate), NULL);
  g_signal_connect(adj_rg, "value-changed", G_CALLBACK(on_adj_rx_changed), NULL);
  g_signal_connect(adj_rs, "value-changed", G_CALLBACK(on_adj_rx_changed), NULL);
  g_signal_connect(adj_rr, "value-changed", G_CALLBACK(on_adj_rx_changed), NULL);
  g_signal_connect(btn_render, "clicked", G_CALLBACK(on_btn_render_clicked), NULL);
  g_signal_connect(btn_rcolors, "clicked", G_CALLBACK(on_btn_rcolors_clicked), NULL);
  g_signal_connect(btn_ring, "clicked", G_CALLBACK(on_btn_ring_clicked), NULL);

  window_colorPalette = GTK_WIDGET(gtk_builder_get_object(builder, "window_colorPalette"));
  cbt_select = GTK_WIDGET(gtk_builder_get_object(builder, "cbt_select"));
  btn_new = GTK_WIDGET(gtk_builder_get_object(builder, "btn_new"));
  btn_delete = GTK_WIDGET(gtk_builder_get_object(builder, "btn_delete"));
  adj_prob = GTK_ADJUSTMENT(gtk_builder_get_object(builder, "adj_prob"));
  ccwidget_color = GTK_WIDGET(gtk_builder_get_object(builder, "ccwidget_color"));
  btn_close = GTK_WIDGET(gtk_builder_get_object(builder, "btn_close"));
  gtk_window_set_transient_for(GTK_WINDOW(window_colorPalette), GTK_WINDOW(window_main));
  g_signal_connect(window_colorPalette, "delete-event", G_CALLBACK(on_btn_close_clicked), NULL);
  g_signal_connect(cbt_select, "changed", G_CALLBACK(on_cbt_select_changed), NULL);
  g_signal_connect(btn_new, "clicked", G_CALLBACK(on_btn_new_clicked), NULL);
  g_signal_connect(btn_close, "clicked", G_CALLBACK(on_btn_close_clicked), NULL);

  window_userPreferences = GTK_WIDGET(gtk_builder_get_object(builder, "window_userPreferences"));
  btnSaveUserPreferences = GTK_BUTTON(gtk_builder_get_object(builder, "btnSaveUserPreferences"));
  stackDevices = GTK_STACK(gtk_builder_get_object(builder, "stackDevices"));
  cbt_cp_devices = GTK_COMBO_BOX_TEXT(gtk_builder_get_object(builder, "cbt_cp_devices"));
  cbt_gp_devices = GTK_COMBO_BOX_TEXT(gtk_builder_get_object(builder, "cbt_gp_devices"));
  g_signal_connect(btnSaveUserPreferences, "clicked", G_CALLBACK(on_btnSaveUserPreferences_clicked), NULL);

  gtk_gl
  gtk_gl_area_make_current(area_render);

  cl_context_properties platform = (cl_context_properties)cl::Platform::getDefault()();
#ifdef _WIN32
  cl_context_properties contextProps[] = {
    CL_GL_CONTEXT_KHR, (cl_context_properties)wglGetCurrentContext(),
    CL_WGL_HDC_KHR, (cl_context_properties)wglGetCurrentDC(),
    CL_CONTEXT_PLATFORM, platform,
    0
  };
#endif
#ifdef __unix__
    cl_context_properties contextProps[] = {
      CL_GL_CONTEXT_KHR, (cl_context_properties)glXGetCurrentContext(),
      CL_GLX_DISPLAY_KHR, (cl_context_properties)glXGetCurrentDrawable(),
      CL_CONTEXT_PLATFORM, platform(),
      0
  };
#endif
  cl::Device device = cl::Device::getDefault();
  cl::Context context(device, contextProps);

  float rs = 1.0f;
  const uint nParticles = 100000;
  camera obsCamera;
  obsCamera.pos = { 30.0f, 0.0f, 0.0f };
  obsCamera.lookDir = -obsCamera.pos;
  obsCamera.upDir = { 0.0f, 0.0f, 1.0f };
  obsCamera.fov = glm::radians(60.0f);
  obsCamera.aspect = 16.0f / 9.0f;
  obsCamera.zNear = 0.1f;
  obsCamera.zFar = 100.0f;

  int bgWidth;
  int bgHeight;
  int bgBpp;
  std::ifstream ifs("E:/tmp/sphere_map", std::ios::binary);
  ifs.read(reinterpret_cast<char*>(&bgWidth), sizeof(int));
  ifs.read(reinterpret_cast<char*>(&bgHeight), sizeof(int));
  ifs.read(reinterpret_cast<char*>(&bgBpp), sizeof(int));
  uint sBgImageData = bgWidth * bgHeight * bgBpp / 8;
  byte* bgImageData = new byte[sBgImageData];
  ifs.read(reinterpret_cast<char*>(bgImageData), sBgImageData);

  render::init(device, context, 1.0f);
  render::clearParticleRings();
  render::setObserverCamera(obsCamera);
  render::setBackgroundTex(sBgImageData, bgImageData, bgWidth, bgBpp, bgBpp);
  render::renderClassic();

  /*std::vector<cl::Platform> platforms;
  cl::Platform::get(&platforms);
  byte platform_id = 0;
  byte device_id = 0;
  for(std::vector<cl::Platform>::iterator it = platforms.begin(); it != platforms.end(); ++it){
    cl::Platform platform(*it);
    gtk_menu_bar_bar_append(menu_bar);
  }*/

  clPlatform = cl::Platform::getDefault();
  clDevice = cl::Device::getDefault();

  gtk_widget_show(window_main);
  gtk_main();
  return 0;
}

// MainWindow events
void on_gla_render(GtkGLArea *glArea, GdkGLContext *glContext) {
  g_print("render call\n");
  cl_platform_id clPlatformId = clPlatform();
  cl_context_properties clContextProps[] = {
    CL_GL_CONTEXT_KHR, (cl_context_properties)glContext,
    CL_CONTEXT_PLATFORM, (cl_context_properties)clPlatformId,
    0
  };
  cl::Context clContext(clDevice, clContextProps);
  trender(clDevice, clContext);
}

void on_mi_userPref_activate() {
  std::vector<cl::Platform> platforms;
  cl::Platform::get(&platforms);
  uint nDevices = 0;
  for (int i = 0; i < platforms.size(); i++) {
    std::vector<cl::Device> platformGPDevices;
    platforms[i].getDevices(CL_DEVICE_TYPE_GPU, &platformGPDevices);
    nDevices += (uint)platformGPDevices.size();
    for (int i = 0; i < platformGPDevices.size(); i++) {
      gtk_combo_box_text_append_text(cbt_gp_devices, platformGPDevices[i].getInfo<CL_DEVICE_NAME>().c_str());
    }
  }
  if (nDevices > 0)
    gtk_combo_box_set_active(GTK_COMBO_BOX(cbt_gp_devices), 0);
  gtk_widget_show(window_userPreferences);
  gtk_stack_set_visible_child(stackDevices, GTK_WIDGET(cbt_gp_devices));
}
void on_key_press(GtkWidget* widget, GdkEventKey* event) {
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
    render::renderClassic();
  }
  default:
    g_print("else\n");
  }
}
void on_key_release(GtkWidget* widget, GdkEventKey* event) {
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
  float rg_value = (float)gtk_adjustment_get_value(adj_rg);
  float rs_value = (float)gtk_adjustment_get_value(adj_rs);
  float rr_value = (float)gtk_adjustment_get_value(adj_rr);
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
  float rr = (float)gtk_adjustment_get_value(adj_rr);
  float rtheta = (float)gtk_adjustment_get_value(adj_rtheta);
  float rphi = (float)gtk_adjustment_get_value(adj_rphi);
  float rdr = (float)gtk_adjustment_get_value(adj_rdr);
  float rdtheta = (float)gtk_adjustment_get_value(adj_rdtheta);
  float rdphi = (float)gtk_adjustment_get_value(adj_rdphi);

  vector rn = { sin(rtheta) * cos(rphi), sin(rtheta) * sin(rphi), cos(rtheta) };
  render::createParticleRing(nParticles, rr, rn, rdr, rdtheta, rdphi, 5, nullptr);
  render::renderClassic();
}
void on_window_main_destroy() {
  g_print("Exit\n");
  gtk_main_quit();
}

// ColorPaletteWindow events
void on_btn_new_clicked() {
  if (!is_on_btn_new_clicked_user_call)
    return;
  is_on_btn_new_clicked_user_call = false;
  is_on_cbt_select_changed_user_call = false;

  std::cout << "on_btn_new_clicked: active color = " << (int)activeColor << std::endl;
  save_color(activeColor);
  color *newColorPalette = new color[nColors + 1];
  for (uint i = 0; i < nColors; i++) {
    newColorPalette[i] = colorPalette[i];
  }
  newColorPalette[nColors] = { 0, 0, 0, 0 };
  delete[] colorPalette;
  colorPalette = newColorPalette;
  activeColor = nColors++;
  std::cout << "on_btn_new_clicked: active color = " << (int)activeColor << std::endl;
  load_color(activeColor);
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(cbt_select), ("Color " + std::to_string(nColors)).c_str());
  gtk_combo_box_set_active(GTK_COMBO_BOX(cbt_select), activeColor - 1);

  is_on_cbt_select_changed_user_call = true;
  is_on_btn_new_clicked_user_call = true;
}
void on_cbt_select_changed() {
  if (!is_on_cbt_select_changed_user_call)
    return;
  is_on_cbt_select_changed_user_call = false;

  save_color(activeColor);
  std::cout << "on_cbt_select_changed: activeColor = " << (int)activeColor << std::endl;
  activeColor = gtk_combo_box_get_active(GTK_COMBO_BOX(cbt_select));
  std::cout << "on_cbt_select_changed: activeColor = " << (int)activeColor << std::endl;
  load_color(activeColor);

  is_on_cbt_select_changed_user_call = true;
}
void on_btn_close_clicked() {
  save_color(activeColor);
  gtk_widget_hide(window_colorPalette);
}

// ColorPaletteWindow helper functions
void save_color(byte index) {
  GdkRGBA gcolor;
  gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(ccwidget_color), &gcolor);
  colorPalette[index].r = (float)gcolor.red;
  colorPalette[index].g = (float)gcolor.green;
  colorPalette[index].b = (float)gcolor.blue;
  colorPalette[index].a = (float)gtk_adjustment_get_value(adj_prob);

  std::cout << std::endl;
  std::cout << "save_color: colorPalette[index].r = " << colorPalette[index].r << std::endl;
  std::cout << "save_color: colorPalette[index].g = " << colorPalette[index].g << std::endl;
  std::cout << "save_color: colorPalette[index].b = " << colorPalette[index].b << std::endl;
  std::cout << std::endl;
}
void load_color(byte index) {
  GdkRGBA gcolor;
  gcolor.red = colorPalette[index].r;
  gcolor.green = colorPalette[index].g;
  gcolor.blue = colorPalette[index].b;
  gcolor.alpha = 1.0;
  gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(ccwidget_color), &gcolor);
  gtk_adjustment_set_value(adj_prob, colorPalette[index].a);

  std::cout << std::endl;
  std::cout << "load_color: colorPalette[index].r = " << colorPalette[index].r << std::endl;
  std::cout << "load_color: colorPalette[index].g = " << colorPalette[index].g << std::endl;
  std::cout << "load_color: colorPalette[index].b = " << colorPalette[index].b << std::endl;
  std::cout << std::endl;
}

void on_btnSaveUserPreferences_clicked() {
  // todo
}

#else
#include "tmain.hpp"

int main(int argc, char** argv) {
  return tmain(argc, argv);
}
#endif
