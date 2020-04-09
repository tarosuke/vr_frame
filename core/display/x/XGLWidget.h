/* captured X root windows by XGL
 * Copyright (C) 2017 tarosuke<webmaster@tarosuke.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#pragma once

#if 0

#include <X11/Xlib.h>

#include <toolbox/gl/gl.h>
#include <toolbox/gl/glx.h>
#include <toolbox/thread/pthread.h>
#include <toolbox/lock/pthreadLock.h>
#include <toolbox/child.h>
#include <toolbox/geometry/vector.h>

#include "../window/framebufferWidget.h"
#include "../view/appWidget.h"



class App;
class XDisplay;



class XGLWindow : public FramebufferWidget, TB::PThread{
	XGLWindow();
	XGLWindow(const XGLWindow&);
	void operator=(const XGLWindow&);
public:

	static void New(TB::GLX& glxParent, App&);

	~XGLWindow(){
		delete child;
	};

private:

	CommandPack pack;
	Lock::PThreadLock frameWaiter;
	TB::GLX glx;
	TB::Child* const child;

	XGLWindow(
		TB::Child*,
		TB::GLX& glxParent,
		Window,
		App&,
		AppWidget&,
		unsigned width,
		unsigned height);

	void DrawContent() final;
	void CreatePack(unsigned width, unsigned height);
	void ThreadBody() final;

	struct FXW{
		Display* display;
		Atom pidAtom;
		const char* name;
	};
	static Window FindXWindowByPID(FXW&, Window);

	static const char* xephyr[];
};





















#if 0
	//X用イベントハンドラ
	static void AtXCreate(XDisplay&, const XCreateWindowEvent&);
	static void AtXCreate(XDisplay&, Display* const, Window);


private:

	XDisplay& parent;
	Display* const xDisplay;
	Window xWindow;
	Damage dID; //xDamageID(更新情報取得に使う)
	static unsigned serial;

	XGLWindow(XDisplay&, Display*, Window, const CommandPack&);
	~XGLWindow();

	//ポインターイベント
	void OnMouseEnter(const MouseParams&)final;
	void OnMouseMove(const MouseParams&)final;
	void OnMouseLeave(const MouseParams&)final;
	void OnMouseButton(const MouseParams&)final;
	//キーイベント
	void OnKeyDown(unsigned keyCode, unsigned charCode)final;
	void OnKeyRepeat(unsigned keyCode, unsigned charCode)final;
	void OnKeyUp(unsigned keyCode, unsigned charCode)final;
	//無視するハンドラ
	void OnSightEnter(int x, int y)final{};
	void OnSightMove(int x, int y)final{};
	void OnSightLeave(int x, int y)final{};
	void OnScroll(int x, int y)final{};
	void OnGrabStart(const MouseParams&)final{};
	void OnGrabMove(const MouseParams&)final{};
	void OnGrabEnd(const MouseParams&)final{};

	//X用イベント
	void OnMoved(int, int);
	void OnResized(unsigned, unsigned)final;
	void OnUnfocused()final{};


	static CommandPack GetCreateCommandPack(Window);

	void SendMouseButtonXEvent(int type, const MouseParams&, unsigned);
};

#endif


#if 0
class XDISPLAY;
class XWINDOW : public WINDOW{
	XWINDOW();
public:
	XWINDOW(float x, float y, int w, int h, Window, Display*);

	//X用イベントハンドラ
	static void AtXCreate(const XCreateWindowEvent&, unsigned, unsigned);
	static void AtXCreate(Display* const, Window, unsigned, unsigned);
	static void AtXDestroy(const XDestroyWindowEvent&);
	static void AtXMap(const XMapEvent&);
	static void AtXUnmap(const XUnmapEvent&);
	static void AtXDamage(const XEvent&);
	static void AtXConfigure(const XConfigureEvent&);

private:
	~XWINDOW(){}; //Xのメッセージに追従し、外部からは呼ばれないのでprivate

	//X関連
	Display* const display;
	const Window wID; //窓ID
	Damage dID; //xDamageID

	//X側の位置メモ
	int vx;
	int vy;
	int xCenter;
	int yCenter;

	//窓全体関連
	static TOOLBOX::QUEUE<XWINDOW> xWindowList; //X窓リスト
	static XWINDOW* FindWindowByID(const Display*, Window);

	//単体窓関連
	TOOLBOX::NODE<XWINDOW> xNode; //X窓リストのノード

	void AssignXTexture();

	//子窓探索
	void SendEvent(Window, XButtonEvent&);

	//内部イベントハンドラ
	void OnMoved(int x, int y); //X側で窓が移動した
	void OnResized(unsigned w, unsigned h); //X側でリサイズされた

	//窓固有のハンドラ
	void OnDamage(XDamageNotifyEvent&);
	void OnKeyDown(const KEY_EVENT&);
	void OnKeyUp(const KEY_EVENT&);
	void OnMouseDown(const MOUSE_EVENT&);
	void OnMouseUp(const MOUSE_EVENT&);
	void OnMouseEnter(const MOUSE_EVENT&);
	void OnMouseMove(const MOUSE_EVENT&);
	void OnMouseLeave(const MOUSE_EVENT&);
	void OnSightEnter(const SIGHT_EVENT&);
	void OnSightMove(const SIGHT_EVENT&);
	void OnSightLeave(const SIGHT_EVENT&);
};

#endif
#endif
