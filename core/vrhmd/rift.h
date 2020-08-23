#pragma once

#include <pthread.h>

#include "vrhmd_relative.h"



namespace core{

	class RIFT : public VRHMD_Relative{
		RIFT();
		RIFT(const RIFT&);
		void operator=(const RIFT&);
	public:
	protected:
		static const int VendorID = 0x2833;

		const int fd;
		RIFT(int fd, Profile& p);
		virtual ~RIFT();

		void UpdateSensor() final;

		// 温度センサ[℃]
		float temperature;

		/////センサ関連
		void UpdateTemperature(float){};

		//受信データのデコード
		void DecodeSensor(const unsigned char*, int[3]);
		void Decode(const char*);

		struct Scale{
				unsigned char accel;
				short rotate;
				short mag;
		}scale;
		struct Config{
			uint16_t command_id;
			uint8_t flags;
			uint16_t packet_interval;
			uint16_t keep_alive_interval; // in ms
		}config;
		static const unsigned RIFT_SCF_RAW_MODE = 0x01;
		static const unsigned RIFT_SCF_CALIBRATION_TEST = 0x02;
		static const unsigned RIFT_SCF_USE_CALIBRATION = 0x04;
		static const unsigned RIFT_SCF_AUTO_CALIBRATION = 0x08;
		static const unsigned RIFT_SCF_MOTION_KEEP_ALIVE = 0x10;
		static const unsigned RIFT_SCF_COMMAND_KEEP_ALIVE = 0x20;
		static const unsigned RIFT_SCF_SENSOR_COORDINATES = 0x40;
		//順次取得
		template<typename T> void Read(T& v, unsigned char*& p){
			v = *(T*)p; p += sizeof(T);
		};
		void ReadConfig();
		void ReadScale();
		void ReadInfo();

		// Keepalive処理
		pthread_t keepaliveThread;
		static const long keepaliveInterval = 1000;
		void Keepalive();
		static void* KeepaliveThread(void*);
	};

}
