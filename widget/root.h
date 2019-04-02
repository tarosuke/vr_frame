/** Root
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
#pragma once

#include <toolbox/prefs.h>
#include <toolbox/geometry/vector.h>
#include <toolbox/complex/complex.h>

#include <widget.h>
#include "navigator.h"
#include "../core/pose.h"



namespace vr_core{

	class Root : public Widget, public Navigator{
	public:
		Root() : Widget(this){ instance = this; };

		//Coreから呼ばれるための特別なハンドラ
		static void SetView(COMPLEX<4>);
		static void DrawAll();
		static void DrawTransparentAll();
		static void UpdateAll();

		//キーイベントのためのハンドラ
		static void OnKey(const KeyEvent&);

		// center,directionで表される直線がdepthに達する点を計算
		static TB::Vector<float, 2> GetOnPos(
			const TB::Vector<float, 2>& center,
			const TB::Vector<float, 2>& direction,
			float depth);

		//Rootはシングルトンなのでそのインスタンス
		static Root* instance;

	private:
		static const float baseDistance;
		static const float depthScale;
		static const float scale;

		static TB::Vector<float, 2> lookingPoint;
		static bool lookingFront;
		static double roll;

		static TB::Prefs<bool> restoreWidgetRotation;

		void DrawAllPoints() final{ Widget::DrawPoint(); };


		TB::Vector<float, 3> SightCast(
			const TB::Vector<float, 3>&, float newDepth);

		static void UpdateLookingPoint(const COMPLEX<4>&);

	};

}