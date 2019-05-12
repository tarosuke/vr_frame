/** BorderWidget
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
#include "borderWidget.h"

#include <math.h>

#include <toolbox/gl/gl.h>

#include "root.h"



namespace vr_core{

	TB::Prefs<TB::String> BorderWidget::initialCursorImageFile(
		"--cursor", "data/cursor.png");

	/** 領域付きWidget
	 * 位置＋サイズによるクリッピングと背景色
	 */
	BorderWidget::BorderWidget(
		Widget* parent,
		int x,
		int y,
		int z,
		unsigned width,
		unsigned height,
		unsigned c,
		unsigned attributes) :
			PositionWidget(parent, x, y, z, attributes |
				(c & 0xff000000) ? hasContent : 0),
			size(width, height){
		color[0] = c >> 16; //R
		color[1] = c >> 8; //G
		color[2] = c; //B
		color[3] = c >> 24; //A
	}


	void BorderWidget::Draw(){
		if(!IsVisible() || IsClippedOut()){
			return;
		}
		glPushMatrix();
		glTranslatef(Position()[0], Position()[1], Position()[2]);
		Widget::Draw();
		if(!IsTransparent() && HasContent() && clip){
			DrawContent();
		}
		glPopMatrix();
	}
	void BorderWidget::DrawTransparent(){
		if(!IsVisible() || IsClippedOut()){
			return;
		}
		glPushMatrix();
		glTranslatef(Position()[0], Position()[1], Position()[2]);
		if(IsTransparent() && HasContent() && clip){
			DrawContent();
		}
		Widget::DrawTransparent();
		glPopMatrix();
	}

	void BorderWidget::Update(const Widget::Clip& c){
		if(c){
			//空だったので以下判定せずそのまま子に渡す
			PositionWidget::Update(c);
			return;
		}
		clip = Clip(c, Position(), size);
		PositionWidget::Update(clip);
	}
	void BorderWidget::Update(){
		//クリップがないので自身でクリップ
		clip = Clip(0, 0, size[0], size[1]);
		PositionWidget::Update(clip);
	}



	//
	// Widget検索
	//
	Widget::Found BorderWidget::Find(
		const TB::Vector<float, 2>& pos,
		const TB::Vector<float, 2>& direction,
		float z){
		TB::Vector<float, 2> center(
			pos[0] - Position()[0],
			pos[1] - Position()[1]);
		z += Position()[2];

		//範囲チェック
		TB::Vector<float, 2> onPos(Root::GetOnPos(
			center,
			direction,
			Position()[2]));
		//TODO:マージンを設定してマージン内なら子要素を検索
		//斜めに見て触った時対策
		if(0 < onPos[0] && onPos[0] <= size[0] &&
			0 < onPos[1] && onPos[1] <= size[1]){
			//自身の範囲内なので子要素の検索
			Found f(Widget::Find(center, direction, z));

			if(!f || (f.Depth() < z)){
				//自身が子要素のどれよりも近いので自身の値を使う
				f = Found(this, onPos[0], onPos[1], z);
			}

			return f;
		}
		return Found();
	}


	//
	// コンテント描画
	//
	void BorderWidget::DrawContent(){
		glColor4ubv(color);
		glBegin(GL_TRIANGLE_STRIP);
		glVertex3f(clip.left, clip.top, -1);
		glVertex3f(clip.left, clip.bottom, -1);
		glVertex3f(clip.right, clip.top, -1);
		glVertex3f(clip.right, clip.bottom, -1);
		glEnd();
		glColor4f(1,1,1,1);
	}


	/** カーソル描画
	 */
	BorderWidget::CursorSet* BorderWidget::CursorSet::activeSet(0);

	BorderWidget::CursorSet::CursorSet(
		const TB::Image& image,
		unsigned size) :
		TB::Texture(image),
		frames(image.GetWidth() / size),
		states(image.GetHeight() / size),
		size(size),
		uSize(1.0 / frames),
		vSize(1.0 / states){
		if(activeSet){
			delete activeSet;
		}
		activeSet = this;
	}

	void BorderWidget::CursorSet::Draw(
		int x,
		int y,
		Widget::Cursor::State state){
		if(!activeSet){
			//有効なカーソルセットがないので何もしない
			return;
		}
		CursorSet& c(*activeSet);
		static unsigned frame(0);

		//フレーム数を32に制限
		frame %= c.frames;

		//U/V座標決定
		const float u0(c.uSize * frame);
		const float v0(c.vSize * state);
		const float u1(u0 + c.uSize);
		const float v1(v0 + c.vSize);
		const int h(c.size / 2);

		//draw cursor
		glEnable(GL_TEXTURE_2D);
		TB::Texture::Binder b(c);

		glBegin(GL_TRIANGLE_STRIP);
		glTexCoord2f(u0, v0);
		glVertex3i(x - h, y - h, 1);
		glTexCoord2f(u0, v1);
		glVertex3i(x - h, y + h, 1);
		glTexCoord2f(u1, v0);
		glVertex3i(x + h, y - h, 1);
		glTexCoord2f(u1, v1);
		glVertex3i(x + h, y + h, 1);
		glEnd();
	}

}
