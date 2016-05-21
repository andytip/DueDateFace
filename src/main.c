#include <pebble.h>

#define KEY_TEMPERATURE 0
#define KEY_CONDITIONS 1
#define KEY_NAME 2
#define KEY_DUEDATE 3
//#define KEY_SHOWLOCATION 4

//static pointer to a Window variable
//prefixed with s_ to denote its static nature (static here means it is accessible only within this file
static Window *s_main_window;
#define TAP_NOT_DATA false
//Static pointer to watch text
TextLayer *s_time_layer;
TextLayer *s_countdown_layer;
TextLayer *s_countup_layer;
TextLayer *s_date_layer;
TextLayer *s_day_layer;
static TextLayer *s_weather_layer;
static TextLayer *s_weather_condition_layer;
static TextLayer *s_weather_name_layer;
static Layer *s_battery_layer;
static BitmapLayer *s_bt_icon_layer;
static GBitmap *s_bt_icon_bitmap;
//Custome Font
static GFont s_time_font;
//Funciton Declartion
static char char_calculate_countdown_time[6];
static int difference = 0;
static int display_time = 0;
static bool display_on = false;
static bool display_reset = true;
static int s_battery_level;
//static bool showlocations = 0;
static int s_duedate = 1478131200;
typedef struct {
  char size[45];  // Size of Baby
  int mins;       // Size to week
} BabySize;
BabySize baby_array[] = {
  {"So Small", 1},
  {"So Small", 2},
  {"So Small", 3},
  {"So Small", 4},
  {"2mm - Poppy Seed", 5},
  {"3mm - Sesame Seed", 6},
  {"5mm - Lentil", 7},
  {"1.2cm - Blueberry", 8},
  {"1.6cm - Kidney Bean", 9},
  {"2.3cm - Grape", 10},
  {"3.1cm - Green Olive", 11},
  {"4.1cm - Fig", 12},
  {"5.4cm - Lime", 13},
  {"7.4cm - Pea Pod", 14},
  {"8.7cm - Lemon", 15},
  {"10.1cm - Apple", 16},
  {"11.6cm - Avocado", 17},
  {"13cm - Turnip", 18},
  {"14.2cm - Bell Pepper", 19},
  {"15.3cm - Heirloom Tomato", 20},
  {"25.6cm - Small Banana", 21},
  {"26.7cm - Carrot", 22},
  {"27.8cm - Spaghetti Squash", 23},
  {"28.9cm - Large Mango", 24},
  {"30cm - Ear of Corn", 25},
  {"34.6cm - Avarage Swede", 26},
  {"35.6cm - Red Cabbage", 27},
  {"36.6cm - Cauliflower", 28},
  {"37.6cm - Aubergine", 29},
  {"38.6cm - Butternut Squash", 30},
  {"39.9cm - Good-Sized Cabbage", 31},
  {"41.1cm - Coconut", 32},
  {"42.4cm - Long as a Kale", 33},
  {"43.7cm - Pineapple", 34},
  {"45cm - Cantaloupe Melon", 35},
  {"46.2cm - honeydew Melon", 36},
  {"47.4cm - Romaine Lettuce", 37},
  {"48.6cm - Swiss Chard", 38},
  {"49.8cm - Lond Leek", 39},
  {"50.7cm - Mini Watermelon", 40},
  {"51.2cm - Small Pumpkin", 41}
};
// Functions
static void update_countup() {
 
  time_t now = time(NULL);
  //time_t nownow; 
  //struct tm * timeinfo;
  
  //timeinfo = localtime(&now);
  // Conception Date
  //timeinfo->tm_year = 116;
  //timeinfo->tm_mon = 1;
  //timeinfo->tm_mday = 8;

  //nownow = mktime(timeinfo);
  
  int conseptiondate = s_duedate - (SECONDS_PER_DAY * 280);
  
  difference = (now - conseptiondate) / SECONDS_PER_DAY;
  
  int weeks = difference / 7;
  
  char* size = baby_array[weeks].size;
  
  //Write the current hours and minutes into a buffer
  static char s_buffer[40];
   
  snprintf(s_buffer,sizeof(s_buffer),"%d Weeks\n %s", weeks, size);
  text_layer_set_text(s_countup_layer, s_buffer);
  
}
static void update_countdown() {
  time_t now = time(NULL);
  //time_t nownow; 
  //struct tm * timeinfo;
  
  //timeinfo = localtime(&now);
  // Due Date
  //timeinfo->tm_year = 116;
  //timeinfo->tm_mon = 10;
  //timeinfo->tm_mday = 3;
  //timeinfo->tm_hour = 0;
  //timeinfo->tm_min = 0;
  //timeinfo->tm_sec = 0;

  //nownow = mktime(timeinfo); 
  
  difference = (s_duedate - now);
    
  int days = difference / (60 * 60 * 24);
  difference -= days * (60 * 60 * 24);
  int hours = difference / (60 * 60);
  difference -= hours * (60 * 60);
  int minutes = difference / 60;
  difference -= minutes * 60;
  
  //Write the current hours and minutes into a buffer
  static char s_buffer[25];
   
  //snprintf(s_buffer,sizeof(s_buffer),"%d days %d hours %d Mins %d\n seconds",days,hours,minutes,difference);
  snprintf(s_buffer,sizeof(s_buffer),"%d days %d:%d:%d",days, hours, minutes, difference);
  text_layer_set_text(s_countdown_layer, s_buffer);
}
//******************************** Config ***********8
static void reload_config(){
  //showlocations = persist_read_bool(KEY_SHOWLOCATION);
  //layer_set_hidden((Layer*)s_weather_name_layer, showlocations);
  update_countup();
  update_countdown();
}
//******************************** End Config Handler ****************************
//******************************** WEATHER SECTION *******************************
static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
// Store incoming information
  static char temperature_buffer[8];
  static char conditions_buffer[32];
  static char name_buffer[20];
  //static char weather_layer_buffer[32];

  // Read tuples for data
  Tuple *temp_tuple = dict_find(iterator, KEY_TEMPERATURE);
  Tuple *conditions_tuple = dict_find(iterator, KEY_CONDITIONS);
  Tuple *name_tuple = dict_find(iterator,KEY_NAME);
  
  // If all data is available, use it
  if(temp_tuple && conditions_tuple) {
    snprintf(temperature_buffer, sizeof(temperature_buffer), "%dC", (int)temp_tuple->value->int32);
    snprintf(conditions_buffer, sizeof(conditions_buffer), "%s", conditions_tuple->value->cstring);
    snprintf(name_buffer, sizeof(name_buffer), "%s", name_tuple->value->cstring);

    // Assemble full string and display
   // snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "%s, %s", temperature_buffer, conditions_buffer);
    text_layer_set_text(s_weather_layer, temperature_buffer);
    text_layer_set_text(s_weather_condition_layer, conditions_buffer);
    text_layer_set_text(s_weather_name_layer, name_buffer);
    
}
  // Config Data Recieved
  /*
  Tuple *showlocations_t = dict_find(iterator, KEY_SHOWLOCATION);
  if(showlocations_t) {
    showlocations = showlocations_t->value->int32 == 1;
    persist_write_bool(KEY_SHOWLOCATION, showlocations);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Locations Config Setting %d", showlocations);   
  }
  */
  Tuple *duedate_t = dict_find(iterator, KEY_DUEDATE); 
  if(duedate_t) {
     s_duedate = duedate_t->value->int32;
     persist_write_int(KEY_DUEDATE, s_duedate);
     APP_LOG(APP_LOG_LEVEL_DEBUG, "Due Date Config Setting %d", s_duedate);    
  }
  
 // App should now update to take the user's preferences into account
 reload_config();
}
static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}
static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}
static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}
//*************************************************************** END WEATHER
//******************BAttery***************************
static void battery_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  // Find the width of the bar
  int width = (int)(float)(((float)s_battery_level / 100.0F) * 140.0F);

  // Draw the background
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  // Draw the bar
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, GRect(0, 0, width, bounds.size.h), 0, GCornerNone);
}
static void battery_callback(BatteryChargeState state) {
  // Record the new battery level
  s_battery_level = state.charge_percent;
}
//******************End BAttery***********************
//******************Bluetooth*************************
static void bluetooth_callback(bool connected) {
  // Show icon if disconnected
  layer_set_hidden(bitmap_layer_get_layer(s_bt_icon_layer), connected);

  if(!connected) {
    // Issue a vibrating alert
    vibes_double_pulse();    
  }
  else
  {
    // pulse to show weather being got from bluetooth connect.
    vibes_long_pulse();
    // Begin dictionary
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);

    // Add a key-value pair
    dict_write_uint8(iter, 0, 0);

    // Send the message!
    app_message_outbox_send();
  }
}
//******************End Bluetooth********************
static void add_bluetooth_layer(Window *window){
  
  // Create the Bluetooth icon GBitmap
s_bt_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BT_ICON);

// Create the BitmapLayer to display the GBitmap
s_bt_icon_layer = bitmap_layer_create(GRect(65, 2, 10, 10));
bitmap_layer_set_bitmap(s_bt_icon_layer, s_bt_icon_bitmap);
layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_bt_icon_layer));
}
static void add_weather_layer(Window *window) {
 
   // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  // Create temperature Layer
  s_weather_condition_layer = text_layer_create(
      GRect(0, PBL_IF_ROUND_ELSE(125, 138), bounds.size.w, 100));
  
  // Style the text
  text_layer_set_background_color(s_weather_condition_layer, GColorClear);
  text_layer_set_text_color(s_weather_condition_layer, GColorWhite);
  text_layer_set_text_alignment(s_weather_condition_layer, GTextAlignmentCenter);
  text_layer_set_text(s_weather_condition_layer, "");
  // Create GFont
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_OSWALD_MEDIUM_14));

  // Apply to TextLayer
  text_layer_set_font(s_weather_condition_layer, s_time_font);
  
  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_weather_condition_layer));
  
  //s_weather_condition_layer
  s_weather_layer = text_layer_create(
      GRect(108, PBL_IF_ROUND_ELSE(125, 46), 40, 50));
  
  // Style the text
  text_layer_set_background_color(s_weather_layer, GColorClear);
  text_layer_set_text_color(s_weather_layer, GColorWhite);
  text_layer_set_text_alignment(s_weather_layer, GTextAlignmentCenter);
  text_layer_set_text(s_weather_layer, "");
  // Create GFont
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_OSWALD_MEDIUM_16));

  // Apply to TextLayer
  text_layer_set_font(s_weather_layer, s_time_font);
  
  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_weather_layer));
  
  
  // Create location Layer
  s_weather_name_layer = text_layer_create(
      GRect(0, PBL_IF_ROUND_ELSE(125, 151), bounds.size.w, 100));
  
  // Style the text
  text_layer_set_background_color(s_weather_name_layer, GColorClear);
  text_layer_set_text_color(s_weather_name_layer, GColorWhite);
  text_layer_set_text_alignment(s_weather_name_layer, GTextAlignmentCenter);
  text_layer_set_text(s_weather_name_layer, "Loading...");
  // Create GFont
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_OSWALD_MEDIUM_14));

  // Apply to TextLayer
  text_layer_set_font(s_weather_name_layer, s_time_font);
  
  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_weather_name_layer));
  
  
}
static void add_time_layer (Window *window){
  
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  //GRect bounds = layer_get_bounds(window_layer);

  // Create the TextLayer with specific bounds
  s_time_layer = text_layer_create(
      GRect(0, PBL_IF_ROUND_ELSE(58, 38), 110, 100));

  // Improve the layout to be more like a watchface
  text_layer_set_background_color(s_time_layer, GColorBlack);
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_text(s_time_layer, "00:00");
  //text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
  // Create GFont
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_OSWALD_STENCIL_44));

  // Apply to TextLayer
  text_layer_set_font(s_time_layer, s_time_font);
  
  
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentLeft);

  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  
}
static void add_day_layer (Window *window){
  
   // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // Create the TextLayer with specific bounds
  s_day_layer = text_layer_create(
      GRect(0, PBL_IF_ROUND_ELSE(58, 90), bounds.size.w, 100));

  // Improve the layout to be more like a watchface
  text_layer_set_background_color(s_day_layer, GColorBlack);
  text_layer_set_text_color(s_day_layer, GColorWhite);
  text_layer_set_text(s_day_layer, "Wednesday");
  //text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
  // Create GFont
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_OSWALD_MEDIUM_16));

  // Apply to TextLayer
  text_layer_set_font(s_day_layer, s_time_font);
  
  
  text_layer_set_text_alignment(s_day_layer, GTextAlignmentRight);

  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_day_layer));
  
}
static void add_date_layer (Window *window){
  
   // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // Create the TextLayer with specific bounds
  s_date_layer = text_layer_create(
      GRect(0, PBL_IF_ROUND_ELSE(58, 67), bounds.size.w, 100));

  // Improve the layout to be more like a watchface
  text_layer_set_background_color(s_date_layer, GColorBlack);
  text_layer_set_text_color(s_date_layer, GColorWhite);
  text_layer_set_text(s_date_layer, "23.03");
  //text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
  // Create GFont
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_OSWALD_MEDIUM_16));

  // Apply to TextLayer
  text_layer_set_font(s_date_layer, s_time_font);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentRight);
  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));
  
}
static void add_countdown_layer (Window *window){
  
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // Create the TextLayer with specific bounds
  s_countdown_layer = text_layer_create(
      GRect(0, PBL_IF_ROUND_ELSE(58, 15), bounds.size.w, 150));

  // Improve the layout to be more like a watchface
  text_layer_set_background_color(s_countdown_layer, GColorClear);
  text_layer_set_text_color(s_countdown_layer, GColorWhite);

  
  //calculate_countdown_time();
  
  text_layer_set_text(s_countdown_layer, char_calculate_countdown_time);
  //text_layer_set_font(s_countdown_layer, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));
   // Create GFont
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_OSWALD_MEDIUM_16));

  // Apply to TextLayer
  text_layer_set_font(s_countdown_layer, s_time_font);
  
  text_layer_set_text_alignment(s_countdown_layer, GTextAlignmentCenter);
    
  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_countdown_layer));
  
}
static void add_battery_layer (Window *window){
  // Create battery meter Layer 2, 89, 140, 2
s_battery_layer = layer_create(GRect(2, 87, 140, 2));
layer_set_update_proc(s_battery_layer, battery_update_proc);

// Add to Window
layer_add_child(window_get_root_layer(window), s_battery_layer);
  
  // Update meter
layer_mark_dirty(s_battery_layer);
}
static void add_countup_layer (Window *window){
  
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // Create the TextLayer with specific bounds
  s_countup_layer = text_layer_create(
      GRect(0, PBL_IF_ROUND_ELSE(58, 89), bounds.size.w, 50));

  // Improve the layout to be more like a watchface
  text_layer_set_background_color(s_countup_layer, GColorClear);
  text_layer_set_text_color(s_countup_layer, GColorWhite);
  
  time_t now = time(NULL);
  time_t nownow; 
  struct tm * timeinfo;
  
  timeinfo = localtime(&now);
  // Conception Date
  timeinfo->tm_year = 116;
  timeinfo->tm_mon = 1;
  timeinfo->tm_mday = 8;

  nownow = mktime(timeinfo);
  
  difference = (now - nownow) / SECONDS_PER_DAY;
  
  int weeks = difference / 7;
  
  char* size = baby_array[weeks].size;
  
  //Write the current hours and minutes into a buffer
  static char s_buffer[100];
   
  snprintf(s_buffer,sizeof(s_buffer),"%d Weeks\n %s", weeks, size);
 
  text_layer_set_text(s_countup_layer, s_buffer);
  // Create GFont
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_OSWALD_MEDIUM_16));

  // Apply to TextLayer
  text_layer_set_font(s_countup_layer, s_time_font);
  text_layer_set_text_alignment(s_countup_layer, GTextAlignmentCenter);
    
  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_countup_layer));
  
}
/// ****************************
static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  // Write the current hours and minutes into a buffer
  //static char s_buffer[10];
  //strftime(s_buffer, sizeof(s_buffer), "%T", tick_time);

  // Write the current hours and minutes into a buffer
  static char s_buffer[6];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);
  static char s_date_buffer[6];
  strftime(s_date_buffer, sizeof(s_date_buffer), "%d.%m", tick_time);
  static char s_day_buffer[10];
  strftime(s_day_buffer, sizeof(s_day_buffer), "%A", tick_time);
  
  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, s_buffer);
  // Display this date on the TextLayer
  text_layer_set_text(s_date_layer, s_date_buffer);
  // Display this day on the TextLayer
  text_layer_set_text(s_day_layer, s_day_buffer);
}
//Timer hander
static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
update_time();
update_countdown();
  
  //manage display
  if (display_on == true){
    display_time++; 
    if(display_time == 10){      
      display_time = 0;
      display_on = false;
    }   
  } 
  
   // Get weather update every 30 minutes
  if(tick_time->tm_min % 30 == 0) {
    // Begin dictionary
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);

    // Add a key-value pair
    dict_write_uint8(iter, 0, 0);

    // Send the message!
    app_message_outbox_send();
    
    update_countup();
  }
}
static void data_handler(AccelData *data, uint32_t num_samples) {
  // Long lived buffer
  //static char s_buffer[128];

  // Compose string of all data for 3 samples
  //snprintf(s_buffer, sizeof(s_buffer), 
   // "N X,Y,Z\n0 %d,%d,%d\n1", 
   // data[0].x, data[0].y, data[0].z
 // );
    
  if(data[0].y > 800 && data[0].x >-300 && data[0].x < 300)
    {
      //Show the baby data
      if(display_reset == true)
        {
          display_on = true;
          display_reset = false;
          update_countup(); 
          layer_set_hidden((Layer*)s_countup_layer, false);
          layer_set_hidden((Layer*)s_countdown_layer, false);
          layer_set_hidden((Layer*)s_weather_condition_layer, false);
          layer_set_hidden((Layer*)s_weather_name_layer, false); 
          layer_set_hidden((Layer*)s_day_layer, true);
          layer_set_hidden((Layer*)s_weather_layer, false); 
        }
  }
  else if(data[0].y < 50 && data[0].y > -50 && data[0].x >-200 && data[0].x < 200)
  {
    display_reset = true;    
  }
  else if(data[0].x < -800 && data[0].y >-300 && data[0].y < 300)
  {
    if(display_reset == true)
        {
          display_on = true;
          display_reset = false;         
          layer_set_hidden((Layer*)s_weather_condition_layer, false);
          layer_set_hidden((Layer*)s_weather_name_layer, false);
          layer_set_hidden((Layer*)s_weather_layer, false);  
        }    
  }
  else
    {
      if(display_on == false){
        //Show the data
        // Good to use to override weather to see axis information
       // text_layer_set_text(s_weather_layer, s_buffer);
        layer_set_hidden((Layer*)s_countup_layer, true);
        layer_set_hidden((Layer*)s_countdown_layer, true);
        layer_set_hidden((Layer*)s_weather_condition_layer, true);
        layer_set_hidden((Layer*)s_weather_name_layer, true);
        layer_set_hidden((Layer*)s_day_layer, false);
        layer_set_hidden((Layer*)s_weather_layer, true); 
      }           
    }  
}
//Motion Detection
static void tap_handler(AccelAxisType axis, int32_t direction) {
switch (axis) {
  case ACCEL_AXIS_X:
    if (direction > 0) {
      APP_LOG(APP_LOG_LEVEL_INFO, "X axis positive.");
    } else {
      APP_LOG(APP_LOG_LEVEL_INFO, "X axis negative.");
    }
    break;
  case ACCEL_AXIS_Y:
    if (direction > 0) {
      APP_LOG(APP_LOG_LEVEL_INFO, "Y axis positive.");
    } else {
      APP_LOG(APP_LOG_LEVEL_INFO, "Y axis negative.");
    }
    break;
  case ACCEL_AXIS_Z:
    if (direction > 0) {
      APP_LOG(APP_LOG_LEVEL_INFO, "Z axis positive.");          
    } else {
      APP_LOG(APP_LOG_LEVEL_INFO, "Z axis negative.");
    }
    break;
  }

}
//Create an instance of a window
static void main_window_load(Window *window) {
  add_date_layer(window);
  add_time_layer(window);
  add_day_layer(window);
  add_weather_layer(window);
  add_countdown_layer(window); 
  add_countup_layer(window);  
  add_battery_layer(window);
  add_bluetooth_layer(window);
  
}
static void main_window_unload(Window *window) {
// Destroy TextLayer
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_day_layer);
  text_layer_destroy(s_countdown_layer);
  text_layer_destroy(s_countup_layer);
  text_layer_destroy(s_weather_layer);
  text_layer_destroy(s_weather_name_layer);
  text_layer_destroy(s_weather_condition_layer);
  layer_destroy(s_battery_layer);
  // Unload GFont
fonts_unload_custom_font(s_time_font);
gbitmap_destroy(s_bt_icon_bitmap);
bitmap_layer_destroy(s_bt_icon_layer);
}
//make the task of managing memory allocation and deallocation as simple as possible
static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);

  window_set_background_color(s_main_window, GColorBlack);
  
  // Register with TickTimerService
  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);

  // Use tap service? If not, use data service
  if (TAP_NOT_DATA) {
    // Subscribe to the accelerometer tap service
    accel_tap_service_subscribe(tap_handler);
  } else {
    // Subscribe to the accelerometer data service
    int num_samples = 1;
    accel_data_service_subscribe(num_samples, data_handler);

    // Choose update rate
    accel_service_set_sampling_rate(ACCEL_SAMPLING_10HZ);
  }

  
  // Make sure the time is displayed from the start
  update_time();
  
  // Register weather callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  
  // Open AppMessage
  const int inbox_size = 128;
  const int outbox_size = 128;
  app_message_open(inbox_size, outbox_size);
  // If buffer isn't big enough use this
  // app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  
  // Register for battery level updates
battery_state_service_subscribe(battery_callback);
  // Ensure battery level is displayed from the start
battery_callback(battery_state_service_peek());
  
  // Register for Bluetooth connection updates
connection_service_subscribe((ConnectionHandlers) {
  .pebble_app_connection_handler = bluetooth_callback
});
  
  // Show the correct state of the BT connection from the start
  bluetooth_callback(connection_service_peek_pebble_app_connection());
  
  //LoadConfig
  /*
  if (persist_read_bool(KEY_SHOWLOCATION)) {
    showlocations = persist_read_bool(KEY_SHOWLOCATION);   
  }
  */
  if (persist_read_int(KEY_DUEDATE)) {
    s_duedate = persist_read_int(KEY_DUEDATE);
  }  
  reload_config();  
}
static void deinit() {
   // Destroy Window to make sure that memory is given back - its good
  // practice to make sure we match every create with a destroy
  window_destroy(s_main_window);
  if (TAP_NOT_DATA) {
    accel_tap_service_unsubscribe();
  } else {
    accel_data_service_unsubscribe();
  }

}
//App main runs on startup - this is the main function
int main(void) {
  init();
  // lets the watchapp wait for system events until it exits. 
  app_event_loop();
  deinit();
}