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

#include "rift.h"
#include <toolbox/prefs.h>



namespace vr_core{

	class RIFT_DK1 : public RIFT{
	private:
		static Profile profile;
		static FACTORY<Core> factory;
		static TB::Prefs<bool> enable;
		RIFT_DK1(int fd, const Profile& p) : RIFT(fd, p){};
		static Core* New(){
			if(!enable){
				//不許可状態
				return 0;
			}

			const int fd(OpenDeviceFile("/dev/RiftDK1"));
			if(0 <= fd){
				//確保完了
				syslog(LOG_INFO, "Oculus Rift DK1 found.\n");
				return new RIFT_DK1(fd, profile);
			}

			//なかった
			return 0;
		};
		//描画前設定
		void SetupLeftView() final{
			const float tf(GetTanFov() * nearDistance);
			const int hw(width / 2);
			const float ar((float)hw / height);

			//左目
			glViewport(
				0,
				0,
				hw * profile.expandRatio,
				height * profile.expandRatio);
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glFrustum(
				-ar * tf * profile.expandRatio ,
				ar * tf * profile.expandRatio,
				-tf * profile.expandRatio,
				tf * profile.expandRatio,
				nearDistance, farDistance);

			//Model-View行列初期化
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			glTranslatef(0.03, 0, 0);
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
			glTranslatef(-0.03, 0, 0);
		};
	};


	FACTORY<Core> RIFT_DK1::factory(New);
	TB::Prefs<bool> RIFT_DK1::enable("--RiftDK1", true);

	//TODO:Parametors have to tune: Center, d2, d4, d6 and red/blueRatio
	VRHMD::Profile RIFT_DK1::profile = {
		width:1280,
		height:800,
		leftCenter: 0.25f,
		ild: 0.5f,
		fps:60,
		d2:0.195f,
		d4:0.0125f,
		d6:0.0f,
		d8:0.0f,
		redRatio:0.984f,
		blueRatio:1.020f,
		expandRatio: 1.1f,
		accelerometer: true,
		displayName: "Rift DK1",
	};

}
