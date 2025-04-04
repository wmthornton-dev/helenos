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

/** @addtogroup date_cfg
 * @{
 */
/** @file Date configuration application (in UI)
 */

#include <gfx/coord.h>
#include <stdio.h>
#include <str.h>
#include <ui/fixed.h>
#include <ui/label.h>
#include <ui/pbutton.h>
#include <ui/resource.h>
#include <ui/ui.h>
#include <ui/window.h>
#include "date_cfg.h"

static void wnd_close(ui_window_t *, void *);
static void ok_clicked(ui_pbutton_t *, void *);

static ui_window_cb_t window_cb = {
	.close = wnd_close
};

static ui_pbutton_cb_t button_cb = {
	.clicked = ok_clicked
};

/** Window close button was clicked.
 *
 * @param window Window
 * @param arg Argument (date_cfg)
 */
static void wnd_close(ui_window_t *window, void *arg)
{
	date_cfg_t *date_cfg = (date_cfg_t *) arg;
	ui_quit(date_cfg->ui);
}

/** OK button was clicked.
 *
 * @param button Button that was clicked
 * @param arg Argument (date_cfg)
 */
static void ok_clicked(ui_pbutton_t *button, void *arg)
{
	date_cfg_t *date_cfg = (date_cfg_t *) arg;
	ui_quit(date_cfg->ui);
}

/** Run Date Configuration on display server. */
static errno_t date_cfg(const char *display_spec)
{
	ui_t *ui = NULL;
	ui_wnd_params_t params;
	ui_window_t *window = NULL;
	date_cfg_t date_cfg;
	gfx_rect_t rect;
	ui_resource_t *ui_res;
	errno_t rc;

	rc = ui_create(display_spec, &ui);
	if (rc != EOK) {
		printf("Error creating UI on display %s.\n", display_spec);
		return rc;
	}

	ui_wnd_params_init(&params);
	params.caption = "Date Configuration";
	params.placement = ui_wnd_place_center;
	if (ui_is_textmode(ui)) {
		params.rect.p0.x = 0;
		params.rect.p0.y = 0;
		params.rect.p1.x = 45;
		params.rect.p1.y = 15;
	} else {
		params.rect.p0.x = 0;
		params.rect.p0.y = 0;
		params.rect.p1.x = 350; 
		params.rect.p1.y = 275;  
	}

	memset((void *) &date_cfg, 0, sizeof(date_cfg));
	date_cfg.ui = ui;

	rc = ui_window_create(ui, &params, &window);
	if (rc != EOK) {
		printf("Error creating window.\n");
		return rc;
	}

	ui_window_set_cb(window, &window_cb, (void *) &date_cfg);
	date_cfg.window = window;

	ui_res = ui_window_get_res(window);

	rc = ui_fixed_create(&date_cfg.fixed);
	if (rc != EOK) {
		printf("Error creating fixed layout.\n");
		return rc;
	}

	/* Create OK button */
	rc = ui_pbutton_create(ui_res, "OK", &date_cfg.ok_button);
	if (rc != EOK) {
		printf("Error creating OK button.\n");
		return rc;
	}

	ui_pbutton_set_cb(date_cfg.ok_button, &button_cb, (void *) &date_cfg);

	if (ui_is_textmode(ui)) {
		rect.p0.x = 17;
		rect.p0.y = 13;
		rect.p1.x = 28;
		rect.p1.y = 14;
	} else {
		rect.p0.x = 125;
		rect.p0.y = 235;
		rect.p1.x = 225;
		rect.p1.y = rect.p0.y + 28;
	}


	ui_pbutton_set_rect(date_cfg.ok_button, &rect);
	ui_pbutton_set_default(date_cfg.ok_button, true);  /* Set as default button */

	rc = ui_fixed_add(date_cfg.fixed, ui_pbutton_ctl(date_cfg.ok_button));
	if (rc != EOK) {
		printf("Error adding OK button to layout.\n");
		return rc;
	}

	ui_window_add(window, ui_fixed_ctl(date_cfg.fixed));

	rc = ui_window_paint(window);
	if (rc != EOK) {
		printf("Error painting window.\n");
		return rc;
	}

	ui_run(ui);

	ui_window_destroy(window);
	ui_destroy(ui);

	return EOK;
}

static void print_syntax(void)
{
	printf("Syntax: date_cfg [-d <display-spec>]\n");
}

int main(int argc, char *argv[])
{
	const char *display_spec = UI_ANY_DEFAULT;
	errno_t rc;
	int i;

	i = 1;
	while (i < argc && argv[i][0] == '-') {
		if (str_cmp(argv[i], "-d") == 0) {
			++i;
			if (i >= argc) {
				printf("Argument missing.\n");
				print_syntax();
				return 1;
			}

			display_spec = argv[i++];
		} else {
			printf("Invalid option '%s'.\n", argv[i]);
			print_syntax();
			return 1;
		}
	}

	if (i < argc) {
		print_syntax();
		return 1;
	}

	rc = date_cfg(display_spec);
	if (rc != EOK)
		return 1;

	return 0;
}

/** @}
 */
