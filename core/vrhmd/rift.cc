#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glx.h>

#include <assert.h>
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

#include <stdio.h>

#include "rift.h"



namespace core{

	/////ヘッドトラッカー関連
	void* RIFT::KeepaliveThread(void* instance){
		//オブジェクトを設定して監視開始
		(*(RIFT*)instance).Keepalive();
		return 0;
	}

	void RIFT::Keepalive(){
		for(;;){
			static const char keepaliveCommand[5] ={
				8, 0, 0,
				(char)(keepaliveInterval & 0xff),
				(char)(keepaliveInterval >> 8)
			};
			ioctl(fd, HIDIOCSFEATURE(5), keepaliveCommand);
			usleep(keepaliveInterval * 500);
		}
	}

	void RIFT::DecodeSensor(const unsigned char* buff, int v[3]){
		struct {int x:21;} s;

		v[0] = s.x =
			((unsigned)buff[0] << 13) |
			((unsigned)buff[1] << 5) |
			((buff[2] & 0xfb) >> 3);
		v[1] = s.x =
			(((unsigned)buff[2] & 0x07) << 18) |
			((unsigned)buff[3] << 10) |
			((unsigned)buff[4] << 2) |
			((buff[5] & 0xc0) >> 6);
		v[2] = s.x =
			(((unsigned)buff[5] & 0x3f) << 15) |
			((unsigned)buff[6] << 7) |
			(buff[7] >> 1);
	}

	void RIFT::Decode(const char* buff){
		struct{
			int accel[3];
			int rotate[3];
		}sample[3];
		int mag[3];

		unsigned timestamp;
		unsigned char numOfSamples;
		unsigned samples;
		short temp;

		if(buff[0] == 1){
			//フォーマット1
			numOfSamples = buff[1];
			timestamp = *(unsigned short*)&buff[2];

			samples = numOfSamples > 2 ? 3 : numOfSamples;
			for(unsigned char i(0); i < samples; i++){
				DecodeSensor((unsigned char*)buff + 8 + 16 * i, sample[i].accel);
				DecodeSensor((unsigned char*)buff + 16 + 16 * i, sample[i].rotate);
			}
			//磁気センサのデータ取得
			mag[0] = *(short*)&buff[56];
			mag[1] = *(short*)&buff[58];
			mag[2] = *(short*)&buff[60];

			//温度取得
			temp = *(short*)&buff[6];
		}else if(buff[0] == 11){
			//フォーマット11
			numOfSamples = buff[3];
			timestamp = *(unsigned*)&buff[4];

			samples = numOfSamples > 2 ? 2 : numOfSamples;
			for(unsigned char i(0); i < samples; i++){
				DecodeSensor((unsigned char*)buff + 12 + 16 * i, sample[i].accel);
				DecodeSensor((unsigned char*)buff + 20 + 16 * i, sample[i].rotate);
			}
			//磁気センサのデータ取得
			mag[0] = *(short*)&buff[42];
			mag[1] = *(short*)&buff[44];
			mag[2] = *(short*)&buff[48];

			//温度取得
			temp = *(short*)&buff[6];
		}else{
			syslog(LOG_CRIT, "unknowm Rift format:%d", buff[0]);
			return;
		}

		static unsigned short prevTime;
		const unsigned short deltaT(timestamp - prevTime);
		prevTime = timestamp;

		const double qtime(1.0/1000.0);
		const double dt(qtime * deltaT / numOfSamples);

		// 各サンプル値で状況を更新
		for(unsigned char i(0); i < samples; i++){
			const COMPLEX<4> angle(sample[i].rotate, 0.0001 * dt);
			const VECTOR<3> accel(sample[i].accel, 0.0001);
			UpdatePose(angle, accel, dt);
		}

		// 磁界値取得
		UpdateMagneticField((const VECTOR<3>)(mag));

		//温度取得
		UpdateTemperature(0.01 * temp);
	}


	void RIFT::UpdateSensor(){
		char buff[256];
		const int rb(read(fd, buff, 256));
		if(rb == 62 || rb == 64){
			Decode(buff);
			CorrectError();
		}else{
			syslog(LOG_WARNING, "%5d bytes dropped.\n", rb);
		}
	}

	RIFT::RIFT(int fd, const Profile& p) :
		VRHMD_Relative(fd, p),
		fd(fd){
		//スケジューリングポリシーを設定
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_attr_setschedpolicy(&attr, SCHED_FIFO);

		//Keepaliveスレッド開始
		pthread_create(&keepaliveThread, &attr, KeepaliveThread, (void*)this);


		//設定読み込み
		ReadScale();

		ReadConfig();
		//packet_intervalがデフォルトでなかったらデフォルト値を書き込む
		static const uint16_t defaultPacketInterval = 0xe800;
		static const uint8_t defaultFlag = 0x20;
		if(config.packet_interval != defaultPacketInterval || config.flags != defaultFlag){
			unsigned char buff[64];
			config.packet_interval = defaultPacketInterval;
			config.flags = defaultFlag;
			buff[0] = 2;
			buff[1] = 0;
			const int result(ioctl(fd, HIDIOCSFEATURE(sizeof(config)), buff));
			syslog(LOG_DEBUG, "rewrite packet_interval to default valie:%d", result);

			ReadConfig();
		}

		ReadInfo();
	}

	RIFT::~RIFT(){
		pthread_cancel(keepaliveThread);
		pthread_join(keepaliveThread, 0);
		close(fd);
	}

	//センサスケール取得コマンド発行
	void RIFT::ReadScale(){
		unsigned char buff[64];
		memset(buff, 0, sizeof(buff));
		buff[0] = 4;
		const int size(ioctl(fd, HIDIOCGFEATURE(8), buff));
		unsigned char* p(&buff[3]);
		Read(scale.accel, p);
		Read(scale.rotate, p);
		Read(scale.mag, p);
		syslog(LOG_DEBUG, "sensor scale:%d", size);
		syslog(LOG_DEBUG, "  accel scale:%d", scale.accel);
		syslog(LOG_DEBUG, "  rotate scale:%d", scale.rotate);
		syslog(LOG_DEBUG, "  mag scale:%d", scale.mag);
	}

	//センサの設定読み込み
	void RIFT::ReadConfig(){
		unsigned char buff[64];
		memset(buff, 0, sizeof(buff));
		buff[0] = 2;
		const int size(ioctl(fd, HIDIOCGFEATURE(sizeof(config)), buff));
		unsigned char* p(&buff[1]);
		Read(config.command_id, p);
		Read(config.flags, p);
		Read(config.packet_interval, p);
		Read(config.keep_alive_interval, p);
		syslog(LOG_DEBUG, "sensor config:%d", size);
		syslog(LOG_DEBUG, "  command id: %u", config.command_id);
		syslog(LOG_DEBUG, "  flags: %02x", config.flags);
		syslog(LOG_DEBUG, "    raw mode:　%d", !!(config.flags & RIFT_SCF_RAW_MODE));
		syslog(LOG_DEBUG, "    calibration test: %d", !!(config.flags & RIFT_SCF_CALIBRATION_TEST));
		syslog(LOG_DEBUG, "    use calibration:  %d", !!(config.flags & RIFT_SCF_USE_CALIBRATION));
		syslog(LOG_DEBUG, "    auto calibration: %d", !!(config.flags & RIFT_SCF_AUTO_CALIBRATION));
		syslog(LOG_DEBUG, "    motion keep alive: %d", !!(config.flags & RIFT_SCF_MOTION_KEEP_ALIVE));
		syslog(LOG_DEBUG, "    motion command keep alive: %d", !!(config.flags & RIFT_SCF_COMMAND_KEEP_ALIVE));
		syslog(LOG_DEBUG, "    sensor coordinates: %d", !!(config.flags & RIFT_SCF_SENSOR_COORDINATES));
		syslog(LOG_DEBUG, "  packet interval: %u", config.packet_interval);
		syslog(LOG_DEBUG, "  keep alive interval: %u", config.keep_alive_interval);
	}

	//画面情報読み込み
	void RIFT::ReadInfo(){
		unsigned char buff[64];
		struct{
			uint16_t command_id;
			uint8_t distortion_type;
			uint8_t distortion_type_opts;
			uint16_t h_resolution, v_resolution;
			int h_screen_size, v_screen_size;
			int v_center;
			int lens_separation;
			int eye_to_screen_distance[2];
			float distortion_k[6];
		}info;

		memset(buff, 0, sizeof(buff));
		buff[0] = 9;
		const int size(ioctl(fd, HIDIOCGFEATURE(56), buff));
		unsigned char* p(&buff[1]);
		Read(info.command_id, p);
		Read(info.distortion_type, p);
		Read(info.h_resolution, p);
		Read(info.v_resolution, p);
		Read(info.h_screen_size, p);
		Read(info.v_screen_size, p);
		Read(info.v_center, p);
		Read(info.lens_separation, p);
		Read(info.eye_to_screen_distance[0], p);
		Read(info.eye_to_screen_distance[1], p);

		info.distortion_type_opts = 0;

		for(int i = 0; i < 6; i++){
			Read(info.distortion_k[i], p);
		}
		syslog(LOG_DEBUG, "display info:%d", size);
		syslog(LOG_DEBUG, "  command id: %d", info.command_id);
		syslog(LOG_DEBUG, "  distortion_type: %d", info.distortion_type);
		syslog(LOG_DEBUG, "  resolution: %d x %d", info.h_resolution, info.v_resolution);
		syslog(LOG_DEBUG, "  screen size:  %d x %d", info.h_screen_size, info.v_screen_size);
		syslog(LOG_DEBUG, "  vertical center:  %d", info.v_center);
		syslog(LOG_DEBUG, "  lens_separation: %d", info.lens_separation);
		syslog(LOG_DEBUG, "  eye_to_screen_distance: %d, %d", info.eye_to_screen_distance[0], info.eye_to_screen_distance[1]);
		syslog(LOG_DEBUG, "  distortion_k: %f, %f, %f, %f, %f, %f",
			info.distortion_k[0], info.distortion_k[1], info.distortion_k[2],
			info.distortion_k[3], info.distortion_k[4], info.distortion_k[5]);
	}

}
