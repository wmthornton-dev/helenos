/*
 * Copyright (c) 2025 Wayne Michael Thornton (WMT) <wmthornton-dev@outlook.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * - The name of the author may not be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/** @addtogroup clock_widget
 * @{
 */
/**
 * @file Clock widget application (in UI)
 */

#ifndef CLOCK_WIDGET_H
#define CLOCK_WIDGET_H

#include <ui/fixed.h>
#include <ui/label.h>
#include <ui/pbutton.h>
#include <ui/ui.h>
#include <ui/window.h>
#include <ui/entry.h>

/** Clock widget UI application */
typedef struct {
	ui_t *ui;
	ui_window_t *window;
	ui_fixed_t *fixed;
	ui_label_t *date_label;
	ui_label_t *time_label;
	ui_entry_t *date_entry;
	ui_entry_t *time_entry;
	ui_pbutton_t *ok_button;
	ui_pbutton_t *set_button;
	struct tm current_time;
} clock_widget_t;

#endif

/** @}
 */
