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
#define Uses_SCIM_PANEL_AGENT
#include <scim.h>
#include <isf_control.h>
#include <stdio.h>
#include "setting-text-input.h"
#include <glib.h>
#include <efl_assist.h>
#include <scim_config_base.h>
#include <scim_module.h>
#include <scim_setup_module_efl.h>
#include <vconf-internal-isf-keys.h>
#include <vconf.h>

#include "main.h"

using namespace scim;
using namespace std;
#define SETTING_PACKAGE             "w-text-input-setting"
#define SETTING_LOCALEDIR           "/usr/apps/org.tizen.w-text-input-setting/res/locale"
static std::vector<String>          _names;
static std::vector<String>          _uuids;
static std::vector<String>          _module_names;
static std::vector<String>          _langs;
static std::vector<TOOLBAR_MODE_T>  _modes;
static std::vector<String>          _sw_ise_list;
static std::vector<String>          _sw_uuid_list;

typedef enum {
    ISE_OPTION_MODULE_EXIST_SO = 0,
    ISE_OPTION_MODULE_EXIST_XML,
    ISE_OPTION_MODULE_NO_EXIST
} ISE_OPTION_MODULE_STATE;

enum {
    /* value must below 0 */
    LABEL_TYPE_TITLE_SEPARATOR = -2,
    LABEL_TYPE_OPTION = -1
};

static int                          _sw_ise_index = 0;
static Evas_Object                  *_sw_radio_grp = NULL;

static ConfigPointer                _config;
static HelperAgent                  _helper_agent;
static HelperInfo                   _helper_info("fd491a70-22f5-11e2-89f3-eb5999be869e", "ISF Setting", "", "", SCIM_HELPER_STAND_ALONE);
static SetupModule                  *_mdl = NULL;
static String                       _mdl_name;
static std::vector <String>         _setup_modules;
static Connection                   _reload_signal_connection;
static Evas_Object                  *_mainpage_glist = NULL;

struct view_data {
    Evas_Object     *layout_main;
    Evas_Object     *naviframe;
    Evas_Object     *opt_eo;
};
static struct view_data *_st_view_data;

static void update_mainpage_content();
static void add_keyboard_genlist();

static int get_all_ise_uuids()
{
    char **iselist = NULL;
    int count = isf_control_get_ise_list(&iselist);

    for (int i = 0; i < count; i++) {
        if (iselist[i]) {
            _uuids.push_back(String(iselist[i]));
            free(iselist[i]);
        }
    }
    if (iselist)
        free(iselist);

    return count;
}

static bool load_all_ise_vector()
{
    bool result = false;
    int count = get_all_ise_uuids();
    if (count <= 0) {
        TEXT_INPUT_ERROR("get_all_ise_uuids total count:%d", count);
        return false;
    }

    int i = 0;
    for (i = 0; i < count; i++) {
        char *name = NULL;
        char *language = NULL;
        char *module_name = NULL;
        ISE_TYPE_T type = HARDWARE_KEYBOARD_ISE;
        int   option    = 0;

        int ret = isf_control_get_ise_info_and_module_name(_uuids[i].c_str(),
                        &name,
                        &language,
                        &type,
                        &option,
                        &module_name);

        if (ret == 0) {
            TOOLBAR_MODE_T mode = TOOLBAR_KEYBOARD_MODE;
            if (type == SOFTWARE_KEYBOARD_ISE)
                mode = TOOLBAR_HELPER_MODE;
            else
                mode = TOOLBAR_KEYBOARD_MODE;

            _modes.push_back(mode);
            _module_names.push_back(module_name ? String(module_name) : String(""));
            _names.push_back(name ? String(name) : String(""));
            _langs.push_back(language ? String(language) : String(""));

            result = true;
        } else {
            TEXT_INPUT_ERROR("isf_control_get_ise_info_and_module_name FAILED");
        }

        if (name)
            free(name);
        if (language)
            free(language);
        if (module_name)
            free(module_name);
    }

    return result;
}

static void sort_sw_ise()
{
    std::sort(_sw_ise_list.begin(), _sw_ise_list.end());
    //decoupling name and UUID
    std::vector<String> sw_ise_list;
    for (unsigned int i = 0; i < _sw_ise_list.size() ; i++) {
        String str_name_uuid = _sw_ise_list[i];
        std::vector<String> vtmp;
        scim_split_string_list(vtmp, str_name_uuid, ':');
        sw_ise_list.push_back(vtmp[0]);//name
        _sw_uuid_list.push_back(vtmp[1]);//UUID
    }

    _sw_ise_list.clear();
    for (unsigned int j = 0; j < sw_ise_list.size(); j++)
        _sw_ise_list.push_back(sw_ise_list[j]);
}

static void sort_ise_list()
{
    std::vector<String> sw_uuid_list, hw_uuid_list;
    std::vector<String> sw_ise_list, hw_ise_list;

    unsigned int i = 0;
    for (i = 0; i < _uuids.size(); i++) {
        if (_modes[i] == TOOLBAR_HELPER_MODE){
            sw_uuid_list.push_back(_uuids[i]);
            sw_ise_list.push_back(_names[i]);
        } else {
            hw_uuid_list.push_back(_uuids[i]);
            hw_ise_list.push_back(_names[i]);
        }
    }

    // sort by alphabet of name
    //compose name and UUID .
    _sw_ise_list.clear();
    for (unsigned int i = 0 ; i < sw_uuid_list.size(); i++) {
        String name =  sw_ise_list [i];
        String uuid = sw_uuid_list[i];
        String str_name_uuid  = String(name) + String(":") + String(uuid);
        _sw_ise_list.push_back(str_name_uuid);
    }
    sort_sw_ise();
}

static void load_config_module(void)
{
    _config = ConfigBase::get(true, "socket");
    if (_config.null()) {
        std::cerr << "Create dummy config!!!\n";
        _config = new DummyConfig();
    }

    if (_config.null()) {
        std::cerr << "Can not create Config Object!\n";
    }
}

static bool load_ise_info()
{
    bool ret;
    load_config_module();

    ret = load_all_ise_vector();
    if (ret)
        sort_ise_list();

    scim_get_setup_module_list(_setup_modules);

    unsigned int i;
    for (i = 0; i < _sw_ise_list.size(); i++)
        LOGD("%s \n",_sw_ise_list[i].c_str());
    return ret;
}

static char *_gl_exp_sw_label_get(void *data, Evas_Object *obj, const char *part)
{
    int index = (int)(data);
    if (!strcmp(part, "elm.text")) {
        LOGD ("[%d] %s\n", index, _sw_ise_list[index].c_str());
        return strdup(_sw_ise_list[index].c_str());
    }
    return NULL;
}

static void sw_keyboard_radio_cb(void *data, Evas_Object *obj, void *event_info)
{
    int index = GPOINTER_TO_INT (data);
    elm_radio_value_set(_sw_radio_grp, index);
    _sw_ise_index = index;
}

static Evas_Object *_gl_exp_sw_icon_get(void *data, Evas_Object *obj, const char *part)
{
    int index = (int)(data);
    if (!strcmp(part, "elm.icon.1") || !strcmp(part, "elm.icon")) {
        Evas_Object *radio = elm_radio_add(obj);
        elm_object_style_set (radio, "list");
        elm_radio_state_value_set(radio, index);
        if (_sw_radio_grp == NULL)
            _sw_radio_grp = elm_radio_add(obj);
        elm_radio_group_add(radio, _sw_radio_grp);
        evas_object_show(radio);
        evas_object_smart_callback_add(radio, "changed", sw_keyboard_radio_cb, (void *) (index));
        if (_sw_ise_index == index) {
            elm_radio_value_set(_sw_radio_grp, _sw_ise_index);
        }
        return radio;
    }
    return NULL;
}

static void _gl_sw_ise_sel(void *data, Evas_Object *obj, void *event_info)
{
    Elm_Object_Item *item = (Elm_Object_Item *)event_info;
    if (!item)
        return;

    elm_genlist_item_selected_set(item, EINA_FALSE);

    _sw_ise_index = (int)(data);

    elm_exit();
}

static Evas_Object *create_naviframe_layout(Evas_Object* parent)
{
    Evas_Object *naviframe = elm_naviframe_add(parent);
    elm_naviframe_prev_btn_auto_pushed_set(naviframe, EINA_FALSE);
    ea_object_event_callback_add(naviframe, EA_CALLBACK_BACK, ea_naviframe_back_cb, NULL);
    elm_object_part_content_set(parent, "elm.swallow.content", naviframe);
    evas_object_show(naviframe);

    return naviframe;
}

static Eina_Bool back_cb(void *data, Elm_Object_Item *it)
{
    static bool in_exit = false;

    if (in_exit)
        return EINA_TRUE;
    in_exit = true;

    if (data == NULL)
        return EINA_TRUE;

    elm_exit();

    return EINA_TRUE;
}

static void _timeout_exit_cb(void *data, Evas_Object *obj, void *event_info)
{
    evas_object_del(obj);
    elm_exit();
}

static void show_popup()
{
    Evas_Object *popup = NULL;
    popup = elm_popup_add(_st_view_data->layout_main);
    evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    ea_object_event_callback_add(popup, EA_CALLBACK_BACK, _timeout_exit_cb, NULL);
    elm_object_text_set(popup, _("IDS_SR_HEADER_NOT_AVAILABLE_ABB"));
    elm_popup_timeout_set(popup, 3.0);
    evas_object_smart_callback_add(popup, "timeout", _timeout_exit_cb, NULL);
    evas_object_show(popup);
}

static void add_keyboard_genlist() {
    unsigned int i = 0;
    char *active_ise_uuid = NULL;

    if (_sw_ise_list.size() < 1) {
        show_popup();
        return;
    }

    if (_mainpage_glist == NULL) {
        _mainpage_glist = elm_genlist_add(_st_view_data->naviframe);
    }
    elm_genlist_block_count_set(_mainpage_glist, 14);
    elm_genlist_mode_set(_mainpage_glist, ELM_LIST_COMPRESS);
    elm_genlist_homogeneous_set(_mainpage_glist, EINA_TRUE);

    /* Item Class : Keyboards */
    Elm_Genlist_Item_Class *itc = elm_genlist_item_class_new();
    itc->item_style = "1text.1icon.1";
    itc->func.text_get = _gl_exp_sw_label_get;
    itc->func.content_get = _gl_exp_sw_icon_get;
    itc->func.del = NULL;

    isf_control_get_active_ise(&active_ise_uuid);

    /* keyboard list */
    for (i = 0; i < _sw_ise_list.size(); i++) {
        if (String(active_ise_uuid) == _sw_uuid_list[i]) {
            _sw_ise_index = i;
        }
        LOGD ("[%d] %s\n", i, _sw_ise_list[i].c_str());
        elm_genlist_item_append(_mainpage_glist,
            itc,
            (void *)(i),
            NULL,
            ELM_GENLIST_ITEM_NONE,
            _gl_sw_ise_sel,
            (void *)(i));
    }

    if (active_ise_uuid) {
        free(active_ise_uuid);
        active_ise_uuid = NULL;
    }

    elm_genlist_item_class_free(itc);
}

static void update_mainpage_content() {
    add_keyboard_genlist();
}

static void active_ise_changed_cb(keynode_t *key, void* data)
{
    const char *active_ise_uuid = vconf_keynode_get_str(key);
    if (!active_ise_uuid) return;

    LOGD("Active ISE uuid : %s\n", active_ise_uuid);

    for (unsigned int i = 0; i < _sw_ise_list.size(); i++) {
        if (String(active_ise_uuid) == _sw_uuid_list[i]) {
            _sw_ise_index = i;
            if (_sw_radio_grp)
                elm_radio_value_set(_sw_radio_grp, _sw_ise_index);

            LOGD ("active ise index : %d\n", _sw_ise_index);
            break;
        }
    }
}

#ifdef __cplusplus
extern "C"
{
#endif

    void initialize_keyboard(void *data)
    {
        bindtextdomain(SETTING_PACKAGE, SETTING_LOCALEDIR);
        bool loaded = load_ise_info();
        if (!loaded) {
            TEXT_INPUT_ERROR ("load_ise_info fail");
            return ;
        }

        vconf_notify_key_changed (VCONFKEY_ISF_ACTIVE_KEYBOARD_UUID, active_ise_changed_cb, NULL);
    }

    Evas_Object *create_keyboard_list(Evas_Object *parent)
    {
        _mainpage_glist = NULL;

        _st_view_data = (view_data*)calloc(1, sizeof(view_data));
        _st_view_data->layout_main = parent;
        if (NULL == _st_view_data->naviframe) {
            _st_view_data->naviframe = create_naviframe_layout(parent);
        }

        _mainpage_glist = elm_genlist_add(_st_view_data->naviframe);
        elm_genlist_block_count_set(_mainpage_glist, 14);
        elm_genlist_mode_set(_mainpage_glist, ELM_LIST_COMPRESS);

        update_mainpage_content();

        /* Add genlist to naviframe */
        Elm_Object_Item *nf_main_item = elm_naviframe_item_push(_st_view_data->naviframe,
                _("IDS_ST_MBODY_TEXT_INPUT_ABB"),
                NULL,
                NULL,
                _mainpage_glist,
                NULL);
        elm_naviframe_item_title_enabled_set(nf_main_item, EINA_FALSE, EINA_FALSE);
        elm_naviframe_item_pop_cb_set(nf_main_item, back_cb, _st_view_data);

        return _st_view_data->naviframe;
    }

    void finish_keyboard()
    {
        LOGD("finish_keyboard called\n");
        if (_sw_uuid_list.size() > 0) {
            isf_control_set_active_ise_by_uuid(_sw_uuid_list[_sw_ise_index].c_str());
            LOGD("active uuid: %s\n",_sw_uuid_list[_sw_ise_index].c_str());
        }

        _names.clear();
        _uuids.clear();
        _module_names.clear();
        _langs.clear();
        _modes.clear();
        _sw_ise_list.clear();
        _sw_uuid_list.clear();

        if (_mdl) {
            delete _mdl;
            _mdl = NULL;
        }

        if (!_config.null()) {
            _config->flush();
            _config.reset();
        }

        if (_st_view_data != NULL) {
            free(_st_view_data);
            _st_view_data = NULL;
        }
    }

#ifdef __cplusplus
}
#endif
