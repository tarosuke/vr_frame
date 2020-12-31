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

#include <toolbox/container/list.h>



namespace core{

	//
	// モジュールの原型
	//
	class Module : public TB::List<Module>::Node{
		Module(const Module&);
		void operator=(const Module&);
	public:
		virtual void Update(){};
		virtual void Draw(){};
		virtual void DrawTransparent(){};
		virtual void DrawLeft(){};
		virtual void DrawRight(){};
		virtual void AfterDraw(){};

		// vr_core全体を終了
		static void Quit();

		static float GetDelta(){ return 0.01; }; //TODO:計測した値を返すようにする

	protected:
		Module() : Node(true){};
		virtual ~Module(){};

		//自身をリストへ登録
		void RegisterStickies();
		void RegisterExternals();
		void RegisterIndependents();
		void RegisterAfterDraw();
		void RegisterX();
		void RegisterScenery();

	private:
		//NOTE: static Module* New()は0を返すこと
	};

	//
	// 顔面張り付き座標系モジュール
	//
	class StickModule : public Module{
	public:
		StickModule(){ RegisterStickies(); };
	};


	//
	// 自分ローカル座標系モジュール
	//
	class ExternalModule : public Module{
	public:
		ExternalModule(){ RegisterExternals(); };
	};

}
