/** PositionWidget
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
#include <positionWidget.h>

#include <math.h>

#include <toolbox/gl/gl.h>



namespace vr_core{

	//
	// 位置持ちWidget
	// 位置を持つので子クラスへ何かを渡すときはローカル座標に変換する
	//
	PositionWidget::PositionWidget(
			Widget* p,
			int x,
			int y,
			int z,
			unsigned attributes) :
		Widget(p, attributes),
		position(x, y, z),
		targetPosition(x, y, z){}
	PositionWidget::PositionWidget(
			Widget* p,
			int x,
			int y,
			unsigned,
			unsigned,
			unsigned attributes) :
		Widget(p, attributes),
		position(x, y, 0),
		targetPosition(x, y, 0){}

	//
	// Widget検索
	//
	Widget::Found PositionWidget::Find(
		const TB::Vector<float, 2>& pos,
		const TB::Vector<float, 2>& direction,
		float z){
		TB::Vector<float, 2> newPos(
			pos[0] - position[0],
			pos[1] - position[1]);
		return Widget::Find(newPos, direction, z + position[2]);
	}

	//
	// 移動
	//
	void PositionWidget::MoveTo(const TB::Vector<float, 3>& t){
		targetPosition = t;
	}
	void PositionWidget::MoveTo(const TB::Vector<float, 2>& t){
		targetPosition[0] = t[0];
		targetPosition[1] = t[1];
	}
	void PositionWidget::MoveTo(float x, float y, float z){
		targetPosition[0] = x;
		targetPosition[1] = y;
		targetPosition[2] = z;
	}
	void PositionWidget::MoveTo(float x, float y){
		targetPosition[0] = x;
		targetPosition[1] = y;
	}
	void PositionWidget::Move(const TB::Vector<float, 3>& t){
		targetPosition += t;
	}
	void PositionWidget::Move(const TB::Vector<float, 2>& t){
		targetPosition[0] += t[0];
		targetPosition[1] += t[1];
	}
	void PositionWidget::Move(float dx, float dy, float dz){
		targetPosition[0] += dx;
		targetPosition[1] += dy;
		targetPosition[2] += dz;
	}
	void PositionWidget::JumpTo(const TB::Vector<float, 3>& t){
		position = targetPosition = t;
	}
	void PositionWidget::JumpTo(const TB::Vector<float, 2>& t){
		JumpTo(t[0], t[1]);
	}
	void PositionWidget::JumpTo(float x, float y, float z){
		targetPosition[0] = x;
		targetPosition[1] = y;
		targetPosition[2] = z;
		position = targetPosition;
	}
	void PositionWidget::JumpTo(float x, float y){
		position[0] = targetPosition[0] = x;
		position[1] = targetPosition[1] = y;
	}
	void PositionWidget::Jump(const TB::Vector<float, 3>& t){
		Jump(t[0], t[1], t[2]);
	}
	void PositionWidget::Jump(const TB::Vector<float, 2>& t){
		Jump(t[0], t[1], 0);
	}
	void PositionWidget::Jump(float dx, float dy, float dz){
		targetPosition[0] += dx;
		targetPosition[1] += dy;
		targetPosition[2] += dz;
		position = targetPosition;
	}
	bool PositionWidget::SetDepth(float z){
		if(IsAutoDepth()){
			targetPosition[2] = z;
			return true;
		}
		return false;
	}

	//
	// 周回処理ハンドラ
	//
	void PositionWidget::Draw(){
		if(!IsVisible() || clippedOut){
			return;
		}
		glPushMatrix();
		glTranslatef( position.X(), position.Y(), position.Z() );
		Widget::Draw();
		glPopMatrix();
	}
	void PositionWidget::DrawTransparent(){
		if(!IsVisible() || clippedOut){
			return;
		}
		glPushMatrix();
		glTranslatef( position.X(), position.Y(), position.Z() );
		Widget::DrawTransparent();
		glPopMatrix();
	}
	void PositionWidget::DrawPoint(){
//TODO:visible & navigationなら輝点を描画
	}

	void PositionWidget::Update(const Clip& c){
		Moving();

		//移動したClipで子要素探索
		Widget::Update(Clip(c, position));
	}

	void PositionWidget::Update(){
		Moving();
		Widget::Update();
	}

	void PositionWidget::Moving(){
		TB::Vector<float, 3> diff(targetPosition - position);
		if(0.5f <= fabsf(diff[0]) || 0.5f <= fabsf(diff[1]) || 0.5f <= fabsf(diff[2])){
			//要移動(差の一部を移動)
			diff /= 3;
			position += diff;
		}else{
			position = targetPosition;
		}
	}










#if 0
	//Widget探索
	Widget::Founds PositionWidget::Find(int x, int y, int z){
		return Widget::Find(x - position[0], y - position.[1], z + position.[2]); }
#endif


}
