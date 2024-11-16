/*================================================================================
 * Wedget manager
 *================================================================================*/
#include <Arduino.h>

/*--------------------------------------------------------------------------------
 * Check the position of widget by RED rectangle
 *--------------------------------------------------------------------------------*/
#define CHECK_POSITION  false
#if     CHECK_POSITION
#define CHECK_POS(x)  x
#else
#define CHECK_POS(x)
#endif

/*--------------------------------------------------------------------------------
 * State of the screen
 *--------------------------------------------------------------------------------*/
typedef enum {
  STATE_OFF = 0,
  STATE_ON,
  STATE_MAIN,
  STATE_CONFIGURATION,
  STATE_RESOLUTION,
  STATE_THERMOGRAPH,
  STATE_FILE_MANAGER,
  STATE_CAPTURE_MODE,
  STATE_CALIBRATION,
  STATE_ADJUST_OFFSET,
  STATE_INFORMATION,
} State_t;

static State_t state = STATE_ON;

/*--------------------------------------------------------------------------------
 * Widget data definition
 *--------------------------------------------------------------------------------*/
typedef struct {
  const uint8_t   *data;  // pointer to the image
  const size_t    size;   // size of image data (only for TFT_eSPI with bitbank2/PNGdec)
} Image_t;

typedef struct Widget {
  const uint16_t  x, y;   // The top left coordinate of the widget
  const uint16_t  w, h;   // Widget width and height
  const Image_t   *image; // Widget image data
  const Event_t   event;  // The touch event to detect
  void            (*callback)(const struct Widget *widget, const Touch_t &touch);  // Event handler
} Widget_t;

#define N_WIDGETS(w)  (sizeof(w) / sizeof(w[0]))

/*--------------------------------------------------------------------------------
 * Focused widget
 *--------------------------------------------------------------------------------*/
static Widget_t const *focus = NULL;

/*--------------------------------------------------------------------------------
 * Event message to command initialization
 *--------------------------------------------------------------------------------*/
static constexpr Touch_t doInit = { EVENT_INIT, 0, 0 };

/*--------------------------------------------------------------------------------
 * Functions prototyping
 *--------------------------------------------------------------------------------*/
void widget_control(void);
void widget_setup(State_t screen = STATE_OFF);
State_t widget_state(State_t screen = STATE_OFF);

/*--------------------------------------------------------------------------------
 * Widgets
 *--------------------------------------------------------------------------------*/
#include "draw.hpp"
#include "widgets.hpp"

/*--------------------------------------------------------------------------------
 * Get widget table
 *--------------------------------------------------------------------------------*/
static bool widget_get(State_t screen, Widget_t const **widget, int *n) {
  switch (screen) {
    case STATE_MAIN:
      *widget = widget_main;
      *n = N_WIDGETS(widget_main);
      return true;

    case STATE_CONFIGURATION:
      *widget = widget_configuration;
      *n = N_WIDGETS(widget_configuration);
      return true;

    case STATE_RESOLUTION:
      *widget = widget_resolution;
      *n = N_WIDGETS(widget_resolution);
      return true;

    case STATE_THERMOGRAPH:
      *widget = widget_thermograph;
      *n = N_WIDGETS(widget_thermograph);
      return true;

    case STATE_FILE_MANAGER:
      *widget = widget_file_manager;
      *n = N_WIDGETS(widget_file_manager);
      return true;

    case STATE_CAPTURE_MODE:
      *widget = widget_capture_mode;
      *n = N_WIDGETS(widget_capture_mode);
      return true;

    case STATE_CALIBRATION:
      *widget = widget_calibration;
      *n = N_WIDGETS(widget_calibration);
      return true;

    case STATE_ADJUST_OFFSET:
      *widget = widget_adjust_offset;
      *n = N_WIDGETS(widget_adjust_offset);
      return true;

    case STATE_INFORMATION:
      *widget = widget_information;
      *n = N_WIDGETS(widget_information);
      return true;

    default:
      return false;
  }
}

/*--------------------------------------------------------------------------------
 * Handle the widget events on screen
 *--------------------------------------------------------------------------------*/
static bool widget_event(const Widget_t *widgets, const size_t n_widgets, Touch_t &touch) {
  Event_t event = touch.event;

  if (focus == NULL) {
    for (int i = 0; i < n_widgets; i++) {
      // In case the touch events to be handled
      if ((widgets[i].event & touch.event) && widgets[i].callback) {
        // Focus the widget where the event fired
        if (widgets[i].x <= touch.x && touch.x <= widgets[i].x + widgets[i].w &&
            widgets[i].y <= touch.y && touch.y <= widgets[i].y + widgets[i].h) {
          focus = &widgets[i];
          break;
        }
      }
    }
  }

  // execute the callback function for the focused widget
  if (focus && (focus->event & touch.event)) {
    DBG_EXEC(printf("event = %d(%d), x = %d, y = %d\n", touch.event, focus->event, touch.x, touch.y));
    touch.event = (Event_t)(touch.event & focus->event); // mask by target event
    focus->callback(focus, touch);
  }

  // reset focus when touch leaves screen
  focus = (event & EVENT_RISING ? NULL : focus);

  return (focus != NULL);
}

/*--------------------------------------------------------------------------------
 * Watch events on a specific screen
 *--------------------------------------------------------------------------------*/
static bool widget_watch(const Widget_t *widgets, const size_t n_widgets) {
  Touch_t touch;

  if (touch_event(touch)) {
    // Returns true when the touch event is handled by a widget
    if (widget_event(widgets, n_widgets, touch) == true) {
      return true;
    }
  }

  return false;
}

/*--------------------------------------------------------------------------------
 * Draw all widgets at the start of each state
 *--------------------------------------------------------------------------------*/
void widget_setup(State_t screen /* = STATE_OFF */) {
  int n;
  Widget_t const *widget;

  // reset focused widget
  focus = NULL;

  if (widget_get(screen, &widget, &n)) {
    for (int i = 0; i < n; i++, widget++) {
      if (widget->callback) {
        widget->callback(widget, doInit);
      }
      CHECK_POS(GFX_EXEC(drawRect(widget->x, widget->y, widget->w, widget->h, RED)));
    }
  }
}

/*--------------------------------------------------------------------------------
 * Change state
 *--------------------------------------------------------------------------------*/
State_t widget_state(State_t screen /*= STATE_OFF */) {
  state = screen;
  touch_clear();
  widget_setup(state);
  return state;
}

/*--------------------------------------------------------------------------------
 * Finite State Machines
 *--------------------------------------------------------------------------------*/
void widget_control(void) {
  int n;
  Widget_t const *widget;

  if (!widget_get(state, &widget, &n)) {
    state = STATE_ON;
  }

  State_t initial = state;
  switch (state) {
    case STATE_MAIN:
    case STATE_THERMOGRAPH:
      widget_watch(widget, n);
      break;

    case STATE_CONFIGURATION:
    case STATE_RESOLUTION:
    case STATE_FILE_MANAGER:
    case STATE_CAPTURE_MODE:
    case STATE_CALIBRATION:
    case STATE_ADJUST_OFFSET:
    case STATE_INFORMATION:
      do {
        widget_watch(widget, n);
        delay(1); // reset wdt
      } while (state == initial);
      break;

    case STATE_ON:
    default:
      widget_setup(state = STATE_MAIN);
      break;
  }
}