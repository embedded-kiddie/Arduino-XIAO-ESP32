/*=============================================================
 * Wedget manager
 *=============================================================*/
#include <pgmspace.h>

#define ICON_WIDTH  50
#define ICON_HEIGHT 50

/*-------------------------------------------------------------
 * Widget
 *-------------------------------------------------------------*/
typedef struct {
  uint16_t      x, y;   // The top left coordinate of the widget
  uint16_t      w, h;   // Widget width and height
  const uint8_t *img;   // PNG image data
  size_t        size;   // Size of PNG image data
  EventType_t   event;  // The touch event to detect
  void          (*callback)(EventPoint_t &ep);  // Callback event handler
} Widget_t;

/*-------------------------------------------------------------
 * icons
 *-------------------------------------------------------------*/
// icon-capture.png
// https://lang-ship.com/tools/image2data/
// RAW File Dump
const unsigned char icon_camera[686] PROGMEM = {
0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d, 0x49, 0x48, 0x44, 0x52, 
0x00, 0x00, 0x00, 0x32, 0x00, 0x00, 0x00, 0x32, 0x08, 0x04, 0x00, 0x00, 0x00, 0xb4, 0x36, 0x40, 
0x3a, 0x00, 0x00, 0x02, 0x75, 0x49, 0x44, 0x41, 0x54, 0x78, 0xda, 0xed, 0xd7, 0xb1, 0x4b, 0x94, 
0x71, 0x1c, 0xc7, 0xf1, 0x47, 0xcf, 0xa0, 0xd6, 0x5c, 0x6c, 0x48, 0x22, 0x87, 0x9c, 0x0e, 0x5a, 
0xaf, 0xf0, 0xa0, 0x08, 0x82, 0x86, 0xa0, 0x03, 0xff, 0x80, 0x82, 0x6e, 0x29, 0x6a, 0x2a, 0x8c, 
0xda, 0xa2, 0x1a, 0x0c, 0x1c, 0x5a, 0x74, 0x29, 0x9a, 0x5a, 0x8a, 0xb6, 0xe8, 0x24, 0xe3, 0xaa, 
0xa5, 0xd4, 0x45, 0xd0, 0x59, 0x51, 0x5c, 0x1c, 0x1c, 0x14, 0x45, 0xd1, 0xdf, 0xab, 0x67, 0xb8, 
0xe5, 0x8e, 0x7b, 0x7c, 0xd0, 0xfb, 0x51, 0x8b, 0x9f, 0xf7, 0xf6, 0x5d, 0xde, 0xf0, 0x7b, 0x7e, 
0xcf, 0x87, 0xef, 0x2f, 0x39, 0xce, 0x71, 0xfe, 0x6f, 0x14, 0x9c, 0x77, 0xd9, 0xd5, 0x16, 0x2e, 
0x3b, 0xaf, 0x10, 0x4b, 0x71, 0x56, 0xd5, 0x7b, 0x93, 0xe1, 0x47, 0x33, 0x26, 0xbd, 0x57, 0x75, 
0x36, 0x86, 0xa2, 0xdf, 0x1b, 0x2b, 0xb2, 0xb2, 0xe2, 0x8d, 0xfe, 0x4e, 0x15, 0x27, 0xbd, 0xb2, 
0x8d, 0xec, 0x6c, 0x7b, 0xe5, 0x64, 0x67, 0x92, 0x9b, 0x61, 0x0d, 0xf6, 0xcd, 0xf9, 0xd6, 0xc2, 
0x9c, 0x7d, 0x10, 0xd6, 0xdc, 0xec, 0x44, 0x31, 0xe0, 0x27, 0x50, 0x57, 0x31, 0xd4, 0x42, 0x45, 
0x1d, 0xe0, 0xa7, 0x81, 0xe4, 0x30, 0xd1, 0xed, 0x9c, 0x92, 0x2b, 0x29, 0x37, 0x8c, 0xdb, 0x83, 
0x55, 0xd7, 0x25, 0x6d, 0xb8, 0x6e, 0x15, 0xd8, 0x33, 0xee, 0x86, 0x2b, 0x29, 0x25, 0xe7, 0x74, 
0xe7, 0x29, 0xce, 0xb8, 0xed, 0xad, 0xaf, 0xea, 0xea, 0x61, 0xda, 0x16, 0xec, 0x79, 0xee, 0x44, 
0x5b, 0xc9, 0x09, 0xcf, 0xed, 0x01, 0x5b, 0x61, 0x5a, 0x3d, 0xe5, 0xab, 0xb7, 0x6e, 0x3b, 0x73, 
0xb0, 0x62, 0xd4, 0xa2, 0x96, 0x7c, 0xd1, 0x2f, 0xc9, 0xa0, 0xdf, 0x17, 0x5a, 0xb3, 0x68, 0x34, 
0x53, 0xa3, 0xc7, 0xd3, 0xb0, 0xa9, 0x29, 0xeb, 0x3e, 0x2b, 0x49, 0x64, 0x53, 0xf2, 0xd9, 0xba, 
0xe6, 0x84, 0x4d, 0x4f, 0xf5, 0xb4, 0x97, 0x0c, 0x86, 0x65, 0x08, 0x16, 0x7c, 0x37, 0x65, 0xd2, 
0x07, 0x23, 0x2e, 0x4a, 0x72, 0xb8, 0x68, 0xc4, 0x07, 0x93, 0xa6, 0x7c, 0xb7, 0x20, 0x80, 0xb0, 
0x6c, 0xb0, 0xbd, 0xa4, 0x0a, 0xcc, 0x1a, 0x56, 0x4e, 0xb9, 0x64, 0xd0, 0x29, 0x89, 0x7c, 0x4e, 
0x19, 0x74, 0x49, 0x39, 0x65, 0xd8, 0x2c, 0x40, 0xb5, 0xbd, 0x64, 0x0c, 0x78, 0x2c, 0xc9, 0xa0, 
0x57, 0xd1, 0x50, 0x4a, 0x51, 0xaf, 0x24, 0x83, 0xc7, 0x00, 0x63, 0xed, 0x25, 0x13, 0x40, 0x25, 
0x43, 0x70, 0xcb, 0xa8, 0x4f, 0xa6, 0x52, 0x3e, 0x19, 0x75, 0x2b, 0x43, 0x54, 0x01, 0x98, 0x38, 
0xb4, 0x64, 0xc0, 0x4b, 0x73, 0x76, 0x00, 0xec, 0x98, 0xf3, 0xd2, 0x40, 0x4c, 0x49, 0x9f, 0x71, 
0x1b, 0x5a, 0xb3, 0x61, 0x5c, 0x5f, 0x2c, 0x49, 0xc1, 0x88, 0x6d, 0xc0, 0xb2, 0x5f, 0x29, 0xcb, 
0x80, 0x6d, 0x23, 0x0a, 0x71, 0x24, 0x17, 0x2c, 0x81, 0x7d, 0x35, 0x55, 0xd7, 0x52, 0xaa, 0x6a, 
0xf6, 0xc1, 0x92, 0x0b, 0x71, 0x24, 0x0f, 0x01, 0xb3, 0x4a, 0xba, 0x25, 0x12, 0xdd, 0x4a, 0x66, 
0x01, 0x0f, 0xe3, 0x48, 0x3e, 0x82, 0xe0, 0x7e, 0xd3, 0xf4, 0xbe, 0x00, 0x3e, 0xc6, 0x90, 0x74, 
0xf9, 0x0d, 0x76, 0x14, 0x9b, 0xe6, 0x45, 0x3b, 0xe0, 0xb7, 0xae, 0x18, 0x92, 0x3f, 0x60, 0x57, 
0xab, 0x64, 0x17, 0xfc, 0xd1, 0x15, 0xf3, 0xb8, 0xee, 0x35, 0x4d, 0xef, 0xc5, 0x3c, 0xae, 0xc4, 
0x03, 0xc0, 0x4c, 0xd3, 0x87, 0x9f, 0x01, 0x3c, 0x88, 0x7b, 0x85, 0x83, 0x9a, 0xbb, 0xae, 0xa5, 
0xdc, 0x55, 0x13, 0xe2, 0x5e, 0xe1, 0x82, 0x27, 0xb6, 0xda, 0xfe, 0x8c, 0x5b, 0x9e, 0x28, 0xc4, 
0xab, 0x95, 0x89, 0xb6, 0xb5, 0x32, 0xa1, 0x2f, 0x6e, 0x41, 0xbe, 0xd0, 0x5a, 0x90, 0x2f, 0x22, 
0x16, 0x64, 0x83, 0x5e, 0x95, 0xa6, 0xaa, 0xaf, 0x44, 0xaf, 0xfa, 0x06, 0xa7, 0x15, 0x95, 0x53, 
0x8a, 0x4e, 0x6b, 0xcc, 0xe2, 0x4b, 0xf2, 0xc9, 0x97, 0x8c, 0x01, 0x8f, 0x3a, 0x92, 0x3c, 0x02, 
0x18, 0x3b, 0x70, 0x91, 0x98, 0x31, 0xac, 0x6c, 0xe8, 0x08, 0x94, 0x0d, 0x9b, 0x01, 0xa8, 0xe6, 
0xac, 0x44, 0xf3, 0xa6, 0x8e, 0xc8, 0x7c, 0xee, 0x4a, 0xd4, 0xe3, 0x59, 0xd8, 0x14, 0x21, 0x61, 
0xd3, 0x33, 0x3d, 0xd9, 0x6b, 0xea, 0x6b, 0x8b, 0x3a, 0xcd, 0xa2, 0xd7, 0x79, 0xdb, 0xf0, 0x1d, 
0xef, 0xd4, 0xc2, 0x8f, 0xa3, 0xa1, 0xe6, 0x9d, 0x3b, 0x0d, 0x45, 0xfe, 0xd3, 0xe1, 0x48, 0x34, 
0x9e, 0x0e, 0xc7, 0x4f, 0xf9, 0xe3, 0xfc, 0xbb, 0xfc, 0x05, 0x60, 0xf3, 0xc2, 0xea, 0x2c, 0x22, 
0x32, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82, };

// icon-config.png
// https://lang-ship.com/tools/image2data/
// RAW File Dump
const unsigned char icon_config[671] PROGMEM = {
0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d, 0x49, 0x48, 0x44, 0x52, 
0x00, 0x00, 0x00, 0x32, 0x00, 0x00, 0x00, 0x32, 0x08, 0x00, 0x00, 0x00, 0x00, 0x3b, 0x54, 0xd7, 
0x6d, 0x00, 0x00, 0x02, 0x66, 0x49, 0x44, 0x41, 0x54, 0x78, 0x01, 0xed, 0x95, 0x03, 0x8c, 0x1d, 
0x5d, 0x1c, 0x47, 0xe7, 0xfb, 0x6a, 0xdb, 0xb6, 0x1d, 0xac, 0x6d, 0xd5, 0x46, 0xcc, 0x32, 0xce, 
0x36, 0xac, 0x1b, 0x15, 0x71, 0x6d, 0x7b, 0x8d, 0xb8, 0xd6, 0xc3, 0x22, 0x5c, 0xdb, 0x36, 0xe6, 
0xf4, 0xcd, 0x4e, 0x26, 0xfb, 0x30, 0x28, 0xe2, 0xec, 0x2f, 0x3a, 0x17, 0x67, 0x74, 0xef, 0xfd, 
0x8f, 0xf0, 0xaf, 0x19, 0xc8, 0xa8, 0x03, 0xb7, 0xdf, 0x5c, 0x0d, 0x94, 0xd9, 0xff, 0xf2, 0xcb, 
0x1b, 0x7b, 0x46, 0x18, 0x19, 0xdb, 0x2d, 0x00, 0xbc, 0x59, 0x27, 0x08, 0x6b, 0x5f, 0x8b, 0x00, 
0x3f, 0xb7, 0xe8, 0x0a, 0xeb, 0xe3, 0x40, 0x4e, 0xd3, 0xa3, 0x47, 0x8d, 0x0a, 0xbf, 0x5d, 0xab, 
0x6d, 0x1c, 0x6a, 0x04, 0x68, 0xcb, 0x05, 0x25, 0xf9, 0xed, 0x00, 0x0d, 0x07, 0xb5, 0x0c, 0x8f, 
0x0e, 0x40, 0x7c, 0xb6, 0x6e, 0x54, 0x8c, 0x09, 0x00, 0x4c, 0x9b, 0x47, 0x6f, 0x7a, 0x09, 0xd0, 
0xee, 0xa1, 0x6e, 0xfc, 0x17, 0x07, 0xfc, 0x88, 0x90, 0x70, 0xf4, 0xf1, 0xcf, 0xe5, 0xe5, 0x9f, 
0x4f, 0x8c, 0x96, 0x38, 0xca, 0x04, 0xc4, 0xfd, 0xa7, 0xaa, 0x4c, 0xaa, 0x84, 0x8f, 0x63, 0x15, 
0x7f, 0xe2, 0x44, 0x65, 0xd6, 0xd8, 0x8f, 0x50, 0x39, 0x49, 0x55, 0x59, 0xd0, 0x02, 0xa7, 0xd5, 
0x06, 0x4e, 0x43, 0xcb, 0x02, 0x55, 0x65, 0x7e, 0x33, 0x9c, 0x51, 0x1b, 0x38, 0x03, 0xcd, 0xf3, 
0x55, 0x95, 0x59, 0x4d, 0x70, 0x51, 0x69, 0x0c, 0x1a, 0x3b, 0x76, 0x90, 0xc2, 0x17, 0xa1, 0x69, 
0x96, 0xaa, 0x12, 0xda, 0x03, 0x27, 0xe5, 0x17, 0x09, 0xb9, 0x6e, 0x2a, 0x2a, 0x32, 0x5d, 0x0f, 
0x91, 0x5f, 0x27, 0x16, 0x7a, 0x42, 0xd5, 0x8c, 0xcd, 0xb9, 0x20, 0xfa, 0x48, 0xb4, 0x2a, 0x19, 
0xe4, 0x24, 0xaf, 0x92, 0xda, 0xde, 0x22, 0xe4, 0x6e, 0x76, 0x11, 0xd6, 0xc6, 0x03, 0xa4, 0x0f, 
0xb6, 0xa1, 0x7f, 0x05, 0x28, 0xa9, 0xf0, 0xb7, 0x75, 0x0c, 0x4e, 0x03, 0x88, 0x5b, 0xe3, 0x68, 
0x04, 0xd7, 0x00, 0xe4, 0x2f, 0xb3, 0xe1, 0x8a, 0x4a, 0x00, 0xd3, 0xc3, 0x87, 0x26, 0x80, 0xca, 
0x15, 0xb6, 0xae, 0x65, 0xf9, 0x00, 0x35, 0x81, 0xf6, 0xc6, 0xb4, 0x22, 0x80, 0xd7, 0x8b, 0x95, 
0x4b, 0x66, 0x86, 0x0e, 0xb6, 0x51, 0x68, 0x26, 0x90, 0x26, 0xdd, 0x78, 0xf1, 0x6b, 0x80, 0xc2, 
0xa9, 0x76, 0xca, 0x71, 0xc0, 0x12, 0xd1, 0x87, 0x41, 0x40, 0xd6, 0x2c, 0xb9, 0x7b, 0x56, 0x16, 
0x10, 0xdc, 0x87, 0x91, 0x56, 0xe0, 0xa8, 0x9d, 0xf2, 0x04, 0x6a, 0xe7, 0xcb, 0x78, 0x03, 0xba, 
0x83, 0x94, 0xfe, 0xa0, 0x6e, 0xb8, 0x21, 0xe3, 0x82, 0x5a, 0x78, 0x6c, 0xa7, 0x24, 0x41, 0xe6, 
0x20, 0x79, 0x3d, 0xcc, 0x60, 0x92, 0x51, 0x6a, 0x99, 0xc0, 0x3c, 0x58, 0xc6, 0x6c, 0x48, 0x70, 
0x52, 0xe4, 0x91, 0xb1, 0xc5, 0xf0, 0xa0, 0x7f, 0xe0, 0x01, 0x14, 0x8f, 0xeb, 0xa3, 0x21, 0x99, 
0x90, 0xe4, 0xa4, 0x0c, 0xd1, 0x57, 0x06, 0x3b, 0x29, 0x09, 0x90, 0x6d, 0xf4, 0x60, 0x4e, 0xca, 
0x23, 0xa8, 0x5b, 0xa8, 0xff, 0xfa, 0xf3, 0x6b, 0xe0, 0xa9, 0x9d, 0x72, 0x04, 0xb0, 0x46, 0xea, 
0x7d, 0xe4, 0x08, 0x0b, 0x70, 0xdc, 0x4e, 0x99, 0x5a, 0xf0, 0x3b, 0x4b, 0x59, 0x34, 0x4d, 0xb0, 
0x4b, 0xa0, 0xf1, 0x86, 0xa1, 0x56, 0xba, 0x9f, 0x5d, 0xd6, 0xc4, 0x01, 0xa4, 0xe9, 0x6c, 0xcb, 
0x78, 0xd7, 0xd2, 0x14, 0x93, 0x07, 0xa2, 0xb7, 0xda, 0xe6, 0xf7, 0x11, 0x21, 0x6f, 0xb3, 0xd6, 
0x11, 0x8b, 0xd5, 0x3d, 0x62, 0xff, 0x7e, 0x90, 0x8d, 0xcb, 0xc5, 0xbf, 0x17, 0x25, 0xa5, 0xf4, 
0x7d, 0x52, 0x29, 0x7d, 0x63, 0x3e, 0x68, 0x96, 0xbe, 0xff, 0xde, 0x01, 0xa6, 0x68, 0x09, 0x47, 
0x9f, 0xe8, 0x2f, 0xb0, 0x11, 0x3f, 0x80, 0x78, 0xc9, 0x57, 0x89, 0x7b, 0x3b, 0xc0, 0xab, 0x4d, 
0xa3, 0x37, 0x2b, 0x65, 0x3c, 0x66, 0xd4, 0xba, 0x67, 0x22, 0xd0, 0xe1, 0x29, 0x68, 0xe4, 0x60, 
0x03, 0x40, 0x7b, 0x3e, 0x4a, 0xc8, 0x6d, 0x03, 0x68, 0x3c, 0x24, 0x68, 0x66, 0xed, 0x5b, 0xbb, 
0x5f, 0x52, 0x93, 0xc2, 0x71, 0xeb, 0x05, 0xbd, 0x6c, 0xf9, 0x09, 0x20, 0xbe, 0x5e, 0x6b, 0xf3, 
0xdf, 0x00, 0x60, 0xd9, 0x21, 0x18, 0x64, 0xc4, 0x9e, 0x1b, 0x2f, 0x2f, 0xfb, 0xcb, 0x1c, 0x70, 
0xf5, 0xcd, 0xed, 0x03, 0xa3, 0x84, 0x7f, 0xcc, 0x40, 0x7e, 0x01, 0x70, 0xcb, 0x43, 0xf0, 0xf5, 
0x93, 0x2c, 0x3c, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82, };

/*-------------------------------------------------------------
 * Widgets definition
 *-------------------------------------------------------------*/
typedef enum {
  INSIDE = 0,
  INFO,
  COLORS,
  RANGE,
  CAMERA,
  CONFIG,
} WidgetID_t;

void onInside (EventPoint_t &ep);
void onInfo   (EventPoint_t &ep);
void onColors (EventPoint_t &ep);
void onRange  (EventPoint_t &ep);
void onCamera (EventPoint_t &ep);
void onConfig (EventPoint_t &ep);

static const Widget_t widgets[] {
  {   0,   0, 256, 192, NULL, 0, EVENT_ALL, onInside },
  { 256,   0,  64, 120, NULL, 0, EVENT_ALL, onInfo   },
  {   0, 195, 256,  15, NULL, 0, EVENT_ALL, onColors },
  {   0, 210, 255,  30, NULL, 0, EVENT_ALL, onRange  },
  { 265, 120, ICON_WIDTH, ICON_HEIGHT, icon_camera, sizeof(icon_camera), EVENT_CLICK, onCamera },
  { 265, 170, ICON_WIDTH, ICON_HEIGHT, icon_config, sizeof(icon_config), EVENT_CLICK, onConfig },
};

#define N_WIDGETS  (sizeof(widgets) / sizeof(widgets[0]))

/*-------------------------------------------------------------
 * Event callback functions
 *-------------------------------------------------------------*/
void onInside(EventPoint_t &ep) {
  DBG_EXEC(printf("onInside\n"));
}

void onInfo(EventPoint_t &ep) {
  DBG_EXEC(printf("onInfo\n"));
}

void onColors(EventPoint_t &ep) {
  DBG_EXEC(printf("onColors\n"));
}

void onRange(EventPoint_t &ep) {
  DBG_EXEC(printf("onRange\n"));
}

void onCamera(EventPoint_t &ep) {
  DBG_EXEC(printf("onCamera\n"));
  sdcard_save();
}

void onConfig(EventPoint_t &ep) {
  DBG_EXEC(printf("onConfig\n"));
}

#ifdef _TFT_eSPIH_
/*-------------------------------------------------------------
 * Draw PNG
 *-------------------------------------------------------------*/
// https://github.com/Bodmer/TFT_eSPI/tree/master/examples/PNG%20Images
// Include the PNG decoder library, available via the IDE library manager
#include <PNGdec.h>

PNG png; // PNG decoder instance

//=========================================v==========================================
//  pngDraw: Callback function to draw pixels to the display
//====================================================================================
// Position variables must be global (PNGdec does not handle position coordinates)
static uint16_t xpos = 0;
static uint16_t ypos = 0;

// This function will be called during decoding of the png file to render each image
// line to the TFT. PNGdec generates the image line and a 1bpp mask.
static void pngDraw(PNGDRAW *pDraw) {
  uint16_t lineBuffer[TFT_HEIGHT];          // Line buffer for rendering
  uint8_t  maskBuffer[TFT_HEIGHT / 8 + 1];  // Mask buffer

  png.getLineAsRGB565(pDraw, lineBuffer, PNG_RGB565_BIG_ENDIAN, 0xffffffff);

  if (png.getAlphaMask(pDraw, maskBuffer, 255)) {
    // Note: pushMaskedImage is for pushing to the TFT and will not work pushing into a sprite
    tft.pushMaskedImage(xpos, ypos + pDraw->y, pDraw->iWidth, 1, lineBuffer, maskBuffer);
  }
}

static void DrawPNG(uint8_t *img, size_t size, uint16_t x, uint16_t y) {
  xpos = x;
  ypos = y;

  if (png.openFLASH(img, size, pngDraw) == PNG_SUCCESS) {
    png.decode(NULL, 0);
    // png.close(); // Required for files, not needed for FLASH arrays
  }
}
#endif // _TFT_eSPIH_

void widget_setup(void) {
  GFX_EXEC(startWrite());

  for (int i = 0; i < N_WIDGETS; i++) {
    if (widgets[i].img) {
#if defined(_TFT_eSPIH_)
      DrawPNG((uint8_t*)widgets[i].img, widgets[i].size, widgets[i].x, widgets[i].y);
#elif defined(LOVYANGFX_HPP_)
      GFX_EXEC(drawPng((uint8_t*)widgets[i].img, widgets[i].size, widgets[i].x, widgets[i].y));
#endif
    }
  }

  GFX_EXEC(endWrite());
}

void widget_event(EventPoint_t &ep) {
  for (int i = 0; i < N_WIDGETS; i++) {

    // Find the widget where the event fired
    if (widgets[i].x <= ep.x && ep.x <= widgets[i].x + widgets[i].w &&
        widgets[i].y <= ep.y && ep.y <= widgets[i].y + widgets[i].h) {

      // In case the touch event to be detected
      if (widgets[i].event & ep.event) {
        DBG_EXEC(printf("event = %d, x = %d, y = %d\n", ep.event, ep.x, ep.y));
        widgets[i].callback(ep);
        delay(PERIOD_EVENT);
      }
    }
  }
}