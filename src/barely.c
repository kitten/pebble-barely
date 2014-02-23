#include <pebble.h>

enum {
	KEY_INVERTED = 0
};

#define PERSIST_INVERTED 1

static Window *window;
static Layer *canvas;

static Layer *topLeft;
static Layer *topRight;
static Layer *bottomLeft;
static Layer *bottomRight;

int displayTopLeft = 0;
int displayTopRight = 0;
int displayBottomLeft = 0;
int displayBottomRight = 0;

bool isInverted = false;

static unsigned short get_display_hour(unsigned short hour) {
	if (clock_is_24h_style()) {
		return hour;
	}
	unsigned short display_hour = hour % 12;
	return display_hour ? display_hour : 12;
}

void drawHorizontalLine(GContext* ctx, GPoint start, GPoint goal) {
	graphics_fill_rect(ctx, GRect(start.x, start.y - 1, goal.x - start.x, 3), 0, GCornerNone);
}

void drawVerticalLine(GContext* ctx, GPoint start, GPoint goal) {
	graphics_fill_rect(ctx, GRect(start.x - 1, start.y - 1, 3, goal.y - start.y + 3), 0, GCornerNone);
}

void renderNumber(int number, GContext* ctx) {
	if (number == 1) {
		drawHorizontalLine(ctx, GPoint(0,26), GPoint(22,26));
		drawVerticalLine(ctx, GPoint(22,26), GPoint(22,55));
		drawHorizontalLine(ctx, GPoint(0,55), GPoint(22,55));
		drawVerticalLine(ctx, GPoint(47,0), GPoint(47,55));
		drawHorizontalLine(ctx, GPoint(47,55), GPoint(71,55));
	} else if (number == 2) {
		drawHorizontalLine(ctx, GPoint(0,26), GPoint(47,26));
		drawHorizontalLine(ctx, GPoint(22,55), GPoint(71,55));
	} else if (number == 3) {
		drawHorizontalLine(ctx, GPoint(0,26), GPoint(47,26));
		drawHorizontalLine(ctx, GPoint(0,55), GPoint(47,55));
	} else if (number == 4) {
		drawVerticalLine(ctx, GPoint(35,0), GPoint(35,26));
		drawHorizontalLine(ctx, GPoint(0,55), GPoint(35,55));
		drawVerticalLine(ctx, GPoint(35,55), GPoint(35,83));
	} else if (number == 5) {
		drawHorizontalLine(ctx, GPoint(22,26), GPoint(71,26));
		drawHorizontalLine(ctx, GPoint(0,55), GPoint(47,55));
	} else if (number == 6) {
		drawHorizontalLine(ctx, GPoint(22,26), GPoint(71,26));
		drawHorizontalLine(ctx, GPoint(22,55), GPoint(47,55));
	} else if (number == 7) {
		drawHorizontalLine(ctx, GPoint(0,26), GPoint(35,26));
		drawVerticalLine(ctx, GPoint(35,26), GPoint(35,83));
	} else if (number == 8) {
		drawHorizontalLine(ctx, GPoint(22,26), GPoint(47,26));
		drawHorizontalLine(ctx, GPoint(22,55), GPoint(47,55));
	} else if (number == 9) {
		drawHorizontalLine(ctx, GPoint(22,26), GPoint(47,26));
		drawHorizontalLine(ctx, GPoint(0,55), GPoint(47,55));
	} else {
		drawVerticalLine(ctx, GPoint(35,26), GPoint(35,55));
	}
}

void canvas_update_callback(Layer *layer, GContext* ctx) {
	if (isInverted) {
		graphics_context_set_fill_color(ctx, GColorWhite);
	}
	graphics_fill_rect(ctx, GRect(71,0,3,168), 0, GCornerNone);
	graphics_fill_rect(ctx, GRect(0,83,144,3), 0, GCornerNone);
}

void topLeft_update_callback(Layer *layer, GContext* ctx) {
	if (isInverted) {
		graphics_context_set_fill_color(ctx, GColorWhite);
	}
	renderNumber(displayTopLeft, ctx);
}

void topRight_update_callback(Layer *layer, GContext* ctx) {
	if (isInverted) {
		graphics_context_set_fill_color(ctx, GColorWhite);
	}
	renderNumber(displayTopRight, ctx);
}

void bottomLeft_update_callback(Layer *layer, GContext* ctx) {
	if (isInverted) {
		graphics_context_set_fill_color(ctx, GColorWhite);
	}
	renderNumber(displayBottomLeft, ctx);
}

void bottomRight_update_callback(Layer *layer, GContext* ctx) {
	if (isInverted) {
		graphics_context_set_fill_color(ctx, GColorWhite);
	}
	renderNumber(displayBottomRight, ctx);
}

void process_tuple(Tuple *t) {
	int key = t->key;
	int value = t->value->int32;
	char string_value[32];
	strcpy(string_value, t->value->cstring);

	if (key == KEY_INVERTED) {
		if (strcmp(string_value, "on") == 0) {
			isInverted = true;
		} else {
			isInverted = false;
		}
		
		if (isInverted) {
			window_set_background_color(window, GColorBlack);
		} else {
			window_set_background_color(window, GColorWhite);
		}
		layer_mark_dirty(topRight);
		layer_mark_dirty(topLeft);
		layer_mark_dirty(bottomRight);
		layer_mark_dirty(bottomLeft);
		layer_mark_dirty(canvas);
	}
}

void in_received_handler(DictionaryIterator *iter, void *context) {
	Tuple *t = dict_read_first(iter);
	if(t) {
		process_tuple(t);
	}
  	while(t != NULL) {
		t = dict_read_next(iter);
		if(t) {
      		process_tuple(t);
		}
	}
}

void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
	int hour = get_display_hour(tick_time->tm_hour);
	int minute = tick_time->tm_min;
	if (displayTopRight != hour % 10) {
		displayTopRight = hour % 10;
		layer_mark_dirty(topRight);
	}
	if (displayTopLeft != hour / 10 % 10) {
		displayTopLeft = hour / 10 % 10;
		layer_mark_dirty(topLeft);
	}
	if (displayBottomRight != minute % 10) {
		displayBottomRight = minute % 10;
		layer_mark_dirty(bottomRight);
	}
	if (displayBottomLeft != minute / 10 % 10) {
		displayBottomLeft = minute / 10 % 10;
		layer_mark_dirty(bottomLeft);
	}
}

void handle_init(void) {
	window = window_create();
	window_stack_push(window, true);
	
	app_message_register_inbox_received(in_received_handler);           
	app_message_open(512, 512);
	
	isInverted = persist_exists(PERSIST_INVERTED) ? persist_read_bool(PERSIST_INVERTED) : false;
	
	if (isInverted) {
		window_set_background_color(window, GColorBlack);
	} else {
		window_set_background_color(window, GColorWhite);
	}
	
	struct tm *tick_time;
	time_t temp = time(NULL);
	tick_time = localtime(&temp);
	int hour = get_display_hour(tick_time->tm_hour);
	int minute = tick_time->tm_min;
	if (displayTopRight != hour % 10) {
		displayTopRight = hour % 10;
	}
	if (displayTopLeft != hour / 10 % 10) {
		displayTopLeft = hour / 10 % 10;
	}
	if (displayBottomRight != minute % 10) {
		displayBottomRight = minute % 10;
	}
	if (displayBottomLeft != minute / 10 % 10) {
		displayBottomLeft = minute / 10 % 10;
	}
	
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_frame(window_layer);
	
	canvas = layer_create(bounds);
	layer_set_update_proc(canvas, canvas_update_callback);
	layer_add_child(window_layer, canvas);
	
	topLeft = layer_create(GRect(0,0,71,83));
	layer_set_update_proc(topLeft, topLeft_update_callback);
	layer_add_child(window_layer, topLeft);
	
	topRight = layer_create(GRect(73,0,71,83));
	layer_set_update_proc(topRight, topRight_update_callback);
	layer_add_child(window_layer, topRight);
	
	bottomLeft = layer_create(GRect(0,85,71,83));
	layer_set_update_proc(bottomLeft, bottomLeft_update_callback);
	layer_add_child(window_layer, bottomLeft);
	
	bottomRight = layer_create(GRect(73,85,71,83));
	layer_set_update_proc(bottomRight, bottomRight_update_callback);
	layer_add_child(window_layer, bottomRight);
	
	tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
}

void handle_deinit(void) {
	tick_timer_service_unsubscribe();
	window_destroy(window);
	persist_write_bool(PERSIST_INVERTED, isInverted);
}

int main(void) {
	handle_init();
	app_event_loop();
	handle_deinit();
}