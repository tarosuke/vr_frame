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

#pragma once

#include <xmodule.h>

#include <toolbox/gl/gl.h>
#include <toolbox/gl/glx.h>
#include <toolbox/container/list.h>
#include <toolbox/prefs.h>
#include <sys/types.h>

#include "xrandr.h"



namespace core{

	/** 表示用画面
	 */
	class XDisplay{
		XDisplay();
		XDisplay(const XDisplay&);
		void operator=(const XDisplay&);
	public:

		struct Profile{
			TB::Prefs<unsigned> width;
			TB::Prefs<unsigned> height;
			TB::Prefs<float> leftCenter; //左レンズ中心の画面に対する割合
			TB::Prefs<float> ild; //レンズ中心距離の画面に対する割合
			TB::Prefs<unsigned> fps;
			TB::Prefs<float> d2; //自乗項
			TB::Prefs<float> d4; //四乗項
			TB::Prefs<float> d6; //六乗項
			TB::Prefs<float> d8; //八乗項
			TB::Prefs<float> redRatio; //赤の緑に対する比
			TB::Prefs<float> blueRatio; //青の緑に対する比
			TB::Prefs<float> expandRatio; //歪み補正で端が見えてしまわないように拡大描画する程度
			TB::Prefs<bool> accelerometer; //ポジトラできるならtrue
			const char* displayName; //EDIDに書いてある画面名
			TB::Prefs<float> hFov; //水平視野角
			TB::Prefs<float> vFov; //垂直視野角
		};

		XDisplay(Profile& profile);
		~XDisplay();

		void Run();
		void Update();

		//画面の状況
		struct Spec{
			::Display* display;
			::Window root;
			::Window wODMroot;
			TB::GLX* rootGLX;
			int xOffset;
			int yOffset;
			unsigned width;
			unsigned height;
		};
		static const Spec& GetSpec(){ return spec; };

		static void RegisterModule(XModule&);

	protected:

		Profile& proflie;

	private:
		static TB::List<XModule> modules;

		static ::Display* display;
		static Window root;

		int pid; //DM動作時のX鯖
		XRandR xrr; //画面情報
		XRandR::Monitor* monitor;

		//ローカルX用
		static Spec spec;
		static int XErrorHandler(::Display*, XErrorEvent*);
	};

}
