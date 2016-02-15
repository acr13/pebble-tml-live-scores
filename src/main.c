#include <pebble.h>

// Main Window
Window *window;

// Text Layers
TextLayer *text_date_layer;
TextLayer *text_time_layer;
TextLayer *text_score_layer;

// Leafs
GBitmap *leafs;
BitmapLayer *leafs_layer;

int WATCH_WIDTH = 144;
int WATCH_HEIGHT = 168;

static AppSync sync;
static uint8_t sync_buffer[64];

static AppTimer *timer;

void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed)
{
	static char time_text[] = "00:00";
	static char date_text[] = "Xxxxxxxxx 00";
	
	char *time_format;
	
	// Conver the tick time to a date string 
	// Example: November 19
	strftime(date_text, sizeof(date_text), "%B %e", tick_time);
	text_layer_set_text(text_date_layer, date_text);

	// user clock setting
	if (clock_is_24h_style())
	{
		time_format = "%R";
	}
	else
	{
		time_format = "%I:%M";
	}
	
	strftime(time_text, sizeof(time_text), time_format, tick_time);
	
	if (time_text[0] == '0')
	{
        time_text[0] = time_text[1];
		time_text[1] = time_text[2];
		time_text[2] = time_text[3];
		time_text[3] = time_text[4];
		time_text[4] = time_text[5];
	}
	
	text_layer_set_text(text_time_layer, time_text);
}

void send_cmd(void)
{
	Tuplet value = TupletInteger(0, 1);

 	DictionaryIterator *iter;
  	app_message_outbox_begin(&iter);

  	if (iter == NULL) 
	{
		//APP_LOG(APP_LOG_LEVEL_DEBUG, "ITER == NULL");
    	return;
    }

  	dict_write_tuplet(iter, &value);
  	dict_write_end(iter);

  	app_message_outbox_send();
}

void sync_error_callback(DictionaryResult error, AppMessageResult msgError, void *context)
{
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Sync Error: %d", msgError);
}

void sync_tuple_changed_callback(const uint32_t key, const Tuple* new_tuple, const Tuple* old_tuple, void* context) 
{
	switch(key)
	{
		case 0:
		
		if (strlen(new_tuple->value->cstring) < 2)
		{
			return;
		}
		
		//APP_LOG(APP_LOG_LEVEL_DEBUG, "Received value = %s", new_tuple->value->cstring);
		// add score to screen
		text_layer_set_text(text_score_layer, new_tuple->value->cstring);
		
		break;
	}
}

void timer_callback(void *data)
{
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "TIMER CALL BACK ************************");
	send_cmd();
	
	// re start the timer
	timer = app_timer_register(60000, timer_callback, NULL);
}

// Init function to handle the creation of layers,
// event subscribing, etc
void handle_init(void) 
{
	// window init
	window = window_create();
	window_stack_push(window, true);
	window_set_background_color(window, GColorWhite);

	// display layer
	Layer *window_layer = window_get_root_layer(window);
	
	// background
	leafs = gbitmap_create_with_resource(RESOURCE_ID_BKNG);
	leafs_layer = bitmap_layer_create(GRect(0, 0, WATCH_WIDTH, WATCH_HEIGHT));
	bitmap_layer_set_bitmap(leafs_layer, leafs);
	layer_add_child(window_layer, bitmap_layer_get_layer(leafs_layer));
	
	GFont prox_reg = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_PROXIMA_REGULAR_16));
	GFont prox_bold = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_PROXIMA_BOLD_30));

	// time init
	text_time_layer = text_layer_create(GRect(55, 100,(WATCH_WIDTH - 55), 50));
	text_layer_set_font(text_time_layer, prox_bold);
	text_layer_set_background_color(text_time_layer, GColorClear);
    layer_add_child(window_layer, text_layer_get_layer(text_time_layer));
	
	// date init
	text_date_layer = text_layer_create(GRect(55, 130, (WATCH_WIDTH - 55), 20));
	text_layer_set_font(text_date_layer, prox_reg);
	text_layer_set_background_color(text_date_layer, GColorClear);
    layer_add_child(window_layer, text_layer_get_layer(text_date_layer));
	
	// score init
	text_score_layer = text_layer_create(GRect(100, 84, (WATCH_WIDTH - 55), 20));
	text_layer_set_font(text_score_layer, prox_reg);
	text_layer_set_text_color(text_score_layer, GColorWhite);
	text_layer_set_background_color(text_score_layer, GColorClear);
    layer_add_child(window_layer, text_layer_get_layer(text_score_layer));

 	//update_display(tick_time);
	tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
	
	// AppSync stuff
	const int inbound_size = 64;
	const int outbound_size = 64;
	app_message_open(inbound_size, outbound_size);
	
	Tuplet initial_values[] = {
		TupletCString(0, "0-0")
	};
	
	app_sync_init(&sync, sync_buffer, sizeof(sync_buffer), initial_values, ARRAY_LENGTH(initial_values),
				 sync_tuple_changed_callback, sync_error_callback, NULL);
	
	send_cmd();
	
	// setting up timer to send cmds for updates
	timer = app_timer_register(10000, timer_callback, NULL);
}


void handle_deinit(void) 
{
	layer_remove_from_parent(bitmap_layer_get_layer(leafs_layer));
  	bitmap_layer_destroy(leafs_layer);
 	gbitmap_destroy(leafs);
	
	tick_timer_service_unsubscribe();
	
	layer_destroy(text_layer_get_layer(text_time_layer));
	layer_destroy(text_layer_get_layer(text_date_layer));
	layer_destroy(text_layer_get_layer(text_score_layer));
	
	window_destroy(window);
}

int main(void) 
{
	handle_init();
	app_event_loop();
	handle_deinit();
}