/*================================================================================
 * Wedget data
 *================================================================================*/
#include <Arduino.h>
#include "widgets.h"

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
static constexpr Image_t image_adjust_center[]      = { { screen_adjust_center, sizeof(screen_adjust_center) }, }; // 270 x  60
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
static constexpr Image_t image_slider1[] = {
  { slider_bar1, sizeof(slider_bar1) }, // 160 x 26
  { slider_knob, sizeof(slider_knob) }, //  26 x 26
};
static constexpr Image_t image_slider2[] = {
  { slider_bar2, sizeof(slider_bar2) }, // 215 x 26
  { slider_knob, sizeof(slider_knob) }, //  26 x 26
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
static constexpr Image_t image_save_flush[] = {
  { save_flush_off, sizeof(save_flush_off) }, // 70 x 70
  { save_flush_on,  sizeof(save_flush_on ) }, // 70 x 70
  { save_flush_red, sizeof(save_flush_red) }, // 70 x 70
};
static constexpr Image_t image_target[] = {
  { target_off, sizeof(target_off) }, // 32 x 32
  { target_on,  sizeof(target_on ) }, // 32 x 32
};

/*--------------------------------------------------------------------------------
 * List of widget data based on the screen
 * The callback function may be something this:
 * static void onCallbackFunction(const void *w, Touch_t &touch) {
 *   const Widget_t *widget = static_cast<const Widget_t*>(w);
 *   if (touch.event == EVENT_NONE) {
 *     // Setup Process
 *   } else {
 *     // Event Process
 *   }
 * }
 *--------------------------------------------------------------------------------*/
// Screen - Main
static void onMainScreen        (const void *w, Touch_t &touch);
static void onMainInside        (const void *w, Touch_t &touch);
static void onMainOutside       (const void *w, Touch_t &touch);
static void onMainThermograph   (const void *w, Touch_t &touch);
static void onMainCamera        (const void *w, Touch_t &touch);
static void onMainConfiguration (const void *w, Touch_t &touch);

static constexpr Widget_t widget_main[] = {
  {   0,   0, 320, 240, image_main,        EVENT_NONE, onMainScreen        },
  {   0,   0, 256, 192, NULL,              EVENT_ALL,  onMainInside        },
  { 258,   0,  62, 134, NULL,              EVENT_ALL,  onMainOutside       },
  {   0, 195, 256,  45, NULL,              EVENT_ALL,  onMainThermograph   },
  { 265, 135,  50,  50, image_icon_camera, EVENT_UP,   onMainCamera        },
  { 265, 185,  50,  50, NULL,              EVENT_ALL,  onMainConfiguration },
};

// Screen - Configuration
static void onConfigurationScreen       (const void *w, Touch_t &touch);
static void onConfigurationResolution   (const void *w, Touch_t &touch);
static void onConfigurationThermograph  (const void *w, Touch_t &touch);
static void onConfigurationCaptureMode  (const void *w, Touch_t &touch);
static void onConfigurationFileManager  (const void *w, Touch_t &touch);
static void onConfigurationCalibration  (const void *w, Touch_t &touch);
static void onConfigurationInformation  (const void *w, Touch_t &touch);
static void onConfigurationReturn       (const void *w, Touch_t &touch);

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
static void onResolutionScreen  (const void *w, Touch_t &touch);
static void onResolutionSlider1 (const void *w, Touch_t &touch);
static void onResolutionSlider2 (const void *w, Touch_t &touch);
static void onResolutionClose   (const void *w, Touch_t &touch);
static void onResolutionApply   (const void *w, Touch_t &touch);

static constexpr Widget_t widget_resolution[] = {
  {   0,   0, 320, 240, image_resolution, EVENT_NONE,  onResolutionScreen  },
  { 138,  45, 160,  26, image_slider1,    EVENT_DRAG,  onResolutionSlider1 },
  { 138, 105, 160,  26, image_slider1,    EVENT_DRAG,  onResolutionSlider2 },
  {  60, 206,  30,  30, NULL,             EVENT_ALL,   onResolutionClose   },
  { 230, 206,  30,  30, image_icon_apply, EVENT_CLICK, onResolutionApply   },
};

// Screen - Thermograph
static void onThermographScreen   (const void *w, Touch_t &touch);
static void onThermographRadio1   (const void *w, Touch_t &touch);
static void onThermographRadio2   (const void *w, Touch_t &touch);
static void onThermographToggle1  (const void *w, Touch_t &touch);
static void onThermographToggle2  (const void *w, Touch_t &touch);
static void onThermographSlider1  (const void *w, Touch_t &touch);
static void onThermographSlider2  (const void *w, Touch_t &touch);
static void onThermographClose    (const void *w, Touch_t &touch);
static void onThermographApply    (const void *w, Touch_t &touch);

static constexpr Widget_t widget_thermograph[] = {
  {   0,   0, 320, 240, image_thermograph, EVENT_NONE,  onThermographScreen  },
  { 137,   5, 110,  26, image_radio,       EVENT_DOWN,  onThermographRadio1  },
  { 137,  38, 110,  26, image_radio,       EVENT_DOWN,  onThermographRadio2  },
  { 137,  71, 170,  26, image_toggle,      EVENT_DOWN,  onThermographToggle1 },
  { 137, 103, 170,  26, image_toggle,      EVENT_DOWN,  onThermographToggle2 },
  {  60, 136, 216,  26, image_slider2,     EVENT_DRAG,  onThermographSlider1 },
  {  60, 174, 216,  26, image_slider2,     EVENT_DRAG,  onThermographSlider2 },
  {  60, 206,  30,  30, NULL,              EVENT_ALL,   onThermographClose   },
  { 230, 206,  30,  30, image_icon_apply,  EVENT_CLICK, onThermographApply   },
};

// Screen - File manager
static void onFileManagerScreen   (const void *w, Touch_t &touch);
static void onFileManagerCheckAll (const void *w, Touch_t &touch);
static void onFileManagerScrollBox(const void *w, Touch_t &touch);
static void onFileManagerScrollBar(const void *w, Touch_t &touch);
static void onFileManagerThumbnail(const void *w, Touch_t &touch);
static void onFileManagerMovie    (const void *w, Touch_t &touch);
static void onFileManagerFolder   (const void *w, Touch_t &touch);
static void onFileManagerClose    (const void *w, Touch_t &touch);
static void onFileManagerApply    (const void *w, Touch_t &touch);

// Setting for the scroll area
#define FONT_WIDTH    12  // [px] (for setTextSize(2))
#define FONT_HEIGHT   16  // [px] (for setTextSize(2))
#define FONT_MARGIN   3   // [px] (margin for each top, right, bottom, left)
#define VIEW_ITEMS    10  // number of items in a view
#define ITEM_HEIGHT   (FONT_HEIGHT + FONT_MARGIN * 2)
#define VIEW_WIDTH    (FONT_WIDTH * (VIEW_ITEMS + 1) + FONT_MARGIN * 2) // 138
#define VIEW_HEIGHT   (ITEM_HEIGHT * VIEW_ITEMS) // 220
#define SCROLL_COLOR  RGB565(0x01, 0xA1, 0xFF)

static constexpr Widget_t widget_file_manager[] = {
  {   0,   0, 320, 240, image_file_manager, EVENT_NONE,  onFileManagerScreen    },
  {   1,   8,  26,  26, image_checkbox,     EVENT_DOWN,  onFileManagerCheckAll  },
  {  32,  10, 138, 220, NULL,               EVENT_DOWN,  onFileManagerScrollBox }, // VIEW_WIDTH x VIEW_HEIGHT
  { 176,  10,  15, 220, NULL,               EVENT_DRAG,  onFileManagerScrollBar }, // scroll bar x VIEW_HEIGHT
  { 198,  69, 120,  90, NULL,               EVENT_NONE,  onFileManagerThumbnail },
  { 207, 165,  32,  26, image_movie,        EVENT_CLICK, onFileManagerMovie     },
  { 276, 165,  32,  26, image_folder,       EVENT_CLICK, onFileManagerFolder    },
  { 208, 206,  30,  30, NULL,               EVENT_ALL,   onFileManagerClose     },
  { 276, 206,  30,  30, NULL,               EVENT_CLICK, onFileManagerApply     },
};

// Screen - Capture mode
static void onCaptureModeScreen (const void *w, Touch_t &touch);
static void onCaptureModeCamera (const void *w, Touch_t &touch);
static void onCaptureModeVideo  (const void *w, Touch_t &touch);
static void onCaptureModeApply  (const void *w, Touch_t &touch);

static constexpr Widget_t widget_capture_mode[] = {
  {   0,   0, 320, 240, image_capture_mode, EVENT_NONE, onCaptureModeScreen   },
  {  48,  50, 240,  26, image_radio,        EVENT_DOWN, onCaptureModeCamera   },
  {  48, 126, 240,  26, image_radio,        EVENT_DOWN, onCaptureModeVideo    },
  { 145, 206,  30,  30, NULL,               EVENT_ALL,  onCaptureModeApply    },
};

// Screen - Calibration
static void onCalibrationScreen (const void *w, Touch_t &touch);
static void onCalibrationExec   (const void *w, Touch_t &touch);
static void onCalibrationAdjust (const void *w, Touch_t &touch);
static void onCalibrationSave   (const void *w, Touch_t &touch);
static void onCalibrationXup    (const void *w, Touch_t &touch);
static void onCalibrationXdown  (const void *w, Touch_t &touch);
static void onCalibrationYup    (const void *w, Touch_t &touch);
static void onCalibrationYdown  (const void *w, Touch_t &touch);
static void onCalibrationClose  (const void *w, Touch_t &touch);
static void onCalibrationApply  (const void *w, Touch_t &touch);

static constexpr Widget_t widget_calibration[] = {
  {   0,   0, 320, 240, image_calibration, EVENT_NONE,  onCalibrationScreen   },
  {  22,  12,  70,  70, NULL,              EVENT_UP,    onCalibrationExec     },
  { 125,  12,  70,  70, NULL,              EVENT_UP,    onCalibrationAdjust   },
  { 228,  12,  70,  70, image_save_flush,  EVENT_UP,    onCalibrationSave     },
  { 164, 154,  26,  26, image_arrowU,      EVENT_CLICK, onCalibrationXup      },
  { 164, 180,  26,  26, image_arrowD,      EVENT_CLICK, onCalibrationXdown    },
  { 263, 154,  26,  26, image_arrowU,      EVENT_CLICK, onCalibrationYup      },
  { 263, 180,  26,  26, image_arrowD,      EVENT_CLICK, onCalibrationYdown    },
  {  60, 206,  30,  30, NULL,              EVENT_ALL,   onCalibrationClose    },
  { 230, 206,  30,  30, image_icon_apply,  EVENT_CLICK, onCalibrationApply    },
};

// Screen - Adjust Offset
static void onAdjustOffsetScreen(const void *w, Touch_t &touch);
static void onAdjustOffsetTarget(const void *w, Touch_t &touch);
static void onAdjustOffsetClose (const void *w, Touch_t &touch);
static void onAdjustOffsetApply (const void *w, Touch_t &touch);

static constexpr Widget_t widget_adjust_offset[] = {
  {  26,  90, 270,  60, image_adjust_center, EVENT_NONE,  onAdjustOffsetScreen },
  { 144, 104,  32,  32, image_target,        EVENT_DOWN,  onAdjustOffsetTarget },
  {  60, 206,  30,  30, NULL,                EVENT_ALL,   onAdjustOffsetClose  },
  { 230, 206,  30,  30, image_icon_apply,    EVENT_CLICK, onAdjustOffsetApply  },
};

// Screen - Information
static void onInformationScreen(const void *w, Touch_t &touch);
static void onInformationClose (const void *w, Touch_t &touch);

static constexpr Widget_t widget_information[] = {
  {   0,   0, 320, 240, image_information, EVENT_NONE, onInformationScreen   },
  { 145, 206,  30,  30, NULL,              EVENT_ALL,  onInformationClose    },
};

/*--------------------------------------------------------------------------------
 * Callback functions - Main
 *--------------------------------------------------------------------------------*/
static void onMainScreen(const void *w, Touch_t &touch) {
  DBG_EXEC(printf("%s\n", __func__));

  DrawScreen(static_cast<const Widget_t*>(w));
}

static void onMainInside(const void *w, Touch_t &touch) {
  DBG_EXEC(printf("%s\n", __func__));

  if (touch.event != EVENT_NONE) {
    // pick up the point
  }
}

static void onMainOutside(const void *w, Touch_t &touch) {
  DBG_EXEC(printf("%s\n", __func__));

  if (touch.event != EVENT_NONE) {
    // Reset the picked up point
  }
}

static void onMainThermograph(const void *w, Touch_t &touch) {
  DBG_EXEC(printf("%s\n", __func__));

  if (touch.event != EVENT_NONE) {
    widget_state(STATE_THERMOGRAPH);
  }
}

static void onMainCamera(const void *w, Touch_t &touch) {
  DBG_EXEC(printf("%s\n", __func__));

  uint8_t offset;
  if (cnf.capture_mode == false) {
    offset = 0;
  } else if (cnf.video_recording == false) {
    offset = 2;
  } else {
    offset = 3;
  }

  if (touch.event == EVENT_NONE) {
    DrawButton(static_cast<const Widget_t*>(w), cnf.capture_mode == 0 ? 0 : 2);
  }

  else if (cnf.capture_mode == 0) {
    DrawButton(static_cast<const Widget_t*>(w), 1); // draw icon_camera2
    sdcard_save();
    DrawButton(static_cast<const Widget_t*>(w), 0); // draw icon_camera1
  }

  else if (cnf.video_recording == false) {
    DrawButton(static_cast<const Widget_t*>(w), 3); // draw icon_stop
    cnf.video_recording = true;
  }

  else {
    DrawButton(static_cast<const Widget_t*>(w), 2); // draw icon_video
    cnf.video_recording = false;
  }
}

static void onMainConfiguration(const void *w, Touch_t &touch) {
  DBG_EXEC(printf("%s\n", __func__));

  if (touch.event != EVENT_NONE) {
    widget_state(STATE_CONFIGURATION);
  }
}

/*--------------------------------------------------------------------------------
 * Callback functions - Configuration
 *--------------------------------------------------------------------------------*/
static void onConfigurationScreen(const void *w, Touch_t &touch) {
  DBG_EXEC(printf("%s\n", __func__));

  DrawScreen(static_cast<const Widget_t*>(w));
}

static void onConfigurationResolution(const void *w, Touch_t &touch) {
  DBG_EXEC(printf("%s\n", __func__));

  if (touch.event != EVENT_NONE) {
    widget_state(STATE_RESOLUTION);
  }
}

static void onConfigurationThermograph(const void *w, Touch_t &touch) {
  DBG_EXEC(printf("%s\n", __func__));

  if (touch.event != EVENT_NONE) {
    widget_state(STATE_THERMOGRAPH);
  }
}

static void onConfigurationCaptureMode(const void *w, Touch_t &touch) {
  DBG_EXEC(printf("%s\n", __func__));

  if (touch.event != EVENT_NONE) {
    widget_state(STATE_CAPTURE_MODE);
  }
}

static void onConfigurationFileManager(const void *w, Touch_t &touch) {
  DBG_EXEC(printf("%s\n", __func__));

  if (touch.event != EVENT_NONE) {
    widget_state(STATE_FILE_MANAGER);
  }
}

static void onConfigurationCalibration(const void *w, Touch_t &touch) {
  DBG_EXEC(printf("%s\n", __func__));

  if (touch.event != EVENT_NONE) {
    widget_state(STATE_CALIBRATION);
  }
}

static void onConfigurationInformation(const void *w, Touch_t &touch) {
  DBG_EXEC(printf("%s\n", __func__));

  if (touch.event != EVENT_NONE) {
    widget_state(STATE_INFORMATION);
  }
}

static void onConfigurationReturn(const void *w, Touch_t &touch) {
  DBG_EXEC(printf("%s\n", __func__));

  if (touch.event != EVENT_NONE) {
    widget_state(STATE_ON);
  }
}

/*--------------------------------------------------------------------------------
 * Callback functions - Resolution
 *--------------------------------------------------------------------------------*/
static void onResolutionScreen(const void *w, Touch_t &touch) {
  DBG_EXEC(printf("%s\n", __func__));

  DrawScreen(static_cast<const Widget_t*>(w));
}

static void onResolutionSlider1(const void *w, Touch_t &touch) {
  DBG_EXEC(printf("%s\n", __func__));

  const Widget_t *widget = static_cast<const Widget_t*>(w);
  int16_t X = touch.x   - widget->x - widget->h / 2;
  int16_t W = widget->w - widget->h - SLIDER_KNOB_OFFSET;
  DrawSlider(widget, constrain(X, SLIDER_KNOB_OFFSET, W));
}

static void onResolutionSlider2(const void *w, Touch_t &touch) {
  DBG_EXEC(printf("%s\n", __func__));

  const Widget_t *widget = static_cast<const Widget_t*>(w);
  int16_t X = touch.x   - widget->x - widget->h / 2;
  int16_t W = widget->w - widget->h - SLIDER_KNOB_OFFSET;
  DrawSlider(widget, constrain(X, SLIDER_KNOB_OFFSET, W));
}

static void onResolutionClose(const void *w, Touch_t &touch) {
  DBG_EXEC(printf("%s\n", __func__));

  if (touch.event != EVENT_NONE) {
    widget_state(STATE_CONFIGURATION);
  }
}

static void onResolutionApply(const void *w, Touch_t &touch) {
  DBG_EXEC(printf("%s\n", __func__));

  if (touch.event == EVENT_NONE) {
    DrawButton(static_cast<const Widget_t*>(w), 0);
  } else {
    DrawPress(static_cast<const Widget_t*>(w), touch.event);
  }
}

/*--------------------------------------------------------------------------------
 * Callback functions - Thermograph
 *--------------------------------------------------------------------------------*/
static void onThermographScreen(const void *w, Touch_t &touch) {
  DBG_EXEC(printf("%s\n", __func__));

  DrawScreen(static_cast<const Widget_t*>(w));
}

static void onThermographRadio1(const void *w, Touch_t &touch) {
  DBG_EXEC(printf("%s\n", __func__));

  if (touch.event != EVENT_NONE) {
    cnf.color_scheme = 0;
  }

  DrawRadio(static_cast<const Widget_t*>(w), 2, cnf.color_scheme);
}

static void onThermographRadio2(const void *w, Touch_t &touch) {
  DBG_EXEC(printf("%s\n", __func__));

  if (touch.event != EVENT_NONE) {
    cnf.color_scheme = 1;
  }

  DrawRadio(static_cast<const Widget_t*>(w) - 1, 2, cnf.color_scheme);
}

static void onThermographToggle1(const void *w, Touch_t &touch) {
  DBG_EXEC(printf("%s\n", __func__));

  if (touch.event != EVENT_NONE) {
    cnf.minmax_auto = !cnf.minmax_auto;
  }

  DrawToggle(static_cast<const Widget_t*>(w), cnf.minmax_auto);
}

static void onThermographToggle2(const void *w, Touch_t &touch) {
  DBG_EXEC(printf("%s\n", __func__));

  if (touch.event != EVENT_NONE) {
    cnf.range_auto = !cnf.range_auto;
  }

  DrawToggle(static_cast<const Widget_t*>(w), cnf.range_auto);
}

static void onThermographSlider1(const void *w, Touch_t &touch) {
  DBG_EXEC(printf("%s\n", __func__));

  const Widget_t *widget = static_cast<const Widget_t*>(w);
  int16_t X = touch.x   - widget->x - widget->h / 2;
  int16_t W = widget->w - widget->h - SLIDER_KNOB_OFFSET;
  DrawSlider(widget, constrain(X, SLIDER_KNOB_OFFSET, W));
}

static void onThermographSlider2(const void *w, Touch_t &touch) {
  DBG_EXEC(printf("%s\n", __func__));

  const Widget_t *widget = static_cast<const Widget_t*>(w);
  int16_t X = touch.x   - widget->x - widget->h / 2;
  int16_t W = widget->w - widget->h - SLIDER_KNOB_OFFSET;
  DrawSlider(widget, constrain(X, SLIDER_KNOB_OFFSET, W));
}

static void onThermographClose(const void *w, Touch_t &touch) {
  DBG_EXEC(printf("%s\n", __func__));

  if (touch.event != EVENT_NONE) {
    widget_state(STATE_CONFIGURATION);
  }
}

static void onThermographApply(const void *w, Touch_t &touch) {
  DBG_EXEC(printf("%s\n", __func__));

  if (touch.event == EVENT_NONE) {
    DrawButton(static_cast<const Widget_t*>(w), 0);
  } else {
    DrawPress(static_cast<const Widget_t*>(w), touch.event);
  }
}

/*--------------------------------------------------------------------------------
 * Callback functions - File Manager
 *--------------------------------------------------------------------------------*/
static std::vector<FileInfo_t> files;
static int n_files;
static int scroll_pos, scroll_max, bar_height;

static void ScrollView(const Widget_t *widget, int scroll_pos) {
  static LGFX_Sprite sprite_view;
  bool invert = false;

  sprite_view.setTextSize(2);
  sprite_view.setTextWrap(false);
  sprite_view.setTextColor(WHITE);
  sprite_view.createSprite(VIEW_WIDTH, VIEW_HEIGHT);

  int scaled_pos = scroll_pos * widget->h / bar_height;
  int item_head  = scaled_pos / ITEM_HEIGHT;
  int item_tail  = item_head + (VIEW_ITEMS + 1);
  item_tail = min(item_tail, n_files);
  DBG_EXEC(printf("item_head: %d, item_tail: %d\n", item_head, item_tail));

  int base_pos  = item_head * ITEM_HEIGHT;
  int delta_pos = base_pos - scaled_pos + FONT_MARGIN - ITEM_HEIGHT;

  for (int i = item_head; i < item_tail; i++) {
    delta_pos += ITEM_HEIGHT;

    if (invert == false && files[i].isSelected == true) {
      invert = true;
      sprite_view.setTextColor(BLACK);
    } else

    if (invert == true && files[i].isSelected == false) {
      invert = false;
      sprite_view.setTextColor(WHITE);
    }

    if (invert == true) {
      sprite_view.fillRect(0, delta_pos - FONT_MARGIN, VIEW_WIDTH, ITEM_HEIGHT, WHITE);
    }

    sprite_view.setCursor(FONT_MARGIN, delta_pos);
    const char *p = strrchr(files[i].path.c_str(), '/');
    sprite_view.print(p ? p + 1 : "");
  }

  sprite_view.pushSprite(&lcd, widget->x, widget->y);
  sprite_view.deleteSprite();
}

static void onFileManagerScreen(const void *w, Touch_t &touch) {
  DBG_EXEC(printf("%s\n", __func__));

  DrawScreen(static_cast<const Widget_t*>(w));

  if (sdcard_open()) {
    uint32_t total, free;
    sdcard_size(&total, &free);
    GFX_EXEC(setTextSize(2));
    gfx_printf(247, 14, "%4luMB", total);
    gfx_printf(247, 42, "%4luMB", free);

    files.clear();
    GetFileList(SD, "/", 1, files);
    n_files = files.size();
  
    files[0].isSelected = true;
    files[9].isSelected = true;
    files[n_files-1].isSelected = true;
  }
}

static void onFileManagerCheckAll(const void *w, Touch_t &touch) {
  DBG_EXEC(printf("%s\n", __func__));

  if (touch.event != EVENT_NONE) {
    cnf.file_selected = !cnf.file_selected;
    for (auto& file : files) {
      file.isSelected = cnf.file_selected;
    }
    ScrollView(static_cast<const Widget_t*>(w) + 1, scroll_pos);
  }

  DrawCheck(static_cast<const Widget_t*>(w), cnf.file_selected);
}

static void onFileManagerScrollBox(const void *w, Touch_t &touch) {
  DBG_EXEC(printf("%s\n", __func__));

  if (touch.event != EVENT_NONE) {
    const Widget_t *widget = static_cast<const Widget_t*>(w);
    int scaled_pos = scroll_pos * widget->h / bar_height;
    int selected =  (scaled_pos + touch.y - widget->y - FONT_MARGIN) / ITEM_HEIGHT;
    files[selected].isSelected = !files[selected].isSelected;

    ScrollView(widget, scroll_pos);

    const Widget_t *thumbnail = widget + 2; 
    if (files[selected].isSelected) {
      DBG_EXEC(printf("path: %s\n", files[selected].path.c_str()));
      GFX_EXEC(drawBmpFile(
//      SD,
        files[selected].path.c_str(),
        thumbnail->x, thumbnail->y, thumbnail->w, thumbnail->h,
        0, 0,
        0.25, 0.25
      ));
    } else {
      GFX_EXEC(fillRect(thumbnail->x, thumbnail->y, thumbnail->w, thumbnail->h, BLACK));
    }
  }
}

static void onFileManagerScrollBar(const void *w, Touch_t &touch) {
  DBG_EXEC(printf("%s\n", __func__));

  static int drag_pos;
  static LGFX_Sprite sprite_scroll;
  const Widget_t *widget = static_cast<const Widget_t*>(w);

  if (touch.event == EVENT_NONE) {
    scroll_pos = drag_pos = 0;
    if (n_files > VIEW_ITEMS) {
      scroll_max = widget->h * (n_files - VIEW_ITEMS) / n_files;
      bar_height = widget->h - scroll_max;
    } else {
      bar_height = widget->h;
      scroll_max = 0;
    }
  }

  else if (touch.event == EVENT_DOWN) {
    drag_pos = touch.y;
  } // else drag

  // Relative movement from the reference position
  scroll_pos += touch.y - drag_pos;
  scroll_pos = constrain(scroll_pos, 0, scroll_max);
//DBG_EXEC(printf("scroll_pos: %d, scroll_max: %d\n", scroll_pos, scroll_max));

  lcd.beginTransaction();

  sprite_scroll.createSprite(widget->w, widget->h);
  sprite_scroll.fillRect(0, scroll_pos, widget->w, bar_height, SCROLL_COLOR);
  sprite_scroll.pushSprite(&lcd, widget->x, widget->y);
  sprite_scroll.deleteSprite();

  ScrollView(widget - 1, scroll_pos);
  lcd.endTransaction();

  // Update the reference position
  drag_pos = touch.y;
}

static void onFileManagerThumbnail(const void *w, Touch_t &touch) {
  DBG_EXEC(printf("%s\n", __func__));
}

static void onFileManagerMovie(const void *w, Touch_t &touch) {
  DBG_EXEC(printf("%s\n", __func__));

  if (touch.event == EVENT_NONE) {
    DrawButton(static_cast<const Widget_t*>(w));
  } else {
    DrawPress(static_cast<const Widget_t*>(w), touch.event);
  }
}

static void onFileManagerFolder(const void *w, Touch_t &touch) {
  DBG_EXEC(printf("%s\n", __func__));

  if (touch.event == EVENT_NONE) {
    DrawButton(static_cast<const Widget_t*>(w));
  } else {
    DrawPress(static_cast<const Widget_t*>(w), touch.event);
  }
}

static void onFileManagerClose(const void *w, Touch_t &touch) {
  DBG_EXEC(printf("%s\n", __func__));

  if (touch.event != EVENT_NONE) {
    widget_state(STATE_CONFIGURATION);
  }
}

static void onFileManagerApply(const void *w, Touch_t &touch) {
  DBG_EXEC(printf("%s\n", __func__));

  DrawPress(static_cast<const Widget_t*>(w), touch.event);

  if (touch.event != EVENT_NONE) {
    // Remove files
  }
}

/*--------------------------------------------------------------------------------
 * Callback functions - Capture Mode
 *--------------------------------------------------------------------------------*/
static void onCaptureModeScreen(const void *w, Touch_t &touch) {
  DBG_EXEC(printf("%s\n", __func__));

  DrawScreen(static_cast<const Widget_t*>(w));
}

static void onCaptureModeCamera(const void *w, Touch_t &touch) {
  DBG_EXEC(printf("%s\n", __func__));

  if (touch.event != EVENT_NONE) {
    cnf.capture_mode = 0;
  }

  DrawRadio(static_cast<const Widget_t*>(w), 2, cnf.capture_mode);
}

static void onCaptureModeVideo(const void *w, Touch_t &touch) {
  DBG_EXEC(printf("%s\n", __func__));

  if (touch.event != EVENT_NONE) {
    cnf.capture_mode = 1;
  }

  DrawRadio(static_cast<const Widget_t*>(w) - 1, 2, cnf.capture_mode);
}

static void onCaptureModeApply(const void *w, Touch_t &touch) {
  DBG_EXEC(printf("%s\n", __func__));

  if (touch.event != EVENT_NONE) {
    widget_state(STATE_CONFIGURATION);
  }
}

/*--------------------------------------------------------------------------------
 * Callback functions - Calibration
 *--------------------------------------------------------------------------------*/
#define OFFSET_MIN    (-10)
#define OFFSET_MAX    ( 10)
#define OFFSET_X_ROW  123
#define OFFSET_X_COL  172
#define OFFSET_Y_ROW  223
#define OFFSET_Y_COL  172

static void DrawOffsetX(const Widget_t* widget) {
  DrawButton(widget,     (cnf.touch_offset[0] < OFFSET_MAX) ? 1 : 0);
  DrawButton(widget + 1, (cnf.touch_offset[0] > OFFSET_MIN) ? 1 : 0);

  gfx_printf(OFFSET_X_ROW, OFFSET_X_COL, "%3d", (int)cnf.touch_offset[0]);
}

static void DrawOffsetY(const Widget_t* widget) {
  DrawButton(widget,     (cnf.touch_offset[1] < OFFSET_MAX) ? 1 : 0);
  DrawButton(widget + 1, (cnf.touch_offset[1] > OFFSET_MIN) ? 1 : 0);

  gfx_printf(OFFSET_Y_ROW, OFFSET_Y_COL, "%3d", (int)cnf.touch_offset[1]);
}

static void onCalibrationScreen (const void *w, Touch_t &touch) {
  DBG_EXEC(printf("%s\n", __func__));

  GFX_EXEC(setTextSize(2));
  DrawScreen(static_cast<const Widget_t*>(w));
}

static void onCalibrationExec(const void *w, Touch_t &touch) {
  DBG_EXEC(printf("%s\n", __func__));
}

static void onCalibrationAdjust(const void *w, Touch_t &touch) {
  DBG_EXEC(printf("%s\n", __func__));

  if (touch.event != EVENT_NONE) {
    widget_state(STATE_ADJUST_OFFSET);
  }
}

static void onCalibrationSave(const void *w, Touch_t &touch) {
  DBG_EXEC(printf("%s\n", __func__));

  DrawButton(static_cast<const Widget_t*>(w), 0);
}

static void onCalibrationXup(const void *w, Touch_t &touch) {
  DBG_EXEC(printf("%s\n", __func__));

  if (touch.event != EVENT_NONE && cnf.touch_offset[0] < OFFSET_MAX) {
    DrawPress(static_cast<const Widget_t*>(w), touch.event);
    if (touch.event == EVENT_DOWN ) {
      cnf.touch_offset[0]++;
    }
  }

  DrawOffsetX(static_cast<const Widget_t*>(w));
}

static void onCalibrationXdown(const void *w, Touch_t &touch) {
  DBG_EXEC(printf("%s\n", __func__));

  if (touch.event != EVENT_NONE && cnf.touch_offset[0] > OFFSET_MIN) {
    DrawPress(static_cast<const Widget_t*>(w), touch.event);
    if (touch.event == EVENT_DOWN) {
      cnf.touch_offset[0]--;
    }
  }

  DrawOffsetX(static_cast<const Widget_t*>(w) - 1);
}

static void onCalibrationYup(const void *w, Touch_t &touch) {
  DBG_EXEC(printf("%s\n", __func__));

  if (touch.event != EVENT_NONE && cnf.touch_offset[1] < OFFSET_MAX) {
    DrawPress(static_cast<const Widget_t*>(w), touch.event);
    if (touch.event == EVENT_DOWN) {
      cnf.touch_offset[1]++;
    }
  }

  DrawOffsetY(static_cast<const Widget_t*>(w));
}

static void onCalibrationYdown(const void *w, Touch_t &touch) {
  DBG_EXEC(printf("%s\n", __func__));

  if (touch.event != EVENT_NONE && cnf.touch_offset[1] > OFFSET_MIN) {
    DrawPress(static_cast<const Widget_t*>(w), touch.event);
    if (touch.event == EVENT_DOWN) {
      cnf.touch_offset[1]--;
    }
  }

  DrawOffsetY(static_cast<const Widget_t*>(w) - 1);
}

static void onCalibrationClose(const void *w, Touch_t &touch) {
  DBG_EXEC(printf("%s\n", __func__));

  if (touch.event == EVENT_DOWN) {
    widget_state(STATE_CONFIGURATION);
  }
}

static void onCalibrationApply(const void *w, Touch_t &touch) {
  DBG_EXEC(printf("%s\n", __func__));

  if (touch.event == EVENT_NONE) {
    DrawButton(static_cast<const Widget_t*>(w), 0);
  } else {
    DrawPress(static_cast<const Widget_t*>(w), touch.event);
  }
}

/*--------------------------------------------------------------------------------
 * Callback functions - Adjust Offset
 *--------------------------------------------------------------------------------*/
static void onAdjustOffsetScreen(const void *w, Touch_t &touch) {
  DBG_EXEC(printf("%s\n", __func__));

  DrawScreen(static_cast<const Widget_t*>(w));
  DrawButton(static_cast<const Widget_t*>(w) + 1);
}

static void onAdjustOffsetTarget(const void *w, Touch_t &touch) {
  DBG_EXEC(printf("%s\n", __func__));

  if (touch.event != EVENT_NONE) {
    DrawButton(static_cast<const Widget_t*>(w), 1);

    cnf.touch_offset[0] = lcd_width  / 2 - 1 - touch.x;
    cnf.touch_offset[1] = lcd_height / 2 - 1 - touch.y;

    cnf.touch_offset[0] = constrain(cnf.touch_offset[0], OFFSET_MIN, OFFSET_MAX);
    cnf.touch_offset[1] = constrain(cnf.touch_offset[1], OFFSET_MIN, OFFSET_MAX);

    gfx_printf(OFFSET_X_ROW, OFFSET_X_COL, "%3d", (int)cnf.touch_offset[0]);
    gfx_printf(OFFSET_Y_ROW, OFFSET_Y_COL, "%3d", (int)cnf.touch_offset[1]);
  }
}

static void onAdjustOffsetClose(const void *w, Touch_t &touch) {
  DBG_EXEC(printf("%s\n", __func__));

  if (touch.event != EVENT_NONE) {
    widget_state(STATE_CALIBRATION);
  }
}

static void onAdjustOffsetApply(const void *w, Touch_t &touch) {
  DBG_EXEC(printf("%s\n", __func__));

  if (touch.event == EVENT_NONE) {
    DrawButton(static_cast<const Widget_t*>(w), 0);
  } else {
    DrawPress(static_cast<const Widget_t*>(w), touch.event);
  }
}

/*--------------------------------------------------------------------------------
 * Callback functions - Information
 *--------------------------------------------------------------------------------*/
static void onInformationScreen (const void *w, Touch_t &touch) {
  DBG_EXEC(printf("%s\n", __func__));

  DrawScreen(static_cast<const Widget_t*>(w));
}

static void onInformationClose  (const void *w, Touch_t &touch) {
  DBG_EXEC(printf("%s\n", __func__));

  if (touch.event != EVENT_NONE) {
    widget_state(STATE_CONFIGURATION);
  }
}