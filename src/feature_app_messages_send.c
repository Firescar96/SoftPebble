#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"

#define MY_UUID { 0x1A, 0xDF, 0xFA, 0x98, 0x98, 0xB7, 0x43, 0x72, 0xBC, 0x25, 0xE4, 0x6F, 0xAA, 0xA0, 0x73, 0x94 }
PBL_APP_INFO_SIMPLE(MY_UUID, "BreadCrumbs", "Mapping Pebbles", 1 /* App version */);

#define TOTAL_IMAGE_SLOTS 1

#define NUMBER_OF_IMAGES 4

enum {
  CMD_UP = 0x0, // TUPLE_INTEGER
  CMD_DOWN = 0x1,
  CMD_LEFT = 0x2,
  CMD_RIGHT = 0x3
};
/*
	Up 0x00
	Right 0x01
	Down 0x02
	Left 0x03
*/

Layer dirLay;
 bool callbacks_registered;
 AppMessageCallbacksNode app_callbacks;	
	
Window window;

AppTimerHandle timer_handle;

const int IMAGE_RESOURCE_IDS[NUMBER_OF_IMAGES] = {
  RESOURCE_ID_LEFT, RESOURCE_ID_RIGHT, RESOURCE_ID_UP
};

BmpContainer image_containers[TOTAL_IMAGE_SLOTS];
AppContextRef acr;

#define EMPTY_SLOT -1


// The state is either "empty" or the digit of the image currently in
// the slot--which was going to be used to assist with de-duplication
// but we're not doing that due to the one parent-per-layer
// restriction mentioned above.
int image_slot_state[TOTAL_IMAGE_SLOTS] = {EMPTY_SLOT};

void load_digit_image(unsigned short value) {
  /*

     Loads the digit image from the application's resources and
     displays it on-screen in the correct location.

     Each slot is a quarter of the screen.
	 
	 digit 0 = left
	 digit 2 = center
	 digit 1 = right
	 digit 3 = back

   */

  // TODO: Signal these error(s)?

	int slot_number = 0;
	
  if ((slot_number < 0) || (slot_number >= TOTAL_IMAGE_SLOTS)) {
    return;
  }

  if (image_slot_state[slot_number] != EMPTY_SLOT) {
    return;
  }

  image_slot_state[slot_number] = 1;
  bmp_init_container(IMAGE_RESOURCE_IDS[value], &image_containers[slot_number]);
  image_containers[slot_number].layer.layer.frame.origin.x = 0;
  image_containers[slot_number].layer.layer.frame.origin.y = 0;
  layer_add_child(&window.layer, &image_containers[slot_number].layer.layer);

}

void unload_digit_image_from_slot() {
  /*

     Removes the digit from the display and unloads the image resource
     to free up RAM.

     Can handle being called on an already empty slot.

   */
	int slot_number = 0;
  if (image_slot_state[slot_number] != EMPTY_SLOT) {
    layer_remove_from_parent(&image_containers[slot_number].layer.layer);
    bmp_deinit_container(&image_containers[slot_number]);
    image_slot_state[slot_number] = EMPTY_SLOT;
  }

}

void display_value(unsigned short value) {
  /*

     Displays a numeric value between 0 and 99 on screen.

     Rows are ordered on screen as:

       Row 0
       Row 1

     Includes optional blanking of first leading zero,
     i.e. displays ' 0' rather than '00'.

   */

  // Column order is: | Column 0 | Column 1 |
  // (We process the columns in reverse order because that makes
  // extracting the digits from the value easier.)
    unload_digit_image_from_slot();
	  
    load_digit_image(value);
}

 void app_received_msg(DictionaryIterator* iter, void* context) 
{	
	//Create a tuple from the received dictionary using the key	
  Tuple *in_tuple = dict_find(iter, CMD_UP);
	//If the tuple was successfully obtained
  if (in_tuple)
  {
  	display_value(0);
  }
	in_tuple = dict_find(iter, CMD_DOWN);
	//If the tuple was successfully obtained
  if (in_tuple)
  	display_value(2);
	
	in_tuple = dict_find(iter, CMD_LEFT);
	//If the tuple was successfully obtained
  if (in_tuple)
  	display_value(1);
	
	in_tuple = dict_find(iter, CMD_RIGHT);
	//If the tuple was successfully obtained
  if (in_tuple)
  	display_value(3);
  	//Show the recieved string on the watch!
}

bool register_callbacks() {
	if (callbacks_registered) {
		if (app_message_deregister_callbacks(&app_callbacks) == APP_MSG_OK)
			callbacks_registered = false;
	}
	if (!callbacks_registered) {
		app_callbacks = (AppMessageCallbacksNode){
			.callbacks = {
				//.out_failed = app_send_failed,
        .in_received = app_received_msg
			},
			.context = NULL
		};
		if (app_message_register_callbacks(&app_callbacks) == APP_MSG_OK) {
      callbacks_registered = true;
    }
	}
	return callbacks_registered;
}

// Modify these common button handlers

void up_single_click_handler(ClickRecognizerRef recognizer, Window *window) {
  (void)recognizer;
  (void)window;

	display_value(0);
	timer_handle = app_timer_send_event	
	(	
		acr, 1000, 42 
    );	
}


void down_single_click_handler(ClickRecognizerRef recognizer, Window *window) {
  (void)recognizer;
  (void)window;
	
	display_value(1);
	timer_handle = app_timer_send_event	
	(	
		acr, 1000, 42 
    );
}


void select_single_click_handler(ClickRecognizerRef recognizer, Window *window) {
  (void)recognizer;
  (void)window;

  display_value(2);
	timer_handle = app_timer_send_event	
	(	
		acr, 1000, 42 
    );
}

void select_long_click_handler(ClickRecognizerRef recognizer, Window *window) {
  (void)recognizer;
  (void)window;

	display_value(3);
	timer_handle = app_timer_send_event	
	(	
		acr, 1000, 42 
    );
}

void handle_timer(AppContextRef ctx, AppTimerHandle handle, uint32_t cookie) {

  if (cookie == 42) {
      unload_digit_image_from_slot();
  }

  // If you want the timer to run again you need to call `app_timer_send_event()`
  // again here.
}

// This usually won't need to be modified

void click_config_provider(ClickConfig **config, Window *window) {
  (void)window;

  config[BUTTON_ID_SELECT]->click.handler = (ClickHandler) select_single_click_handler;

  config[BUTTON_ID_SELECT]->long_click.handler = (ClickHandler) select_long_click_handler;

  config[BUTTON_ID_UP]->click.handler = (ClickHandler) up_single_click_handler;
  config[BUTTON_ID_UP]->click.repeat_interval_ms = 100;

  config[BUTTON_ID_DOWN]->click.handler = (ClickHandler) down_single_click_handler;
  config[BUTTON_ID_DOWN]->click.repeat_interval_ms = 100;
}


// Standard app initialisation

void handle_init(AppContextRef ctx) {
  (void)ctx;
	acr = ctx;
  window_init(&window, "BreadCrumbs");
  window_stack_push(&window, true /* Animated */);
	window_set_background_color(&window, GColorBlack);

	resource_init_current_app(&APP_RESOURCES);
	

  //timer_handle = app_timer_send_event(acr, 1500 /* milliseconds */, 42);
register_callbacks();
  // Attach our desired button functionality
  window_set_click_config_provider(&window, (ClickConfigProvider) click_config_provider);
}

void pbl_main(void *params) {
  PebbleAppHandlers handlers = {
    .init_handler = &handle_init,
	.timer_handler = &handle_timer,
    .messaging_info = {
      .buffer_sizes = {
        .inbound = 256,
        .outbound = 256,
      }
	}
  };
  app_event_loop(params, &handlers);
}
