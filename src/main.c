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

#include <app.h>
#include <Ecore.h>
#include <unistd.h>
#include <errno.h>
#include "main.h"
#include "log.h"
#include "logic.h"
#include "util_efl.h"
#include "setting-text-input.h"

#ifndef EMULATOR_TYPE
#define USE_ROTATION 0 //USE_ROTATIONS 1
#endif

static bool app_create(void *data)
{
	DBG("app create");
	return true;
}

static void app_terminate(void *data)
{
	struct appdata *ad = data;

	ad->app_state = APP_STATE_TERMINATE;

	finish_keyboard();
	_app_terminate(ad);

#if USE_ROTATION
	/* unregister rotation callback */
	context_item_e context = CONTEXT_ITEM_AUTO_ROTATION;
	context_unset_changed_callback(context);
#endif

	if (ad->win) {
		evas_object_del(ad->win);
		ad->win = NULL;
	}
	app_control_destroy(ad->source_app_control);
}

static void app_pause(void *data)
{
	DBG("%s", __func__);
	struct appdata *ad = data;
	retm_if(ad == NULL, "Invalid argument: appdata is NULL\n");

	ad->app_state = APP_STATE_PAUSE;
	ERR("APP PAUSE");
}

static void app_resume(void *data)
{
	DBG("%s", __func__);
	struct appdata *ad = data;
	retm_if(ad == NULL, "Invalid argument: appdata is NULL\n");

	if(ad->app_state == APP_STATE_PAUSE)
	{
		if (ad->win) {
			elm_win_activate(ad->win);
		}
		DBG("activate");
		ad->app_state = APP_STATE_RESUME;
		return;
	}
	ad->app_state = APP_STATE_RESUME;
}

static void app_control(app_control_h app_control, void *data)
{
	DBG("%s", __func__);
	struct appdata *ad = data;
	retm_if(ad == NULL, "Invalid argument: appdata is NULL\n");

	DBG("appstate %d",ad->app_state);
	if(ad->app_state==APP_STATE_PAUSE||ad->app_state==APP_STATE_RESUME)
	{
		if (ad->win) {
			DBG("activate");
			elm_win_activate(ad->win);
			evas_object_show(ad->win);
		}
		ad->app_state = APP_STATE_SERVICE;
		return;
	}

	char* type = NULL;
	app_control_get_extra_data(app_control, "type", &type);

	DBG("TYPE %s",type);
	if(type!=NULL)
	{
		if(strcmp(type,"setting")==0)
		{
			ad->app_type = APP_TYPE_SETTING;
		}
		else if(strcmp(type,"verify")==0)
		{
			ad->app_type = APP_TYPE_VERIFY;
			DBG("VERIFY APP");
		}
		else
		{
			ad->app_type = APP_TYPE_SETTING;
		}
	}
	else
	{
		ad->app_type = APP_TYPE_SETTING;
	}

	app_control_clone(&(ad->source_app_control), app_control);

	if(_app_create(ad) != 0){
		ERR("_app_create() if failed\n");
		goto END;
	}

	ad->app_state = APP_STATE_SERVICE;

	if (ad->win) {
		elm_win_activate(ad->win);
	}

END:
	if(type!=NULL)
		free(type);
}

int main(int argc, char *argv[])
{
	struct appdata ad;

	app_event_callback_s event_callback;

	event_callback.create = app_create;
	event_callback.terminate = app_terminate;
	event_callback.pause = app_pause;
	event_callback.resume = app_resume;
	event_callback.app_control = app_control;
	event_callback.low_memory = NULL;
	event_callback.low_battery = NULL;
	event_callback.device_orientation = NULL;
	event_callback.language_changed = NULL;
	event_callback.region_format_changed = NULL;

	memset(&ad, 0x0, sizeof(struct appdata));

	DBG("start w-text-input-setting");

	return app_efl_main(&argc, &argv, &event_callback, &ad);
}
