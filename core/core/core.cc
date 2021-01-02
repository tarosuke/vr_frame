/** Core
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

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <syslog.h>

#include <module.h>

#include "core.h"
#include "../widget/root.h"



namespace core{

	//設定値
	TB::Prefs<float> Core::nearClip("--nearClip", 0.001);
	TB::Prefs<float> Core::farClip("--farClip", 10000.0);

	//全Moduleのスタック
	template<> FACTORY<Module> *FACTORY<Module>::start(0);

	//有効なモジュールのリスト
	TB::List<Module> Core::stickModules;
	TB::List<Module> Core::guiModules;
	TB::List<Module> Core::externalModules;

	//各デバイスの姿勢
	vr::TrackedDevicePose_t Core::devicePoses[vr::k_unMaxTrackedDeviceCount];

	//維持フラグ
	bool Core::keep(true);


	//メソッド

	void Core::Update(){
		stickModules.Foreach(&Module::Update);
		guiModules.Foreach(&Module::Update);
		externalModules.Foreach(&Module::Update);
	}

	void Core::Draw(Eye& eye){
		TB::Framebuffer::Key key(eye.framebuffer);
		glViewport(0, 0, renderSize.width, renderSize.height);

		glMatrixMode(GL_PROJECTION);
		glLoadMatrixf((GLfloat*)&eye.projecionMatrix);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glClearColor(0, 0, 1, 1);
		glClear(
			GL_COLOR_BUFFER_BIT |
			GL_DEPTH_BUFFER_BIT |
			GL_STENCIL_BUFFER_BIT);

		//HMD張り付き物体(HMD座標系)
		glDisable(GL_LIGHTING);
		stickModules.Foreach(&Module::Draw);

		//Widget(スライドHMD座標系)
		guiModules.Foreach(&Module::Draw);

		//通常の物体(絶対座標系)
		glEnable(GL_LIGHTING);
		externalModules.Foreach(&Module::Draw);
		externalModules.Foreach(&Module::DrawTransparent);
		glDisable(GL_LIGHTING);

		//Widget(透過)
		guiModules.Reveach(&Module::DrawTransparent);

		//画面張り付き(透過)
		stickModules.Reveach(&Module::DrawTransparent);
	}

	void Core::UpdateView(Eye& eye){
		vr::Texture_t tx = {
			(void*)(uintptr_t)eye.framebuffer.GetColorBufferID(),
			vr::TextureType_OpenGL,
			vr::ColorSpace_Gamma };
		vr::VRCompositor()->Submit(eye.side, &tx );
	}

	vr::IVRSystem& Core::GetOpenVR(){
		Exception exception = { "Failed to initialize OpenVR" };
		if(vr::IVRSystem* const o = vr::VR_Init(&exception.code, vr::VRApplication_Scene)){
			return *o;
		}
		throw exception;
	}

	TB::Framebuffer::Size Core::GetRenderSize(vr::IVRSystem& o){
		TB::Framebuffer::Size size;
		o.GetRecommendedRenderTargetSize(&size.width, &size.height);
		return size;
	}

	//
	//片目分
	//
	Core::Eye::Eye(vr::IVRSystem& hmd, vr::EVREye eye, TB::Framebuffer::Size& size) :
			side(eye),
			framebuffer(size),
			projecionMatrix(hmd.GetProjectionMatrix(
				side,
				(float)Core::nearClip,
				(float)Core::farClip)){}

	//
	// 構築子
	//
	Core::Core() :
			openVR(GetOpenVR()),
			renderSize(GetRenderSize(openVR)),
			left(openVR, vr::Eye_Left, renderSize),
			right(openVR, vr::Eye_Right, renderSize){
		//基本設定
		glEnable(GL_POLYGON_SMOOTH);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_TEXTURE_2D);
	}

	Core::~Core(){
		vr::VR_Shutdown();
	}

	//
	// 主ループ
	//
	int Core::Run(){
		//各モジュール起動
		syslog(LOG_DEBUG, "start modules");
		FACTORY<Module>::New();

		while(keep){

			//全デバイスの姿勢を取得
			vr::VRCompositor()->WaitGetPoses(
				devicePoses,
				vr::k_unMaxTrackedDeviceCount,
				NULL,
				0);

			//描画
			Draw(left);
			Draw(right);

			//各Moduleのアップデート
			Update();

			//視野更新
			UpdateView(left);
			UpdateView(right);
		}

		return 0;
	}

	//モジュール関連でCoreが見える必要があるメソッド
	StickModule::StickModule(){ Core::RegisterStickies(*this); }
	GUIModule::GUIModule(){ Core::RegisterGUIs(*this); }
	ExternalModule::ExternalModule(){ Core::RegisterExternals(*this); }
	float Module::GetDelta(){ return 0.01; }; //TODO:計測した値を返すようにする

	//終了
	void Module::Quit(){
		Core::Quit();
	}

}

//
//main
//
#ifndef PROJECT_NAME
#define PROJECT_NAME "jane_doe"
#endif

namespace{
	TB::Prefs<unsigned> logLevel("--verbose", 1, TB::CommonPrefs::nosave);
}

int main(int argc, const char *argv[]){
	//syslogを準備する
	static const int logLevels[] = { LOG_CRIT, LOG_INFO, LOG_DEBUG };
	if(2 < logLevel){ logLevel = 2; }
	openlog(0, LOG_CONS, LOG_SYSLOG);
	syslog(LOG_INFO, PROJECT_NAME);

	//設定ファイルのパスを作る
	TB::String prefsPath(".");
	prefsPath += PROJECT_NAME;

	//設定ファイル読み込み
	TB::CommonPrefs::Keeper prefs(prefsPath);

	//コマンドラインオプション解釈
	for(int n(1); n < argc; ++n){
		const char* const arg(argv[n]);

		//コマンドラインオプションの解釈
		if(!TB::CommonPrefs::Set(arg)){
			syslog(LOG_CRIT, "Unknown option: %s", arg);
			return -1;
		}
	}

	//コマンドラインオプションに従ってログレベルを設定
	const unsigned logMask(LOG_UPTO(logLevels[logLevel]));
	setlogmask(logMask);

	//本体
	try{
		//Core準備
		static core::Core vrCore;

		//Core起動
		vrCore.Run();
	}
	catch(const char* m){
		syslog(LOG_CRIT,"Fatal error: %s.", m);
		return -1;
	}
	catch(core::Core::Exception& e){
		syslog(LOG_CRIT,"%s(%s)",
			e.message,
			vr::VR_GetVRInitErrorAsEnglishDescription(e.code));
		return -1;
	}
	catch(...){
		syslog(LOG_CRIT,"Unknown errer(uncaught exception)");
		return -1;
	}

	syslog(LOG_INFO, "quit.");
	vr::VR_Shutdown();
	return 0;
}
