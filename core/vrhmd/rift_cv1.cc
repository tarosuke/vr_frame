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



namespace core{

	class RIFT_CV1 : public RIFT{
	private:
		static Profile profile;
		static FACTORY<Core> factory;
		static TB::Prefs<bool> enable;
		RIFT_CV1(int fd) : RIFT(fd, profile){};
		~RIFT_CV1(){
			//画面無効化
			static const char disableFeaturesCommand[4] ={
				0x1d, 0, 0, 0 };
			ioctl(fd, HIDIOCSFEATURE(4), disableFeaturesCommand);
		}
		static Core* New(){
			if(!enable){
				//不許可状態
				return 0;
			}
			const int fd(OpenDeviceFile("/dev/RiftCV1"));
			if(0 <= fd){
				//XDisplayより先に画面有効化
				static const char enableFeaturesCommand[4] ={
					0x1d, 0, 0, 0x03 };
				ioctl(fd, HIDIOCSFEATURE(4), enableFeaturesCommand);
				sleep(1);

				//確保完了
				syslog(LOG_INFO, "Oculus Rift CV1 found.\n");
				return new RIFT_CV1(fd);
			}

			//なかった
			return 0;
		};
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
				-widthAtNear * profile.expandRatio,
				widthAtNear * profile.expandRatio,
				-heightAtNear * profile.expandRatio,
				heightAtNear * profile.expandRatio,
				nearDistance, farDistance);

			//Model-View行列初期化
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			glTranslatef(-0.03, 0, 0);
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
			glTranslatef(0.03, 0, 0);
		};
	};


	FACTORY<Core> RIFT_CV1::factory(New);
	TB::Prefs<bool> RIFT_CV1::enable("--RiftCV1", true);


	VRHMD::Profile RIFT_CV1::profile = {
		width:{ "Rift/width", 2160 },
		height:{ "Rift/height", 1200 },
		leftCenter:{ "Rift/leftCenter", 0.25f + 0.0125f },
		ild:{ "Rift/ild", 0.5f - 0.025f },
		fps:{ "Rift/fps", 90 },
		d2:{ "Rift/d2", 0.0735f },
		d4:{ "Rift/d4", 0.243f },
		d6:{ "Rift/d6", -0.1205f },
		d8:{ "Rift/d8", 0.102375f },
		redRatio:{ "Rift/redRatio", 0.995240f },
		blueRatio:{ "Rift/blueRatio", 1.0115f },
		expandRatio:{ "Rift/expandRatio", 1.5f },
		accelerometer:{ "Rift/accelerometer", true },
		displayName: "Rift",
		hFov: { "Rift/hFov", 90.0f },
		vFov: { "Rift/vFov", 90.0f },
	};

}
