/** Cursor
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
# include <endian.h>

#include "cursor.h"
#include "navigator.h"

#include <toolbox/gl/gl.h>
#include <toolbox/image/image.h>
#include <toolbox/input/mice.h>

#include "navigator.h"



namespace core{




#if 0
	DM::SightCursor Cursor::sightCursor;
	DM::MouseCursor Cursor::mouseCursor;

	void Cursor::NotifyWidgetDeleted(const PositionWidget& w){
		sightCursor.WidgetDeleted(w);
		mouseCursor.WidgetDeleted(w);
	};
	void Cursor::WidgetDeleted(const PositionWidget& w){
		if(&w == target.widget){
			target = 0;
		}
		if(&w == found.widget){
			found = 0;
		}
		if(&w == grabInfo.target){
			grabInfo.target = 0;
		}
	}


	const int* Cursor::GetPoint(){ return sightCursor.Point(); }


	void Cursor::UpdateAll(const TB::Vector<float, 2>& p){
		sightCursor.Update(p);
		mouseCursor.Update(p);
	}
	void Cursor::Update(const TB::Vector<float, 2>& center){
		const auto p(UpdateCenter(center));
		if(p){
			UpdateWidget(Widget::FindAll((*p).X(), (*p).Y()));
		}
	}


	void Cursor::DrawAll(){
		sightCursor.Draw();
		mouseCursor.Draw();
	}




	/** カーソル(Sight/Mose共通)
	 */
	unsigned Cursor::progress(0);
	TB::Vector<int, 2> Cursor::scroll;
	bool Cursor::rolling(false);

	Cursor::Cursor(int zOffset) :
		state(normal),
		buttonState(0),
		pressedButton(0),
		releasedButton(0),
		depth(0),
		frame(0),
		zOffset(zOffset){
		grabInfo.target = 0;
	}

	void Cursor::UpdateWidget(const Widget::Founds& w){
		PositionWidget* const tw(dynamic_cast<PositionWidget*>(target.widget));
		PositionWidget* const ww(dynamic_cast<PositionWidget*>(w.widget));

		//スクロール中専用処理
		if(scroll[0] || scroll[1]){
			if(tw){
				(*tw).OnScroll(scroll[0], scroll[1]);
				scroll.Clear();
			}
			//スクロール中はカーソルは動かないしボタン状態も変化していないので処理終了
			if(rolling){ return; }
		}

		//奥行き更新(カーソルが何もないところにいたら基準面)
		depth = ww ? w.z : 0;

		//カーソルがWidgetを移動したのでイベントを組み立てる
		if(target.widget != w.widget){
			if(tw){
				//Widgetから出た
				OnLeave(tw, target.local.x, target.local.y);
			}
			if(ww){
				//Widgetに入った
				OnEnter(ww, w.local.x, w.local.y);
				//カーソル設定
				UpdateCursorState(*ww);
			}else{
				state = normal;
			}
			target = w;
		}else if(ww){
			//Widget内の移動のみ
			OnMove(tw, w.local.x, w.local.y);
		}

		//状態更新
		found = w;

		//ボタンイベント生成処理
		mouseCursor.ButtonEvent(
				buttonState,
				pressedButton,
				releasedButton);
	}

	void Cursor::ButtonEvent(
		unsigned buttonState,
		unsigned pressedButton,
		unsigned releasedButton){
		PositionWidget* const fw(dynamic_cast<PositionWidget*>(found.widget));

		//ボタンイベント
		if(pressedButton || releasedButton){
			if(grabInfo.target){
				//グラブ終了
				if(!buttonState){
					//正常終了なのでドロップフラグを立てる
					buttonState |= wO::Widget::dropFlag;
				}
				(*grabInfo.target).OnGrabEnd((const Resource::MouseParams){
					x: point.X(),
					y: point.Y(),
					state: buttonState,
					pressed: pressedButton,
					released: releasedButton});
				grabInfo.target = 0;
			}

			if(fw){
				//どこかの窓を叩いた
				if(pressedButton && buttonState == pressedButton){
					//グラブ開始
					grabInfo.target = fw;
					grabInfo.buttonState = buttonState;
					(*grabInfo.target).OnGrabStart((const Resource::MouseParams){
						x: point.X(),
						y: point.Y(),
						state: buttonState,
						pressed: pressedButton,
						released: releasedButton});
				}

				//普通のボタンイベント
				(*fw).OnMouseButton((const Resource::MouseParams){
					x: found.local.x,
					y: found.local.y,
					state: buttonState,
					pressed: pressedButton,
					released: releasedButton});
			}else{
				//何もないところでのボタンイベント
				CursorSet::SendEvent(
					wO::Message::onMouseButton, point.X(), point.Y(),
					buttonState, pressedButton, releasedButton);
			}
			pressedButton = releasedButton = 0;
		}else if(grabInfo.target){
			//グラブ継続(移動)
			(*grabInfo.target).OnGrabMove((const Resource::MouseParams){
				x: point.X(),
				y: point.Y(),
				state: buttonState,
				pressed: pressedButton,
				released: releasedButton});
		}
	}

	void Cursor::Draw(){
		CursorSet::Draw(
			state,
			point[0],
			point[1],
			depth + zOffset + 1,
			frame);
	}
	void Cursor::DrawProgress(){
		CursorSet::Draw(
			inProgress,
			point[0],
			point[1],
			depth + zOffset + 2,
			frame);
	}



	/** 視線カーソル
	 * 視線検出可能なVRHMDでなければ視野中心をそのまま使う
	 */
	const TB::Vector<int, 2>* SightCursor::UpdateCenter(
		const TB::Vector<float, 2>& c){
		const TB::Vector<int, 2> np(c); //視線検出がないので中心をそのまま使う
		if(np == point){
			//カーソルが移動していないので終了
			return 0;
		}
		point = np;
		return &point;
	}
	void SightCursor::Draw(){
		if(visible){
			if(++now < limit){
				frame = CursorSet::GetFrame(limit, now);
			}else{
				const PositionWidget::CursorState& s((*target).GetCursorState());
				if(preWatch){
					//preWatchTime終了
					SetUpCursor(watching, s.watchTime, false);
				}else{
					//待ち終了(イベント発行)
					(*target).OnWatched();
					if(s.repeatable){
						//再びpreWatchへ
						SetUpCursor(sightNormal, s.preWatchTime, true);
					}else{
						visible = false;
					}
				}
			}
			Cursor::Draw();
		}
	}
	void SightCursor::OnEnter(PositionWidget* target, int x, int y){
		assert(target);
		(*target).OnSightEnter(x, y);
	}
	void SightCursor::OnMove(PositionWidget* target, int x, int y){
		assert(target);
		(*target).OnSightMove(x, y);
	}
	void SightCursor::OnLeave(PositionWidget* target, int x, int y){
		assert(target);
		visible = false;
		(*target).OnSightLeave(x, y);
	}
	void SightCursor::UpdateCursorState(PositionWidget& w){
		const PositionWidget::CursorState& s(w.GetCursorState());

		//視線カーソルの設定
		visible = s.useSight;
		SetUpCursor(sightNormal, s.preWatchTime, true);
		target = &w;
	}



	/** マウスカーソル
	 * 視野中心から一定範囲に収めるようにする以外は普通のマウスカーソル
	 */
	TB::Vector<int, 2> MouseCursor::point;
	unsigned MouseCursor::button(0);
	unsigned MouseCursor::pressed(0);
	unsigned MouseCursor::released(0);
	bool MouseCursor::knocked(false);
	void MouseCursor::MiceReporter(const MICE::Report& report){
		if(report.axis[0] || report.axis[1]){
			const int axis[] = { report.axis[0], -report.axis[1] };
			if(button == scrollButton){
				scroll += TB::Vector<int, 2>(axis);
				rolling = true;
			}else{
				point += TB::Vector<int, 2>(axis);
			}
		}

		//update buttonstate
		pressed |= ~button & report.buttons;
		released |= button & ~report.buttons;
		button = report.buttons;

		if(button != scrollButton){
			//スクロール終了
			rolling = false;
		}

		knocked = true;
	}
	void MouseCursor::Draw(){
		++frame;
		Cursor::Draw();
		if(progress){
			Cursor::DrawProgress();
		}
	}
	const TB::Vector<int, 2>* MouseCursor::UpdateCenter(
		const TB::Vector<float, 2>& c){
		if(!knocked){
			return 0;
		}
		knocked = false;

		const auto prev(point);
		point += point;
		point.Clear();

		//update lookingPoint & limit cursor
#if 1
		const TB::Vector<int, 2> range((const int[]){400,400});
		point.Max(c - range);
		point.Min(c + range);
#else
		point = Navigator::LimitCursor(point);
#endif

		buttonState = button;
		pressedButton = pressed;
		releasedButton = released;
		pressed = released = 0;

		return  &point;
	}
	void MouseCursor::OnEnter(PositionWidget* target, int x, int y){
		assert(target);
		(*target).OnMouseEnter((const PositionWidget::MouseParams){
			x: x,
			y: y,
			state: button,
			pressed: pressed,
			released: released,
		});
	}
	void MouseCursor::OnMove(PositionWidget* target, int x, int y){
		assert(target);
		(*target).OnMouseMove((const PositionWidget::MouseParams){
			x: x,
			y: y,
			state: button,
			pressed: pressed,
			released: released,
		});
	}
	void MouseCursor::OnLeave(PositionWidget* target, int x, int y){
		assert(target);
		(*target).OnMouseLeave((const PositionWidget::MouseParams){
			x: x,
			y: y,
			state: button,
			pressed: pressed,
			released: released,
		});
	}

	void MouseCursor::UpdateCursorState(PositionWidget& w){
		const PositionWidget::CursorState& s(w.GetCursorState());
		state = s.mouseCursorState;
	}



	/** カーソルセット
	 * カーソル描画関連
	 */
	GL::TEXTURE::PARAMS CursorSet::textureParams = {
		wrap_s : GL_CLAMP_TO_EDGE,
		wrap_t : GL_CLAMP_TO_EDGE,
		filter_mag : GL_LINEAR,
		filter_min : GL_LINEAR,
		texture_mode : GL_REPLACE,
		pointSprite : false,
	};

	CursorSet* CursorSet::activeSet(0);

	void CursorSet::New(
		const wO::Message::Pack& pack,
		const Resource::Params& params){
		auto& p(*(Cursor::Pack*)&pack);
		new CursorSet(
			params,
			p.width,
			p.height,
			p.size,
			p.bpp,
			p.attributes);
	}
	CursorSet::CursorSet(
		const Resource::Params& params,
		unsigned width,
		unsigned height,
		unsigned size,
		unsigned bpp,
		unsigned a) :
		wO::CursorSet(),
		Resource(params),
		DM::Canvas(width, height, bpp, a, textureParams),
		size(size),
		frames(width / size),
		numOfCursors(height / size),
		uSize((float)size / width),
		vSize((float)size / height){}
	CursorSet::~CursorSet(){
		if(activeSet == this){
			activeSet = 0;
		}
	}
	void CursorSet::OnMessage(wO::Message& m){
		Cursor::Pack& p(m);

		//canvasチェック
		if(OnCanvasMessage(m)){
			return;
		}

		switch(p.head.type){
		case wO::Message::cursorActivated:
			activeSet = this;
			break;
		}
	}

	void CursorSet::Draw(
		State state,
		int x,
		int y,
		int z,
		unsigned frame){
		if(!activeSet){
			//有効なカーソルセットがないので何もしない
			return;
		}
		CursorSet& c(*activeSet);

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
		GL::TEXTURE::BINDER b(c);

		glBegin(GL_TRIANGLE_STRIP);
		(*activeSet).TexCoord(u0, v0);
		glVertex3f(x-h, y-h, z);
		(*activeSet).TexCoord(u0, v1);
		glVertex3f(x-h, y+h, z);
		(*activeSet).TexCoord(u1, v0);
		glVertex3f(x+h, y-h, z);
		(*activeSet).TexCoord(u1, v1);
		glVertex3f(x+h, y+h, z);
		glEnd();
	}
#endif

}
