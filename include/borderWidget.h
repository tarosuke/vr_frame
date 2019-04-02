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
#pragma once

#include <math.h>

#include <toolbox/geometry/vector.h>

#include "positionWidget.h"


namespace vr_core{

	/** 領域付きWidget
	 * 位置＋サイズによるクリッピング
	 */
	class BorderWidget : public PositionWidget{
		BorderWidget();
		BorderWidget(const BorderWidget&);
		void operator=(const BorderWidget&);
	public:
		BorderWidget(
			Widget* parent,
			int x,
			int y,
			int z,
			unsigned width,
			unsigned height,
			unsigned color = 0,
			unsigned attributes = 0);

	protected:
		//
		// 周回処理ハンドラ
		//
		void Draw() final;
		void DrawTransparent() final;
		void Update(const Clip& c) override;
		void Update() override;

		virtual void DrawContent();

		//
		// 位置からWidgetを探索
		//
		Found Find(
			const TB::Vector<float, 2>& pos,
			const TB::Vector<float, 2>& direction,
			float z) override;

		/** アクセサ
		 */
		const TB::Vector<unsigned, 2>& Size(){ return size; };

		/** 属性アクセサ
		 */
		bool HasContent(){
			return !!(attributes & hasContent);
		};
		const Clip& GetClip(){ return clip; };

	private:
		TB::Vector<unsigned, 2> size;
		unsigned char color[4];
		Clip clip;

		void NotifySizeIfContent();
	};

}
