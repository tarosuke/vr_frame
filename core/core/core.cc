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
#include <sched.h>

#include <module.h>

#include "core.h"
#include "../widget/root.h"



namespace core{

	template<> FACTORY<Module> *FACTORY<Module>::start(0);

	TB::List<Module> Core::stickModules;
	TB::List<Module> Core::externalModules;
	TB::List<Module> Core::independentModules;
	TB::List<Module> Core::afterModules;
	TB::List<Module> Core::sceneryModules;

	bool Core::keep(true);

	// float Core::fov(90);
	// float Core::tanFov(tanf(fov *M_PI / 360)); //fovが更新されたらtanf(fov * M_PI / 360)で再計算
	float Core::backColor[3] = {1, 1, 1};

	long long Core::frameDuration;
	Core::Timestamp Core::timestamp;

	//
	// VRHMD探索、new
	//
	template<> FACTORY<Core> *FACTORY<Core>::start(0);
	Core *Core::New() noexcept(false){
		Core *const v(FACTORY<Core>::New());
		if (v){
			return v;
		}

		throw "no VRHMD found";
	}

	//
	// 構築子
	//
	Core::Core(XDisplay::Profile &profile) :
		XDisplay(profile){

		//周期時間計算
		frameDuration = 1000000000LL / profile.fps;

		//基本設定
		glEnable(GL_POLYGON_SMOOTH);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_TEXTURE_2D);
	}

	//
	// 主ループ
	//
	int Core::Run(){
		//各モジュール起動
		syslog(LOG_DEBUG, "start modules");
		FACTORY<Module>::New();

		//初期タイムスタンプ取得
		TB::Timestamp::ns prev = TB::Timestamp();

		//周期処理
		for (int fillFlag(GL_COLOR_BUFFER_BIT); keep; XDisplay::Run()){
			//フレームタイム計測開始
			const TB::Timestamp::ns start = TB::Timestamp();
			const float d(1000000000.0 * (start - prev));
			const float delta(d < 1.0 /60 ? d : 1.0 / 60);
			prev = start;

			{
				//フレームバッファ有効化
				TB::Framebuffer::Key k(Framebuffer());

				//バッファのクリア
				glClearColor(backColor[0], backColor[1], backColor[2], 1);
				glClear(fillFlag | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
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

				//窓描画
				Root::SetView(GetDirection());
				double GUIView[16];
				glGetDoublev(GL_MODELVIEW_MATRIX, GUIView);
				Root::DrawAll();

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

			//アップデート(距離とか位置とか)
			for(TB::List<Module>::I i(stickModules); ++i;){
				(*i).Update(delta);
			}
			for(TB::List<Module>::I i(externalModules); ++i;){
				(*i).Update(delta);
			}
			for(TB::List<Module>::I i(independentModules); ++i;){
				(*i).Update(delta);
			}
			Root::UpdateAll();

			//描画後処理(スクリーンキャプチャなど)
			afterModules.Foreach(&core::Module::AfterDraw);

			//処理終了自国計測
			const TB::Timestamp::ns done = TB::Timestamp();
			timestamp.delta = start - timestamp.start;
			timestamp.start = start;
			timestamp.done = done - start;

			//フレームバッファスワップ
			XDisplay::Update();
		}
		return 0;
	}

	//モジュール登録
	void Module::RegisterStickies(){ Core::RegisterStickies(*this); }
	void Module::RegisterExternals(){ Core::RegisterExternals(*this); }
	void Module::RegisterIndependents(){ Core::RegisterIndependents(*this); }
	void Module::RegisterAfterDraw(){ Core::RegisterAfterDraw(*this); }
	void Module::RegisterScenery(){ Core::RegisterScenery(*this); }

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

	//スケジューラを設定
	sched_param attr = { sched_get_priority_max(SCHED_FIFO) };
	if(sched_setscheduler(0, SCHED_FIFO, &attr)){
		syslog(LOG_WARNING, "sched_setscheduler failed");
	}

	//本体
	try{
		//Core準備
		core::Core* const v(core::Core::New());
		if(!v){
			throw "Failed to start core";
		}

		//根Widget生成
		core::Root root;

		//Core起動
		(*v).Run();

		//終了
		delete v;
	}
	catch(const char* m){
		syslog(LOG_CRIT,"Fatal error: %s.", m);
		return -1;
	}
	catch(...){
		syslog(LOG_CRIT,"Unknown errer(uncaught exception)");
		return -1;
	}
	return 0;
}
