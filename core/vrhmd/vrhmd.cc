/** Abstract VRHMD
 * Copyright (C) 2017 tarosuke<webmaster@tarosuke.net>
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

#include <toolbox/gl/gl.h>

#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/file.h>
#include <assert.h>
#include <linux/types.h>
#include <linux/input.h>
#include <linux/hidraw.h>
#include <syslog.h>

#include "vrhmd.h"



namespace core{

	/////ヘッドトラッカー関連
	const float VRHMD::nearDistance(0.1f);
	const float VRHMD::farDistance(10000.0f);



	void VRHMD::SensorThread(){
		//優先度設定
		pthread_setschedprio(
			sensorThread,
			sched_get_priority_max(SCHED_FIFO));

		for(;; pthread_testcancel()){
			UpdateSensor();
		}
	}
	void* VRHMD::_SensorThread(void* initialData){
		//オブジェクトを設定して監視開始
		(*(VRHMD*)initialData).SensorThread();
		return 0;
	}

	int VRHMD::OpenDeviceFile(const char* name){
		const int fd(open(name, O_RDWR));
		if(fd < 0){
			//開けなかった
			return fd;
		}

		if(flock(fd, LOCK_EX | LOCK_NB) < 0){
			//使用中
			close(fd);
			return -1;
		}

		//確保完了
		return fd;
	}
	int VRHMD::ScanDeviceFile(const int vid, const int pid, const char* baseName){
		//デバイスファイルを探索
		for(int i(0); i < 99; i++){
			char name[32];
			snprintf(name, 32, "/dev/%s%d", baseName, i);
			const int fd(open(name, O_RDWR));
			if(fd < 0){
				//開けなかった
				continue;
			}

			struct hidraw_devinfo info;
			if(ioctl(fd, HIDIOCGRAWINFO, &info) < 0){
				//ioctlできない
				close(fd);
				continue;
			}
			if(vid != info.vendor || pid != info.product){
				//探しているデバイスではない
				close(fd);
				continue;
			}

			if(flock(fd, LOCK_EX | LOCK_NB) < 0){
				//使用中
				close(fd);
				continue;
			}

			//確保完了
			return fd;
		}

		//なかった
		return -1;
	}


	//
	// シェーダーのソース
	//
	extern "C"{
		extern char _binary_core_vrhmd_deDistore_vert_glsl_start[];
		extern char _binary_core_vrhmd_landscape_frag_glsl_start[];
		extern char _binary_core_vrhmd_portrait_frag_glsl_start[];
		extern char _binary_core_vrhmd_deDistore_vert_glsl_end[];
		extern char _binary_core_vrhmd_landscape_frag_glsl_end[];
		extern char _binary_core_vrhmd_portrait_frag_glsl_end[];
	};


	//
	// 構築子
	//
	VRHMD::VRHMD(int fd, const Profile& hp) :
		Core(hp),
		width(hp.width),
		height(hp.height),
		params(hp),
		landscape(height <= width),
		fd(fd),
		framebuffer(width * hp.expandRatio, height * hp.expandRatio){
		//シェーダーコードの整形(0終端)
		_binary_core_vrhmd_landscape_frag_glsl_end[-1] =
		_binary_core_vrhmd_portrait_frag_glsl_end[-1] =
		_binary_core_vrhmd_deDistore_vert_glsl_end[-1] = 0;

		//シェーダーコードの設定
		const char* const vertexShaderSource(
			_binary_core_vrhmd_deDistore_vert_glsl_start);
		const char* const fragmentShaderSource(landscape ?
			_binary_core_vrhmd_landscape_frag_glsl_start :
			_binary_core_vrhmd_portrait_frag_glsl_start);


		//プログラマブルシェーダの設定
		GLuint vShader(glCreateShader(GL_VERTEX_SHADER));
		GLuint fShader(glCreateShader(GL_FRAGMENT_SHADER));

		if(!vShader || !fShader){
			throw "シェーダの確保に失敗";
		}

		//エラークリア
		GL::ErrorCheck();

		//シェーダをビルド
		GLint built(false);
		try{
			glShaderSource(vShader, 1, &vertexShaderSource, NULL);
			glCompileShader(vShader);
			glGetShaderiv(vShader, GL_COMPILE_STATUS, &built);
			if(built == GL_FALSE){
				throw vShader;
			}

			glShaderSource(fShader, 1, &fragmentShaderSource, NULL);
			glCompileShader(fShader);
			glGetShaderiv(fShader, GL_COMPILE_STATUS, &built);
			if(built == GL_FALSE){
				throw fShader;
			}
		}catch(GLuint shader){
			GLint charsWritten, infoLogLength;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
			char * infoLog(new char[infoLogLength]);
			glGetShaderInfoLog(shader, infoLogLength, &charsWritten, infoLog);
			syslog(LOG_CRIT, "%s", infoLog);
			delete[] infoLog;
			throw "シェーダのコンパイルに失敗";
		}
		try{
			deDistorShaderProgram = glCreateProgram();
			glAttachShader(deDistorShaderProgram, vShader);
			glAttachShader(deDistorShaderProgram, fShader);

			assert(GL::ErrorCheck());

			glLinkProgram(deDistorShaderProgram);
			glGetProgramiv(deDistorShaderProgram, GL_LINK_STATUS, &built);
			if(GL_FALSE == built){
				throw deDistorShaderProgram;
			}
		}catch(GLuint shader){
			GLint charsWritten, infoLogLength;
			glGetProgramiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
			char * infoLog(new char[infoLogLength]);
			glGetProgramInfoLog(shader, infoLogLength, &charsWritten, infoLog);
			syslog(LOG_CRIT, "%s", infoLog);
			delete[] infoLog;
			throw "シェーダのリンクに失敗";
		}

		assert(GL::ErrorCheck());

		glUseProgram(0);

		//歪み補正テクスチャ生成、登録
		NewDeDistoreCoords();

		//スケジューリングポリシーを設定
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_attr_setschedpolicy(&attr, SCHED_FIFO);

		//センサデータ取得開始
		pthread_create(&sensorThread, &attr, _SensorThread, (void*)this);
	}


	VRHMD::~VRHMD(){
		pthread_cancel(sensorThread);
		pthread_join(sensorThread, 0);
		close(fd);
	}


	/** 描画後処理
	 * 結局の所光学系の歪み除去
	 * 下のPrefsはテクスチャフレームバッファに対応してから使う
	 */
	void VRHMD::PostDraw(){
		//次パスのための準備
		glGetError();
		glViewport(0, 0, width, height);
		assert(GL::ErrorCheck());

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_STENCIL_TEST);
		assert(GL::ErrorCheck());

		//フレームバッファをテクスチャとして有効化
		TB::Texture::Binder k(framebuffer);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

		//フラグメントシェーダによる歪み除去
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, deDistorTexture);
		glActiveTexture(GL_TEXTURE0);

		//シェーダーにパラメタ設定
		assert(GL::ErrorCheck());
		glUseProgram(deDistorShaderProgram);
		assert(GL::ErrorCheck());
		glUniform1i(glGetUniformLocation(deDistorShaderProgram, "buffer"), 0);
		assert(GL::ErrorCheck());
		glUniform1i(glGetUniformLocation(deDistorShaderProgram, "de_distor"), 1);
		assert(GL::ErrorCheck());
		glUniform2f(
			glGetUniformLocation(deDistorShaderProgram, "chromatic"),
			params.redRatio, params.blueRatio);
		glUniform2f(
			glGetUniformLocation(deDistorShaderProgram, "center"),
			params.leftCenter, params.ild);
		glUniform1f(
			glGetUniformLocation(deDistorShaderProgram, "expandRatio"),
			params.expandRatio);
		assert(GL::ErrorCheck());

		//視野いっぱいにフレームバッファテクスチャを貼り付ける
		glBegin(GL_TRIANGLE_STRIP);
		glTexCoord2f(0, 0); glVertex3f(-1, -1, 0.5);
		glTexCoord2f(0, 1); glVertex3f(-1, 1, 0.5);
		glTexCoord2f(1, 0); glVertex3f(1, -1, 0.5);
		glTexCoord2f(1, 1); glVertex3f(1, 1, 0.5);
		glEnd();
		glUseProgram(0);
		assert(GL::ErrorCheck());

		glBindTexture(GL_TEXTURE_2D, 0);
	}

	/** 新規歪み補正テクスチャ
	 */
	void VRHMD::NewDeDistoreCoords(){
		BuildDeDistoreCoords();
	}

	/** 歪み補正テクスチャアップデート
	 */
	void VRHMD::UpdateDeDistoreCoords(){
	}

	//歪み補正済の中心からの距離の自乗
	float VRHMD::D2(float h, float v){
		const float dd(h*h + v*v);
		return 1.0f +
			params.d2 * dd +
			params.d4 * dd * dd +
			params.d6 * dd * dd * dd +
			params.d8 * dd * dd * dd * dd;
	}

	/** 歪み情報データを作る
	 */
	void VRHMD::BuildDeDistoreCoords(){
		DeDistoreElement* body(new DeDistoreElement[width * height]);
		assert(body);

		//レンズ中心(どちらも片目分の左端から)
		const float leftCenter(params.leftCenter);
		const float rightCenter(params.leftCenter + params.ild - 0.5f);

		//場合分けを減らすためにランドスケープとして計算
		const unsigned hh(landscape ? width / 2 : height / 2);
		const unsigned vv(landscape ? height : width);

		for(unsigned v(0); v < vv; ++v){
			for(unsigned h(0); h < hh; ++h){
				//中心からの距離計算(視野全体の幅を2.0に正規化)
				const float lhd((2.0f * h) / hh - (leftCenter * 4));
				const float rhd((2.0f * h) / hh - (rightCenter * 4));
				const float vd((2.0f * v) / vv - 1.0f);

				const float lrd(D2(lhd, vd));
				const float rrd(D2(rhd, vd));

				//テクスチャ座標算出(視野全体の幅を1.0に正規化／範囲外は思い切り範囲外にする)
				float lrh(lhd * lrd * 0.5f);
				float rrh(rhd * rrd * 0.5f);
				float lrv(vd * lrd *  0.5f);
				float rrv(vd * rrd *  0.5f);

				//格納
				if(landscape){
					DeDistoreElement& target(body[(v * width) + h]);
					DeDistoreElement& mirror(body[(v * width) + h + hh]);

					target.u = lrh * 0.5f;
					target.v = lrv;

					mirror.u = rrh * 0.5f;
					mirror.v = rrv;
				}else{
					DeDistoreElement& target(body[(h * width) + v]);
					DeDistoreElement& mirror(body[((h + hh) * width) + v]);

					target.u = lrv;
					target.v = lrh * 0.5f;

					mirror.u = rrv;
					mirror.v = rrh * 0.5f;
				}

			}
		}

		//テクスチャ登録
		glGenTextures(1, &deDistorTexture);
		glBindTexture(GL_TEXTURE_2D, deDistorTexture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		assert(GL::ErrorCheck());
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexImage2D(
			GL_TEXTURE_2D, 0, GL_RG16F,
			width, height, 0, GL_RG, GL_FLOAT, body);
		assert(GL::ErrorCheck());
		glBindTexture(GL_TEXTURE_2D, 0);

		//登録したので用済み
		delete body;
	}


	/** 姿勢管理
	 * NOTE:VRHMDは絶対角、絶対位置しか扱わないので相対データの場合はVRHMD_Relativeを使う
	 */
	//位置なし(なので位置を作って位置ありとして処理)
	void VRHMD::SetPose(const COMPLEX<4>& dir){
		const double origin[] = { 0.0, 0.15, -0.15 };
		VECTOR<3> pos(origin);
		pos.Rotate(dir);

		POSE::SetPose(dir, pos);
	}

}
