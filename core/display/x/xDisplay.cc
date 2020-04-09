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

	static TB::Prefs<unsigned> xOffset("--XOffset", 0);
	static TB::Prefs<unsigned> yOffset("--YOffset", 0);
	static TB::Prefs<bool> grab("+grab", false, TB::CommonPrefs::nosave);
	static TB::Prefs<bool> captureRoot("+X", true);

	XDisplay::Spec XDisplay::spec;
	TB::List<XModule> XDisplay::modules;


	XDisplay::XDisplay(const Profile& profile) : pid(-1), monitor(0){
		//Xのスレッド対応設定
		XInitThreads();

		//幅と高さを記録
		spec.width = profile.width;
		spec.height = profile.height;

		//displayの準備
		if(!(display = XOpenDisplay(NULL))){
			//X鯖を起動(DM動作)
			pid = fork();
			if(!pid){
				//X鯖起動
				execlp("Xorg", "X",
					"-background none",
					":0",
					"vt01",
					"-nolisten",
					"tcp",
					"-novtswitch",
					"-nocursor", 0);
				syslog(LOG_CRIT, "Failed to exec X server.");
				_exit(-1);
			}else if(pid < 0){
				//forkもできない状態であれば、どうしようもない
				syslog(LOG_CRIT, "Failed to fork to exec X server.");
				_exit(-1);
			}

			/** X鯖へ接続
			 * 接続未了でかつ子プロセスが起動しているなら1秒待って繰り返す
			 * とりあえず5回まで試行するが、子プロセスが終了してたら即失敗
			 */
			for(unsigned t(0); !display && t < 5; ++t, sleep(1)){
				//子プロセスチェック
				if(waitpid(pid, 0, WNOHANG)){
					pid = -1;
					break;
				}
				//画面を開いてみる
				if(!!(display = XOpenDisplay(NULL))){
					//X鯖へ接続できたのでルート窓を取得して完了
					root = DefaultRootWindow(display);
				}
			}

			//試験モードではないのでマウスをグラブに設定
			grab = true;
		}else{
			//描画先の窓を準備(テストモード)
			XSetWindowAttributes attr;
			attr.override_redirect = true;
			attr.event_mask = KeyPressMask;

			syslog(LOG_DEBUG,
				"wODM windowoffset:%u, %u.",
				(unsigned)xOffset,
				(unsigned)yOffset);

			root = XCreateWindow(
				display,DefaultRootWindow(display),
				(int)xOffset, (int)yOffset, profile.width, profile.height, 0,
				CopyFromParent, InputOutput, CopyFromParent,
				CWOverrideRedirect | CWEventMask, &attr);
			XMapWindow(display, root);
		}

		//画面が開かれているのを確認
		if(!display){
			syslog(LOG_CRIT, "Failed to connect X server.");
			_exit(-1);
		}

		spec.xOffset = xOffset;
		spec.yOffset = yOffset;
		spec.display = display;
		spec.root = DefaultRootWindow(display);
		spec.wODMroot = root;

		//エラーハンドラを設定
		XSetErrorHandler(XErrorHandler);

		//マウスのグラブ
		if(grab){
			if(GrabSuccess != XGrabPointer(
				display,
				root,
				false,
				0,
				GrabModeAsync,
				GrabModeAsync,
				root,
				None,
				CurrentTime)){
				syslog(LOG_ERR, "XGrabPointer failed.");
			}
		}

		/** GLXコンテキスト設定
		 */
		spec.rootGLX = new TB::GLX(display, root);

		//この根窓の描画条件をカレントにする
		(*spec.rootGLX).MakeCurrent();

		//glew初期化
		if(GLEW_OK != glewInit()){
			throw "GLEWが使えません";
		}

		/** 画面探索、有効化
		 */
		if(!!(monitor = XRandR::GetMonitor(profile.displayName))){
			(*monitor).Enable(xOffset, yOffset);
		}else{
			syslog(LOG_ERR, "monitor:%s not found.", profile.displayName);
		}
	}

	::Display* XDisplay::display;
	Window XDisplay::root;


	XDisplay::~XDisplay(){
		if(spec.rootGLX){
			delete spec.rootGLX;
		}

		if(monitor){
			(*monitor).Disable();
		}

		XCloseDisplay(display);
		if(0 < pid){
			kill(pid, SIGKILL);
		}

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


	void XDisplay::Update(){
			//フレームバッファスワップ
		glXSwapBuffers(display, root);
	}

	void XDisplay::Run(){
		if(!display){ return; }

		while(XPending(display)){
			//Xのイベントを読む
			XEvent e;
			XNextEvent(display, &e);

			switch(e.type){
			case ButtonPress:
			case ButtonRelease:
				for(TB::List<XModule>::I i(modules); ++i;){
					if((*i).OnXButtonEvent(
						*reinterpret_cast<XButtonEvent*>(&e))){
						break;
					}
				}
				break;
			default:  //XModuleのどれかで処理(trueが返ったら終了)
				for(TB::List<XModule>::I i(modules); ++i;){
					if((*i).OnXEvent(e)){
						break;
					}
				}
			break;
			}
		}
	}


	//モジュール関連
	template<> FACTORY<XModule>* FACTORY<XModule>::start(0);

	void XDisplay::RegisterModule(XModule& module){
		modules.Add(module);
	}
	void XModule::RegisterX(){
		XDisplay::RegisterModule(*this);
	}

}
