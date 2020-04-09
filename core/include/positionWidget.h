/** PositionWidget
 * Copyright (C) 2016.2019 tarosuke<webmaster@tarosuke.net>
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
#pragma once

#include <toolbox/geometry/vector.h>

#include <widget.h>



namespace core{

	/** 位置持ちWidget
	 * 位置を持つので子クラスへ何かを渡すときはローカル座標に変換する
	 */
	class PositionWidget : public Widget{
		PositionWidget();
		PositionWidget(const PositionWidget&);
		void operator=(const PositionWidget&);
	public:

		//
		// コンストラクタ
		//
		PositionWidget(
			Widget* parent,
			int x,
			int y,
			int z,
			unsigned attributes = 0);
		PositionWidget(
			Widget* parent,
			int x,
			int y,
			unsigned, unsigned, //ダックタイプのためのダミー
			unsigned attributes = 0);

		//
		// 移動
		//
		void MoveTo(const TB::Vector<float, 3>&);
		void MoveTo(const TB::Vector<float, 2>&);
		void MoveTo(float x, float y, float z);
		void MoveTo(float x, float y);
		void Move(const TB::Vector<float, 3>&);
		void Move(const TB::Vector<float, 2>&);
		void Move(float dx, float dy, float dz = 0);
		void JumpTo(const TB::Vector<float, 3>&);
		void JumpTo(const TB::Vector<float, 2>&);
		void JumpTo(float x, float y, float z);
		void JumpTo(float x, float y);
		void Jump(const TB::Vector<float, 3>&);
		void Jump(const TB::Vector<float, 2>&);
		void Jump(float dx, float dy, float dz = 0);
		bool SetDepth(float) override;

	protected:
		//
		// 位置からWidgetを探索
		// PositionWidgetは範囲を持たないので座標変換
		//
		Found Find(
			const TB::Vector<float, 2>& pos,
			const TB::Vector<float, 2>& direction,
			float z) override;

		//
		// 周回処理ハンドラ
		//
		void Draw() override;
		void DrawTransparent() override;
		void DrawPoint() override;
		void Update(const Clip&) override;
		void Update() override;

		//
		// アクセサ
		//
		const TB::Vector<float, 3>& Position(){ return position; };
		void UpdateClippedOut(bool value){ clippedOut = value; };
		bool IsClippedOut(){ return clippedOut; };

	private:
		//
		// 移動処理
		//
		void Moving();

		TB::Vector<float, 3> position;
		TB::Vector<float, 3> targetPosition;

		//
		// 境界の外側で見えない時にtrue
		// Updateで毎回更新
		//
		bool clippedOut;
	};

}
