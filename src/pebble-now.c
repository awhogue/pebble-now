#include <pebble.h>

#define QUERY_DATA 0
#define ANSWER_DATA 1

Window *s_main_window;
static ScrollLayer *s_scroll_layer;
static TextLayer *s_output_layer;

static DictationSession *s_dictation_session;
static char s_last_text[512];
static char s_buffer[512];

static void request_answer(char *query) {
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  if (!iter) {
    // Error creating outbound message
    return;
  }

  dict_write_cstring(iter, QUERY_DATA, query);
  dict_write_end(iter);

  app_message_outbox_send();
}

static void set_text(char *text) {
  APP_LOG(APP_LOG_LEVEL_INFO, "set_text()");
  APP_LOG(APP_LOG_LEVEL_INFO, text);

/*   Layer *window_layer = window_get_root_layer(s_main_window); */
/*   GRect bounds = layer_get_frame(window_layer); */

  text_layer_set_text(s_output_layer, text);

/*   GSize max_size = text_layer_get_content_size(s_output_layer); */
/*   text_layer_set_size(s_output_layer, max_size); */
/*   scroll_layer_set_content_size(s_scroll_layer, GSize(bounds.size.w, max_size.h + 4)); */
}

static void dictation_session_callback(DictationSession *session, DictationSessionStatus status, 
                                       char *transcription, void *context) {
  if(status == DictationSessionStatusSuccess) {
    request_answer(transcription);
  } else {
    // Display the reason for any error
    static char s_failed_buff[128];
    snprintf(s_failed_buff, sizeof(s_failed_buff), "Transcription failed.\n\nError ID:\n%d", (int)status);
    set_text(s_failed_buff);
  }
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Message received!");
  Tuple *data = dict_find(iterator, ANSWER_DATA);
  if (data) {
    snprintf(s_buffer, sizeof(s_buffer), "%s", data->value->cstring);
    set_text(s_buffer);
  }
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  // Start voice dictation UI
  set_text("Searching...");
  dictation_session_start(s_dictation_session);
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler); 
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);
  GRect frame = grect_inset(bounds, GEdgeInsets(2));

/*   s_scroll_layer = scroll_layer_create(bounds); */
/*   scroll_layer_set_click_config_onto_window(s_scroll_layer, window); */

  s_output_layer = text_layer_create(frame);
  text_layer_set_font(s_output_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(s_output_layer, GTextAlignmentCenter);
  text_layer_set_overflow_mode(s_output_layer, GTextOverflowModeWordWrap);

/*   scroll_layer_add_child(s_scroll_layer, text_layer_get_layer(s_output_layer)); */
/*   layer_add_child(window_layer, scroll_layer_get_layer(s_scroll_layer)); */
  layer_add_child(window_layer, text_layer_get_layer(s_output_layer));

/*   text_layer_enable_screen_text_flow_and_paging(s_output_layer, 2); */
/*   scroll_layer_set_paging(s_scroll_layer, true); */
}

static void window_unload(Window *window) {
  text_layer_destroy(s_output_layer);
  scroll_layer_destroy(s_scroll_layer);
}

static void init() {
  s_main_window = window_create();
  window_set_click_config_provider(s_main_window, click_config_provider);
  window_set_window_handlers(s_main_window, (WindowHandlers) {
      .load = window_load,
        .unload = window_unload,
        });
  window_stack_push(s_main_window, true);

  // Create new dictation session
  s_dictation_session = dictation_session_create(sizeof(s_last_text), dictation_session_callback, NULL);

  app_message_register_inbox_received(inbox_received_callback);
  app_message_open(512, 512);

  dictation_session_start(s_dictation_session);
}

static void deinit() {
  // Free the last session data
  dictation_session_destroy(s_dictation_session);

  window_destroy(s_main_window);
}

int main() {
  init();
  app_event_loop();
  deinit();
}
