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

#ifndef __MAIN_H__
#define __MAIN_H__

#include <Elementary.h>
#include <appcore-efl.h>
#include <app.h>
#include <efl_assist.h>

#if !defined(PACKAGE)
#  define PACKAGE "w-text-input-setting"
#endif

#if !defined(RESDIR)
#  define RESDIR "/usr/apps/org.tizen.w-text-input-setting/res"
#endif

#if !defined(LOCALEDIR)
#  define LOCALEDIR RESDIR"/locale"
#endif

#if !defined(IMGDIR)
#  define IMGDIR RESDIR"/images"
#endif

#if !defined(PKGNAME)
#  define PKGNAME "org.tizen.w-text-input-setting"
#endif

#define S_(str) dgettext("sys_string", str)
#define T_(str) dgettext(PACKAGE, str)
#undef _
#define _(str) gettext(str)

#define _EDJ(x) elm_layout_edje_get(x)

#define EXPORT_PUBLIC __attribute__ ((visibility ("default")))

enum {
	APP_STATE_PAUSE = 1,
	APP_STATE_SERVICE,
	APP_STATE_RESUME,
	APP_STATE_TERMINATE,
};

enum {
	APP_TYPE_SETTING = 1,
	APP_TYPE_VERIFY,
};

struct appdata
{
	Evas_Object *win;
	Evas_Object *naviframe;
	Evas_Object *conform;

	Evas_Object *list;

	int app_state;
	int app_type;
	app_control_h source_app_control;
	int reply_result;
};

#endif /* __MAIN_H__ */

