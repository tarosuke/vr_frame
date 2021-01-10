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

#include <sys/types.h>

#include <toolbox/gl/gl.h>
#include <toolbox/gl/glx.h>
#include <toolbox/prefs.h>
#include <toolbox/gl/texture.h>



namespace core{

	/** 表示用画面
	 */
	class XDisplay{
		XDisplay(const XDisplay&);
		void operator=(const XDisplay&);
	public:
		XDisplay();
		~XDisplay();

		void Run();
		void Update();

	protected:

		void DrawCompanion(TB::Texture&);

	private:
		class XD{
		public:
			XD();
			~XD();

			operator ::Display*(){
				return display;
			};

		private:
			::Display* const display;
		};

		XD display;
		::Window cWindow;
		TB::GLX glx;

		//初期化ヘルパー
		static ::Display* OpenDisplay();


		//ローカルX用
		static int glxAttrs[];
		static TB::Prefs<bool> companion;
		static ::Window OpenCompanion(::Display*);
		static int XErrorHandler(::Display*, XErrorEvent*);
	};

}
