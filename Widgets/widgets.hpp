
/*================================================================================
 * Wedget manager
 *================================================================================*/
#include <Arduino.h>
#include "widgets.h"

/*--------------------------------------------------------------------------------
 * Image data of widget
 *--------------------------------------------------------------------------------*/
static constexpr Image_t image_black[]              = { { screen_black,         sizeof(screen_black         ) }, }; // 320 x 240
static constexpr Image_t image_main[]               = { { screen_main,          sizeof(screen_main          ) }, }; // 320 x 240
static constexpr Image_t image_configuration[]      = { { screen_configuration, sizeof(screen_configuration ) }, }; // 320 x 240
static constexpr Image_t image_resolution[]         = { { screen_resolution,    sizeof(screen_resolution    ) }, }; // 320 x 240
static constexpr Image_t image_thermograph[]        = { { screen_thermograph,   sizeof(screen_thermograph   ) }, }; // 320 x 240
static constexpr Image_t image_file_manager[]       = { { screen_file_manager,  sizeof(screen_file_manager  ) }, }; // 320 x 240
static constexpr Image_t image_capture_mode[]       = { { screen_capture_mode,  sizeof(screen_capture_mode  ) }, }; // 320 x 240
static constexpr Image_t image_calibration[]        = { { screen_calibration,   sizeof(screen_calibration   ) }, }; // 320 x 240
static constexpr Image_t image_information[]        = { { screen_information,   sizeof(screen_information   ) }, }; // 320 x 240
static constexpr Image_t image_icon_configuration[] = { { icon_configuration,   sizeof(icon_configuration   ) }, }; //  50 x  50
static constexpr Image_t image_icon_video[]         = { { icon_video,           sizeof(icon_video           ) }, }; //  50 x  50
static constexpr Image_t image_icon_stop[]          = { { icon_stop,            sizeof(icon_stop            ) }, }; //  50 x  50
static constexpr Image_t image_icon_apply[]         = { { icon_apply,           sizeof(icon_apply           ) }, }; //  50 x  50
static constexpr Image_t image_icon_cancel[]        = { { icon_cancel,          sizeof(icon_cancel          ) }, }; //  50 x  50

static constexpr Image_t image_icon_camera[] = {
  { icon_camera1, sizeof(icon_camera1) }, // 50 x 50
  { icon_camera2, sizeof(icon_camera2) }, // 50 x 50
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
  { radio_on,  sizeof(radio_on ) }, // 26 x 26
  { radio_off, sizeof(radio_off) }, // 26 x 26
};
static constexpr Image_t image_toggle[] = {
  { toggle_on,  sizeof(toggle_on ) }, // 42 x 26
  { toggle_off, sizeof(toggle_off) }, // 42 x 26
};
static constexpr Image_t image_arrowL[] = {
  { icon_arrow_Lon,  sizeof(icon_arrow_Lon ) }, // 26 x 26
  { icon_arrow_Loff, sizeof(icon_arrow_Loff) }, // 26 x 26
};
static constexpr Image_t image_arrowR[] = {
  { icon_arrow_Ron,  sizeof(icon_arrow_Ron ) }, // 26 x 26
  { icon_arrow_Roff, sizeof(icon_arrow_Roff) }, // 26 x 26
};
static constexpr Image_t image_checkbox[] = {
  { icon_checkbox_on,  sizeof(icon_checkbox_on ) }, // 18 x 18
  { icon_checkbox_off, sizeof(icon_checkbox_off) }, // 18 x 18
};
static constexpr Image_t image_save_eeprom[] = {
  { save_eeprom_on,  sizeof(save_eeprom_on ) }, // 52 x 52
  { save_eeprom_off, sizeof(save_eeprom_off) }, // 52 x 52
};

/*--------------------------------------------------------------------------------
 * List of widget instances on the screen
 *--------------------------------------------------------------------------------*/
// Screen - Main
static void onMainScreen        (const void *w, Touch_t &touch);
static void onMainInside        (const void *w, Touch_t &touch);
static void onMainOutside       (const void *w, Touch_t &touch);
static void onMainThermograph   (const void *w, Touch_t &touch);
static void onMainCamera        (const void *w, Touch_t &touch);
static void onMainConfiguration (const void *w, Touch_t &touch);

static constexpr Widget_t widget_main[] = {
  {   0,   0, 320,         240,          image_main,        EVENT_NONE, onMainScreen        },
  {   0,   0, 256,          92,          NULL,              EVENT_ALL,  onMainInside        },
  { 256,   0,  64,         140,          NULL,              EVENT_ALL,  onMainOutside       },
  {   0, 195, 256,          45,          NULL,              EVENT_ALL,  onMainThermograph   },
  { 265, 135, ICON1_WIDTH, ICON1_HEIGHT, image_icon_camera, EVENT_DOWN, onMainCamera        },
  { 265, 185, ICON1_WIDTH, ICON1_HEIGHT, NULL,              EVENT_ALL,  onMainConfiguration },
};

// Screen - Configuration
static void onConfigurationScreen       (const void *w, Touch_t &touch);
static void onConfigurationResolution   (const void *w, Touch_t &touch);
static void onConfigurationThermograph  (const void *w, Touch_t &touch);
static void onConfigurationFileManager  (const void *w, Touch_t &touch);
static void onConfigurationCaptureMode  (const void *w, Touch_t &touch);
static void onConfigurationCalibration  (const void *w, Touch_t &touch);
static void onConfigurationInformation  (const void *w, Touch_t &touch);
static void onConfigurationReturn       (const void *w, Touch_t &touch);

static constexpr Widget_t widget_configuration[] = {
  {   0,   0, 320,         240,          image_configuration,     EVENT_NONE, onConfigurationScreen      },
  {  22,  10, ICON2_WIDTH, ICON2_HEIGHT, NULL,                    EVENT_ALL,  onConfigurationResolution  },
  { 124,  10, ICON2_WIDTH, ICON2_HEIGHT, NULL,                    EVENT_ALL,  onConfigurationThermograph },
  { 226,  10, ICON2_WIDTH, ICON2_HEIGHT, NULL,                    EVENT_ALL,  onConfigurationFileManager },
  {  22, 100, ICON2_WIDTH, ICON2_HEIGHT, NULL,                    EVENT_ALL,  onConfigurationCaptureMode },
  { 124, 100, ICON2_WIDTH, ICON2_HEIGHT, NULL,                    EVENT_ALL,  onConfigurationCalibration },
  { 226, 100, ICON2_WIDTH, ICON2_HEIGHT, NULL,                    EVENT_ALL,  onConfigurationInformation },
  { 134, 192, ICON1_WIDTH, ICON1_HEIGHT, NULL,                    EVENT_ALL,  onConfigurationReturn      },
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
  {  55, 200,  45,  45, NULL,             EVENT_ALL,   onResolutionClose   },
  { 222, 200,  45,  45, NULL,             EVENT_CLICK, onResolutionApply   },
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
  {  60, 136, 213,  26, image_slider2,     EVENT_DRAG,  onThermographSlider1 },
  {  60, 174, 213,  26, image_slider2,     EVENT_DRAG,  onThermographSlider2 },
  {  55, 200,  45,  45, NULL,              EVENT_ALL,   onThermographClose   },
  { 222, 200,  45,  45, NULL,              EVENT_CLICK, onThermographApply   },
};

// Screen - File manager
static void onFileManagerScreen   (const void *w, Touch_t &touch);
static void onFileManagerCheckAll (const void *w, Touch_t &touch);
static void onFileManagerList1    (const void *w, Touch_t &touch);
static void onFileManagerList2    (const void *w, Touch_t &touch);
static void onFileManagerPrev     (const void *w, Touch_t &touch);
static void onFileManagerNext     (const void *w, Touch_t &touch);
static void onFileManagerClose    (const void *w, Touch_t &touch);
static void onFileManagerApply    (const void *w, Touch_t &touch);

static constexpr Widget_t widget_file_manager[] = {
  {   0,   0, 320, 240, image_file_manager, EVENT_NONE,  onFileManagerScreen   },
  {  12,  30, 112,  18, image_checkbox,     EVENT_DOWN,  onFileManagerCheckAll },
  {  30,  52, 128, 110, NULL,               EVENT_DOWN,  onFileManagerList1    },
  { 168,  52, 128, 110, NULL,               EVENT_DOWN,  onFileManagerList2    },
  { 110, 175,  26,  32, image_arrowL,       EVENT_CLICK, onFileManagerPrev     },
  { 180, 175,  26,  32, image_arrowR,       EVENT_CLICK, onFileManagerNext     },
  {  55, 200,  45,  45, NULL,               EVENT_ALL,   onFileManagerClose    },
  { 222, 200,  45,  45, NULL,               EVENT_CLICK, onFileManagerApply    },
};

// Screen - Capture mode
static void onCaptureModeScreen (const void *w, Touch_t &touch);
static void onCaptureModeCamera (const void *w, Touch_t &touch);
static void onCaptureModeVideo  (const void *w, Touch_t &touch);
static void onCaptureModeApply  (const void *w, Touch_t &touch);

static constexpr Widget_t widget_capture_mode[] = {
  {   0,   0, 320, 240, image_capture_mode, EVENT_NONE,  onCaptureModeScreen   },
  {  48,  50, 240,  26, image_radio,        EVENT_DOWN,  onCaptureModeCamera   },
  {  48, 126, 240,  26, image_radio,        EVENT_DOWN,  onCaptureModeVideo    },
  { 136, 200,  45,  45, NULL,               EVENT_ALL,   onCaptureModeApply    },
};

// Screen - Calibration
static void onCalibrationScreen (const void *w, Touch_t &touch);
static void onCalibrationExec   (const void *w, Touch_t &touch);
static void onCalibrationSave   (const void *w, Touch_t &touch);
static void onCalibrationClose  (const void *w, Touch_t &touch);
static void onCalibrationApply  (const void *w, Touch_t &touch);

static constexpr Widget_t widget_calibration[] = {
  {   0,   0, 320, 240, image_calibration,  EVENT_NONE,  onCalibrationScreen   },
  {  80,  24,  52,  52, NULL,               EVENT_DOWN,  onCalibrationExec     },
  { 186,  24,  52,  52, image_save_eeprom,  EVENT_DOWN,  onCalibrationSave     },
  {  55, 200,  45,  45, NULL,               EVENT_ALL,   onCalibrationClose    },
  { 222, 200,  45,  45, NULL,               EVENT_CLICK, onCalibrationApply    },
};

// Screen - Information
static void onInformationScreen (const void *w, Touch_t &touch);
static void onInformationClose  (const void *w, Touch_t &touch);

static constexpr Widget_t widget_information[] = {
  {   0,   0, 320, 240, image_information, EVENT_NONE,  onInformationScreen   },
  { 136, 200,  45,  45, NULL,              EVENT_ALL,   onInformationClose    },
};

/*--------------------------------------------------------------------------------
 * Widget callback functions for Main
 *--------------------------------------------------------------------------------*/
static void onMainScreen(const void *w, Touch_t &touch) {
  POS_CHECK((const Widget_t*)w);
  DBG_EXEC(printf("onMainScreen\n"));

  DrawSetup((const Widget_t*)w, touch, DRAW_WIDGET);
}

static void onMainInside(const void *w, Touch_t &touch) {
  POS_CHECK((const Widget_t*)w);
  DBG_EXEC(printf("onMainInside\n"));
}

static void onMainOutside(const void *w, Touch_t &touch) {
  POS_CHECK((const Widget_t*)w);
  DBG_EXEC(printf("onMainOutside\n"));
}

static void onMainThermograph(const void *w, Touch_t &touch) {
  POS_CHECK((const Widget_t*)w);
  DBG_EXEC(printf("onMainThermograph\n"));

  widget_state(STATE_THERMOGRAPH);
}

static void onMainCamera(const void *w, Touch_t &touch) {
  POS_CHECK((const Widget_t*)w);
  DBG_EXEC(printf("onMainCamera\n"));

  if (!DrawSetup((const Widget_t*)w, touch, DRAW_SPRITE)) {
    DrawSprite((const Widget_t*)w, 1); // icon_camera2
    sdcard_save();
    DrawSprite((const Widget_t*)w, 0); // icon_camera1
  }
}

static void onMainConfiguration(const void *w, Touch_t &touch) {
  POS_CHECK((const Widget_t*)w);
  DBG_EXEC(printf("onMainConfiguration\n"));

  widget_state(STATE_CONFIGURATION);
}

/*--------------------------------------------------------------------------------
 * Widget callback functions for Configuration
 *--------------------------------------------------------------------------------*/
static void onConfigurationScreen(const void *w, Touch_t &touch) {
  POS_CHECK((const Widget_t*)w);
  DBG_EXEC(printf("onConfigurationScreen\n"));

  DrawSetup((const Widget_t*)w, touch, DRAW_WIDGET);
}

static void onConfigurationResolution(const void *w, Touch_t &touch) {
  POS_CHECK((const Widget_t*)w);
  DBG_EXEC(printf("onConfigurationResolution\n"));

  if (!DrawSetup((const Widget_t*)w, touch, DRAW_SPRITE)) {
    widget_state(STATE_RESOLUTION);
  }
}

static void onConfigurationThermograph(const void *w, Touch_t &touch) {
  POS_CHECK((const Widget_t*)w);
  DBG_EXEC(printf("onConfigurationThermograph\n"));

  if (!DrawSetup((const Widget_t*)w, touch, DRAW_SPRITE)) {
    widget_state(STATE_THERMOGRAPH);
  }
}

static void onConfigurationFileManager(const void *w, Touch_t &touch) {
  POS_CHECK((const Widget_t*)w);
  DBG_EXEC(printf("onConfigurationFileManager\n"));

  if (!DrawSetup((const Widget_t*)w, touch, DRAW_SPRITE)) {
    widget_state(STATE_FILE_MANAGER);
  }
}

static void onConfigurationCaptureMode(const void *w, Touch_t &touch) {
  POS_CHECK((const Widget_t*)w);
  DBG_EXEC(printf("onConfigurationCaptureMode\n"));

  if (!DrawSetup((const Widget_t*)w, touch, DRAW_SPRITE)) {
    widget_state(STATE_CAPTURE_MODE);
  }
}

static void onConfigurationCalibration(const void *w, Touch_t &touch) {
  POS_CHECK((const Widget_t*)w);
  DBG_EXEC(printf("onConfigurationCalibration\n"));

  if (!DrawSetup((const Widget_t*)w, touch, DRAW_SPRITE)) {
    widget_state(STATE_CALIBRATION);
  }
}

static void onConfigurationInformation(const void *w, Touch_t &touch) {
  POS_CHECK((const Widget_t*)w);
  DBG_EXEC(printf("onConfigurationInformation\n"));

  if (!DrawSetup((const Widget_t*)w, touch, DRAW_SPRITE)) {
    widget_state(STATE_INFORMATION);
  }
}

static void onConfigurationReturn(const void *w, Touch_t &touch) {
  POS_CHECK((const Widget_t*)w);
  DBG_EXEC(printf("onConfigurationReturn\n"));

  if (!DrawSetup((const Widget_t*)w, touch, DRAW_SPRITE)) {
    widget_state(STATE_ON);
  }
}

/*--------------------------------------------------------------------------------
 * Widget callback functions for Resolution
 *--------------------------------------------------------------------------------*/
static void onResolutionScreen(const void *w, Touch_t &touch) {
  POS_CHECK((const Widget_t*)w);
  DBG_EXEC(printf("onResolutionScreen\n"));

  DrawSetup((const Widget_t*)w, touch, DRAW_WIDGET);
}

static void onResolutionSlider1(const void *w, Touch_t &touch) {
  POS_CHECK((const Widget_t*)w);
  DBG_EXEC(printf("onResolutionSlider1\n"));

  DrawSetup((const Widget_t*)w, touch, DRAW_SLIDER);
}

static void onResolutionSlider2(const void *w, Touch_t &touch) {
  POS_CHECK((const Widget_t*)w);
  DBG_EXEC(printf("onResolutionSlider2\n"));

  DrawSetup((const Widget_t*)w, touch, DRAW_SLIDER);
}

static void onResolutionClose(const void *w, Touch_t &touch) {
  POS_CHECK((const Widget_t*)w);
  DBG_EXEC(printf("onResolutionClose\n"));

  widget_state(STATE_CONFIGURATION);
}

static void onResolutionApply(const void *w, Touch_t &touch) {
  POS_CHECK((const Widget_t*)w);
  DBG_EXEC(printf("onResolutionApply\n"));

  DrawPressed((const Widget_t*)w, touch);
}

/*--------------------------------------------------------------------------------
 * Widget callback functions for Thermograph
 *--------------------------------------------------------------------------------*/
static void onThermographScreen(const void *w, Touch_t &touch) {
  POS_CHECK((const Widget_t*)w);
  DBG_EXEC(printf("onResolutionScreen\n"));

  DrawSetup((const Widget_t*)w, touch, DRAW_WIDGET);
}

static void onThermographRadio1(const void *w, Touch_t &touch) {
  POS_CHECK((const Widget_t*)w);
  DBG_EXEC(printf("onThermographRadio1\n"));

  DrawSetup((const Widget_t*)w, touch, DRAW_SPRITE, 1);
}

static void onThermographRadio2(const void *w, Touch_t &touch) {
  POS_CHECK((const Widget_t*)w);
  DBG_EXEC(printf("onThermographRadio2\n"));

  DrawSetup((const Widget_t*)w, touch, DRAW_SPRITE, 0);
}

static void onThermographToggle1(const void *w, Touch_t &touch) {
  POS_CHECK((const Widget_t*)w);
  DBG_EXEC(printf("onThermographToggle1\n"));

  DrawSetup((const Widget_t*)w, touch, DRAW_SPRITE, 1);
}

static void onThermographToggle2(const void *w, Touch_t &touch) {
  POS_CHECK((const Widget_t*)w);
  DBG_EXEC(printf("onThermographToggle2\n"));

  DrawSetup((const Widget_t*)w, touch, DRAW_SPRITE, 0);
}

static void onThermographSlider1(const void *w, Touch_t &touch) {
  POS_CHECK((const Widget_t*)w);
  DBG_EXEC(printf("onThermographSlider1\n"));

  DrawSetup((const Widget_t*)w, touch, DRAW_SLIDER);
}

static void onThermographSlider2(const void *w, Touch_t &touch) {
  POS_CHECK((const Widget_t*)w);
  DBG_EXEC(printf("onThermographSlider2\n"));

  DrawSetup((const Widget_t*)w, touch, DRAW_SLIDER);
}

static void onThermographClose(const void *w, Touch_t &touch) {
  POS_CHECK((const Widget_t*)w);
  DBG_EXEC(printf("onThermographClose\n"));

  widget_state(STATE_CONFIGURATION);
}

static void onThermographApply(const void *w, Touch_t &touch) {
  POS_CHECK((const Widget_t*)w);
  DBG_EXEC(printf("onThermographApply\n"));

  DrawPressed((const Widget_t*)w, touch);
}

/*--------------------------------------------------------------------------------
 * Widget callback functions for File Manager
 *--------------------------------------------------------------------------------*/
static void onFileManagerScreen(const void *w, Touch_t &touch) {
  POS_CHECK((const Widget_t*)w);
  DBG_EXEC(printf("onFileManagerScreen\n"));

  DrawSetup((const Widget_t*)w, touch, DRAW_WIDGET);
}
static void onFileManagerCheckAll(const void *w, Touch_t &touch) {
  POS_CHECK((const Widget_t*)w);
  DBG_EXEC(printf("onFileManagerCheckAll\n"));

  DrawSetup((const Widget_t*)w, touch, DRAW_SPRITE);
}

static void onFileManagerList1(const void *w, Touch_t &touch) {
  POS_CHECK((const Widget_t*)w);
  DBG_EXEC(printf("onFileManagerList1\n"));
}

static void onFileManagerList2(const void *w, Touch_t &touch) {
  POS_CHECK((const Widget_t*)w);
  DBG_EXEC(printf("onFileManagerList2\n"));
}

static void onFileManagerPrev(const void *w, Touch_t &touch) {
  POS_CHECK((const Widget_t*)w);
  DBG_EXEC(printf("onFileManagerPrev\n"));

  if (!DrawSetup((const Widget_t*)w, touch, DRAW_SPRITE)) {
    DrawPressed((const Widget_t*)w, touch);
  }
}

static void onFileManagerNext(const void *w, Touch_t &touch) {
  POS_CHECK((const Widget_t*)w);
  DBG_EXEC(printf("onFileManagerNext\n"));

  if (!DrawSetup((const Widget_t*)w, touch, DRAW_SPRITE)) {
    DrawPressed((const Widget_t*)w, touch);
  }
}

static void onFileManagerClose(const void *w, Touch_t &touch) {
  POS_CHECK((const Widget_t*)w);
  DBG_EXEC(printf("onFileManagerClose\n"));

  widget_state(STATE_CONFIGURATION);
}

static void onFileManagerApply(const void *w, Touch_t &touch) {
  POS_CHECK((const Widget_t*)w);
  DBG_EXEC(printf("onFileManagerApply\n"));

  DrawPressed((const Widget_t*)w, touch);
}

/*--------------------------------------------------------------------------------
 * Widget callback functions for Capture Mode
 *--------------------------------------------------------------------------------*/
static void onCaptureModeScreen(const void *w, Touch_t &touch) {
  POS_CHECK((const Widget_t*)w);
  DBG_EXEC(printf("onCaptureModeScreen\n"));

  DrawSetup((const Widget_t*)w, touch, DRAW_WIDGET);
}

static void onCaptureModeCamera(const void *w, Touch_t &touch) {
  POS_CHECK((const Widget_t*)w);
  DBG_EXEC(printf("onCaptureModeCamera\n"));

  if (!DrawSetup((const Widget_t*)w, touch, DRAW_SPRITE)) {
  }
}

static void onCaptureModeVideo(const void *w, Touch_t &touch) {
  POS_CHECK((const Widget_t*)w);
  DBG_EXEC(printf("onCaptureModeVideo\n"));

  if (!DrawSetup((const Widget_t*)w, touch, DRAW_SPRITE)) {
  }
}

static void onCaptureModeApply(const void *w, Touch_t &touch) {
  POS_CHECK((const Widget_t*)w);
  DBG_EXEC(printf("onCaptureModeApply\n"));

  widget_state(STATE_CONFIGURATION);
}

/*--------------------------------------------------------------------------------
 * Widget callback functions for Calibration
 *--------------------------------------------------------------------------------*/
static void onCalibrationScreen (const void *w, Touch_t &touch) {
  POS_CHECK((const Widget_t*)w);
  DBG_EXEC(printf("onCalibrationScreen\n"));

  DrawSetup((const Widget_t*)w, touch, DRAW_WIDGET);
}

static void onCalibrationExec   (const void *w, Touch_t &touch) {
  POS_CHECK((const Widget_t*)w);
  DBG_EXEC(printf("onCalibrationExec\n"));
}

static void onCalibrationSave   (const void *w, Touch_t &touch) {
  POS_CHECK((const Widget_t*)w);
  DBG_EXEC(printf("onCalibrationSave\n"));

  DrawSetup((const Widget_t*)w, touch, DRAW_SPRITE);
}

static void onCalibrationClose  (const void *w, Touch_t &touch) {
  POS_CHECK((const Widget_t*)w);
  DBG_EXEC(printf("onCalibrationClose\n"));

  widget_state(STATE_CONFIGURATION);
}

static void onCalibrationApply  (const void *w, Touch_t &touch) {
  POS_CHECK((const Widget_t*)w);
  DBG_EXEC(printf("onCalibrationApply\n"));

  DrawPressed((const Widget_t*)w, touch);
}

/*--------------------------------------------------------------------------------
 * Widget callback functions for Information
 *--------------------------------------------------------------------------------*/
static void onInformationScreen (const void *w, Touch_t &touch) {
  POS_CHECK((const Widget_t*)w);
  DBG_EXEC(printf("onInformationScreen\n"));

  DrawSetup((const Widget_t*)w, touch, DRAW_WIDGET);
}

static void onInformationClose  (const void *w, Touch_t &touch) {
  POS_CHECK((const Widget_t*)w);
  DBG_EXEC(printf("onInformationClose\n"));

  widget_state(STATE_CONFIGURATION);
}