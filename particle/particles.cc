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
#include <particles.h>

#include <toolbox/gl/gl.h>



namespace vr_core{

	/** 常数
	 */
	const float Particles::distanceAttenuation[] = { 1.0 / 65536, 1.0, 0 };
	const float Particles::resetAttenuation[] = { 1, 0, 0 };
	const float Particles::minSize(4);
	const TB::Texture::Style Particles::spriteStyle = {
		wrap_s : GL_REPEAT,
		wrap_t : GL_REPEAT,
		filter_mag : GL_LINEAR,
		filter_min : GL_LINEAR,
		texture_mode : GL_BLEND,
		pointSprite : true,
	};

	/* コンストラクタ
	 */
	Particles::Particles(
		float size,
		const TB::Image& image,
		unsigned numOfParticles) :
			elements(new Element[numOfParticles]),
			particle(image),
			numOfParticles(numOfParticles){
		for(unsigned n(0); n < numOfParticles; ++n){
			//仮初期化
			auto e(elements[n]);
			e.position.x =
			e.position.y =
			e.position.z =
			e.velocity.x =
			e.velocity.y =
			e.velocity.z = 0;
		}
	}

	/** 描画
	 */
	void Particles::DrawLeft(){
		glPointParameterfv(GL_POINT_DISTANCE_ATTENUATION, distanceAttenuation);
		glPointParameterf(GL_POINT_FADE_THRESHOLD_SIZE, minSize);
		displayList.Playback();
		glPointParameterfv(GL_POINT_DISTANCE_ATTENUATION, resetAttenuation);
		glPointParameterf(GL_POINT_FADE_THRESHOLD_SIZE, 1);
	}
	void Particles::DrawRight(){
		DrawLeft();
	}

	/** プロセッサ処理用のアップデータ
	 */
	void Particles::UpdateElement(Element& e, float delta){
		e.position.x += e.velocity.x * delta;
		e.position.y += e.velocity.y * delta;
		e.position.z += e.velocity.z * delta;
	}
	void Particles::Update(float delta){
		for(unsigned n(0); n < numOfParticles; ++n){
			UpdateElement(elements[n], delta);
		}
	}


}
