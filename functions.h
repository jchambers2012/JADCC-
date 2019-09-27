#ifndef BLOWERCONTROLLER_FUNCTIONS_H
#define BLOWERCONTROLLER_FUNCTIONS_H
#define BLOWERCONTROLLER_DEBUG
void do_sensor_control();
void check_MCP();
void run_blower_control();
void run_lcd_control();
void run_lcd_draw();
void read_button_green();
void read_button_black();
void read_button_blue();
void read_button_yellow();
void run_debug();

//WEB SERVER FUNCTIONS
String formatBytes(size_t bytes);
String getContentType(String filename);
bool handleFileRead(String path);
void handleFileUpload();
void handleFileDelete();
void handleFileCreate();
void handleFileList();
#ifdef BLOWER_CONTROL_WIFI
void web_setup();
void wificonfigModeCallback(WiFiManager *myWiFiManager);
#endif
#endif /* functions.h */
