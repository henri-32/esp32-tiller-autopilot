#pragma once

#include "protocol/autopilotWireProtocol.h"
#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Window.H>
#include <cstring>

// Shared state read and updated by the UI and socket callbacks.
struct WindowContext
//{{{
{
  AccumulatedLogMessage ALM;
  InputHandleData_t ihData{};
};
//}}}
//
class Window : public Fl_Window
//{{{
{
public:
  // Builds the window around externally owned application state.
  Window(int x, int y, const char* name, WindowContext* context)
      : Fl_Window(x, y, name), ctx_(context)
  {
    initialize_value_boxes();
	Fl::add_timeout(1, synchronize_data_with_ap, this);
  };

  // Handles keyboard input for changing the target course.
  int handle(int event) override;

  // Refreshes all telemetry boxes from the current window context.
  void display_AccumulatedLogMessage_values();
  
  // Reads the snapshot values into the local state
  static void synchronize_data_with_ap(void* WindowInstance);

  WindowContext* ctx_;

private:
  // Creates the box for displayed_information
  Fl_Box* make_info_box();

  // Creates and arranges the monitor's value boxes.
  void initialize_value_boxes();

  //Callback adapter to hide windows
  static void hide_box(void* Window_ptr);

  // Updates the target-course box to reflect the current synchronization state.
  void refresh_target_course_box();
  void refresh_engage_box();
  void write_to_info_box(const char* text);

  // Formats decimal degrees as degrees and minutes with a hemisphere.
  void format_coordinate_for_display(char* text, std::size_t text_size, const char* label,
                                     float decimal_degrees, int degree_width,
                                     char positive_hemisphere, char negative_hemisphere);

  static constexpr int begin_of_info_part_ = 200;
  static constexpr int margin_ = 15;
  static constexpr int gap_ = 10;
  static constexpr int width_ = 280;
  static constexpr int height_ = 60;

  Fl_Box* sog_box_;
  Fl_Box* cog_box_;
  Fl_Box* target_box_;
  Fl_Box* latitude_box_;
  Fl_Box* longitude_box_;
  Fl_Box* engage_box_;
  Fl_Box* info_box_;
};
//}}}

// Creates a consistently styled value box for the monitor grid.
inline Fl_Box* make_standard_box(int x, int y, int w, int h)
//{{{
{
  auto* box = new Fl_Box(x, y, w, h, "Waiting for source data");
  box->box(FL_UP_BOX);
  box->labelfont(FL_ITALIC);
  box->labelsize(20);
  box->labeltype(FL_NORMAL_LABEL);
  return box;
}
//}}}
