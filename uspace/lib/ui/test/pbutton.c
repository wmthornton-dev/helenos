/*
 * Copyright (c) 2023 Jiri Svoboda
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

#include <gfx/context.h>
#include <gfx/coord.h>
#include <mem.h>
#include <pcut/pcut.h>
#include <stdbool.h>
#include <ui/control.h>
#include <ui/pbutton.h>
#include <ui/resource.h>
#include "../private/pbutton.h"
#include "../private/testgc.h"

PCUT_INIT;

PCUT_TEST_SUITE(pbutton);

static void test_pbutton_clicked(ui_pbutton_t *, void *);
static void test_pbutton_down(ui_pbutton_t *, void *);
static void test_pbutton_up(ui_pbutton_t *, void *);

static ui_pbutton_cb_t test_pbutton_cb = {
	.clicked = test_pbutton_clicked,
	.down = test_pbutton_down,
	.up = test_pbutton_up
};

static ui_pbutton_cb_t dummy_pbutton_cb = {
};

typedef struct {
	bool clicked;
	bool down;
	bool up;
} test_cb_resp_t;

/** Create and destroy button */
PCUT_TEST(create_destroy)
{
	ui_pbutton_t *pbutton = NULL;
	errno_t rc;

	rc = ui_pbutton_create(NULL, "Hello", &pbutton);
	PCUT_ASSERT_ERRNO_VAL(EOK, rc);
	PCUT_ASSERT_NOT_NULL(pbutton);

	ui_pbutton_destroy(pbutton);
}

/** ui_pbutton_destroy() can take NULL argument (no-op) */
PCUT_TEST(destroy_null)
{
	ui_pbutton_destroy(NULL);
}

/** ui_pbutton_ctl() returns control that has a working virtual destructor */
PCUT_TEST(ctl)
{
	ui_pbutton_t *pbutton;
	ui_control_t *control;
	errno_t rc;

	rc = ui_pbutton_create(NULL, "Hello", &pbutton);
	PCUT_ASSERT_ERRNO_VAL(EOK, rc);

	control = ui_pbutton_ctl(pbutton);
	PCUT_ASSERT_NOT_NULL(control);

	ui_control_destroy(control);
}

/** Set flags sets internal field */
PCUT_TEST(set_flags)
{
	ui_pbutton_t *pbutton;
	errno_t rc;

	rc = ui_pbutton_create(NULL, "Hello", &pbutton);
	PCUT_ASSERT_ERRNO_VAL(EOK, rc);

	ui_pbutton_set_flags(pbutton, ui_pbf_no_text_depress);
	PCUT_ASSERT_INT_EQUALS(ui_pbf_no_text_depress, pbutton->flags);

	ui_pbutton_destroy(pbutton);
}

/** Set button rectangle sets internal field */
PCUT_TEST(set_rect)
{
	ui_pbutton_t *pbutton;
	gfx_rect_t rect;
	errno_t rc;

	rc = ui_pbutton_create(NULL, "Hello", &pbutton);
	PCUT_ASSERT_ERRNO_VAL(EOK, rc);

	rect.p0.x = 1;
	rect.p0.y = 2;
	rect.p1.x = 3;
	rect.p1.y = 4;

	ui_pbutton_set_rect(pbutton, &rect);
	PCUT_ASSERT_INT_EQUALS(rect.p0.x, pbutton->rect.p0.x);
	PCUT_ASSERT_INT_EQUALS(rect.p0.y, pbutton->rect.p0.y);
	PCUT_ASSERT_INT_EQUALS(rect.p1.x, pbutton->rect.p1.x);
	PCUT_ASSERT_INT_EQUALS(rect.p1.y, pbutton->rect.p1.y);

	ui_pbutton_destroy(pbutton);
}

/** Set default flag sets internal field */
PCUT_TEST(set_default)
{
	ui_pbutton_t *pbutton;
	errno_t rc;

	rc = ui_pbutton_create(NULL, "Hello", &pbutton);
	PCUT_ASSERT_ERRNO_VAL(EOK, rc);

	ui_pbutton_set_default(pbutton, true);
	PCUT_ASSERT_TRUE(pbutton->isdefault);

	ui_pbutton_set_default(pbutton, false);
	PCUT_ASSERT_FALSE(pbutton->isdefault);

	ui_pbutton_destroy(pbutton);
}

/** Get light gets internal field */
PCUT_TEST(get_light)
{
	ui_pbutton_t *pbutton;
	errno_t rc;

	rc = ui_pbutton_create(NULL, "Hello", &pbutton);
	PCUT_ASSERT_ERRNO_VAL(EOK, rc);

	pbutton->light = true;
	PCUT_ASSERT_TRUE(ui_pbutton_get_light(pbutton));

	pbutton->light = false;
	PCUT_ASSERT_FALSE(ui_pbutton_get_light(pbutton));

	ui_pbutton_destroy(pbutton);
}

/** Set light sets internal field */
PCUT_TEST(set_light)
{
	ui_pbutton_t *pbutton;
	errno_t rc;

	rc = ui_pbutton_create(NULL, "Hello", &pbutton);
	PCUT_ASSERT_ERRNO_VAL(EOK, rc);

	ui_pbutton_set_light(pbutton, true);
	PCUT_ASSERT_TRUE(pbutton->light);

	ui_pbutton_set_light(pbutton, false);
	PCUT_ASSERT_FALSE(pbutton->light);

	ui_pbutton_destroy(pbutton);
}

/** Set caption sets internal field */
PCUT_TEST(set_caption)
{
	ui_pbutton_t *pbutton;
	errno_t rc;

	rc = ui_pbutton_create(NULL, "Hello", &pbutton);
	PCUT_ASSERT_ERRNO_VAL(EOK, rc);

	PCUT_ASSERT_STR_EQUALS("Hello", pbutton->caption);

	rc = ui_pbutton_set_caption(pbutton, "World");
	PCUT_ASSERT_ERRNO_VAL(EOK, rc);

	PCUT_ASSERT_STR_EQUALS("World", pbutton->caption);

	ui_pbutton_destroy(pbutton);
}

/** Paint button */
PCUT_TEST(paint)
{
	errno_t rc;
	gfx_context_t *gc = NULL;
	test_gc_t tgc;
	ui_resource_t *resource = NULL;
	ui_pbutton_t *pbutton;

	memset(&tgc, 0, sizeof(tgc));
	rc = gfx_context_new(&ops, &tgc, &gc);
	PCUT_ASSERT_ERRNO_VAL(EOK, rc);

	rc = ui_resource_create(gc, false, &resource);
	PCUT_ASSERT_ERRNO_VAL(EOK, rc);
	PCUT_ASSERT_NOT_NULL(resource);

	rc = ui_pbutton_create(resource, "Hello", &pbutton);
	PCUT_ASSERT_ERRNO_VAL(EOK, rc);

	rc = ui_pbutton_paint(pbutton);
	PCUT_ASSERT_ERRNO_VAL(EOK, rc);

	ui_pbutton_destroy(pbutton);
	ui_resource_destroy(resource);

	rc = gfx_context_delete(gc);
	PCUT_ASSERT_ERRNO_VAL(EOK, rc);
}

/** Test ui_pbutton_clicked() */
PCUT_TEST(clicked)
{
	errno_t rc;
	ui_pbutton_t *pbutton;
	test_cb_resp_t resp;

	rc = ui_pbutton_create(NULL, "Hello", &pbutton);
	PCUT_ASSERT_ERRNO_VAL(EOK, rc);

	/* Clicked with no callbacks set */
	ui_pbutton_clicked(pbutton);

	/* Clicked with callback not implementing clicked */
	ui_pbutton_set_cb(pbutton, &dummy_pbutton_cb, NULL);
	ui_pbutton_clicked(pbutton);

	/* Clicked with real callback set */
	resp.clicked = false;
	ui_pbutton_set_cb(pbutton, &test_pbutton_cb, &resp);
	ui_pbutton_clicked(pbutton);
	PCUT_ASSERT_TRUE(resp.clicked);

	ui_pbutton_destroy(pbutton);
}

/** Test ui_pbutton_down() */
PCUT_TEST(down)
{
	errno_t rc;
	ui_pbutton_t *pbutton;
	test_cb_resp_t resp;

	rc = ui_pbutton_create(NULL, "Hello", &pbutton);
	PCUT_ASSERT_ERRNO_VAL(EOK, rc);

	/* Down with no callbacks set */
	ui_pbutton_clicked(pbutton);

	/* Down with callback not implementing down */
	ui_pbutton_set_cb(pbutton, &dummy_pbutton_cb, NULL);
	ui_pbutton_down(pbutton);

	/* Down with real callback set */
	resp.down = false;
	ui_pbutton_set_cb(pbutton, &test_pbutton_cb, &resp);
	ui_pbutton_down(pbutton);
	PCUT_ASSERT_TRUE(resp.down);

	ui_pbutton_destroy(pbutton);
}

/** Test ui_pbutton_up() */
PCUT_TEST(up)
{
	errno_t rc;
	ui_pbutton_t *pbutton;
	test_cb_resp_t resp;

	rc = ui_pbutton_create(NULL, "Hello", &pbutton);
	PCUT_ASSERT_ERRNO_VAL(EOK, rc);

	/* Up with no callbacks set */
	ui_pbutton_clicked(pbutton);

	/* Up with callback not implementing up */
	ui_pbutton_set_cb(pbutton, &dummy_pbutton_cb, NULL);
	ui_pbutton_up(pbutton);

	/* Up with real callback set */
	resp.up = false;
	ui_pbutton_set_cb(pbutton, &test_pbutton_cb, &resp);
	ui_pbutton_up(pbutton);
	PCUT_ASSERT_TRUE(resp.up);

	ui_pbutton_destroy(pbutton);
}

/** Press and release button */
PCUT_TEST(press_release)
{
	errno_t rc;
	gfx_context_t *gc = NULL;
	test_gc_t tgc;
	ui_resource_t *resource = NULL;
	ui_pbutton_t *pbutton;
	test_cb_resp_t resp;

	memset(&tgc, 0, sizeof(tgc));
	rc = gfx_context_new(&ops, &tgc, &gc);
	PCUT_ASSERT_ERRNO_VAL(EOK, rc);

	rc = ui_resource_create(gc, false, &resource);
	PCUT_ASSERT_ERRNO_VAL(EOK, rc);
	PCUT_ASSERT_NOT_NULL(resource);

	rc = ui_pbutton_create(resource, "Hello", &pbutton);
	PCUT_ASSERT_ERRNO_VAL(EOK, rc);

	resp.clicked = false;
	resp.down = false;
	resp.up = false;
	ui_pbutton_set_cb(pbutton, &test_pbutton_cb, &resp);

	PCUT_ASSERT_FALSE(pbutton->held);
	PCUT_ASSERT_FALSE(pbutton->inside);

	ui_pbutton_press(pbutton);
	PCUT_ASSERT_TRUE(pbutton->held);
	PCUT_ASSERT_TRUE(pbutton->inside);
	PCUT_ASSERT_TRUE(resp.down);
	PCUT_ASSERT_FALSE(resp.up);
	PCUT_ASSERT_FALSE(resp.clicked);

	ui_pbutton_release(pbutton);
	PCUT_ASSERT_FALSE(pbutton->held);
	PCUT_ASSERT_TRUE(pbutton->inside);
	PCUT_ASSERT_TRUE(resp.up);
	PCUT_ASSERT_TRUE(resp.clicked);

	ui_pbutton_destroy(pbutton);
	ui_resource_destroy(resource);

	rc = gfx_context_delete(gc);
	PCUT_ASSERT_ERRNO_VAL(EOK, rc);
}

/** Press, leave and release button */
PCUT_TEST(press_leave_release)
{
	errno_t rc;
	gfx_context_t *gc = NULL;
	test_gc_t tgc;
	ui_resource_t *resource = NULL;
	ui_pbutton_t *pbutton;
	test_cb_resp_t resp;

	memset(&tgc, 0, sizeof(tgc));
	rc = gfx_context_new(&ops, &tgc, &gc);
	PCUT_ASSERT_ERRNO_VAL(EOK, rc);

	rc = ui_resource_create(gc, false, &resource);
	PCUT_ASSERT_ERRNO_VAL(EOK, rc);
	PCUT_ASSERT_NOT_NULL(resource);

	rc = ui_pbutton_create(resource, "Hello", &pbutton);
	PCUT_ASSERT_ERRNO_VAL(EOK, rc);

	resp.clicked = false;
	ui_pbutton_set_cb(pbutton, &test_pbutton_cb, &resp);

	PCUT_ASSERT_FALSE(pbutton->held);
	PCUT_ASSERT_FALSE(pbutton->inside);

	ui_pbutton_press(pbutton);
	PCUT_ASSERT_TRUE(pbutton->held);
	PCUT_ASSERT_TRUE(pbutton->inside);
	PCUT_ASSERT_FALSE(resp.clicked);

	ui_pbutton_leave(pbutton);
	PCUT_ASSERT_TRUE(pbutton->held);
	PCUT_ASSERT_FALSE(pbutton->inside);
	PCUT_ASSERT_FALSE(resp.clicked);

	ui_pbutton_release(pbutton);
	PCUT_ASSERT_FALSE(pbutton->held);
	PCUT_ASSERT_FALSE(pbutton->inside);
	PCUT_ASSERT_FALSE(resp.clicked);

	ui_pbutton_destroy(pbutton);
	ui_resource_destroy(resource);

	rc = gfx_context_delete(gc);
	PCUT_ASSERT_ERRNO_VAL(EOK, rc);
}

/** Press, leave, enter and release button */
PCUT_TEST(press_leave_enter_release)
{
	errno_t rc;
	gfx_context_t *gc = NULL;
	test_gc_t tgc;
	ui_resource_t *resource = NULL;
	ui_pbutton_t *pbutton;
	test_cb_resp_t resp;

	memset(&tgc, 0, sizeof(tgc));
	rc = gfx_context_new(&ops, &tgc, &gc);
	PCUT_ASSERT_ERRNO_VAL(EOK, rc);

	rc = ui_resource_create(gc, false, &resource);
	PCUT_ASSERT_ERRNO_VAL(EOK, rc);
	PCUT_ASSERT_NOT_NULL(resource);

	rc = ui_pbutton_create(resource, "Hello", &pbutton);
	PCUT_ASSERT_ERRNO_VAL(EOK, rc);

	resp.clicked = false;
	ui_pbutton_set_cb(pbutton, &test_pbutton_cb, &resp);

	PCUT_ASSERT_FALSE(pbutton->held);
	PCUT_ASSERT_FALSE(pbutton->inside);

	ui_pbutton_press(pbutton);
	PCUT_ASSERT_TRUE(pbutton->held);
	PCUT_ASSERT_TRUE(pbutton->inside);
	PCUT_ASSERT_FALSE(resp.clicked);

	ui_pbutton_leave(pbutton);
	PCUT_ASSERT_TRUE(pbutton->held);
	PCUT_ASSERT_FALSE(pbutton->inside);
	PCUT_ASSERT_FALSE(resp.clicked);

	ui_pbutton_enter(pbutton);
	PCUT_ASSERT_TRUE(pbutton->held);
	PCUT_ASSERT_TRUE(pbutton->inside);
	PCUT_ASSERT_FALSE(resp.clicked);

	ui_pbutton_release(pbutton);
	PCUT_ASSERT_FALSE(pbutton->held);
	PCUT_ASSERT_TRUE(pbutton->inside);
	PCUT_ASSERT_TRUE(resp.clicked);

	ui_pbutton_destroy(pbutton);
	ui_resource_destroy(resource);

	rc = gfx_context_delete(gc);
	PCUT_ASSERT_ERRNO_VAL(EOK, rc);
}

/** ui_pos_event() correctly translates POS_PRESS/POS_RELEASE */
PCUT_TEST(pos_event_press_release)
{
	errno_t rc;
	gfx_context_t *gc = NULL;
	test_gc_t tgc;
	ui_resource_t *resource = NULL;
	ui_pbutton_t *pbutton;
	ui_evclaim_t claim;
	pos_event_t event;
	gfx_rect_t rect;

	memset(&tgc, 0, sizeof(tgc));
	rc = gfx_context_new(&ops, &tgc, &gc);
	PCUT_ASSERT_ERRNO_VAL(EOK, rc);

	rc = ui_resource_create(gc, false, &resource);
	PCUT_ASSERT_ERRNO_VAL(EOK, rc);
	PCUT_ASSERT_NOT_NULL(resource);

	rc = ui_pbutton_create(resource, "Hello", &pbutton);
	PCUT_ASSERT_ERRNO_VAL(EOK, rc);

	PCUT_ASSERT_FALSE(pbutton->held);

	rect.p0.x = 10;
	rect.p0.y = 20;
	rect.p1.x = 30;
	rect.p1.y = 40;
	ui_pbutton_set_rect(pbutton, &rect);

	/* Press outside is not claimed and does nothing */
	event.type = POS_PRESS;
	event.hpos = 9;
	event.vpos = 20;
	claim = ui_pbutton_pos_event(pbutton, &event);
	PCUT_ASSERT_FALSE(pbutton->held);
	PCUT_ASSERT_EQUALS(ui_unclaimed, claim);

	/* Press inside is claimed and depresses button */
	event.type = POS_PRESS;
	event.hpos = 10;
	event.vpos = 20;
	claim = ui_pbutton_pos_event(pbutton, &event);
	PCUT_ASSERT_TRUE(pbutton->held);
	PCUT_ASSERT_EQUALS(ui_claimed, claim);

	/* Release outside (or anywhere) is claimed and relases button */
	event.type = POS_RELEASE;
	event.hpos = 9;
	event.vpos = 20;
	claim = ui_pbutton_pos_event(pbutton, &event);
	PCUT_ASSERT_FALSE(pbutton->held);
	PCUT_ASSERT_EQUALS(ui_claimed, claim);

	ui_pbutton_destroy(pbutton);
	ui_resource_destroy(resource);

	rc = gfx_context_delete(gc);
	PCUT_ASSERT_ERRNO_VAL(EOK, rc);
}

/** ui_pos_event() correctly translates POS_UPDATE to enter/leave */
PCUT_TEST(pos_event_enter_leave)
{
	errno_t rc;
	gfx_context_t *gc = NULL;
	test_gc_t tgc;
	ui_resource_t *resource = NULL;
	ui_pbutton_t *pbutton;
	pos_event_t event;
	gfx_rect_t rect;

	memset(&tgc, 0, sizeof(tgc));
	rc = gfx_context_new(&ops, &tgc, &gc);
	PCUT_ASSERT_ERRNO_VAL(EOK, rc);

	rc = ui_resource_create(gc, false, &resource);
	PCUT_ASSERT_ERRNO_VAL(EOK, rc);
	PCUT_ASSERT_NOT_NULL(resource);

	rc = ui_pbutton_create(resource, "Hello", &pbutton);
	PCUT_ASSERT_ERRNO_VAL(EOK, rc);

	PCUT_ASSERT_FALSE(pbutton->inside);

	rect.p0.x = 10;
	rect.p0.y = 20;
	rect.p1.x = 30;
	rect.p1.y = 40;
	ui_pbutton_set_rect(pbutton, &rect);

	/* Moving outside does nothing */
	event.type = POS_UPDATE;
	event.hpos = 9;
	event.vpos = 20;
	ui_pbutton_pos_event(pbutton, &event);
	PCUT_ASSERT_FALSE(pbutton->inside);

	/* Moving inside sets inside flag */
	event.type = POS_UPDATE;
	event.hpos = 10;
	event.vpos = 20;
	ui_pbutton_pos_event(pbutton, &event);
	PCUT_ASSERT_TRUE(pbutton->inside);

	/* Moving outside clears inside flag */
	event.type = POS_UPDATE;
	event.hpos = 9;
	event.vpos = 20;
	ui_pbutton_pos_event(pbutton, &event);
	PCUT_ASSERT_FALSE(pbutton->inside);

	ui_pbutton_destroy(pbutton);
	ui_resource_destroy(resource);

	rc = gfx_context_delete(gc);
	PCUT_ASSERT_ERRNO_VAL(EOK, rc);
}

static void test_pbutton_clicked(ui_pbutton_t *pbutton, void *arg)
{
	test_cb_resp_t *resp = (test_cb_resp_t *) arg;

	resp->clicked = true;
}

static void test_pbutton_down(ui_pbutton_t *pbutton, void *arg)
{
	test_cb_resp_t *resp = (test_cb_resp_t *) arg;

	resp->down = true;
}

static void test_pbutton_up(ui_pbutton_t *pbutton, void *arg)
{
	test_cb_resp_t *resp = (test_cb_resp_t *) arg;

	resp->up = true;
}

PCUT_EXPORT(pbutton);
