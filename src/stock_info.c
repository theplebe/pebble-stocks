#include <pebble.h>
#include "stock_info.h"

//144 x 168

#define GRAPH_SIZE 20


static Window *window;
static TextLayer *text_stock_name;
static TextLayer *text_stock_value;
static float stock_current_value;
static float stock_start_value;
static char str_stock_name[20];
static char str_stock_value[20];

static GPoint graph_points[GRAPH_SIZE+2];
static GPathInfo graph_info = { GRAPH_SIZE+2, graph_points };
static GPath* graph;

//static GFont font_stock_name;

static void init_graph()
{
   srand(0);
   int i;
   graph_points[0] = (GPoint){ 8, 40 };
   for(i=1;i<=GRAPH_SIZE;i++)
   {
      graph_points[i].x = 8+i*5;
      graph_points[i].y = 40+(int) ((float)rand()/RAND_MAX*40);
   }
   graph_points[GRAPH_SIZE+1] = (GPoint){ 8+GRAPH_SIZE*5, 40 };
}

static void graph_draw(struct Layer* layer, GContext* ctx)
{
   gpath_draw_outline(ctx, graph);
}

static void set_stock_value_str()
{
   float percent;
   if (stock_start_value != 0)
   {
      percent = 100.0f*(stock_current_value-stock_start_value)/
                     stock_start_value;
   }
   else
      percent = 0;
   float absolute = stock_current_value-stock_start_value;

   int dec_pc = (int)(percent*100);
   int int_pc = dec_pc/100;
   if (dec_pc < 0) dec_pc = -dec_pc;
   int frc_pc = dec_pc%100;

   int dec_ab = (int)(absolute*100);
   int int_ab = dec_ab/100;
   if (dec_ab < 0) dec_ab = -dec_ab;
   int frc_ab = dec_ab%100;

   if (stock_start_value != 0)
      snprintf(str_stock_value, sizeof(str_stock_value),
               "%+d.%02d (%%%+d.%02d)", int_part, frac_part);
   else
      snprintf(str_stock_value, sizeof(str_stock_value),
               "%+d.%02d (%%-----)", int_part);
   text_layer_set_text(text_stock_value, str_stock_value);
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context)
{
   stock_current_value+=1;
   set_stock_value_str();
   //TODO: Go to previous stock symbol
}
static void down_click_handler(ClickRecognizerRef recognizer, void *context)
{
   stock_current_value-=1;
   set_stock_value_str();
   //TODO: Go to next stock symbol
}

static void click_config_provider(void *context)
{
   window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
   window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
   window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void window_load(Window *window)
{
   Layer *window_layer = window_get_root_layer(window);
   GRect bounds = layer_get_bounds(window_layer);

   //Stock name (at the top)
   text_stock_name = text_layer_create((GRect) {
         .origin = { 8, 8 },
         .size = { bounds.size.w-16, 20 }
   });
   text_layer_set_text(text_stock_name, str_stock_name);
   text_layer_set_text_alignment(text_stock_name, GTextAlignmentCenter);
   text_layer_set_background_color(text_stock_name, GColorBlack);
   text_layer_set_text_color(text_stock_name, GColorWhite);
   //text_layer_set_font(text_stock_name, font_stock_name);


   //Stock value (at the bottom)
   text_stock_value = text_layer_create((GRect) {
         .origin = { 8, bounds.size.h-28 },
         .size = { bounds.size.w-16, 20 }
   });
   set_stock_value_str();
   text_layer_set_text_alignment(text_stock_value, GTextAlignmentCenter);


   layer_add_child(window_layer, text_layer_get_layer(text_stock_name));
   layer_add_child(window_layer, text_layer_get_layer(text_stock_value));
   layer_set_update_proc(window_layer, graph_draw);
}

static void window_unload(Window *window)
{
   text_layer_destroy(text_stock_name);
   text_layer_destroy(text_stock_value);
   gpath_destroy(graph);
}

/*Initialize the stock info page*/
void stock_info_init(void)
{
   //Create window
   window = window_create();
   window_set_click_config_provider(window, click_config_provider);
   window_set_window_handlers(window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload,
   });

   init_graph();

   graph = gpath_create(&graph_info);

   //font_stock_name = fonts_load_custom_font(
   //      resource_get_handle(RESOURCE_ID_FONT_ROBOTO_BOLD_20));


   strcpy(str_stock_name, "???");
   stock_current_value = 0;
   stock_start_value = 0;

   window_stack_push(window, true);
}

/*Destroy the stock info page*/
void stock_info_deinit(void)
{
   window_destroy(window);
}

/*Get the stock info page window*/
Window* stock_info_get_window(void)
{
   return window;
}

/*Sets the symbol for the stock page*/
void stock_info_set_symbol(char* symbol)
{
   strncpy(str_stock_name, symbol, sizeof(str_stock_name)-1);
   //TODO: Get info based on symbol
}
