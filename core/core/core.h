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

#include <toolbox/gl/displayList.h>
#include <toolbox/factory/factory.h>
#include <toolbox/container/list.h>
#include <toolbox/complex/complex.h>
#include <toolbox/geometry/vector.h>
#include <toolbox/gl/framebuffer.h>
#include <toolbox/timestamp.h>

#include <module.h>

#include "keyboard.h"
#include "mice.h"
#include "pose.h"
#include "../display/x/xDisplay.h"



namespace vr_core{

	class Core : protected XDisplay, protected POSE{
		Core();
		Core(const Core &);
		void operator=(const Core &);
	public:
		//環境をチェックして適切なインスタンスを返す
		static Core *New() noexcept(false);

		int Run();

		virtual ~Core(){};

		//終了
		static void Quit() { keep = false; };

		//モジュールの登録
		static void RegisterStickies(Module &m){
			stickModules.Insert(m); };
		static void RegisterExternals(Module &m){
			externalModules.Insert(m); };
		static void RegisterIndependents(Module &m){
			independentModules.Insert(m); };
		static void RegisterAfterDraw(Module &m){
			afterModules.Insert(m); };
		static void RegisterScenery(Module& m){
			sceneryModules.Insert(m); };

		//getting value
		static float GetTanFov() { return tanFov; };

		//現在の「色」を取得
		static float *GetBackColor() { return backColor; };

		//タイムスタンプ
		struct Timestamp{
			TB::Timestamp::ns start;
			TB::Timestamp::ns delta;
			TB::Timestamp::ns done;
		};
		static const Timestamp &GetTimestamp() { return timestamp; };
		static long long GetFrameDuration() { return frameDuration; };

	protected:
		Core(const XDisplay::Profile &);

		//共通描画処理の前後に呼ばれるデバイス固有処理のハンドラ
		virtual void SetupLeftView() = 0;  //左目設定
		virtual void SetupRightView() = 0; //右目設定
		virtual void PostDraw(){};		   //描画後処理
		virtual TB::Framebuffer &Framebuffer() = 0;

		static unsigned durationFrames; //起動してからのフレーム数

	private:
		//描画内容
		GL::DisplayList displayList;

		//描画対象物
		static TB::List<Module> stickModules;
		static TB::List<Module> externalModules;
		static TB::List<Module> independentModules;
		static TB::List<Module> afterModules;
		static TB::List<Module> sceneryModules;

		//feature of display
		static float fov;
		static float tanFov;
		const unsigned fps;

		//終了？
		static bool keep;

		//周期処理ヘルパー
		static long long frameDuration; //ns

		//UID
		Keyboard keyboard;
		Mice mice;

		//時刻による色計算
		static float backColor[3];

		//タイムスタンプ
		static Timestamp timestamp;
	};


}