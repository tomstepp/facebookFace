///----- My first watchface (time + date + weather + battery) -----///

// --- libraries and definitions --- //
#include <pebble.h>
#define KEY_TEMPERATURE 0
#define KEY_CONDITIONS 1

// --- elements of watchface --- //
static Window *main_window;
static TextLayer *facebook_layer;
static TextLayer *time_layer;
static TextLayer *value_layer;
static TextLayer *weather_layer;
static GFont weather_font;
static GFont time_font;
static int value = 4;

// --- battery --- //
static int battery_level;
static void battery_callback(BatteryChargeState state) {
  // Record the new battery level
  battery_level = state.charge_percent;
}

// --- app message --- //
static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // Store incoming information
  static char temperature_buffer[8];
  static char conditions_buffer[32];
  static char weather_layer_buffer[32];

  // Read tuples for data
  Tuple *temp_tuple = dict_find(iterator, MESSAGE_KEY_TEMPERATURE);
  Tuple *conditions_tuple = dict_find(iterator, MESSAGE_KEY_CONDITIONS);

  // If all data is available, use it
  if(temp_tuple && conditions_tuple) {
    snprintf(temperature_buffer, sizeof(temperature_buffer), "%d F", (int)temp_tuple->value->int32);
    snprintf(conditions_buffer, sizeof(conditions_buffer), "%s", conditions_tuple->value->cstring);

    // Assemble full string and display
    snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "%d %%  %s", battery_level, temperature_buffer);
    text_layer_set_text(weather_layer, weather_layer_buffer);
  }
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

// --- time handler and updates --- //
static void update_time() {
  // get time structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  
  // write time into buffer
  static char buf[8];
  strftime(buf, sizeof(buf), "%I:%M", tick_time);
  
  // display time on textlayer
  text_layer_set_text(time_layer, buf);
  
  // Copy date into buffer from tm structure
  static char date_buffer[16];
  strftime(date_buffer, sizeof(date_buffer), "%a %d %b", tick_time);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
  
  // Get weather update every 30 minutes
  if(tick_time->tm_min % 30 == 0) {
    // Begin dictionary
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
  
    // Add a key-value pair
    dict_write_uint8(iter, 0, 0);
  
    // Send the message
    app_message_outbox_send();
  }
  
  if(tick_time->tm_min % 1 == 0) {
    value = (value + 1) % 5;
    
    switch(value){
      case 0:
         text_layer_set_text(value_layer, "1. Be Bold");
         break;
      case 1:
         text_layer_set_text(value_layer, "2. Focus on Impact");
         break;
      case 2:
         text_layer_set_text(value_layer, "3. Move Fast");
         break;
      case 3:
         text_layer_set_text(value_layer, "4. Be Open");
         break;
      case 4:
         text_layer_set_text(value_layer, "5. Build Social Value");
         break;
    }
  }
}

// --- main window load/unload --- //
static void main_load(Window *w) {
  // get window info
  Layer *window_layer = window_get_root_layer(w);
  GRect bounds = layer_get_bounds(window_layer);
  
  // create time layer
  time_layer = text_layer_create(GRect(0, 50, bounds.size.w, 50));
  // style time layer
  text_layer_set_background_color(time_layer, GColorWhite);
  text_layer_set_text_color(time_layer, GColorBlue);
  text_layer_set_text(time_layer, "00:00");
  text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);
  // create text layer font
  time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_EXTROS_45));
  text_layer_set_font(time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  
  // create weather layer
  weather_layer = text_layer_create(GRect(0, 135, bounds.size.w, 25));
  // style weather layer
  text_layer_set_background_color(weather_layer, GColorBlue);
  text_layer_set_text_color(weather_layer, GColorWhite);
  text_layer_set_text_alignment(weather_layer, GTextAlignmentCenter);
  text_layer_set_text(weather_layer, "?? %% ?? F");
  // create weather layer font
  text_layer_set_font(weather_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  
  // create facebook layer
  facebook_layer = text_layer_create(GRect(0, 15, bounds.size.w, 25));
  // style facebook layer
  text_layer_set_background_color(facebook_layer, GColorBlue);
  text_layer_set_text_color(facebook_layer, GColorWhite);
  text_layer_set_text_alignment(facebook_layer, GTextAlignmentCenter);
  text_layer_set_text(facebook_layer, "facebook");
  // create facebook layer font
  text_layer_set_font(facebook_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  
  // create value layer
  value_layer = text_layer_create(GRect(0, 100, bounds.size.w, 30));
  // style value layer
  text_layer_set_background_color(value_layer, GColorWhite);
  text_layer_set_text_color(value_layer, GColorBlue);
  text_layer_set_text_alignment(value_layer, GTextAlignmentCenter);
  text_layer_set_text(value_layer, "FB Values");
  // create value layer font
  text_layer_set_font(value_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  
  // add children
  layer_add_child(window_layer, text_layer_get_layer(time_layer));
  layer_add_child(window_get_root_layer(w), text_layer_get_layer(weather_layer));
  layer_add_child(window_get_root_layer(w), text_layer_get_layer(facebook_layer));
  layer_add_child(window_get_root_layer(w), text_layer_get_layer(value_layer));
}

static void main_unload(Window *w) {
  // destroy layers
  text_layer_destroy(time_layer);
  text_layer_destroy(weather_layer);
  text_layer_destroy(facebook_layer);
  text_layer_destroy(value_layer);
  
  // destory fonts
  fonts_unload_custom_font(time_font);
  fonts_unload_custom_font(weather_font);
}

// --- init, deinit, and main --- //
static void init() {
  // create new window
  main_window = window_create();
  
  // update parameters - black background
  window_set_background_color(main_window, GColorBlue);
  
  // set handlers for window elements
  window_set_window_handlers(main_window, (WindowHandlers) {
    .load = main_load,
    .unload = main_unload
  });
  
  // show window, make it animated
  window_stack_push(main_window, true);
  
  // display time
  update_time();
  
  // register with timer service
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  // app message register calls
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  
  // Open AppMessage
  const int inbox_size = 128;
  const int outbox_size = 128;
  app_message_open(inbox_size, outbox_size);
  
  // Register for battery level updates
  battery_state_service_subscribe(battery_callback);
  
  // Ensure battery level is displayed from the start
  battery_callback(battery_state_service_peek());
}

static void deinit() {
  // destroy main window
  window_destroy(main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}