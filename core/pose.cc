#include <syslog.h>
#include <stdlib.h>
#include <math.h>

#include <toolbox/gl/gl.h>

#include "pose.h"


#define MaxFloat (3.40282347e+38F)
#define G (9.80665)

COMPLEX<4> POSE::direction;
VECTOR<3> POSE::position;
bool POSE::stickAzimuth(false);

//加速度センサ(絶対座標系)
VECTOR<3> POSE::gravity((const double[]){ 0.0, -G, 0.0 }); //平均加速度(絶対座標系)
VECTOR<3> POSE::velocity; //移動速度

//磁気センサ
POSE::VecPref POSE::prefMagMax(
	"magMax",
	(const VECTOR<3>)((const double[]){
		-MaxFloat,
		-MaxFloat,
		-MaxFloat })); //磁気センサの最大値
POSE::VecPref POSE::prefMagMin(
	"magMin",
	(const VECTOR<3>)((const double[]){
		MaxFloat,
		MaxFloat,
		MaxFloat })); //磁気センサの最小値
VECTOR<3> POSE::magMax;
VECTOR<3> POSE::magMin;
bool POSE::magReady; //磁化情報取得完了
VECTOR<3> POSE::magneticField((const double[3]){ 0.0, 0.0, 0.01 }); //磁気の向き(絶対座標系)

int POSE::averageRatio(1);
float POSE::initialProgress;
double POSE::correctionGain;

TB::Prefs<bool> POSE::resetMag("+m", false, TB::CommonPrefs::nosave);


POSE::POSE(){
	if(resetMag){
		syslog(LOG_DEBUG, "clearing magnetic information.");
		magMin = VECTOR<3>((const double[3]){ MaxFloat, MaxFloat, MaxFloat });
		magMax = VECTOR<3>((const double[3]){ -MaxFloat, -MaxFloat, -MaxFloat });
	}else{
		magMax = prefMagMax;
		magMin = prefMagMin;
	}

	syslog(LOG_DEBUG,
		"magx:%lf->%lf.\n",
		magMin[0],
		magMax[0]);
	syslog(LOG_DEBUG,
		"magy:%lf->%lf.\n",
		magMin[1],
		magMax[1]);
	syslog(LOG_DEBUG,
		"magz:%lf->%lf.\n",
		magMin[2],
		magMax[2]);
}

POSE::~POSE(){
	prefMagMax = magMax;
	prefMagMin = magMin;
	syslog(LOG_DEBUG,
		"magx:%lf->%lf.\n",
		magMin[0],
		magMax[0]);
	syslog(LOG_DEBUG,
		"magy:%lf->%lf.\n",
		magMin[1],
		magMax[1]);
	syslog(LOG_DEBUG,
		"magz:%lf->%lf.\n",
		magMin[2],
		magMax[2]);
}




void POSE::SetupGLPose(){
	auto d(GetDirection());
	if(d.W() < 1.0 - 1.0 / MaxFloat){
		const COMPLEX<4>::ROTATION r(d);
		glRotated(-r.angle * 180 / M_PI, r.axis[0], r.axis[1], r.axis[2]);
	}
	const double* const pp(position);
	glTranslated(-pp[0], -pp[1], -pp[2]);
}


void POSE::SetPose(const COMPLEX<4>& d, const VECTOR<3>& p){
	direction = d;
	position = p;
	initialProgress = 1.0;
}


static TB::Median<double> accx(0.0), accy(-G), accz(0.0);
void POSE::UpdatePose(
	const COMPLEX<4>& rotate,
	const VECTOR<3>& acc,
	double dt){
	//補正値の更新
	if(averageRatio < maxAverageRatio){
		correctionGain = 0.99 / averageRatio;
		initialProgress = (float)averageRatio / maxAverageRatio;
		++averageRatio;
	}

	//向きアップデート
	direction *= rotate;

	//位置アップデート
	if(0.2 < dt){
		//経過時間が長過ぎるので無視
		return;
	}

	//Tap検出
	static double prevZ(0.0);
	if(40.0 < fabs(prevZ - acc[2])){
		// 現在の状態を中央にする
		ResetAzimuth();
	}
	prevZ = acc[2];

	/** メディアンフィルタ
	 */
	double a[] = {
		accx = acc[0],
		accy = acc[1],
		accz = acc[2],
	};

	/** 座標変換、移動
	 */
	VECTOR<3> accel(a);
	const double scale(accel.Length());
	accel.Rotate(direction); //絶対座標系へ変換
	accel *= scale;

	//重力加速度更新
	gravity *= 1.0 - correctionGain;
	gravity += accel * correctionGain;

	if(initialProgress < 0.9){
		//キャリブレーション中なので動かさない
		velocity *= 0;
		position *= 0;
	}else{
		//重力除去
		VECTOR<3> a(accel - gravity);

		//update position
		velocity += a * dt;
		position += velocity * dt;

		//枠を出ていたら止める
// 		const double limit(1.0);
// 		double* p(position);
// 		double* v(velocity);
// 		for(unsigned n(0); n < 3; ++n){
// 			if(p[n] < -limit){
// 				v[n] = 0;
// 				p[n] = -limit;
// 			}else if(limit < p[n]){
// 				v[n] = 0;
// 				p[n] = limit;
// 			}
// 		}

		//ドリフト避けに位置をちょっとづつ中央に戻す
#if 1
#if 0
		double distance(position.Length() * 0.5);
		double lead(1.0 - distance*distance*distance*distance);
		velocity *= lead;
		position *= lead;
#else
		velocity *= 0.999;
		position *= 0.999999;
#endif
#endif
	}
}

void POSE::UpdateMagneticField(const VECTOR<3>& m){
#if 0
	VECTOR<3> mag(m);

	if(magReady){
		//磁化分を除去
		VECTOR<3> offset(magMax + magMin);
		offset *= 0.5;
		mag -= offset;

		//絶対座標系に変換
		mag.Rotate(direction);
		double* v(mag);
		v[1] = 0; //欲しいのは方位角であり鉛直方向は邪魔なので除去
		mag.Normalize();

		//平均化処理
		mag *= correctionGain;
		magneticField += mag;;
		magneticField.Normalize();;
	}else{
		//磁化情報収集
		magMax.Max(mag);
		magMin.Min(mag);

		//キャリブレーション
		const VECTOR<3> deGain((VECTOR<3>)magMax - (VECTOR<3>)magMin);
		const double* const d(deGain);

		//キャリブレーション可能判定
		if(6000 < abs(d[0]) && 6000 < abs(d[1]) && 6000 < abs(d[2])){
			magReady = true;
			averageRatio = 3;
			syslog(LOG_DEBUG, "magnetic azimuth correction READY.");
		}
	}
#endif
}

void POSE::CorrectError(){
	COMPLEX<4> diff;

	//重力補正
	static const VECTOR<3> vertical((const double[3]){ 0, -G, 0 });
	COMPLEX<4> gdiff(gravity, vertical);
	gdiff.Normalize();
	gdiff *= correctionGain;
	const double gl(gravity.Length());
	gravity.Rotate(gdiff);
	gravity *= gl;
	diff *= gdiff;

	//磁気補正
#if 0
	if(magReady){
		static const VECTOR<3> facingAzimuth((const double[]){ -1, 0, 0 });
		COMPLEX<4> mdiff(magneticField, facingAzimuth);
		mdiff.FilterAxis(2);
		mdiff.Normalize();
		mdiff *= correctionGain;
		magneticField.Rotate(mdiff);
		diff *= mdiff;
	}
#endif

	diff *= direction;
	direction = diff;
}


void POSE::ResetAzimuth(){
	direction.FilterAxis(5);

	//中央調整モード終了
	stickAzimuth = false;
}

void POSE::StickAzimuth(){
	//中央調整モード開始
	stickAzimuth = true;
}

const COMPLEX<4> POSE::GetDirection(){
	if(stickAzimuth){
		direction.FilterAxis(5);
	}

	return direction;
};
