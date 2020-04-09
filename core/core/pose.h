#pragma once

#include <toolbox/complex/complex.h>
#include <toolbox/geometry/vector.h>
#include <toolbox/filters/median.h>
#include <toolbox/prefs.h>


class POSE{
	POSE(const POSE&);
	void operator=(const POSE&);

public:
	using VecPref = TB::Prefs<VECTOR<3> >;

	//オプション
	static TB::Prefs<bool> resetMag; //磁化情報を初期化(再キャリブレーション)

	static float GetProgress(){ return initialProgress; };
	static void ResetAzimuth(); //現在の方位を正面にする
	static void StickAzimuth();

protected:
	POSE();
	virtual ~POSE();

	static const COMPLEX<4> GetDirection();
	static const VECTOR<3>& GetPosition(){ return position; };
	static void SetupGLPose();

	//絶対指定
	static void SetPose(const COMPLEX<4>&, const VECTOR<3>&);

	//相対指定
	static void UpdatePose(const COMPLEX<4>&, const VECTOR<3>&, double dt);
	static void UpdateMagneticField(const VECTOR<3>&);
	static void CorrectError();

private:
	static COMPLEX<4> direction;
	static VECTOR<3> position;

	//加速度センサ(絶対座標系)
	static VECTOR<3> gravity; //平均加速度(絶対座標系)
	static VECTOR<3> velocity; //移動速度

	//磁気センサ
	static VecPref prefMagMax; //磁気センサの最大値
	static VecPref prefMagMin; //磁気センサの最小値
	static VECTOR<3> magMax;
	static VECTOR<3> magMin;
	static bool magReady; //磁化情報取得完了
	static VECTOR<3> magneticField; //磁気の向き(絶対座標系)

	//補正関連
	static float initialProgress;
	static const int maxAverageRatio = 3000;
	static int averageRatio;
	static double correctionGain;
	static bool stickAzimuth;
};
