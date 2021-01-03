/** 抽象Core
 * ヘッドトラック込みの画面描画制御クラス
 * 描画など共通の処理はCoreで行うが、デバイス固有の処理は子クラスで行う

 * Copyright (C) 2016 tarosuke<webmaster@tarosuke.net>
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

#include <openvr/openvr.h>

#include <toolbox/gl/displayList.h>
#include <toolbox/factory/factory.h>
#include <toolbox/container/list.h>
#include <toolbox/complex/complex.h>
#include <toolbox/geometry/vector.h>
#include <toolbox/gl/framebuffer.h>
#include <toolbox/timestamp.h>
#include <toolbox/prefs.h>

#include <module.h>

#include "keyboard.h"
#include "mice.h"
#include "pose.h"
#include "../display/x/xDisplay.h"



namespace core{

	class Core : XDisplay{
		Core(const Core &);
		void operator=(const Core &);
	public:

		struct Exception{
			const char* message;
			vr::HmdError code;
		};

		Core();
		~Core();

		int Run();

		//終了
		static void Quit() { keep = false; };

		//モジュールの登録
		static void RegisterStickies(Module &m){
			stickModules.Insert(m); };
		static void RegisterGUIs(Module &m){
			guiModules.Insert(m); };
		static void RegisterExternals(Module &m){
			externalModules.Insert(m); };

	protected:

	private:
		struct Mat44{
			Mat44(const vr::HmdMatrix44_t&);
			Mat44(const vr::HmdMatrix34_t&);
			float body[16];
		};



		vr::IVRSystem& openVR;

		//描画対象物
		static TB::List<Module> stickModules;
		static TB::List<Module> guiModules;
		static TB::List<Module> externalModules;

		//各デバイスの位置
		static vr::TrackedDevicePose_t devicePoses[];

		//終了？
		static bool keep;

		//UID
		Keyboard keyboard;
		Mice mice;

		//フレームバッファ他
		TB::Framebuffer::Size renderSize;
		struct Eye{
			Eye(vr::IVRSystem&, vr::EVREye, TB::Framebuffer::Size&);
			const vr::EVREye side;
			TB::Framebuffer framebuffer;
			Mat44 projecionMatrix;
			Mat44 eye2HeadMatrix;
			vr::Texture_t fbFeature;
		}left, right;;

		//初期化サポート
		static vr::IVRSystem& GetOpenVR(); //失敗したら例外
		static TB::Framebuffer::Size GetRenderSize(vr::IVRSystem&);

		//周期処理
		static void Update();
		void Draw(Eye&);
		void UpdateView(Eye&);

		//設定
		static TB::Prefs<float> nearClip;
		static TB::Prefs<float> farClip;
	};


}