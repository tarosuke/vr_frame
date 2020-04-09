/** Widget
 * Copyright (C) 2019 tarosuke<webmaster@tarosuke.net>
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
#include <widget.h>

#include "root.h"
#include "cursor.h"



namespace core{

	//
	// Widget
	// parentが0ならrootへ
	//
	Widget::Widget(Widget* parent, unsigned attr) :
			attributes(attr),
			parent(parent),
			focus(0){
		if(parent){
			if(parent != this){
				(*parent).Add(*this);
			}
		}else{
			Root::Add(*this);
		}
	}
	Widget::Widget() : attributes(0), parent(0), focus(0){}

	Widget::~Widget(){
		if(parent){
			(*parent).Defocus(*this);
		}
	}

	//
	// 子要素登録
	//
	void Widget::Add(Widget& child){
		children.Insert(child);
		ReDepth();
	}


	//デフォルトの動作
	void Widget::Draw(){
		for(TB::List<Widget>::I i(children); ++i;){
			(*i).Draw();
		}
	}
	void Widget::DrawTransparent(){
		for(TB::List<Widget>::I i(children); ++i;){
			(*i).DrawTransparent();
		}
	}
	void Widget::DrawPoint(){
		for(TB::List<Widget>::I i(children); ++i;){
			(*i).DrawPoint();
		}
	}

	void Widget::Update(const Clip& c){
		OnUpdate();
		for(TB::List<Widget>::I i(children); ++i;){
			(*i).Update(c);
		}
	}

	void Widget::Update(){
		OnUpdate();
		for(TB::List<Widget>::I i(children); ++i;){
			(*i).Update();
		}
	}

	float Widget::Pick(Widget& child){
		if(parent){
			depthGap = (*parent).Pick(*this);
			(*parent).Add(*this);
			return depthGap / gapRatio;
		}
		return baseGap;
	}

	// 子の奥行きを再配置
	void Widget::ReDepth(){
		float d(0.0f);
		if(parent){
			//Window内Widgetなので逆方向
			for(TB::List<Widget>::I i(children); --i;){
				if((*i).SetDepth(d)){
					d += depthGap;
				}
			}
		}else{
			//WindowレベルのWidgetなので順方向
			for(TB::List<Widget>::I i(children); ++i;){
				if((*i).SetDepth(d)){
					d -= depthGap;
				}
			}
		}
	}

	void Widget::Defocus(Widget& child){
		if(&child == focus){
			focus = 0;
		}
		ReDepth();
	}

	void Widget::AtKeyEvent(const KeyEvent& e){
		if(focus){
			(*focus).AtKeyEvent(e);
		}else{
			switch (e.type){
			case KeyEvent::down:
				OnKeyDown(e);
				break;
			case KeyEvent::up:
				OnKeyUp(e);
				break;
			case KeyEvent::repeat:
				OnKeyRepeat(e);
				break;
			case KeyEvent::none:
			default:
				break;
			}
		}
	}

	/** クリップ領域の移動、再クリップ
	 */
	Widget::Clip::Clip(const Clip& o, const TB::Vector<float, 3>& pos) :
		left(o.left - pos[0]),
		top(o.top - pos[1]),
		right(o.right - pos[0]),
		bottom(o.bottom - pos[1]){};

	Widget::Clip::Clip(
		const Clip& org,
		const TB::Vector<float, 3>& pos,
		const TB::Vector<float, 2>& size) :
			left(org.left - pos[0]),
			top(org.top - pos[1]),
			right(org.right - pos[0]),
			bottom(org.bottom - pos[1]){
		const float nr(left + size[0]);
		right = right < nr ? right : nr;
		const float nb(top + size[1]);
		bottom = bottom < nb ? bottom : nb;
	}

	//
	// Widget探索←AtMouseMoveにする
	//
	Widget::Found Widget::Find(
		const TB::Vector<float, 2>& pos,
		const TB::Vector<float, 2>& direction,
		float z){
		Found f(0,0,0,0);

		for(TB::List<Widget>::I i(children); ++i;){
			if(Found ff = (*i).Find(pos, direction, z)){
				if(f.Depth() < ff.Depth()){
					f = ff;
				}
			}
		}
		return f;
	}

}
