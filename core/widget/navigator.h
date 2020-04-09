/***** Window navigator
 * Copyright (C) 2018,2019 tarosuke<webmaster@tarosuke.net>
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

#include <toolbox/factory/factory.h>
#include <toolbox/geometry/vector.h>
#include <toolbox/gl/texture.h>
#include <toolbox/gl/vbo.h>

#include "../core/core.h"
#include <widget.h>



namespace core{

	class Navigator : public Module{
	public:
		/** Navigator表示の基本パラメタ
		 * 実スケール[m]
		 */
		static const float depth; //奥行き
		static const float radious;
		static const float innerRadious; //内径

		/** 輝点のためのメソッド
		 * UpdateCenter：当該フレームの中心座標を設定する
		 * DrawPoint：輝点を描画
		 */
		static void UpdateCenter(const TB::Vector<float, 2>&);
		static void UpdateRotation(float r){ roll = r; };
		static void DrawPoint(const TB::Vector<float, 2>&, float);

	protected:
		/** コンストラクタ
		 */
		Navigator();

		// 点を描画する(子クラスが)
		virtual void DrawAllPoints() = 0;

	private:

		static const unsigned size = 512;
		static const unsigned areaWidth = 25;
		static Navigator* instance;
		static TB::Vector<float, 2> center;
		static float roll;
		static const unsigned indexes[];
		static TB::VBO::V_UV vertexes[];
		static TB::VBO* vbo;

		/** テクスチャ
		 */
		TB::Texture texture;

		//Core::Module関連
		static FACTORY<Module> factory;
		static Module* New();
		void DrawTransparent() final;
	};

}
