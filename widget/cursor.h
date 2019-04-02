/** cursor
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
#include <toolbox/gl/texture.h>
#include <toolbox/geometry/vector.h>


#if 0
★今度のカーソルは直線で表現されるので結構書き直す必要がある★
カーソルの移動範囲はNavigatorの内側の円を縁とする円錐
移動範囲から出てたら引き戻す(向き、奥行きはそのまま半径で調整)
なのでPickされると画面外になる窓などのオブジェクトをプレスすると自動的にドラッグして画面内へ引き寄せる。
#endif


namespace vr_core{

	class Cursor{
	public:



		bool IsInRange(
			const TB::Vector<float, 3>& position,
			const TB::Vector<unsigned, 2>& size);

	private:
		/** カーソルの方向
		 * 奥行きが1の面を通る点
		 */
		TB::Vector<float, 2> direction;
	};

















#if 0
	/** カーソル
	 * カーソル制御の共通部
	 */
	class Cursor : public wO::Cursor{
		Cursor(const Cursor&);
		void operator=(const Cursor&);
	public:

		static void NotifyWidgetDeleted(const PositionWidget&);
		static const int* GetPoint();
		static void UpdateAll(const TB::Vector<float, 2>&);
		void Update(const TB::Vector<float, 2>&);
		static void DrawAll();

		static void EnterProgress(){ ++progress; };
		static void ExitProgress(){ --progress; };

		virtual const TB::Vector<int, 2>* UpdateCenter(
			const TB::Vector<float, 2>&)=0;
		void UpdateWidget(const Widget::Founds&);
		void Draw();
		void DrawProgress();

		const int* Point(){ return point; };
		void WidgetDeleted(const PositionWidget&);

	protected:
		//TODO:WidgetResourceへ巻き取って廃止
		virtual void OnEnter(PositionWidget* target, int x, int y)=0;
		virtual void OnMove(PositionWidget* target, int x, int y)=0;
		virtual void OnLeave(PositionWidget* target, int x, int y)=0;

		Cursor(int zOffset=0);
		void virtual UpdateCursorState(PositionWidget&)=0;

		State state;
		static unsigned progress; //progressカウンタ
		unsigned buttonState;
		unsigned pressedButton;
		unsigned releasedButton;
		TB::Vector<int, 2> point;
		int depth;
		unsigned frame;

		static TB::Vector<int, 2> scroll;
		static const unsigned scrollButton = 2;
		static bool rolling;

	private:
		PositionWidget::Founds target;
		PositionWidget::Founds found;
		int zOffset;

		struct{
			PositionWidget* target;
			unsigned buttonState;
		}grabInfo;
		void ButtonEvent(unsigned, unsigned, unsigned);

		static class SightCursor sightCursor;
		static class MouseCursor mouseCursor;
	};

	/** 視線カーソル
	 * FOVEのように視線機能があるなら視線
	 * なければ視野中心
	 */
	class SightCursor : public Cursor{
	public:
		SightCursor() : visible(false){ state = sightNormal; };
		void Draw();

	private:

		const TB::Vector<int, 2>*
			UpdateCenter(const TB::Vector<float, 2>&) final;

		void OnEnter(PositionWidget* target, int x, int y) final;
		void OnMove(PositionWidget* target, int x, int y) final;
		void OnLeave(PositionWidget* target, int x, int y) final;

		void UpdateCursorState(PositionWidget&) final;

		void SetUpCursor(State s, unsigned f, bool pre){
			state = s;
			limit = f;
			now = 0;
			preWatch = pre;
			frame = 0;
		};

		bool visible;
		bool preWatch;
		unsigned limit;
		unsigned now;
		PositionWidget* target;
	};

	/** マウスカーソル
	 */
	class MouseCursor : public Cursor{
	public:
		MouseCursor() : Cursor(1), mice(MiceReporter){};
		~MouseCursor(){ mice.Quit(); };
		void Draw();

	private:
		MICE mice;

		static TB::Vector<int, 2> position;
		static unsigned button;
		static unsigned pressed;
		static unsigned released;
		static bool knocked;

		const TB::Vector<int, 2>* UpdateCenter(const TB::Vector<float, 2>&) final;
		static void MiceReporter(const MICE::Report&);

		void OnEnter(PositionWidget* target, int x, int y) final;
		void OnMove(PositionWidget* target, int x, int y) final;
		void OnLeave(PositionWidget* target, int x, int y) final;

		void UpdateCursorState(PositionWidget&) final;
	};



	/** カーソルセット
	 * カーソル描画関連
	 */
	class CursorSet :
		public wO::CursorSet,
		public Resource,
		private DM::Canvas{
	public:
		static void New(const wO::Message::Pack&, const Resource::Params&);
		static void Draw(
			State,
			int x,
			int y,
			int z,
			unsigned frame);
		static unsigned GetFrame(unsigned l, unsigned n){
			return activeSet ? (*activeSet).frames * n / l : 0;
		};
		static void SendEvent(
			wO::Message::Types type, int x, int y,
			unsigned buttonState = 0,
			unsigned pressed = 0,
			unsigned released = 0){
			if(!activeSet){ return; }
			(*activeSet).Resource::SendEvent(
				type,
				x,
				y,
				buttonState,
				pressed,
				released);
		};
		static void SendEvent(wO::Message& m){
			if(activeSet){
				(*activeSet).Send(m);
			}
		};

	private:
		static GL::TEXTURE::PARAMS textureParams;
		static CursorSet* activeSet;

		//each cursorset
		const unsigned size;
		const unsigned frames; //アニメーションフレーム数
		const unsigned numOfCursors; //カーソルの種類数
		const float uSize;
		const float vSize;

		CursorSet(
			const Resource::Params&,
			unsigned width,
			unsigned height,
			unsigned size,
			unsigned bpp,
			unsigned a);
		~CursorSet();

		void OnMessage(class wO::Message&) final;
	};
#endif
}
