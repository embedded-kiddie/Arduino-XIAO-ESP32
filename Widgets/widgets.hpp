/*================================================================================
 * Widget definitions for each screen
 *================================================================================*/
#include <Arduino.h>
#include "widgets.h"

#define DEMO_MODE true
#if     DEMO_MODE
#include "demo.h"
#endif

#if     false
#define DBG_FUNC(x) DBG_EXEC(x)
#else
#define DBG_FUNC(x)
#endif

/*--------------------------------------------------------------------------------
 * MLX90640 configuration and Widget control da
 *--------------------------------------------------------------------------------*/
extern MLXConfig_t    mlx_cnf;
extern MLXCapture_t   mlx_cap;
extern TouchConfig_t  tch_cnf;

static MLXConfig_t    mlx_copy;
static MLXConfig_t    cnf_copy;
static TouchConfig_t  tch_copy;
static TouchConfig_t  tch_ajst;

/*--------------------------------------------------------------------------------
 * Image data of widget
 *--------------------------------------------------------------------------------*/
static constexpr Image_t image_main[]               = { { screen_main,          sizeof(screen_main         ) }, }; // 320 x 240
static constexpr Image_t image_configuration[]      = { { screen_configuration, sizeof(screen_configuration) }, }; // 320 x 240
static constexpr Image_t image_resolution[]         = { { screen_resolution,    sizeof(screen_resolution   ) }, }; // 320 x 240
static constexpr Image_t image_thermograph[]        = { { screen_thermograph,   sizeof(screen_thermograph  ) }, }; // 320 x 240
static constexpr Image_t image_file_manager[]       = { { screen_file_manager,  sizeof(screen_file_manager ) }, }; // 320 x 240
static constexpr Image_t image_capture_mode[]       = { { screen_capture_mode,  sizeof(screen_capture_mode ) }, }; // 320 x 240
static constexpr Image_t image_calibration[]        = { { screen_calibration,   sizeof(screen_calibration  ) }, }; // 320 x 240
static constexpr Image_t image_adjust_offset[]      = { { screen_adjust_offset, sizeof(screen_adjust_offset) }, }; // 270 x  60
static constexpr Image_t image_information[]        = { { screen_information,   sizeof(screen_information  ) }, }; // 320 x 240
static constexpr Image_t image_icon_configuration[] = { { icon_configuration,   sizeof(icon_configuration  ) }, }; //  50 x  50

static constexpr Image_t image_icon_apply[] = {
  { icon_apply_off, sizeof(icon_apply_off) }, // 30 x 30
  { icon_apply_on,  sizeof(icon_apply_on ) }, // 30 x 30
};
static constexpr Image_t image_icon_close[] = {
  { icon_close_on,  sizeof(icon_close_on ) }, // 30 x 30
};
static constexpr Image_t image_icon_camera[] = {
  { icon_camera1, sizeof(icon_camera1) }, // 50 x 50
  { icon_camera2, sizeof(icon_camera2) }, // 50 x 50
  { icon_video,   sizeof(icon_video  ) }, // 50 x 50
  { icon_stop,    sizeof(icon_stop   ) }, // 50 x 50
};
static constexpr Image_t image_radio[] = {
  { radio_off, sizeof(radio_off) }, // 26 x 26
  { radio_on,  sizeof(radio_on ) }, // 26 x 26
};
static constexpr Image_t image_toggle[] = {
  { toggle_off, sizeof(toggle_off) }, // 42 x 26
  { toggle_on,  sizeof(toggle_on ) }, // 42 x 26
};
static constexpr Image_t image_arrowL[] = {
  { icon_arrow_Loff, sizeof(icon_arrow_Loff) }, // 26 x 26
  { icon_arrow_Lon,  sizeof(icon_arrow_Lon ) }, // 26 x 26
};
static constexpr Image_t image_arrowR[] = {
  { icon_arrow_Roff, sizeof(icon_arrow_Roff) }, // 26 x 26
  { icon_arrow_Ron,  sizeof(icon_arrow_Ron ) }, // 26 x 26
};
static constexpr Image_t image_arrowU[] = {
  { icon_arrow_Uoff, sizeof(icon_arrow_Uoff) }, // 26 x 26
  { icon_arrow_Uon,  sizeof(icon_arrow_Uon ) }, // 26 x 26
};
static constexpr Image_t image_arrowD[] = {
  { icon_arrow_Doff, sizeof(icon_arrow_Doff) }, // 26 x 26
  { icon_arrow_Don,  sizeof(icon_arrow_Don ) }, // 26 x 26
};
static constexpr Image_t image_checkbox[] = {
  { icon_checkbox_off, sizeof(icon_checkbox_off) }, // 26 x 26
  { icon_checkbox_on,  sizeof(icon_checkbox_on ) }, // 26 x 26
};
static constexpr Image_t image_movie[] = {
  { icon_movie_off, sizeof(icon_movie_off) }, // 32 x 26
  { icon_movie_on,  sizeof(icon_movie_on ) }, // 32 x 26
};
static constexpr Image_t image_folder[] = {
  { icon_folder_off, sizeof(icon_folder_off) }, // 32 x 26
  { icon_folder_on,  sizeof(icon_folder_on ) }, // 32 x 26
};
static constexpr Image_t image_save_flash[] = {
  { save_flash_off, sizeof(save_flash_off) }, // 70 x 70
  { save_flash_on,  sizeof(save_flash_on ) }, // 70 x 70
  { save_flash_red, sizeof(save_flash_red) }, // 70 x 70
};
static constexpr Image_t image_target[] = {
  { target_off, sizeof(target_off) }, // 32 x 32
  { target_on,  sizeof(target_on ) }, // 32 x 32
};
#if defined (DEMO_MODE) && DEMO_MODE
static constexpr Image_t image_demo[] = {
  { screen_demo0, sizeof(screen_demo0) }, // 8:1 (256 x 192)
  { screen_demo1, sizeof(screen_demo1) }, // 6:1 (192 x 144)
  { screen_demo2, sizeof(screen_demo2) }, // 4:1 (128 x  96)
  { screen_demo3, sizeof(screen_demo3) }, // 2:1 ( 64 x  48)
  { screen_demo4, sizeof(screen_demo4) }, // 1:1 ( 32 x  24)
  { screen_demo5, sizeof(screen_demo5) }, // 2:1 ( 64 x  48)
};
#endif

/*--------------------------------------------------------------------------------
 * Slider model
 * <------------------ Bar width -------------------->
 *      <- width ->                   <- width ->
 * +---+----------+------------------+----------+---+ ^
 * |   |    ||    |                  |    ||    |   | |
 * |   |   Knob   |                  |   Knob   |   | Bar height = Knob height
 * |   | (pos_min)|                  | (pos_max)|   | | (including background)
 * +---+----------+------------------+----------+---+ v
 * <--->                                        <--->
 * SLIDER_KNOB_OFFSET               SLIDER_KNOB_OFFSET
 * "||" ... The center of the knob (touching point)
 *--------------------------------------------------------------------------------*/
#define SLIDER_KNOB_OFFSET  6 // Offset from both ends of the bar

static constexpr Image_t image_slider1[] = {
  { slider_bar1, sizeof(slider_bar1) }, // 160 x 26
  { slider_knob, sizeof(slider_knob) }, //  26 x 26
};
static constexpr Image_t image_slider2[] = {
  { slider_bar2,     sizeof(slider_bar2    ) }, // 238 x 26
  { slider_knob,     sizeof(slider_knob    ) }, //  26 x 26
  { slider_knob_off, sizeof(slider_knob_off) }, //  26 x 26
};

/*--------------------------------------------------------------------------------
 * List of widget data based on the screen
 * The callback function may be something this:
 * static void onCallbackFunction(const Widget_t *widget, const Touch_t &touch) {
 *   if (touch.event == EVENT_INIT) {
 *     // Setup Process
 *   } else {
 *     // Event Process
 *   }
 * }
 *--------------------------------------------------------------------------------*/
// Screen - Main
static void onMainScreen        (const Widget_t *widget, const Touch_t &touch);
static void onMainInside        (const Widget_t *widget, const Touch_t &touch);
static void onMainOutside       (const Widget_t *widget, const Touch_t &touch);
static void onMainThermograph   (const Widget_t *widget, const Touch_t &touch);
static void onMainCapture       (const Widget_t *widget, const Touch_t &touch);
static void onMainConfiguration (const Widget_t *widget, const Touch_t &touch);

static constexpr Widget_t widget_main[] = {
  {   0,   0, 320, 240, image_main,        EVENT_NONE, onMainScreen        },
  {   0,   0, 256, 192, NULL,              EVENT_ALL,  onMainInside        },
  { 258,   0,  62, 134, NULL,              EVENT_ALL,  onMainOutside       },
  {   0, 195, 256,  45, NULL,              EVENT_ALL,  onMainThermograph   },
  { 265, 135,  50,  50, image_icon_camera, EVENT_UP,   onMainCapture       },
  { 265, 185,  50,  50, NULL,              EVENT_ALL,  onMainConfiguration },
#if defined (DEMO_MODE) && DEMO_MODE
  {   0,   0, 320, 240, image_demo,        EVENT_NONE, nullptr             },
#endif
};

// Screen - Configuration
static void onConfigurationScreen       (const Widget_t *widget, const Touch_t &touch);
static void onConfigurationResolution   (const Widget_t *widget, const Touch_t &touch);
static void onConfigurationThermograph  (const Widget_t *widget, const Touch_t &touch);
static void onConfigurationCaptureMode  (const Widget_t *widget, const Touch_t &touch);
static void onConfigurationFileManager  (const Widget_t *widget, const Touch_t &touch);
static void onConfigurationCalibration  (const Widget_t *widget, const Touch_t &touch);
static void onConfigurationInformation  (const Widget_t *widget, const Touch_t &touch);
static void onConfigurationReturn       (const Widget_t *widget, const Touch_t &touch);

static constexpr Widget_t widget_configuration[] = {
  {   0,   0, 320, 240, image_configuration, EVENT_NONE, onConfigurationScreen      },
  {  24,  12,  70,  70, NULL,                EVENT_ALL,  onConfigurationResolution  },
  { 125,  12,  70,  70, NULL,                EVENT_ALL,  onConfigurationThermograph },
  { 226,  12,  70,  70, NULL,                EVENT_ALL,  onConfigurationCaptureMode },
  {  24, 104,  70,  70, NULL,                EVENT_ALL,  onConfigurationFileManager },
  { 125, 104,  70,  70, NULL,                EVENT_ALL,  onConfigurationCalibration },
  { 226, 104,  70,  70, NULL,                EVENT_ALL,  onConfigurationInformation },
  { 145, 206,  30,  30, NULL,                EVENT_ALL,  onConfigurationReturn      },
};

// Screen - Resolution
static void onResolutionScreen  (const Widget_t *widget, const Touch_t &touch);
static void onResolutionSlider1 (const Widget_t *widget, const Touch_t &touch);
static void onResolutionSlider2 (const Widget_t *widget, const Touch_t &touch);
static void onResolutionClose   (const Widget_t *widget, const Touch_t &touch);
static void onResolutionApply   (const Widget_t *widget, const Touch_t &touch);

static constexpr Widget_t widget_resolution[] = {
  {   0,   0, 320, 240, image_resolution, EVENT_NONE,  onResolutionScreen  },
  { 138,  45, 160,  26, image_slider1,    EVENT_DRAG,  onResolutionSlider1 },
  { 138, 105, 160,  26, image_slider1,    EVENT_DRAG,  onResolutionSlider2 },
  {  60, 206,  30,  30, NULL,             EVENT_ALL,   onResolutionClose   },
  { 230, 206,  30,  30, image_icon_apply, EVENT_CLICK, onResolutionApply   },
};

// Screen - Thermograph
static void onThermographScreen   (const Widget_t *widget, const Touch_t &touch);
static void onThermographRadio1   (const Widget_t *widget, const Touch_t &touch);
static void onThermographRadio2   (const Widget_t *widget, const Touch_t &touch);
static void onThermographToggle1  (const Widget_t *widget, const Touch_t &touch);
static void onThermographToggle2  (const Widget_t *widget, const Touch_t &touch);
static void onThermographSlider1  (const Widget_t *widget, const Touch_t &touch);
static void onThermographSlider2  (const Widget_t *widget, const Touch_t &touch);
static void onThermographClose    (const Widget_t *widget, const Touch_t &touch);
static void onThermographApply    (const Widget_t *widget, const Touch_t &touch);

static constexpr Widget_t widget_thermograph[] = {
  {   0,   0, 320, 240, image_thermograph, EVENT_NONE,  onThermographScreen  },
  { 137,   5, 110,  26, image_radio,       EVENT_DOWN,  onThermographRadio1  },
  { 137,  38, 110,  26, image_radio,       EVENT_DOWN,  onThermographRadio2  },
  { 137,  71, 170,  26, image_toggle,      EVENT_DOWN,  onThermographToggle1 },
  { 137, 103, 170,  26, image_toggle,      EVENT_DOWN,  onThermographToggle2 },
  {  40, 134, 238,  26, image_slider2,     EVENT_DRAG,  onThermographSlider1 },
  {  40, 173, 238,  26, image_slider2,     EVENT_DRAG,  onThermographSlider2 },
  {  60, 206,  30,  30, NULL,              EVENT_ALL,   onThermographClose   },
  { 230, 206,  30,  30, image_icon_apply,  EVENT_CLICK, onThermographApply   },
#if defined (DEMO_MODE) && DEMO_MODE
  {   0,   0, 128, 135, image_demo,        EVENT_NONE,  nullptr              },
#endif
};

// Screen - Capture mode
static void onCaptureModeScreen (const Widget_t *widget, const Touch_t &touch);
static void onCaptureModeCamera (const Widget_t *widget, const Touch_t &touch);
static void onCaptureModeVideo  (const Widget_t *widget, const Touch_t &touch);
static void onCaptureModeApply  (const Widget_t *widget, const Touch_t &touch);

static constexpr Widget_t widget_capture_mode[] = {
  {   0,   0, 320, 240, image_capture_mode, EVENT_NONE, onCaptureModeScreen   },
  {  48,  50, 240,  26, image_radio,        EVENT_DOWN, onCaptureModeCamera   },
  {  48, 126, 240,  26, image_radio,        EVENT_DOWN, onCaptureModeVideo    },
  { 145, 206,  30,  30, NULL,               EVENT_ALL,  onCaptureModeApply    },
};

// Configuration for the scroll box
#define FONT_WIDTH    12  // [px] (for setTextSize(2))
#define FONT_HEIGHT   16  // [px] (for setTextSize(2))
#define FONT_MARGIN   3   // [px] (margin for each top, right, bottom, left)
#define ITEM_WIDTH    11  // text length of an item (mlx0000.bmp)
#define VIEW_ITEMS    10  // number of items in a view
#define ITEM_HEIGHT   (FONT_HEIGHT + FONT_MARGIN * 2)
#define VIEW_WIDTH    (FONT_WIDTH  * ITEM_WIDTH + FONT_MARGIN * 2) // 138
#define VIEW_HEIGHT   (ITEM_HEIGHT * VIEW_ITEMS) // 220
#define SCROLL_COLOR  RGB565(0x01, 0xA1, 0xFF)

// Screen - File manager
static void onFileManagerScreen   (const Widget_t *widget, const Touch_t &touch);
static void onFileManagerCheckAll (const Widget_t *widget, const Touch_t &touch);
static void onFileManagerScrollBox(const Widget_t *widget, const Touch_t &touch);
static void onFileManagerScrollBar(const Widget_t *widget, const Touch_t &touch);
static void onFileManagerThumbnail(const Widget_t *widget, const Touch_t &touch);
static void onFileManagerMovie    (const Widget_t *widget, const Touch_t &touch);
static void onFileManagerFolder   (const Widget_t *widget, const Touch_t &touch);
static void onFileManagerClose    (const Widget_t *widget, const Touch_t &touch);
static void onFileManagerApply    (const Widget_t *widget, const Touch_t &touch);

static constexpr Widget_t widget_file_manager[] = {
  {   0,   0, 320, 240, image_file_manager, EVENT_NONE,   onFileManagerScreen    },
  {   0,   9,  26,  26, image_checkbox,     EVENT_DOWN,   onFileManagerCheckAll  },
  {  29,  10, 138, 220, NULL,               EVENT_SELECT, onFileManagerScrollBox }, // VIEW_WIDTH x VIEW_HEIGHT
  { 170,   9,  15, 220, NULL,               EVENT_DRAG,   onFileManagerScrollBar }, // scroll bar x VIEW_HEIGHT
  { 191,  62, 128,  96, NULL,               EVENT_NONE,   onFileManagerThumbnail },
  { 207, 166,  32,  28, image_movie,        EVENT_CLICK,  onFileManagerMovie     }, // 32 x 26 --> 32 x 28 for DrawPress()
  { 276, 166,  32,  28, image_folder,       EVENT_CLICK,  onFileManagerFolder    }, // 32 x 26 --> 32 x 28 for DrawPress()
  { 208, 206,  30,  32, NULL,               EVENT_ALL,    onFileManagerClose     }, // 30 x 30 --> 30 x 32 for DrawPress()
  { 276, 206,  30,  32, NULL,               EVENT_CLICK,  onFileManagerApply     }, // 30 x 30 --> 30 x 32 for DrawPress()
};

// Screen - Calibration
static void onCalibrationScreen (const Widget_t *widget, const Touch_t &touch);
static void onCalibrationExec   (const Widget_t *widget, const Touch_t &touch);
static void onCalibrationAdjust (const Widget_t *widget, const Touch_t &touch);
static void onCalibrationSave   (const Widget_t *widget, const Touch_t &touch);
static void onCalibrationXup    (const Widget_t *widget, const Touch_t &touch);
static void onCalibrationXdown  (const Widget_t *widget, const Touch_t &touch);
static void onCalibrationYup    (const Widget_t *widget, const Touch_t &touch);
static void onCalibrationYdown  (const Widget_t *widget, const Touch_t &touch);
static void onCalibrationClose  (const Widget_t *widget, const Touch_t &touch);
static void onCalibrationApply  (const Widget_t *widget, const Touch_t &touch);

static constexpr Widget_t widget_calibration[] = {
  {   0,   0, 320, 240, image_calibration, EVENT_NONE,  onCalibrationScreen   },
  {  22,  12,  70,  70, NULL,              EVENT_UP,    onCalibrationExec     },
  { 125,  12,  70,  70, NULL,              EVENT_UP,    onCalibrationAdjust   },
  { 228,  12,  70,  70, image_save_flash,  EVENT_UP,    onCalibrationSave     },
  { 164, 154,  26,  26, image_arrowU,      EVENT_CLICK, onCalibrationXup      },
  { 164, 180,  26,  26, image_arrowD,      EVENT_CLICK, onCalibrationXdown    },
  { 263, 154,  26,  26, image_arrowU,      EVENT_CLICK, onCalibrationYup      },
  { 263, 180,  26,  26, image_arrowD,      EVENT_CLICK, onCalibrationYdown    },
  {  60, 206,  30,  30, NULL,              EVENT_ALL,   onCalibrationClose    },
  { 230, 206,  30,  30, image_icon_apply,  EVENT_CLICK, onCalibrationApply    },
};

// Screen - Adjust Offset
static void onAdjustOffsetScreen(const Widget_t *widget, const Touch_t &touch);
static void onAdjustOffsetTarget(const Widget_t *widget, const Touch_t &touch);
static void onAdjustOffsetClose (const Widget_t *widget, const Touch_t &touch);
static void onAdjustOffsetApply (const Widget_t *widget, const Touch_t &touch);

static constexpr Widget_t widget_adjust_offset[] = {
  {  25,  90, 270,  60, image_adjust_offset, EVENT_NONE,  onAdjustOffsetScreen },
  { 144, 104,  32,  32, image_target,        EVENT_DOWN,  onAdjustOffsetTarget },
  {  60, 105,  30,  30, NULL,                EVENT_ALL,   onAdjustOffsetClose  },
  { 230, 105,  30,  30, image_icon_apply,    EVENT_CLICK, onAdjustOffsetApply  },
};

// Screen - Information
static void onInformationScreen(const Widget_t *widget, const Touch_t &touch);
static void onInformationClose (const Widget_t *widget, const Touch_t &touch);

static constexpr Widget_t widget_information[] = {
  {   0,   0, 320, 240, image_information, EVENT_NONE, onInformationScreen   },
  { 145, 206,  30,  30, NULL,              EVENT_ALL,  onInformationClose    },
};

/*--------------------------------------------------------------------------------
 * Common method - Apply
 *--------------------------------------------------------------------------------*/
static bool Apply(const Widget_t *widget, const Touch_t &touch, bool enable) {
  if (touch.event == EVENT_INIT) {
    DrawButton(widget, enable);
  }

  else if (enable) {
    DrawPress(widget, touch.event);

    if (touch.event & EVENT_RISING) {
      DrawButton(widget, 0);
      return true; // ready to apply
    }
  }

  return false;
}

/*--------------------------------------------------------------------------------
 * Callback functions - Main
 *--------------------------------------------------------------------------------*/
static void onMainScreen(const Widget_t *widget, const Touch_t &touch) {
  DBG_FUNC(printf("%s\n", __func__));

  DrawScreen(widget);

#if defined (DEMO_MODE) && DEMO_MODE
  int N = 0;
  switch (mlx_cnf.interpolation * mlx_cnf.box_size) {
    case 8: N = 0; break;
    case 6: N = 1; break;
    case 4: N = 2; break;
    case 2: N = 3; break;
    case 1: N = 4; break;
  }
  DrawWidget(widget + 6, N);
#endif

  // Draw colorbar and range
  DrawColorRange(3);

  // Draw resolution
  GFX_EXEC(setTextSize(2));
  GFX_EXEC(setTextDatum(TL_DATUM));
  gfx_printf(260 + FONT_WIDTH, LINE_HEIGHT * 0.5, "%2d:%d", mlx_cnf.interpolation, mlx_cnf.box_size);
}

static void onMainInside(const Widget_t *widget, const Touch_t &touch) {
  DBG_FUNC(printf("%s\n", __func__));

  if (touch.event != EVENT_INIT) {
    // ToDo: plot the points and show values
  }
}

static void onMainOutside(const Widget_t *widget, const Touch_t &touch) {
  DBG_FUNC(printf("%s\n", __func__));

  if (touch.event != EVENT_INIT) {
    // ToDo: reset the picked up points
  }
}

static void onMainThermograph(const Widget_t *widget, const Touch_t &touch) {
  DBG_FUNC(printf("%s\n", __func__));

  if (touch.event != EVENT_INIT) {
    widget_state(STATE_THERMOGRAPH);
  }
}

static void onMainCapture(const Widget_t *widget, const Touch_t &touch) {
  DBG_FUNC(printf("%s\n", __func__));

  if (touch.event == EVENT_INIT) {
    DrawButton(widget, mlx_cap.capture_mode == 0 ? 0 : 2);
  }

  else if (mlx_cap.capture_mode == 0) {
    DrawButton(widget, 1); // draw icon_camera2
    sdcard_save();
    DrawButton(widget, 0); // draw icon_camera1
  }

  else if (mlx_cap.recording == false) {
    DrawButton(widget, 3); // draw icon_stop
    mlx_cap.recording = true;
  }

  else {
    DrawButton(widget, 2); // draw icon_video
    mlx_cap.recording = false;
  }
}

static void onMainConfiguration(const Widget_t *widget, const Touch_t &touch) {
  DBG_FUNC(printf("%s\n", __func__));

  if (touch.event != EVENT_INIT) {
    widget_state(STATE_CONFIGURATION);
  }
}

/*--------------------------------------------------------------------------------
 * Callback functions - Configuration
 *--------------------------------------------------------------------------------*/
static void onConfigurationScreen(const Widget_t *widget, const Touch_t &touch) {
  DBG_FUNC(printf("%s\n", __func__));

  // copy MLX90640 configuration data and stop recording video
  cnf_copy = mlx_cnf;
  tch_copy = tch_cnf;
  mlx_cap.recording = false;

  DrawScreen(widget);
}

static void onConfigurationResolution(const Widget_t *widget, const Touch_t &touch) {
  DBG_FUNC(printf("%s\n", __func__));

  if (touch.event != EVENT_INIT) {
    widget_state(STATE_RESOLUTION);
  }
}

static void onConfigurationThermograph(const Widget_t *widget, const Touch_t &touch) {
  DBG_FUNC(printf("%s\n", __func__));

  if (touch.event != EVENT_INIT) {
    widget_state(STATE_THERMOGRAPH);
  }
}

static void onConfigurationCaptureMode(const Widget_t *widget, const Touch_t &touch) {
  DBG_FUNC(printf("%s\n", __func__));

  if (touch.event != EVENT_INIT) {
    widget_state(STATE_CAPTURE_MODE);
  }
}

static void onConfigurationFileManager(const Widget_t *widget, const Touch_t &touch) {
  DBG_FUNC(printf("%s\n", __func__));

  if (touch.event != EVENT_INIT) {
    widget_state(STATE_FILE_MANAGER);
  }
}

static void onConfigurationCalibration(const Widget_t *widget, const Touch_t &touch) {
  DBG_FUNC(printf("%s\n", __func__));

  if (touch.event != EVENT_INIT) {
    widget_state(STATE_CALIBRATION);
  }
}

static void onConfigurationInformation(const Widget_t *widget, const Touch_t &touch) {
  DBG_FUNC(printf("%s\n", __func__));

  if (touch.event != EVENT_INIT) {
    widget_state(STATE_INFORMATION);
  }
}

static void onConfigurationReturn(const Widget_t *widget, const Touch_t &touch) {
  DBG_FUNC(printf("%s\n", __func__));

  if (touch.event != EVENT_INIT) {
    widget_state(STATE_MAIN);
  }
}

/*--------------------------------------------------------------------------------
 * Callback functions - Resolution
 *--------------------------------------------------------------------------------*/
static void MakeSliderPos(const Widget_t *widget, const int16_t *scale, const int16_t n_scale, int16_t *pos) {
  // Here it's assumed that the knob width is equal to its height.
  // maximum  knob position on the bar
  int16_t pos_max = widget->w - widget->h - SLIDER_KNOB_OFFSET;
  const float step = (float)(pos_max - SLIDER_KNOB_OFFSET) / (float)(scale[n_scale-1] - scale[0]);

  for (int i = 0; i < n_scale; i++) {
    pos[i] = (int)((float)(scale[i] - scale[0]) * step + 0.5f) + SLIDER_KNOB_OFFSET;

#if CHECK_POSITION
    GFX_EXEC(drawFastVLine(widget->x + pos[i] + widget->h / 2, widget->y - 10, 10, RED));
#endif
  }
}

static int UpdateSliderPos(const Widget_t *widget, const Touch_t &touch, int16_t *pos, const int16_t n_pos) {
  // Here it's assumed that the knob width is equal to its height.
  // relative knob position on the bar
  int16_t X = touch.x - widget->x - widget->h / 2;
  X = constrain(X, pos[0], pos[n_pos - 1]);

  for (int i = 0; i < n_pos - 1 ; i++) {
    if (pos[i] <= X && X <= pos[i + 1]) {
      if (X < (pos[i] + pos[i + 1]) / 2) {
        DrawSlider(widget, pos[i  ]);
        return i;
      } else {
        DrawSlider(widget, pos[i+1]);
        return i + 1;
      }
    }
  }

  // never reach
  return 0;
}

static void onResolutionScreen(const Widget_t *widget, const Touch_t &touch) {
  DBG_FUNC(printf("%s\n", __func__));

  DrawScreen(widget);
}

static void onResolutionSlider1(const Widget_t *widget, const Touch_t &touch) {
  DBG_FUNC(printf("%s\n", __func__));

  const int16_t scale[] = {1, 2, 4, 6, 8};
  const int n = sizeof(scale) / sizeof(scale[0]);
  Touch_t t = touch;
  int16_t pos[n];

  // Calculate the position inside the widget (scale --> pos)
  MakeSliderPos(widget, scale, n, pos);

  if (touch.event == EVENT_INIT) {
    // Here it's assumed that the knob width is equal to its height.
    t.x = widget->x + widget->h / 2;
    for (int i = 0; i < n; i++) {
      if (scale[i] == cnf_copy.interpolation) {
        t.x += pos[i];
        break;
      }
    }
  }

  // update the knob position and the configuration
  int i = UpdateSliderPos(widget, t, pos, n);
  cnf_copy.interpolation = scale[i];

  // restrict pixel interpolation and block size
  if (cnf_copy.interpolation * cnf_copy.box_size > 8) {
    cnf_copy.box_size = 8 / cnf_copy.interpolation;
    onResolutionSlider2(widget + 1, doInit);
  }

  // Enable apply if somethig is changed
  onResolutionApply(widget + 3, doInit);
}

static void onResolutionSlider2(const Widget_t *widget, const Touch_t &touch) {
  DBG_FUNC(printf("%s\n", __func__));

  const int16_t scale[] = {1, 2, 4, 8};
  const int n = sizeof(scale) / sizeof(scale[0]);
  Touch_t t = touch;
  int16_t pos[n];

  // Calculate the position inside the widget (scale --> pos)
  MakeSliderPos(widget, scale, n, pos);

  if (touch.event == EVENT_INIT) {
    // Here it's assumed that the knob width is equal to its height.
    t.x = widget->x + widget->h / 2;
    for (int i = 0; i < n; i++) {
      if (scale[i] == cnf_copy.box_size) {
        t.x += pos[i];
        break;
      }
    }
  }

  // update the knob position and the configuration
  int i = UpdateSliderPos(widget, t, pos, n);
  cnf_copy.box_size = scale[i];

  // restrict pixel interpolation and block size
  if (cnf_copy.interpolation * cnf_copy.box_size > 8) {
    cnf_copy.interpolation = 8 / cnf_copy.box_size;
    onResolutionSlider1(widget - 1, doInit);
  }

  // Enable apply if somethig is changed
  onResolutionApply(widget + 2, doInit);
}

static void onResolutionClose(const Widget_t *widget, const Touch_t &touch) {
  DBG_FUNC(printf("%s\n", __func__));

  if (touch.event != EVENT_INIT) {
    widget_state(STATE_CONFIGURATION);
  }
}

static void onResolutionApply(const Widget_t *widget, const Touch_t &touch) {
  DBG_FUNC(printf("%s\n", __func__));

  if (Apply(widget, touch, (mlx_cnf >= cnf_copy))) {
    mlx_cnf = cnf_copy;
    mlx_refresh();
  }
}

/*--------------------------------------------------------------------------------
 * Callback functions - Thermograph
 *--------------------------------------------------------------------------------*/
#define TERMOGRAPH_MIN      (-20)
#define TERMOGRAPH_MAX      (180)
#define TERMOGRAPH_STEP     (5)
#define TERMOGRAPH_DIFF     (10)
#define TERMOGRAPH_MIN_ROW  282
#define TERMOGRAPH_MIN_COL  139
#define TERMOGRAPH_MAX_ROW  282
#define TERMOGRAPH_MAX_COL  178

static __attribute__((optimize("O0"))) int16_t GetThermoSlider(const Widget_t *widget, const Touch_t &touch) {
  // Here it's assumed that the knob width is equal to its height.
  int16_t X = touch.x - widget->x - widget->h / 2;        // Relative x coordinate of top left of knob
  int16_t Y = SLIDER_KNOB_OFFSET;                         // Minimum value of knob top left coordinate
  int16_t Z = widget->w - widget->h - SLIDER_KNOB_OFFSET; // Maximum value of knob top left coordinate
  DrawSlider(widget, X = constrain(X, Y, Z));

  // Set the X coordinate in 5 degree increments
  X -= Y;
  X = (X / TERMOGRAPH_STEP) * TERMOGRAPH_STEP;
  X += Y;

  return TERMOGRAPH_MIN + ((X - Y) * (TERMOGRAPH_MAX - TERMOGRAPH_MIN)) / (Z - Y); // value
}

static void PutThermoSlider(const Widget_t *widget, int16_t V, bool enable) {
  // Here it's assumed that the knob width is equal to its height.
  int16_t Y = SLIDER_KNOB_OFFSET;                         // Minimum value of knob top left coordinate
  int16_t Z = widget->w - widget->h - SLIDER_KNOB_OFFSET; // Maximum value of knob top left coordinate

  int16_t X = (V - TERMOGRAPH_MIN) * (Z - Y) / (TERMOGRAPH_MAX - TERMOGRAPH_MIN) + Y;
  DrawSlider(widget, X = constrain(X, Y, Z), enable);
}

static void onThermographScreen(const Widget_t *widget, const Touch_t &touch) {
  DBG_FUNC(printf("%s\n", __func__));

  DrawScreen(widget);

  // copy MLX90640 configuration data and stop recording video
  cnf_copy = mlx_copy = mlx_cnf;
  mlx_cnf.box_size = 1;
  mlx_cnf.interpolation = 4;
  mlx_cap.recording = false;
  mlx_refresh();
}

static void onThermographRadio1(const Widget_t *widget, const Touch_t &touch) {
  DBG_FUNC(printf("%s\n", __func__));

  if (touch.event != EVENT_INIT) {
    mlx_cnf.color_scheme = cnf_copy.color_scheme = 0;
  }

  DrawRadio(widget, 2, cnf_copy.color_scheme);

#if defined (DEMO_MODE) && DEMO_MODE
  DrawWidget(widget + 8, 2);
#endif

  // Enable apply if somethig is changed
  DrawColorRange(1);
  onThermographApply(widget + 7, doInit);
}

static void onThermographRadio2(const Widget_t *widget, const Touch_t &touch) {
  DBG_FUNC(printf("%s\n", __func__));

  if (touch.event != EVENT_INIT) {
    mlx_cnf.color_scheme = cnf_copy.color_scheme = 1;
  }

  DrawRadio(widget - 1, 2, cnf_copy.color_scheme);

#if defined (DEMO_MODE) && DEMO_MODE
  if (touch.event != EVENT_INIT) DrawWidget(widget + 7, 5);
#endif

  // Enable apply if somethig is changed
  DrawColorRange(1);
  onThermographApply(widget + 6, doInit);
}

static void onThermographToggle1(const Widget_t *widget, const Touch_t &touch) {
  DBG_FUNC(printf("%s\n", __func__));

  if (touch.event != EVENT_INIT) {
    mlx_cnf.minmax_auto = cnf_copy.minmax_auto = !cnf_copy.minmax_auto;
  }

  DrawToggle(widget, cnf_copy.minmax_auto);

  // Enable apply if somethig is changed
  onThermographApply(widget + 5, doInit);
}

static void onThermographToggle2(const Widget_t *widget, const Touch_t &touch) {
  DBG_FUNC(printf("%s\n", __func__));

  if (touch.event != EVENT_INIT) {
    mlx_cnf.range_auto = cnf_copy.range_auto = !cnf_copy.range_auto;
  }

  DrawToggle(widget, cnf_copy.range_auto);

  // Disable Slider
  onThermographSlider1(widget + 1, doInit);
  onThermographSlider2(widget + 2, doInit);

  // Enable apply if somethig is changed
  onThermographApply(widget + 4, doInit);
}

static void onThermographSlider1(const Widget_t *widget, const Touch_t &touch) {
  DBG_FUNC(printf("%s\n", __func__));

  if (touch.event == EVENT_INIT) {
    // when range_auto == true then make slider disable
    PutThermoSlider(widget, cnf_copy.range_min, !cnf_copy.range_auto);
    gfx_printf(TERMOGRAPH_MIN_ROW, TERMOGRAPH_MIN_COL, "%3d", cnf_copy.range_min);
  }

  else if (!cnf_copy.range_auto) {
    // Convert to temperature and display without flickering
    int16_t v = GetThermoSlider(widget, touch);
    static int16_t V = 0xFFFF;
    if (V != v) {
      mlx_cnf.range_min = cnf_copy.range_min = V = v;
      gfx_printf(TERMOGRAPH_MIN_ROW, TERMOGRAPH_MIN_COL, "%3d", cnf_copy.range_min);

      // Temperature minimum and maximum restrictions
      if (cnf_copy.range_max - cnf_copy.range_min < TERMOGRAPH_DIFF) {
        mlx_cnf.range_max = cnf_copy.range_max = cnf_copy.range_min + TERMOGRAPH_DIFF;
        onThermographSlider2(widget + 1, doInit);
      }
    }
  }

  // Enable apply if somethig is changed
  DrawColorRange(2);
  onThermographApply(widget + 3, doInit);
}

static void onThermographSlider2(const Widget_t *widget, const Touch_t &touch) {
  DBG_FUNC(printf("%s\n", __func__));

  if (touch.event == EVENT_INIT) {
    // when range_auto == true then make slider disable
    PutThermoSlider(widget, cnf_copy.range_max, !cnf_copy.range_auto);
    gfx_printf(TERMOGRAPH_MAX_ROW, TERMOGRAPH_MAX_COL, "%3d", cnf_copy.range_max);
  }

  else if (!cnf_copy.range_auto) {
    // Convert to temperature and display without flickering
    int16_t v = GetThermoSlider(widget, touch);
    static int16_t V = 0xFFFF;
    if (V != v) {
      mlx_cnf.range_max = cnf_copy.range_max = V = v;
      gfx_printf(TERMOGRAPH_MAX_ROW, TERMOGRAPH_MAX_COL, "%3d", cnf_copy.range_max);

      // Temperature minimum and maximum restrictions
      if (cnf_copy.range_max - cnf_copy.range_min < TERMOGRAPH_DIFF) {
        mlx_cnf.range_min = cnf_copy.range_min = cnf_copy.range_max - TERMOGRAPH_DIFF;
        onThermographSlider1(widget - 1, doInit);
      }
    }
  }

  // Enable apply if somethig is changed
  DrawColorRange(2);
  onThermographApply(widget + 2, doInit);
}

static void onThermographClose(const Widget_t *widget, const Touch_t &touch) {
  DBG_FUNC(printf("%s\n", __func__));

  if (touch.event != EVENT_INIT) {
    mlx_cnf = mlx_copy;
    mlx_refresh();
    widget_state(STATE_CONFIGURATION);
  }
}

static void onThermographApply(const Widget_t *widget, const Touch_t &touch) {
  DBG_FUNC(printf("%s\n", __func__));

  if (Apply(widget, touch, (mlx_copy != cnf_copy))) {
    mlx_cnf = mlx_copy = cnf_copy;
    mlx_cnf.box_size = 1;
    mlx_cnf.interpolation = 4;
  }
}

/*--------------------------------------------------------------------------------
 * Callback functions - Capture Mode
 *--------------------------------------------------------------------------------*/
static void onCaptureModeScreen(const Widget_t *widget, const Touch_t &touch) {
  DBG_FUNC(printf("%s\n", __func__));

  DrawScreen(widget);
}

static void onCaptureModeCamera(const Widget_t *widget, const Touch_t &touch) {
  DBG_FUNC(printf("%s\n", __func__));

  if (touch.event != EVENT_INIT) {
    mlx_cap.capture_mode = 0;
  }

  DrawRadio(widget, 2, mlx_cap.capture_mode);
}

static void onCaptureModeVideo(const Widget_t *widget, const Touch_t &touch) {
  DBG_FUNC(printf("%s\n", __func__));

  if (touch.event != EVENT_INIT) {
    mlx_cap.capture_mode = 1;
  }

  DrawRadio(widget - 1, 2, mlx_cap.capture_mode);
}

static void onCaptureModeApply(const Widget_t *widget, const Touch_t &touch) {
  DBG_FUNC(printf("%s\n", __func__));

  if (touch.event != EVENT_INIT) {
    widget_state(STATE_CONFIGURATION);
  }
}

/*--------------------------------------------------------------------------------
 * Callback functions - File Manager
 *--------------------------------------------------------------------------------*/
static std::vector<FileInfo_t> files;
static int n_files;
static int scroll_pos, scroll_max, bar_height;
static bool file_selected;

static void ScrollView(const Widget_t *widget, int scroll_pos) {
#if   defined (LOVYANGFX_HPP_)

  static LGFX_Sprite sprite_view(&lcd);

#elif defined (_TFT_eSPIH_)

  static TFT_eSprite sprite_view(&tft);

#endif

  sprite_view.setTextSize(2);
  sprite_view.setTextWrap(false);
  sprite_view.setTextColor(WHITE, BLACK);
  sprite_view.createSprite(VIEW_WIDTH, VIEW_HEIGHT);

  int scaled_pos = scroll_pos * widget->h / bar_height;
  int item_head  = scaled_pos / ITEM_HEIGHT;
  int item_tail  = item_head + (VIEW_ITEMS + 1);
  item_tail = min(item_tail, n_files);
  DBG_FUNC(printf("item_head: %d, item_tail: %d\n", item_head, item_tail));

  bool invert = false;
  int base_pos = item_head * ITEM_HEIGHT;
  int delta_pos = base_pos - scaled_pos + FONT_MARGIN - ITEM_HEIGHT;

  for (int i = item_head; i < item_tail; i++) {
    delta_pos += ITEM_HEIGHT;

    if (invert == false && files[i].isSelected == true) {
      invert = true;
      sprite_view.setTextColor(BLACK, WHITE);
    } else

    if (invert == true && files[i].isSelected == false) {
      invert = false;
      sprite_view.setTextColor(WHITE, BLACK);
    }

    if (invert == true) {
      sprite_view.fillRect(0, delta_pos - FONT_MARGIN, VIEW_WIDTH, ITEM_HEIGHT, WHITE);
    }

    // skip directory and extract file name only
    sprite_view.setCursor(FONT_MARGIN, delta_pos);
    const char *p = strrchr(files[i].path.c_str(), '/');
    sprite_view.print(p ? p + 1 : p);
  }

//GFX_EXEC(startWrite());
  sprite_view.pushSprite(widget->x, widget->y);
  sprite_view.deleteSprite();
//GFX_EXEC(endWrite());
}

static void onFileManagerScreen(const Widget_t *widget, const Touch_t &touch) {
  DBG_FUNC(printf("%s\n", __func__));

  DrawScreen(widget);

  file_selected = false;

  if (sdcard_open()) {
    uint32_t total, free;
    sdcard_size(&total, &free);
    GFX_EXEC(setTextSize(2));
    if (total < 10000UL) {
      gfx_printf(245, 13, "%4luMB", total);
      gfx_printf(245, 37, "%4luMB", free);
    } else {
      gfx_printf(245, 13, "%4.1fGB", (float)total / 1000.0);
      gfx_printf(245, 37, "%4.1fGB", (float)free  / 1000.0);
    }

    files.clear();
    GetFileList(SD, "/", 1, files);
    n_files = files.size();
/*
    DBG_FUNC({
      for (const auto& file : files) {
        printf("%s, %lu\n", file.path.c_str(), file.size);
      }

      files[0].isSelected = true;
      files[9].isSelected = true;
      files[n_files-1].isSelected = true;
    });//*/
  }
}

static void onFileManagerCheckAll(const Widget_t *widget, const Touch_t &touch) {
  DBG_FUNC(printf("%s\n", __func__));

  if (touch.event != EVENT_INIT) {
    file_selected = !file_selected;
    for (auto& file : files) {
      file.isSelected = file_selected;
    }
    ScrollView(widget + 1, scroll_pos);
  }

  DrawCheck(widget, file_selected);
}

static void onFileManagerScrollBox(const Widget_t *widget, const Touch_t &touch) {
  DBG_FUNC(printf("%s\n", __func__));

  if (touch.event != EVENT_INIT) {
    int scaled_pos = scroll_pos * widget->h / bar_height;
    int selected =  (scaled_pos + touch.y - widget->y - FONT_MARGIN) / ITEM_HEIGHT;
    files[selected].isSelected = !files[selected].isSelected;

    ScrollView(widget, scroll_pos);

    const Widget_t *thumbnail = widget + 2; 
    if (files[selected].isSelected) {
      DBG_FUNC(printf("path: %s\n", files[selected].path.c_str()));
      DrawThumb(thumbnail, files[selected].path.c_str());
    } else {
      GFX_EXEC(fillRect(thumbnail->x, thumbnail->y, thumbnail->w, thumbnail->h, BLACK));
    }
  }
}

static void onFileManagerScrollBar(const Widget_t *widget, const Touch_t &touch) {
  DBG_FUNC(printf("%s\n", __func__));

  static int drag_pos;

  if (touch.event == EVENT_INIT) {
    scroll_pos = drag_pos = 0;
    if (n_files > VIEW_ITEMS) {
      scroll_max = widget->h * (n_files - VIEW_ITEMS) / n_files + 1; // '1' for round up
      bar_height = widget->h - scroll_max;
    } else {
      bar_height = widget->h;
      scroll_max = 0;
    }
  }

  else if (touch.event & EVENT_DOWN) {
    drag_pos = touch.y;
  } // else drag

  // Relative movement from the previous position
  scroll_pos += touch.y - drag_pos;
  scroll_pos = constrain(scroll_pos, 0, scroll_max);
//DBG_FUNC(printf("scroll_pos: %d, scroll_max: %d\n", scroll_pos, scroll_max));

#if   defined (LOVYANGFX_HPP_)

  static LGFX_Sprite sprite_scroll(&lcd);

#elif defined (_TFT_eSPIH_)

  static TFT_eSprite sprite_scroll(&tft);

#endif

  sprite_scroll.createSprite(widget->w, widget->h);
  sprite_scroll.fillRect(0, scroll_pos, widget->w, bar_height, SCROLL_COLOR);
  sprite_scroll.pushSprite(widget->x, widget->y);
  sprite_scroll.deleteSprite();

  ScrollView(widget - 1, scroll_pos);

  // Update the previous position
  drag_pos = touch.y;
}

static void onFileManagerThumbnail(const Widget_t *widget, const Touch_t &touch) {
  DBG_FUNC(printf("%s\n", __func__));

  if (touch.event == EVENT_INIT) {
    GFX_EXEC(drawRect(widget->x - 1, widget->y - 1, widget->w + 2, widget->h + 2, DARKGREY));
  }
}

static void onFileManagerMovie(const Widget_t *widget, const Touch_t &touch) {
  DBG_FUNC(printf("%s\n", __func__));

  if (touch.event == EVENT_INIT) {
    DrawButton(widget);
  } else {
    DrawPress(widget, touch.event);
  }
}

static void onFileManagerFolder(const Widget_t *widget, const Touch_t &touch) {
  DBG_FUNC(printf("%s\n", __func__));

  if (touch.event == EVENT_INIT) {
    DrawButton(widget);
  } else {
    DrawPress(widget, touch.event);
  }
}

static void onFileManagerClose(const Widget_t *widget, const Touch_t &touch) {
  DBG_FUNC(printf("%s\n", __func__));

  if (touch.event != EVENT_INIT) {
    widget_state(STATE_CONFIGURATION);
  }
}

static void onFileManagerApply(const Widget_t *widget, const Touch_t &touch) {
  DBG_FUNC(printf("%s\n", __func__));

  DrawPress(widget, touch.event);

  if (touch.event != EVENT_INIT) {
    // ToDo: remove selected files
  }
}

/*--------------------------------------------------------------------------------
 * Callback functions - Calibration
 *--------------------------------------------------------------------------------*/
static bool enable_save;
static bool SaveCalibration(TouchConfig_t &config) {
  if (touch_save(&tch_cnf)) {
    enable_save = false;
    return true;
  } else {
    return false;
  }
}

#define TOUCH_OFFSET_MIN    (-10)
#define TOUCH_OFFSET_MAX    ( 10)
#define TOUCH_OFFSET_X_ROW  123
#define TOUCH_OFFSET_X_COL  172
#define TOUCH_OFFSET_Y_ROW  223
#define TOUCH_OFFSET_Y_COL  172

static void DrawOffsetX(const Widget_t* widget, const Touch_t &touch, const Widget_t *apply) {
  // draw button when touch.event == EVENT_INIT or EVENT_UP
  if (touch.event != EVENT_DOWN) {
    DrawButton(widget,     (tch_copy.offset[0] < TOUCH_OFFSET_MAX) ? 1 : 0);
    DrawButton(widget + 1, (tch_copy.offset[0] > TOUCH_OFFSET_MIN) ? 1 : 0);
  }

  gfx_printf(TOUCH_OFFSET_X_ROW, TOUCH_OFFSET_X_COL, "%3d", (int)tch_copy.offset[0]);

  // Enable apply if somethig is changed
  onCalibrationApply(apply, doInit);
}

static void DrawOffsetY(const Widget_t* widget, const Touch_t &touch, const Widget_t *apply) {
  // draw button when touch.event == EVENT_INIT or EVENT_UP
  if (touch.event != EVENT_DOWN) {
    DrawButton(widget,     (tch_copy.offset[1] < TOUCH_OFFSET_MAX) ? 1 : 0);
    DrawButton(widget + 1, (tch_copy.offset[1] > TOUCH_OFFSET_MIN) ? 1 : 0);
  }

  gfx_printf(TOUCH_OFFSET_Y_ROW, TOUCH_OFFSET_Y_COL, "%3d", (int)tch_copy.offset[1]);

  // Enable apply if somethig is changed
  onCalibrationApply(apply, doInit);
}

static void onCalibrationScreen(const Widget_t *widget, const Touch_t &touch) {
  DBG_FUNC(printf("%s\n", __func__));

  tch_ajst = tch_copy;

  DrawScreen(widget);

  GFX_EXEC(setTextSize(1));

  uint16_t *c = tch_cnf.cal;
  gfx_printf(115,  94, "%4d, %4d, %4d, %4d", c[0], c[1], c[2], c[3]);
  gfx_printf(115, 106, "%4d, %4d, %4d, %4d", c[4], c[5], c[6], c[7]);

  if (!(tch_cnf >= tch_copy)) {
    c = tch_copy.cal;
    gfx_printf(115, 126, "%4d, %4d, %4d, %4d", c[0], c[1], c[2], c[3]);
    gfx_printf(115, 138, "%4d, %4d, %4d, %4d", c[4], c[5], c[6], c[7]);
  }

  GFX_EXEC(setTextSize(2));
}

static void onCalibrationExec(const Widget_t *widget, const Touch_t &touch) {
  DBG_FUNC(printf("%s\n", __func__));

  if (touch.event != EVENT_INIT) {
    touch_calibrate(&tch_copy);
    widget_state(STATE_CALIBRATION);
  }
}

static void onCalibrationAdjust(const Widget_t *widget, const Touch_t &touch) {
  DBG_FUNC(printf("%s\n", __func__));

  if (touch.event != EVENT_INIT) {
    widget_state(STATE_ADJUST_OFFSET);
  }
}

static void onCalibrationSave(const Widget_t *widget, const Touch_t &touch) {
  DBG_FUNC(printf("%s\n", __func__));

  if (touch.event == EVENT_INIT) {
    DrawButton(widget, enable_save);
  }

  // now ready to save touch configuration data into flash
  else if (enable_save) {
    DrawButton(widget, 2);
    if (SaveCalibration(tch_cnf)) {
      widget_state(STATE_CALIBRATION);
    }
  }
}

static void onCalibrationXup(const Widget_t *widget, const Touch_t &touch) {
  DBG_FUNC(printf("%s\n", __func__));

  if (touch.event != EVENT_INIT && tch_copy.offset[0] < TOUCH_OFFSET_MAX) {
    DrawPress(widget, touch.event);
    if (touch.event & EVENT_DOWN ) {
      tch_copy.offset[0]++;
    }
  }

  DrawOffsetX(widget, touch, widget + 5);
}

static void onCalibrationXdown(const Widget_t *widget, const Touch_t &touch) {
  DBG_FUNC(printf("%s\n", __func__));

  if (touch.event != EVENT_INIT && tch_copy.offset[0] > TOUCH_OFFSET_MIN) {
    DrawPress(widget, touch.event);
    if (touch.event & EVENT_DOWN) {
      tch_copy.offset[0]--;
    }
  }

  DrawOffsetX(widget - 1, touch, widget + 4);
}

static void onCalibrationYup(const Widget_t *widget, const Touch_t &touch) {
  DBG_FUNC(printf("%s\n", __func__));

  if (touch.event != EVENT_INIT && tch_copy.offset[1] < TOUCH_OFFSET_MAX) {
    DrawPress(widget, touch.event);
    if (touch.event & EVENT_DOWN) {
      tch_copy.offset[1]++;
    }
  }

  DrawOffsetY(widget, touch, widget + 3);
}

static void onCalibrationYdown(const Widget_t *widget, const Touch_t &touch) {
  DBG_FUNC(printf("%s\n", __func__));

  if (touch.event != EVENT_INIT && tch_copy.offset[1] > TOUCH_OFFSET_MIN) {
    DrawPress(widget, touch.event);
    if (touch.event & EVENT_DOWN) {
      tch_copy.offset[1]--;
    }
  }

  DrawOffsetY(widget - 1, touch, widget + 2);
}

static void onCalibrationClose(const Widget_t *widget, const Touch_t &touch) {
  DBG_FUNC(printf("%s\n", __func__));

  if (touch.event & EVENT_DOWN) {
    widget_state(STATE_CONFIGURATION);
  }
}

static void onCalibrationApply(const Widget_t *widget, const Touch_t &touch) {
  DBG_FUNC(printf("%s\n", __func__));

  if (Apply(widget, touch, (tch_cnf != tch_copy))) {
    tch_cnf = tch_copy;

    // Enable to save if somethig configuration data
    enable_save = true;
    onCalibrationSave(widget - 6, doInit);
  }
}

/*--------------------------------------------------------------------------------
 * Callback functions - Adjust Offset
 *--------------------------------------------------------------------------------*/
static void onAdjustOffsetScreen(const Widget_t *widget, const Touch_t &touch) {
  DBG_FUNC(printf("%s\n", __func__));

#if   defined (LOVYANGFX_HPP_)

  GFX_EXEC(beginTransaction());
  GFX_EXEC(fillRectAlpha(0, 0, lcd_width, lcd_height, 160, BLACK));
  GFX_EXEC(endTransaction());

#elif defined (_TFT_eSPIH_)

  GFX_EXEC(startWrite());
  DrawPNG(screen_calibration_opacity70, sizeof(screen_calibration_opacity70), 0, 0, pngDraw);
  GFX_EXEC(endWrite());

#endif

  DrawScreen(widget);
}

static void onAdjustOffsetTarget(const Widget_t *widget, const Touch_t &touch) {
  DBG_FUNC(printf("%s\n", __func__));

  if (touch.event == EVENT_INIT) {
    DrawButton(widget);
  }

  else {
    DrawButton(widget, 1);

    tch_ajst.offset[0] = lcd_width  / 2 - 1 - touch.x;
    tch_ajst.offset[1] = lcd_height / 2 - 1 - touch.y;

    tch_ajst.offset[0] = constrain(tch_ajst.offset[0], TOUCH_OFFSET_MIN, TOUCH_OFFSET_MAX);
    tch_ajst.offset[1] = constrain(tch_ajst.offset[1], TOUCH_OFFSET_MIN, TOUCH_OFFSET_MAX);

    gfx_printf(TOUCH_OFFSET_X_ROW, TOUCH_OFFSET_X_COL, "%3d", (int)tch_ajst.offset[0]);
    gfx_printf(TOUCH_OFFSET_Y_ROW, TOUCH_OFFSET_Y_COL, "%3d", (int)tch_ajst.offset[1]);

    // Enable apply if somethig is changed
    onAdjustOffsetApply(widget + 2, doInit);
  }
}

static void onAdjustOffsetClose(const Widget_t *widget, const Touch_t &touch) {
  DBG_FUNC(printf("%s\n", __func__));

  if (touch.event != EVENT_INIT) {
    widget_state(STATE_CALIBRATION);
  }
}

static void onAdjustOffsetApply(const Widget_t *widget, const Touch_t &touch) {
  DBG_FUNC(printf("%s\n", __func__));

  if (Apply(widget, touch, (tch_copy <= tch_ajst))) {
    tch_copy = tch_ajst;
    widget_state(STATE_CALIBRATION);
  }
}

/*--------------------------------------------------------------------------------
 * Callback functions - Information
 *--------------------------------------------------------------------------------*/
static void onInformationScreen (const Widget_t *widget, const Touch_t &touch) {
  DBG_FUNC(printf("%s\n", __func__));

  DrawScreen(widget);
}

static void onInformationClose  (const Widget_t *widget, const Touch_t &touch) {
  DBG_FUNC(printf("%s\n", __func__));

  if (touch.event != EVENT_INIT) {
    widget_state(STATE_CONFIGURATION);
  }
}