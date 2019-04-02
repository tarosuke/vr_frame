/** modules
 * Copyright (C) 2019 tarosuke<webmaster@tarosuke.net>
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

#include <X11/Xlib.h>

#include <toolbox/container/list.h>



namespace vr_core{

	//
	// Xモジュールの原型
	//
	class XModule : public TB::List<XModule>::Node{
		XModule(const XModule&);
		void operator=(const XModule&);
	public:
		virtual bool OnXButtonEvent(const XButtonEvent&){ return false; };
		virtual bool OnXKeyEvent(const XKeyEvent&){ return false; };
		virtual bool OnXEvent(const XEvent&){ return false; };

	protected:
		XModule() : Node(true){};
		virtual ~XModule(){};

		//自身をリストへ登録
		void RegisterStickies();
		void RegisterExternals();
		void RegisterIndependents();
		void RegisterAfterDraw();
		void RegisterX();

		// vr_core全体を終了
		static void Quit();

	private:
		//NOTE: static Module* New()は0を返すこと
	};


}
