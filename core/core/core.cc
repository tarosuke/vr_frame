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

	template<> FACTORY<Module> *FACTORY<Module>::start(0);

	TB::List<Module> Core::stickModules;
	TB::List<Module> Core::externalModules;

	vr::TrackedDevicePose_t Core::devicePoses[vr::k_unMaxTrackedDeviceCount];


	bool Core::keep(true);

	void Core::Update(){
		stickModules.Foreach(&Module::Update);
		externalModules.Foreach(&Module::Update);
		// Root::UpdateAll();
	}

	void Core::Draw(vr::EVREye eye, TB::Framebuffer& framebuffer){
		TB::Framebuffer::Key key(framebuffer);

		glClearColor(1, 1, 1, 1);
		glClear(
			GL_COLOR_BUFFER_BIT |
			GL_DEPTH_BUFFER_BIT |
			GL_STENCIL_BUFFER_BIT);

		//普通の物体
		stickModules.Foreach(&Module::Draw);
		externalModules.Foreach(&Module::Draw);

		//Widgetの描画
		Root::DrawAll();

		//透過な物体
		externalModules.Foreach(&Module::DrawTransparent);
		stickModules.Foreach(&Module::DrawTransparent);
	}

	void Core::UpdateView(){
		vr::Texture_t leftEyeTexture = {
			(void*)(uintptr_t)left.GetColorBufferID(),
			vr::TextureType_OpenGL,
			vr::ColorSpace_Gamma };
		vr::VRCompositor()->Submit(vr::Eye_Left, &leftEyeTexture );
		vr::Texture_t rightEyeTexture = {
			(void*)(uintptr_t)right.GetColorBufferID(),
			vr::TextureType_OpenGL,
			vr::ColorSpace_Gamma };
		vr::VRCompositor()->Submit(vr::Eye_Right, &rightEyeTexture );
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
	// 構築子
	//
	Core::Core() :
			openVR(GetOpenVR()),
			renderSize(GetRenderSize(openVR)),
			left(renderSize),
			right(renderSize){
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
			Draw(vr::Eye_Left, left);
			Draw(vr::Eye_Right, right);

			//アップデート(移動など)
			Update();

			//視野更新
			UpdateView();
		}



#if 0
		//周期処理
		for (int fillFlag(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); keep;){

			{
				//左
				TB::Framebuffer::Key k(left);

				//バッファのクリア
				glClearColor(0, 0, 0, 1);
				glClear(
					GL_COLOR_BUFFER_BIT |
					GL_DEPTH_BUFFER_BIT |
					GL_STENCIL_BUFFER_BIT);
				glEnable(GL_DEPTH_TEST);

				//左目設定
				SetupLeftView();

				//描画記録設定
				displayList.StartRecord(true);

				//色設定
				glColor3fv(backColor);

				//GUI向け設定
				glDisable(GL_LIGHTING);
				glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

				double stickView[16];
				glGetDoublev(GL_MODELVIEW_MATRIX, stickView);
				stickModules.Foreach(&core::Module::Draw);

#if 0
				//窓描画(取得方法を考えなければならないのでオミット中)
				Root::SetView(GetDirection());
				double GUIView[16];
				glGetDoublev(GL_MODELVIEW_MATRIX, GUIView);
				Root::DrawAll();
#endif

				//頭の向きと位置をModel-View行列に反映
				SetupGLPose();
				double worldView[16];
				glGetDoublev(GL_MODELVIEW_MATRIX, worldView);

				//非GUI向け設定
				glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

				//external(通常)を描画
				externalModules.Foreach(&core::Module::Draw);

				//背景を描画
				TB::List<Module>::Key sk(sceneryModules);
				if(Module* m = sceneryModules.Top(sk)){
					(*m).Draw();
					fillFlag = 0;
				}else{
					fillFlag = GL_COLOR_BUFFER_BIT;
				}

				//external(透過)を描画
				externalModules.Foreach(&core::Module::DrawTransparent);

				//透過GUI向け設定
				glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
				glColor4f(1, 1, 1, 1);
				glAlphaFunc(GL_GREATER, 0);
				glEnable(GL_ALPHA_TEST);

				//透過窓描画
				glLoadMatrixd(GUIView);
				Root::DrawTransparentAll(); //Widget(透過)

				//描画記録終了
				displayList.EndRecord();

				//independent(左)
				glLoadMatrixd(worldView);
				independentModules.Foreach(&core::Module::DrawRight);

				//stick(透過)
				glLoadMatrixd(stickView);
				stickModules.Foreach(&core::Module::DrawTransparent);
				glDisable(GL_ALPHA_TEST);

				//右目分描画
				SetupRightView();
				displayList.Playback();

				//indeyendent(右)
				glLoadMatrixd(worldView);
				independentModules.Foreach(&core::Module::DrawLeft);

				//stick(透過)
				glLoadMatrixd(stickView);
				stickModules.Foreach(&core::Module::DrawTransparent);
				glDisable(GL_ALPHA_TEST);
			}

			//描画後処理
			PostDraw();

			//HIDからの入力を取得
			const Widget::KeyEvent kev(keyboard.GetEvent());

			//入力を処理
			if(kev.type != Widget::KeyEvent::none){
				Root::OnKey(kev);
			}




			//描画



		}

#endif
		return 0;
	}

	//モジュール登録
	void Module::RegisterStickies(){ Core::RegisterStickies(*this); }
	void Module::RegisterExternals(){ Core::RegisterExternals(*this); }

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
