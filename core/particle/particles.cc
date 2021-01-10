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



namespace core{

	/** 常数
	 */
	const float Particles::distanceAttenuation[] = { 1.0 / 65536, 1.0, 0 };
	const float Particles::resetAttenuation[] = { 1, 0, 0 };
	const float Particles::minSize(4);
	const TB::Texture::Style Particles::spriteStyle = {
		wrap_s : GL_CLAMP_TO_EDGE,
		wrap_t : GL_CLAMP_TO_EDGE,
		filter_mag : GL_LINEAR,
		filter_min : GL_LINEAR,
		texture_mode : GL_REPLACE,
		pointSprite : true,
	};

	/* コンストラクタ／デストラクタ
	 */
	Particles::Particles(
		float size,
		const TB::Image& image,
		unsigned numOfParticles) :
			elements(new Element[numOfParticles]),
			numOfParticles(numOfParticles),
			particle(image, spriteStyle),
			size(size){
		for(unsigned n(0); n < numOfParticles; ++n){
			//仮初期化
			Element& e(elements[n]);
			e.position.x =
			e.position.y =
			e.position.z =
			e.velocity.x =
			e.velocity.y =
			e.velocity.z = 0;
		}
	}

	Particles::~Particles(){
		delete elements;
	}

	/** 描画
	 */
	void Particles::Draw(){
		glPointParameterfv(GL_POINT_DISTANCE_ATTENUATION, distanceAttenuation);
		glPointParameterf(GL_POINT_FADE_THRESHOLD_SIZE, minSize);
		glDepthMask(GL_FALSE);
		glDisable(GL_LIGHTING);
		glEnable(GL_POINT_SPRITE);
		glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
		glBlendFunc(GL_ONE, GL_ONE);
		glPointSize(size);
		glColor3f(1, 1, 1);
		TB::Texture::Binder b(particle);
		glBegin(GL_POINTS);
		for(unsigned n(0); n < numOfParticles; ++n){
			glVertex3fv(elements[n].position.raw);
		}
		glEnd();
		glPointParameterfv(GL_POINT_DISTANCE_ATTENUATION, resetAttenuation);
		glPointParameterf(GL_POINT_FADE_THRESHOLD_SIZE, 1);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDepthMask(GL_TRUE);
	}

	/** プロセッサ処理用のアップデータ
	 */
	void Particles::UpdateElement(Element& e, float delta){
		e.position.x += e.velocity.x * delta;
		e.position.y += e.velocity.y * delta;
		e.position.z += e.velocity.z * delta;
	}
	void Particles::Update(){
		//移動
		for(unsigned n(0); n < numOfParticles; ++n){
			UpdateElement(elements[n], GetDelta());
		}

		//描画して記録
//		GL::DisplayList r(displayList);
	}


}
