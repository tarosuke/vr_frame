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


#if 0
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <syslog.h>
#include <signal.h>
#include <X11/Xatom.h>

#include <toolbox/gl/framebuffer.h>

#include "xWindow.h"
#include "xDisplay.h"



//Xephyrのコマンドラインオプション
const char* XGLWindow::xephyr[] = {
#if 0
	"Xephyr", "-screen 3840x2160", ":15", 0 };
#else
	"Xephyr", "-screen", "1024x480", ":15", 0 };
#endif


void XGLWindow::New(TB::GLX& g, App& a){
	const char** childParams(xephyr);
	FXW p;

	p.display = g.GetDisplay();

	//AppWidget生成
	AppWidget* const aw(new AppWidget(a));
	if(!aw){ return; };

	//子プロセス起動
	TB::Child* const c(new TB::Child(childParams));
	if(!c){
		//子プロセス起動失敗
		delete aw;
		return;
	}
	sleep(2);
	p.name = childParams[0];

	//X窓探索
	p.pidAtom = XInternAtom(p.display, "WM_CLASS", True);
	if(p.pidAtom == None){
		syslog(LOG_ERR, "Could not get Atom:_NET_WM_PID");
		kill((*c).pid, SIGKILL);
		delete aw;
		delete c;
		return;
	}
	Window w(FindXWindowByPID(p, DefaultRootWindow(p.display)));
	if(!w){
		//該当窓なし
		syslog(LOG_ERR, "No Window found:%s", childParams[0]);
		kill((*c).pid, SIGKILL);
		delete aw;
		delete c;
		return;
	}

	//窓サイズ取得
	XWindowAttributes attr;
	XGetWindowAttributes(p.display, w, &attr);

	//XGLWindow生成
	new XGLWindow(c, g, w, a, *aw, (unsigned)attr.width, (unsigned)attr.height);
}


XGLWindow::XGLWindow(
	TB::Child* child,
	TB::GLX& g,
	Window target,
	App& a,
	AppWidget& aw,
	unsigned width,
	unsigned height) :
	FramebufferWidget(width, height),
	frameWaiter(true),
	glx(g, target),
	child(child){
	RaiseThread();
}

void XGLWindow::CreatePack(unsigned width, unsigned height){
	pack = (CommandPack){
		(const wO::Message::Head){},
		parent: 0,
		attributes: 0,
		left: -(int)width / 2,
		top: -(int)height / 2,
		depth: 0,
		width: width,
		height: height,
	};
}


void XGLWindow::DrawContent(){
	//キャプチャを許可
	frameWaiter.Unlock();

	//ToDo:キャプチャすると上下が反転するので戻すようにする

	//本来のタスク
	CanvasWidget::DrawContent();
}

void XGLWindow::ThreadBody(){
	//最初の許可待ち(本体が初期化された後に動くことを補償するため)
	frameWaiter.Lock();

	//根窓用G?Xコンテキストを設定
	glx.MakeCurrent();
	glViewport(0, 0, pack.width, pack.height);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glReadBuffer(GL_FRONT);

	//キャプチャ処理
	for(;;){
		//許可待ち
		frameWaiter.Lock();

		//根窓をキャプチャ
		GL::ErrorCheck();
		BINDER b(*this);
		glCopyTexSubImage2D(
			GL_TEXTURE_2D,
			0,
			0,
			0,
			0,
			0,
			pack.width,
			pack.height);
		assert(GL::ErrorCheck());
	}
}



Window XGLWindow::FindXWindowByPID(FXW& p, Window w){
return w;
	//寝窓は検索対象外なので子窓のチェックからスタート
	Window r;
	Window parent;
	Window *c;
	unsigned  n;
	if(!XQueryTree(p.display, w, &r, &parent, &c, &n)){
		//エラーにつき終了
		syslog(LOG_ERR, "Failed:QueryTree");
		return 0;
	}
	if(!c){
		//子なし
		return 0;
	}
	for(unsigned i(0); i < n; ++i){
	    //c[i]のPIDを調べてp.pidと一致していたらその窓を返す
        Atom type;
        int format;
        unsigned long m;
        unsigned long remain;
        unsigned char*name(0);
        if(Success == XGetWindowProperty(
			p.display,
			c[i],
			p.pidAtom,
			0,
			(strlen(p.name) + 3) / 4,
			False,
			XA_STRING,
			&type,
			&format,
			&m,
			&remain,
			&name)){
			//取得したPIDのチェック
			if(name){
				if(!strcmp(p.name, (char*)name)){
					XFree(name);
					XFree(c);
					return c[i];
				}
				XFree(name);
			}

			//さらに子窓をチェック
			if(Window ww = FindXWindowByPID(p, c[i])){
				XFree(c);
				return ww;
			}
        }
	}
	XFree(c);

	//なかった
	return 0;
}


#if 0

void XGLWindow::OnMoved(int x, int y){
	CommandPack c = {
		head: {},
		id: 0,
		attributes: 0,
		left: x,
		top: y,
	};
	SetTargetTo(c);
}

void XGLWindow::OnResized(unsigned w, unsigned h){
	//TODO:Canvasのりサイズなど
// 	Resize(w, h);
// 	if(IsVisible()){
// 		AssignXTexture();
// 	}
}



unsigned XGLWindow::serial(0);

void XGLWindow::SendMouseButtonXEvent(
	int type,
	const MouseParams& p,
	unsigned buttons){
	auto pos(Position());
	XButtonEvent xe;
	xe.time = 0; //TODO:タイムスタンプを設定
	xe.serial = serial++;
	xe.button = buttons;
	xe.type = type;
	xe.window = xWindow;
	xe.display = xDisplay;
	xe.x = p.x;
	xe.y = p.y;
	xe.x_root = p.x + pos.X();
	xe.y_root = p.y + pos.Y();
	xe.root = RootWindow(xDisplay, 0);

	//Xへ転送
	XSendEvent(
		const_cast<Display*>(xDisplay),
		xWindow,
		false,
		0,
		(XEvent*)&xe);
}

void XGLWindow::OnMouseButton(const MouseParams& p){
	if(p.pressed){
		SendMouseButtonXEvent(ButtonPress, p, p.pressed);
	}
	if(p.released){
		SendMouseButtonXEvent(ButtonRelease, p, p.released);
	}
}

void XGLWindow::OnMouseEnter(const MouseParams& p){
	//TODO:XCrossingEventを発行する
}

void XGLWindow::OnMouseMove(const MouseParams& p){
	auto pos(Position());
	XMotionEvent xe;
	xe.time = 0;
	xe.serial = serial++;
	xe.type = MotionNotify;
	xe.window = xWindow;
	xe.display = xDisplay;
	xe.x = p.x;
	xe.y = p.y;
	xe.x_root = p.x + pos.X();
	xe.y_root = p.y + pos.Y();
	xe.root = RootWindow(xDisplay, 0);
	XSendEvent(
		xDisplay,
		xWindow,
		true,
		0,
		(XEvent*)&xe);
}

void XGLWindow::OnMouseLeave(const MouseParams& p){
	//TODO:XCrossingEventを発行する
}

void XGLWindow::OnKeyDown(unsigned keyCode, unsigned charCode){
	XKeyEvent xe;
	xe.type = KeyPress;
	xe.display = xDisplay;
	xe.window = xWindow;
	xe.state = 0;
//TODO:ステートを組み立てる
// 		(e.modifiers & EVENT::ShiftKey ? ShiftMask : 0) |
// 		(e.modifiers & EVENT::CtrlKey ? ControlMask : 0) |
// 		(e.modifiers & EVENT::AltKey ? Mod1Mask : 0);
	xe.keycode = keyCode;
	xe.send_event = 0;
	XSendEvent(
		xDisplay,
		xWindow,
		true,
		0,
		(XEvent*)&xe);
}
void XGLWindow::OnKeyUp(unsigned keyCode, unsigned charCode){
	XKeyEvent xe;
	xe.type = KeyRelease;
	xe.display = xDisplay;
	xe.window = xWindow;
	xe.state = 0;
//TODO:ステートを組み立てる
// 		(e.modifiers & EVENT::ShiftKey ? ShiftMask : 0) |
// 		(e.modifiers & EVENT::CtrlKey ? ControlMask : 0) |
// 		(e.modifiers & EVENT::AltKey ? Mod1Mask : 0);
	xe.keycode = keyCode;
	xe.send_event = 0;
	XSendEvent(
		xDisplay,
		xWindow,
		true,
		0,
		(XEvent*)&xe);
}
void XGLWindow::OnKeyRepeat(unsigned keyCode, unsigned charCode){
	XKeyEvent xe;
	xe.type = KeyPress;
	xe.display = xDisplay;
	xe.window = xWindow;
	xe.state = 0;
//TODO:ステートを組み立てる
// 		(e.modifiers & EVENT::ShiftKey ? ShiftMask : 0) |
// 		(e.modifiers & EVENT::CtrlKey ? ControlMask : 0) |
// 		(e.modifiers & EVENT::AltKey ? Mod1Mask : 0);
	xe.keycode = keyCode;
	xe.send_event = 0;
	XSendEvent(
		xDisplay,
		xWindow,
		true,
		0,
		(XEvent*)&xe);
}


#endif
#endif
