#ifndef VISUAL_H
#define VISUAL_H

void vis_setup();
void vis_cleanup();

void vis_set_aspect(float aspect);

void vis_pin_observer(float x, float y);
void vis_move_pinned_observer(float x, float y);

void vis_update();

#endif /* VISUAL_H */
