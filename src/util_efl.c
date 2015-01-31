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
#include <Ecore_X.h>

#include "main.h"
#include "log.h"

Evas_Object *_add_window(const char *name)
{
	Evas_Object *eo = NULL;
	int w = -1, h = -1;
	eo = elm_win_add(NULL, name, ELM_WIN_BASIC);
	if (eo) {
		elm_win_title_set(eo, name);
		elm_win_borderless_set(eo, EINA_TRUE);
		elm_win_alpha_set(eo, EINA_FALSE);
		ecore_x_window_size_get(ecore_x_window_root_first_get(), &w, &h);
		if(w == -1 || h == -1){
			ERR("ecore_x_window_seiz_get() is failed\n");
			return NULL;
		}
		evas_object_resize(eo, w, h);
		evas_object_show(eo);
	}
	return eo;
}

Evas_Object *_add_layout(Evas_Object *parent, const char *file,
			     const char *group)
{
	DBG("%s", __func__);
	Evas_Object *eo = NULL;
	int r = -1;

	retvm_if(parent == NULL, NULL, "Invalid argument: parent is NULL\n");
	retvm_if(file == NULL, NULL, "Invalid argument: file is NULL\n");
	retvm_if(group == NULL, NULL, "Invalid argument: group is NULL\n");

	eo = elm_layout_add(parent);
	retvm_if(eo == NULL, NULL, "Failed to add layout\n");

	r = elm_layout_file_set(eo, file, group);
	if (!r) {
		ERR("Failed to set file[%s]\n", file);
		evas_object_del(eo);
		return NULL;
	}

	evas_object_show(eo);
	return eo;
}
