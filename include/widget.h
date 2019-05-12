/** Widget
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

#pragma once

#include <limits.h>

#include <toolbox/container/list.h>
#include <toolbox/timestamp.h>
#include <toolbox/geometry/vector.h>
#include <toolbox/image.h>



namespace vr_core{

	class Widget : public TB::List<Widget>::Node{
		Widget(const Widget&);
		void operator=(const Widget&);
	public:

		//events
		struct MouseEvent{
			TB::Timestamp time;
			int x;
			int y;
			unsigned pressedButton;
			unsigned releasedButton;
			unsigned buttonState;
		};

		struct KeyEvent{
			static const unsigned shiftL = 0x01;
			static const unsigned shiftR = 0x02;
			static const unsigned ctrlL = 0x04;
			static const unsigned ctrlR = 0x08;
			static const unsigned altL = 0x10;
			static const unsigned altR = 0x20;
			static const unsigned caps = 0x40;
			static const unsigned shift = (shiftL | shiftR);
			static const unsigned ctrl = (ctrlL | ctrlR);
			static const unsigned alt = (altL | altR);

			enum{ none, down, up, repeat }type;
			TB::Timestamp times;
			unsigned keyCode;
			unsigned charCode;
			unsigned modifiers;
		};

		//カーソル関連
		class Cursor{
		public:
			enum State{
				inherited,
				notInService,
				normal,
				busy,
				text,
				crossHair,
			};

		protected:
			Cursor(){};
		};

		//奥行き制御系数
		static const int baseGap = 50;
		static const int gapRatio = 5;

		//属性
		static const unsigned autoDepth = 1; //奥行き自動制御する
		static const unsigned visible = 2; //可視
		static const unsigned navigation = 4; //ナビゲーションに光点を表示
		static const unsigned transparent = 8; //透過
		static const unsigned hasContent = 16; //中身がある
		static const unsigned noFocus = 32; //フォーカスしない

		//イベントハンドラ
		virtual void OnInited(){};
		virtual void OnMouseEnter(const MouseEvent&){};
		virtual void OnMouseMove(const MouseEvent&){};
		virtual void OnMouseLeave(const MouseEvent&){};
		virtual void OnSightEnter(const MouseEvent&){};
		virtual void OnSightMove(const MouseEvent&){};
		virtual void OnSightLeave(const MouseEvent&){};
		virtual void OnButtonDown(const MouseEvent&){};
		virtual void OnButtonUp(const MouseEvent&){};
		void AtKeyEvent(const KeyEvent&);
		virtual void OnKeyDown(const KeyEvent&){};
		virtual void OnKeyUp(const KeyEvent&){}
		virtual void OnKeyRepeat(const KeyEvent&){};
		virtual bool CanClose(){ return true; };
		virtual void OnUpdate(){};

		// コマンドハンドラ
		virtual void SetState(Cursor::State){};

	protected:

		//
		// クリップデータとUpdateハンドラ
		// UpdateはClipが空なら描画しないようにする
		//
		class Clip{
		public:
			float left;
			float top;
			float right;
			float bottom;
			operator bool() const{
				return left < right && top < bottom;
			};
			Clip() : left(0), top(0), right(0), bottom(0){};
			Clip(float l, float t, float r, float b) :
				left(l), top(t), right(r), bottom(b){};
			Clip(const Clip& org, const TB::Vector<float, 3>& pos);
			Clip(
				const Clip& org,
				const TB::Vector<float, 3>& pos,
				const TB::Vector<float, 2>& size);
		};
		virtual void Update(const Clip&);
		virtual void Update();

		//描画
		virtual void Draw();
		virtual void DrawTransparent();
		virtual void DrawPoint();

		//子要素登録
		void Add(Widget&);

		//Widget検索
		class Found{
		public:
			Found(Widget* w = 0, int x = 0, int y = 0, int z = INT_MIN) :
				widget(w), z(z){
				local.x = x;
				local.y = y;
			};
			operator bool(){ return !!widget; };
			operator Widget*(){ return widget; };
			int Depth(){ return z; };

		private:
			Widget* widget;
			struct{
				int x;
				int y;
			}local;
			float z; //絶対奥行き
		};
		virtual Found Find(
			const TB::Vector<float, 2>& pos,
			const TB::Vector<float, 2>& direction,
			float z);

		virtual ~Widget();

		const unsigned attributes;

		Widget(Widget* parent, unsigned attributes = 0);
		Widget(); // Root用

		//
		// 奥行きだけ設定する(中身はPositionWidgetで)
		//  自動奥行き制御が可能であれば奥行きを設定してtrueを返す
		virtual bool SetDepth(float){ return false; };

		//
		// 属性アクセサ
		//
		bool IsVisible(){ return !!(attributes & visible); };
		bool IsAutoDepth(){ return !!(attributes & autoDepth); };
		bool IsTransparent(){ return !!(attributes & transparent); };

	private:
		Widget* parent;
		TB::List<Widget> children;

		//フォーカス、奥行き制御(使うときはchildには*thisを与える)
		Widget* focus;
		float depthGap;
		float Pick(Widget& child);
		void ReDepth();
		void Defocus(Widget& child);
	};

}
