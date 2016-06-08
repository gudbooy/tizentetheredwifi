/*
 * Wi-Fi
 *
 * Copyright 2012 Samsung Electronics Co., Ltd
 *
 * Licensed under the Flora License, Version 1.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.tizenopensource.org/license
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef __APPCORE_WRAPPER_H__
#define __APPCORE_WRAPPER_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <Evas.h>

Evas_Object* appcore_create_win(const char *name);
Evas_Object* appcore_load_edj(Evas_Object *parent, const char *file,const char *group);

#ifdef __cplusplus
}
#endif

#endif /* __APPCORE_WRAPPER_H__ */
