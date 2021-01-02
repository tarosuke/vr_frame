/** X Display
 * Copyright (C) 2016,2019 tarosuke<webmaster@tarosuke.net>
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
#include "xDisplay.h"

#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <assert.h>

#include <X11/Xlib.h>

#include <toolbox/prefs.h>
#include <toolbox/gl/gl.h>
#include <toolbox/factory/factory.h>



namespace core{

	TB::Prefs<bool> XDisplay::companion("--companion", false, TB::CommonPrefs::nosave);
	int XDisplay::glxAttrs[] = {
		GLX_USE_GL,
		GLX_LEVEL, 0,
		GLX_RGBA,
		GLX_RED_SIZE, 8,
		GLX_GREEN_SIZE, 8,
		GLX_BLUE_SIZE, 8,
		GLX_ALPHA_SIZE, 8,
		GLX_DEPTH_SIZE, 24,
		GLX_STENCIL_SIZE, 8,
		GLX_ACCUM_RED_SIZE, 0,
		GLX_ACCUM_GREEN_SIZE, 0,
		GLX_ACCUM_BLUE_SIZE, 0,
		GLX_ACCUM_ALPHA_SIZE, 0,
		None
	};

	XDisplay::XD::XD() :
			display(XOpenDisplay(NULL)){
		if(!display){
			throw "画面を開けませんでした";
		}
	}

	XDisplay::XD::~XD(){
		XCloseDisplay(display);
	}

	XDisplay::XDisplay() :
			cWindow(OpenCompanion(display)),
			glx(display, cWindow, glxAttrs){
		//Xのスレッド対応設定
		XInitThreads();

		glx.MakeCurrent();

		//glew初期化
		if(GLEW_OK != glewInit()){
			throw "GLEWが使えません";
		}
	}

	XDisplay::~XDisplay(){
		if(!!cWindow){
			XDestroyWindow(display, cWindow);
		}
	}

	::Window XDisplay::OpenCompanion(::Display* display){
		//コンパニオン窓
		if((bool)companion){
			::Window c(XCreateSimpleWindow(display, XDefaultRootWindow(display), 0, 0, 480, 640, 0, 0, 0));
			if(!!c){
				XMapWindow(display, c);
			}
			return c;
		}
		return XDefaultRootWindow(display);
	}

	void XDisplay::DrawCompanion(TB::Texture& texture){
		if(!(bool)companion){
			return;
		}

		glViewport(0, 0, 480, 640);
		glDisable(GL_DEPTH_TEST);
		glColor3f(1,1,1);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		TB::Texture::Binder b(texture);
		glBegin(GL_TRIANGLE_STRIP);
		glTexCoord2f(0,0);
		glVertex3f(-1,-1,1);
		glTexCoord2f(0,1);
		glVertex3f(-1,1,-1);
		glTexCoord2f(1,0);
		glVertex3f(1,-1,-1);
		glTexCoord2f(1,1);
		glVertex3f(1,1,-1);
		glEnd();
		glFinish();
	}

	//エラーハンドラ
	int XDisplay::XErrorHandler(::Display* d, XErrorEvent* e){
		char description[256];
		XGetErrorText(d, (*e).error_code, description, 256);
		syslog(LOG_ERR, "X error %s(%u) serial:%lu reqCode:%u minCode:%u",
			description,
			(*e).error_code,
			(*e).serial,
			(*e).request_code,
			(*e).minor_code);
		return 0;
	}

}
