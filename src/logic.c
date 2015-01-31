/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <Ecore.h>
#include <efl_assist.h>
#include <stdlib.h>

#include "main.h"
#include "log.h"
#include "logic.h"
#include "util_efl.h"
#include "setting-text-input.h"

#define BOOTING_STATE_DONE 3
#define BUFSIZE 1024

#ifdef EMULATOR_TYPE
#define USE_ROTATION 0
#else
#define USE_ROTATION 0 //USE_ROTATION 1
#endif

void reply_to_sender(void* data,int result)
{
	retif(data == NULL, , "invalid parameter");

	struct appdata *ad = (struct appdata *)data;

	app_control_h app_control;

	if(app_control_create(&app_control) == 0)
	{
		DBG("reply to caller :: app_control_reply_to_launch_request(%d)",result);
		app_control_reply_to_launch_request(app_control, ad->source_app_control, result);
		app_control_destroy(app_control);
	}
}

int _cache_flush(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	retvm_if(ad == NULL, -1, "Invalid argument: appdata is NULL\n");

	Evas *evas = NULL;

	int file_cache = -1;
	int collection_cache = -1;
	int image_cache = -1;
	int font_cache = -1;

	evas = evas_object_evas_get(ad->win);
	retvm_if(evas == NULL, -1, "Evas is NULL\n");

	file_cache = edje_file_cache_get();
	collection_cache = edje_collection_cache_get();
	image_cache = evas_image_cache_get(evas);
	font_cache = evas_font_cache_get(evas);

	edje_file_cache_set(file_cache);
	edje_collection_cache_set(collection_cache);
	evas_image_cache_set(evas, 0);
	evas_font_cache_set(evas, 0);

	evas_image_cache_flush(evas);
	evas_render_idle_flush(evas);
	evas_font_cache_flush(evas);

	edje_file_cache_flush();
	edje_collection_cache_flush();

	edje_file_cache_set(file_cache);
	edje_collection_cache_set(collection_cache);
	evas_image_cache_set(evas, image_cache);
	evas_font_cache_set(evas, font_cache);

	return 0;
}

static int _lang_change_cb(void *event_info, void *data)
{
	DBG("%s : Language is changed", __func__);
	struct appdata *ad = (struct appdata *)data;
	retv_if(ad == NULL, -1);
	return EXIT_SUCCESS;
}

static Evas_Object* _create_layout_main(Evas_Object* parent)
{
	Evas_Object *layout;

	if (parent == NULL)
	{
		DBG("parent NULL");
		return NULL;
	}

	layout = elm_layout_add(parent);

	elm_layout_theme_set(layout, "layout", "application", "default");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	evas_object_show(layout);

	return layout;
}

static Evas_Object* _create_bg(Evas_Object *parent)
{
	Evas_Object *bg;

	if (parent == NULL) return NULL;

	bg = elm_bg_add(parent);
	evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(parent, bg);
	evas_object_show(bg);

	return bg;
}

void _app_terminate(struct appdata *ad)
{
	DBG("%s", __func__);
}

int _app_create(struct appdata *ad)
{
	DBG("%s", __func__);
	Evas_Object *win = NULL;

	/* create main window */
	if(ad->win == NULL) {
		win = _add_window(PACKAGE);
		retvm_if(win == NULL, -1, "Failed add window\n");
		ad->win = win;
		DBG("create window\n");
	}

	_create_bg(ad->win);

	// Main Layout
	ad->ly_main = _create_layout_main(ad->win);
	retvm_if(ad->ly_main == NULL, -1, "Failed to add main layout\n");
	evas_object_size_hint_weight_set(ad->ly_main, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(ad->win, ad->ly_main);

	initialize_keyboard(ad);

	ad->list = create_keyboard_list(ad->ly_main);

	elm_object_part_content_set(ad->ly_main, "elm.swallow.content", ad->list);

	evas_object_show(ad->ly_main);

	evas_object_show(ad->win);

	/* language change callback */
	if (appcore_set_event_callback(APPCORE_EVENT_LANG_CHANGE, _lang_change_cb, ad) < 0) {
		ERR("Cannot register the language change callback.");
	}

	/* Apply Theme */
	ea_theme_changeable_ui_enabled_set(EINA_TRUE);
	return 0;
}
