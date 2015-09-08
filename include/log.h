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

#ifndef __LOG_H__
#define __LOG_H__

#include <unistd.h>
#include <dlog.h>

#undef LOG_TAG
#define LOG_TAG "TEXT_INPUT_SETTING"
#define ERR(fmt, arg...) LOGE("[%s:%d] "fmt,__FUNCTION__,__LINE__,##arg)
#define DBG(fmt, arg...) LOGD("[%s:%d] "fmt,__FUNCTION__,__LINE__,##arg)
#define _SECURE_E(fmt, arg...) SECURE_LOGE("[%s:%d] "fmt,__FUNCTION__,__LINE__,##arg)
#define _SECURE_D(fmt, arg...) SECURE_LOGD("[%s:%d] "fmt,__FUNCTION__,__LINE__,##arg)
#define SECURE_ERR(str, args...)	SECURE_LOGE("%s[%d]\t " #str "\n", \
					__func__, __LINE__, ##args)
#define SECURE_DBG(str, args...)	SECURE_LOGD("%s[%d]\t " #str "\n", \
					__func__, __LINE__, ##args)
#define SECURE_INFO(str, args...)	SECURE_LOGI(#str"\n", ##args)

#define retvm_if_timer(timer, expr, val, fmt, arg...) do { \
	if(expr) { \
		ERR(fmt, ##arg); \
		ERR("(%s) -> %s() return", #expr, __FUNCTION__); \
		timer = NULL; \
		return (val); \
	} \
} while (0)

#define retvm_if(expr, val, fmt, arg...) do { \
	if(expr) { \
		ERR(fmt, ##arg); \
		ERR("(%s) -> %s() return", #expr, __FUNCTION__); \
		return (val); \
	} \
} while (0)

#define retv_if(expr, val) do { \
	if(expr) { \
		ERR("(%s) -> %s() return", #expr, __FUNCTION__); \
		return (val); \
	} \
} while (0)

#define retm_if(expr, fmt, arg...) do { \
	if(expr) { \
		ERR(fmt, ##arg); \
		ERR("(%s) -> %s() return", #expr, __FUNCTION__); \
		return; \
	} \
} while (0)

#define ret_if(expr) do { \
	if(expr) { \
		ERR("(%s) -> %s() return", #expr, __FUNCTION__); \
		return; \
	} \
} while (0)

#define retif(cond, ret, str, args...) do { \
		if (cond) { \
			ERR(str, ##args);\
			return ret;\
		} \
} while (0);

#define goto_if(expr, val) do { \
	if(expr) { \
		ERR("(%s) -> goto", #expr); \
		goto val; \
	} \
} while (0)

#endif /* __LOG_H__ */
