/** Particles
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

#include <toolbox/gl/texture.h>
#include <toolbox/gl/displayList.h>

#include <module.h>



namespace vr_core{

	class Particles : public Module{
	public:
	protected:
		/** 個別のパーティクル情報
		 * VBOで流すために全体をひとつのバッファにする
		 */
		struct Element{
			union{
				struct{
					float x;
					float y;
					float z;
				};
				float raw[3];
			}position, velocity;
		}*const elements;

		/* コンストラクタ
		 */
		Particles(
			float size,
			const TB::Image& image,
			unsigned numOfParticles = 1000);

		/** 周回処理
		 * 描画が左右で分かれているのは左右独立でparticle用の設定が必要だから
		 * 内容は一緒
		 */
		void DrawLeft() override;
		void DrawRight() override;

		/** プロセッサ処理用の周回処理
		 * 不要ならオーバーライド
		 */
		virtual void UpdateElement(Element&, float delta);
		void Update(float) override;

	private:
		static const TB::Texture::Style spriteStyle;

		static const float minSize;

		static const float distanceAttenuation[];
		static const float resetAttenuation[];

		TB::Texture particle;
		GL::DisplayList displayList;

		const unsigned numOfParticles;
		float size;
	};

}
