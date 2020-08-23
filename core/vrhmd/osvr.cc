/************************************************************************ OSVR
 * Copyright (C) -2020 tarosuke<webmaster@tarosuke.net>
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

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glx.h>

#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <syslog.h>
#include <unistd.h>
#include <sys/file.h>
#include <assert.h>
#include <linux/types.h>
#include <linux/input.h>
#include <linux/hidraw.h>

#include "vrhmd.h"
#include "../core/pose.h"
#include <toolbox/prefs.h>



namespace core{

	/** OSVR共通
	 */
	class OSVR : public VRHMD{
		OSVR();
		OSVR(const OSVR&);
		void operator=(const OSVR&);
	public:
		static Core* New();

	protected:
		//Coreとしてのインターフェイス
		OSVR(int f, Profile& p) : VRHMD(f, p), fd(f){}
		~OSVR(){};

	private:
		struct Packet{
			unsigned char version;
			unsigned char sequence;
			short i;
			short j;
			short k;
			short w;
			short rot[3];
		}__attribute__((packed));

		static FACTORY<Core> factory;
		const int fd;
		static COMPLEX<4> osvrAzimuth;

		// HID
		static const int VendorID = 0x1532;
		static const int ProductID = 0x0b00;
		static TB::Prefs<bool> enable;

		//姿勢読み取り
		void UpdateSensor() final{
			Packet r;
			const int rb(read(fd, &r, sizeof(r)));
			if(sizeof(r) == rb){
				COMPLEX<4> q(osvrAzimuth);
				q *= COMPLEX<4>(
					(double)r.w / 16384,
					(double)r.i / 16384,
					(double)-r.j / 16384,
					(double)-r.k / 16384);
				SetPose(q);
			}else{
				syslog(LOG_ERR, "wrong size:%d.\n", rb);
			}
		};
	};



	/** ランドスケープモード用
	 */
	class OSVR_LANDSCAPE : public OSVR{
	public:
		OSVR_LANDSCAPE(int f) : OSVR(f, profile){};
		~OSVR_LANDSCAPE(){};

	private:
		static Profile profile;

		//描画前設定
		void SetupLeftView() final{
			const int hw(width / 2);

			//左目
			glViewport(
				0,
				0,
				hw * profile.expandRatio,
				height * profile.expandRatio);
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glFrustum(
				-widthAtNear  * profile.expandRatio,
				widthAtNear * profile.expandRatio,
				-heightAtNear * profile.expandRatio,
				heightAtNear * profile.expandRatio,
				nearDistance, farDistance);

			//Model-View行列初期化
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			glTranslatef(-0.015, 0, 0);
		};
		void SetupRightView() final{
			const int hw(width / 2);
			glViewport(
				hw * profile.expandRatio,
				0,
				hw * profile.expandRatio,
				height * profile.expandRatio);

			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			glTranslatef(0.015, 0, 0);
		};
	};

	VRHMD::Profile OSVR_LANDSCAPE::profile = {
		width:{ "OSVRSLANDSCAPE/width", 1920 },
		height:{ "OSVRSLANDSCAPE/height", 1080 },
		leftCenter:{ "OSVRSLANDSCAPE/leftCenter", 0.25f },
		ild:{ "OSVRSLANDSCAPE/ild", 0.5f },
		fps:{ "OSVRSLANDSCAPE/fps", 60 },
		d2:{ "OSVRSLANDSCAPE/d2", 0.0f },
		d4:{ "OSVRSLANDSCAPE/d4", 0.0f },
		d6:{ "OSVRSLANDSCAPE/d6", 0.0f },
		d8:{ "OSVRSLANDSCAPE/d8", 0.0f },
		redRatio:{ "OSVRSLANDSCAPE/redRatio", 1.0f },
		blueRatio:{ "OSVRSLANDSCAPE/blueRatio", 1.0f },
		expandRatio:{ "OSVRSLANDSCAPE/expandRatio", 1.0f },
		accelerometer:{ "OSVRSLANDSCAPE/accelerometer", false },
		displayName: "OSVR_HDK",
		hFov: { "OSVRSLANDSCAPE/hFov", 90.0f },
		vFov: { "OSVRSLANDSCAPE/vFov", 90.0f },
	};



	/** ポートレートモード用
	 */
	class OSVR_PORTRAIT : public OSVR{
	public:
		OSVR_PORTRAIT(int f) : OSVR(f, profile){};
		~OSVR_PORTRAIT(){};

	private:
		static Profile profile;

		//描画前設定
		void SetupLeftView() final{
			const int hh(height / 2);

			//左目
			glViewport(0, hh, width, hh);
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glFrustum(
				-widthAtNear  * profile.expandRatio,
				widthAtNear * profile.expandRatio,
				-heightAtNear * profile.expandRatio,
				heightAtNear * profile.expandRatio,
				nearDistance, farDistance);

			//Model-View行列初期化
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			glRotatef(90,0,0,1);
			glTranslatef(0.015, 0, 0);
		};
		void SetupRightView() final{
			const int hh(height / 2);

			//右目
			glViewport(0, 0, width, hh);
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();

			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			glRotatef(90,0,0,1);
			glTranslatef(-0.015, 0, 0);
		};
	};

	VRHMD::Profile OSVR_PORTRAIT::profile = {
		width:{ "OSVRPORTLAIT/width", 1080 },
		height:{ "OSVRPORTLAIT/height", 1920 },
		leftCenter:{ "OSVRPORTLAIT/leftCenter", 0.25f },
		ild:{ "OSVRPORTLAIT/ild", 0.5f },
		fps:{ "OSVRPORTLAIT/fps", 60 },
		d2:{ "OSVRPORTLAIT/d2", -0.30f },
		d4:{ "OSVRPORTLAIT/d4", 1.48f },
		d6:{ "OSVRPORTLAIT/d6", -0.825f },
		d8:{ "OSVRPORTLAIT/d8", 0.0f },
		redRatio:{ "OSVRPORTLAIT/redRatio", 1.0f },
		blueRatio:{ "OSVRPORTLAIT/blueRatio", 1.0f },
		expandRatio:{ "OSVRPORTLAIT/expandRatio", 1.0f },
		accelerometer:{ "OSVRPORTLAIT/accelerometer", false },
		displayName: "OSVR_HDK",
		hFov: { "OSVRPORTLAIT/hFov", 90.0f },
		vFov: { "OSVRPORTLAIT/vFov", 90.0f },
	};



	/** OSVR共通
	 */

	FACTORY<Core> OSVR::factory(New);
	TB::Prefs<bool> OSVR::enable("--OSVR", true);

	// OSVRが出力する姿勢をwOCEで使っている「正面」に合わせるための値
	COMPLEX<4> OSVR::osvrAzimuth(0.7, 0.7, 0.0, 0.0);

	/** OSVRを検出してインスタンスを生成して返す
	 * ハードウェアを検出できなければ0を返す
	 */
	Core* OSVR::New(){
		if(!enable){
			//不許可状態
			return 0;
		}

		const int fd(OpenDeviceFile("/dev/OSVRraw"));
		if(0 <= fd){
			//「正面」の正規化
			osvrAzimuth.Normalize();

			for(unsigned t(0); t < 5; ++t){
				//データを一つ読む
				Packet p;
				const int rb(read(fd, &p, sizeof(p)));
				if(rb != sizeof(p)){
					//読めなかったので何かの間違い
					syslog(LOG_ERR, "OSVR HDK found but could not read.\n");
					close(fd);
					return 0;
				}

				//確保完了
				switch(p.version){
				case 0x13:
					//ランドスケープモード検出
					syslog(LOG_INFO, "OSVR HDK found ( landscape mode ).\n");
					return new OSVR_LANDSCAPE(fd);
				case 0x33:
					//ポートレートモード検出
					syslog(LOG_INFO, "OSVR HDK found ( portpait mode ).\n");
					return new OSVR_PORTRAIT(fd);
				default:
					if(p.version < 3){
						//旧版検出
						syslog(LOG_WARNING,
							"*OLD* OSVR HDK found ( Anyway, portpait mode activated ).\n");
						return new OSVR_PORTRAIT(fd);
					}else{
						syslog(LOG_WARNING, "It is may be OSVR-HDK, But got illigal response.\n");
					}
				}
			}
		}

		//開いていたら閉じる
		if(0 <= fd){
			close(fd);
		}

		//なかった
		return 0;
	}

}
