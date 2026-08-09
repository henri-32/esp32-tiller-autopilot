#include "window_handling.h"
#include <cmath>
#include <cstring>

int Window::handle(int event)
//{{{
{

  if (event != FL_KEYDOWN && event != FL_SHORTCUT)
  {
    return Fl_Window::handle(event);
  }

  if (Fl::event_key() == '+')
  {
    if (++ctx_->ihData.target_course >= 360)
    {
      ctx_->ihData.target_course %= 360;
    }
    ctx_->ihData.target_course_synced = false;
    refresh_target_course_box();
    return 1;
  }

  else if (Fl::event_key() == '-')
  {
    if (--ctx_->ihData.target_course < 0)
    {
      ctx_->ihData.target_course += 360;
    }
    ctx_->ihData.target_course_synced = false;
    refresh_target_course_box();
    return 1;
  }

  else if (strcmp(Fl::event_text(), "E") == 0 || strcmp(Fl::event_text(), "e") == 0)
  {
    ctx_->ihData.steering_engaged = !ctx_->ihData.steering_engaged;
    ctx_->ihData.steering_engaged_synced = false;
    refresh_engage_box();
    return 1;
  }

  else if (strcmp(Fl::event_text(), "S") == 0 || strcmp(Fl::event_text(), "s") == 0)
  {
    synchronize_data_with_ap(this);

    const char* text = "Autopilot data synchronized successfully to navigation Monitor";
    write_to_info_box(text);

    Fl::remove_timeout(hide_box, this);
    Fl::add_timeout(2, hide_box, this);

    return 1;
  }

  return Fl_Window::handle(event);
};
//}}}

void Window::display_AccumulatedLogMessage_values()
//{{{
{
  AccumulatedLogMessage msg = ctx_->ALM;

  const auto snapshot = msg.snapshot;
  const auto sog = msg.snapshot.gps_sog_kts;
  const auto cog = msg.snapshot.gps_cog_dg;

  //================================ Check if ihData is synced to autopilot ================
  if (ctx_->ihData.target_course == snapshot.target_course)
  {
    ctx_->ihData.target_course_synced = true;
  }
  else
  {
    ctx_->ihData.target_course_synced = false;
  }

  if (ctx_->ihData.steering_engaged == snapshot.steering_engaged)
  {
    ctx_->ihData.steering_engaged_synced = true;
  }
  else
  {
    ctx_->ihData.steering_engaged_synced = false;
  }

  char text[128];

  //================================ Print SOG Box =========================================
  if (sog.valid)
  {
    snprintf(text, sizeof(text), "SOG: %.1f kts", sog.value);
  }
  else
  {
    snprintf(text, sizeof(text), "SOG: invalid");
  }
  sog_box_->copy_label(text);
  sog_box_->redraw();

  //================================ Print COG Box =========================================
  if (cog.valid)
  {
    snprintf(text, sizeof(text), "COG: %d deg", cog.value);
  }
  else
  {
    snprintf(text, sizeof(text), "COG: invalid");
  }
  cog_box_->copy_label(text);
  cog_box_->redraw();

  //================================ Print LAT/LON Box =========================================
  if (msg.error.validFix)
  {
    format_coordinate_for_display(text, sizeof(text), "LAT", snapshot.gps_lat, 2, 'N', 'S');
  }
  else
  {
    snprintf(text, sizeof(text), "LAT: invalid");
  }
  latitude_box_->copy_label(text);
  latitude_box_->redraw();

  if (msg.error.validFix)
  {
    format_coordinate_for_display(text, sizeof(text), "LON", snapshot.gps_lon, 3, 'E', 'W');
  }
  else
  {
    snprintf(text, sizeof(text), "LON: invalid");
  }
  longitude_box_->copy_label(text);
  longitude_box_->redraw();

  //================================ Print Target Box =========================================
  refresh_target_course_box();

  //=============================== Print Engage Box ============================
  refresh_engage_box();
}
//}}}

void Window::synchronize_data_with_ap(void* WindowInstance)
//{{{
{
  auto* window = static_cast<Window*>(WindowInstance);
  auto ctx = window->ctx_;
  ctx->ihData.target_course = ctx->ALM.snapshot.target_course;
  ctx->ihData.target_course_synced = true;
  ctx->ihData.steering_engaged = ctx->ALM.snapshot.steering_engaged;
  ctx->ihData.steering_engaged_synced = true;
}
//}}}

Fl_Box* Window::make_info_box()
{
  //{{{
  auto* box = new Fl_Box(10, 10, 860, 190);
  box->box(FL_UP_BOX);
  box->labelfont(FL_TIMES);
  box->labelsize(20);
  box->labeltype(FL_NORMAL_LABEL);
  return box;
}
//}}}

void Window::initialize_value_boxes()
//{{{
{
  static auto box = [&](int col, int row)
  {
    return make_standard_box(margin_ + col * (width_ + gap_),
                             begin_of_info_part_ + margin_ + row * (height_ + gap_), width_,
                             height_);
  };

  info_box_ = make_info_box();
  target_box_ = box(0, 0);
  engage_box_ = box(1, 0);
  sog_box_ = box(0, 2);
  cog_box_ = box(1, 2);
  latitude_box_ = box(0, 3);
  longitude_box_ = box(1, 3);

  add(sog_box_);
  add(cog_box_);
  add(target_box_);
  add(latitude_box_);
  add(longitude_box_);
  add(engage_box_);
  add(info_box_);
  info_box_->hide();
  show();
}
//}}}

void Window::hide_box(void* Window_ptr)
//{{{
{
  auto* window = static_cast<Window*>(Window_ptr);
  window->info_box_->hide();
}
//}}}

void Window::refresh_target_course_box()
//{{{
{
  char text[30];
  if (ctx_->ihData.target_course_synced)
  {
    snprintf(text, sizeof(text), "TARGET: %d deg \n", ctx_->ihData.target_course);
  }
  else
  {
    snprintf(text, sizeof(text), "TARGET: %d deg sync \n", ctx_->ihData.target_course);
  }
  target_box_->copy_label(text);
  target_box_->redraw();
}
//}}}

void Window::refresh_engage_box()
//{{{
{
  char text[20];
  if (ctx_->ihData.steering_engaged)
  {
    if (ctx_->ihData.steering_engaged_synced)
    {
      snprintf(text, sizeof(text), "ENGAGED");
      engage_box_->labelcolor(FL_DARK_GREEN);
    }
    else
    {
      snprintf(text, sizeof(text), "sync");
      engage_box_->labelcolor(FL_FOREGROUND_COLOR);
    }
  }
  else
  {
    if (ctx_->ihData.steering_engaged_synced)
    {
      snprintf(text, sizeof(text), "DISENGAGED");
      engage_box_->labelcolor(FL_DARK_RED);
    }
    else
    {
      snprintf(text, sizeof(text), "sync");
      engage_box_->labelcolor(FL_FOREGROUND_COLOR);
    }
  }

  engage_box_->copy_label(text);
  engage_box_->redraw();
}
//}}}

void Window::write_to_info_box(const char* text)
//{{{
{
  info_box_->copy_label(text);
  info_box_->show();
  info_box_->redraw();
}
//}}}

void Window::format_coordinate_for_display(char* text, std::size_t text_size, const char* label,
                                           float decimal_degrees, int degree_width,
                                           char positive_hemisphere, char negative_hemisphere)
//{{{
{
  const double absolute_degrees = std::fabs(static_cast<double>(decimal_degrees));
  int degrees = static_cast<int>(absolute_degrees);
  double decimal_minutes = (absolute_degrees - degrees) * 60.0;

  // Keep rounding from ever producing an invalid "60.000 minutes" display.
  decimal_minutes = std::round(decimal_minutes * 1000.0) / 1000.0;
  if (decimal_minutes >= 60.0)
  {
    decimal_minutes = 0.0;
    ++degrees;
  }

  const char hemisphere = std::signbit(decimal_degrees) ? negative_hemisphere : positive_hemisphere;
  snprintf(text, text_size, "%s: %0*d° %06.3f' %c", label, degree_width, degrees, decimal_minutes,
           hemisphere);
}
//}}}
