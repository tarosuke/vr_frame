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
			glx(display){
		//Xのスレッド対応設定
		XInitThreads();

		glx.MakeCurrent();

		//glew初期化
		if(GLEW_OK != glewInit()){
			throw "GLEWが使えません";
		}
	}

	XDisplay::~XDisplay(){}

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
