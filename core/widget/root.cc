/******************************************************************* Root
 * Copyright (C) 2017,2019 tarosuke<webmaster@tarosuke.net>
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

#include <math.h>
#include <syslog.h>

#include <toolbox/gl/gl.h>

#include "root.h"
#include "cursor.h"



namespace core{

	Root* Root::instance(0);

	/** パラメタ例
	 */
	const float Root::baseDistance(0.8f);
	const float Root::depthScale(-1000);
	const float Root::scale(0.0011); //1.1mm/pix @ baseDistance

	/** 注視点などの計算結果
	 */
	bool Root::lookingFront(false);
	double Root::roll(0);
	TB::Vector<float, 2> Root::lookingPoint;

	/** 設定
	 */
	TB::Prefs<bool> Root::restoreWidgetRotation("+R", false);

	/** 窓座標系の奥行きで座標変換
	 */
	TB::Vector<float, 3> Root::SightCast(
		const TB::Vector<float, 3>& v, float newDepth){
			const float bd(baseDistance * depthScale);
			const float dr((bd - newDepth)/ (bd - v[2]));

			return TB::Vector<float, 3>(
				v[0] * dr,
				v[1] * dr,
				newDepth);
	}


	/** Widget用の座標系を設定
	 */
	void Root::SetView(const Pose& pose){
#if 0

		//注視点計算
		UpdateLookingPoint(dir);

		//スケール設定
		glScalef(scale, scale, 0.001);
#if 0
		const int* const lp(DM::Cursor::GetPoint());
#endif

		//窓の傾きを戻す
		glRotatef(roll, 0, 0, -1);

		//VIEW設定
		glTranslatef(
			-lookingPoint[0],
			-lookingPoint[1],
			baseDistance * depthScale);
#endif
	}


	//キーイベント生成、カーソルと視線の処理
	void Root::UpdateLookingPoint(const COMPLEX<4>& direction){
		//ロール情報を保存しておく
		COMPLEX<4> p(direction);
		p.FilterAxis(4);
		COMPLEX<4>::ROTATION r;
		p.GetRotation(r);
		if(restoreWidgetRotation){
			roll = -r.axis[2] * r.angle * 180 / M_PI;

			//Navigatorの回転情報も与える
			Navigator::UpdateRotation(roll);
		}

		//視線計算
		VECTOR<3> viewLine((const double[]){ 0, 0, 1 });
		viewLine.Rotate(direction * 0.5);

		//前後チェック
		if(!(lookingFront = (0 < viewLine[2]))){
			return;
		}

		//注視点計算
		viewLine *= (5 / (viewLine[2] * scale));
		const float vl[] = { (float)-viewLine[0], (float)viewLine[1] };

		static TB::Vector<float, 2> newLookingPoint;
		const float xGain(1.0f/(0.01*fabs(vl[0])*scale + 1.0f));
		const float yGain(1.0f/(0.01*fabs(vl[1])*scale + 1.0f));
		const float cvl[] = {
			vl[0] * xGain + newLookingPoint[0] * (1.0f - xGain),
			vl[1] * yGain + newLookingPoint[1] * (1.0f - yGain) };
		newLookingPoint = cvl;
#if 0
		//Cursorの処理
		DM::Cursor::UpdateAll(newLookingPoint);
#endif

		//窓ナビゲーションの更新
		Navigator::UpdateCenter(newLookingPoint * scale);
	}


	/** 全体ハンドラ
	 */
	void Root::DrawAll(){
		if(lookingFront){
			(*instance).Widget::Draw();
		}
	}
	void Root::DrawTransparentAll(){
		if(lookingFront){
			(*instance).Widget::DrawTransparent();
		}
	}
	void Root::UpdateAll(){
		(*instance).Widget::Update();
	}


	//
	// 傾斜と場所、奥行きから接触点を計算
	//
	TB::Vector<float, 2> Root::GetOnPos(
		const TB::Vector<float, 2>& center,
		const TB::Vector<float, 2>& direction,
		float depth){

		// 奥行き反転＆視点からの距離に変換
		depth = -(depth - baseDistance * depthScale);

		//centerからdepth分directionを伸ばした先の点を返す
		return center + TB::Vector<float, 2>(
			direction[0] * depth, direction[1] * depth);
	}

	//
	// キーイベント処理
	// 特殊なもの以外は根から順にフォーカスを持っている子に伝達
	//
	void Root::OnKey(const KeyEvent& e){
		if((e.modifiers & e.shift) &&
			(e.modifiers & e.ctrl) &&
			(e.modifiers & e.alt)){
			// 強制終了
			Core::Quit();
			return;
		}
	}

	/** コンストラクタ
	 */
	Root::Root(){
		assert(!instance);
		instance = this;
	}

}
