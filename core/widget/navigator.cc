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

#include <memory.h>
#include <math.h>
#include <syslog.h>

#include <toolbox/image.h>

#include "navigator.h"

#define elementsOf(a) (sizeof(a) / sizeof(a[0]))



namespace core{

	Navigator* Navigator::instance(0);
	TB::Vector<float, 2> Navigator::center;
	float Navigator::roll(0);
	TB::VBO* Navigator::vbo(0);

	const float Navigator::radious(0.3);
	const float Navigator::depth(0.4);
	#define sideRatio (1.0f/(1.0f + 1.41421356237f))
	#define GP (radious * sideRatio)
	const float Navigator::innerRadious(
		radious - (2.0f * areaWidth * radious / size));

	const unsigned Navigator::indexes[] = {
		0,1,8,  8,1,9,
		9,1,2,  9,2,10,
		2,3,10, 10,3,11,
		3,4,11,  11,4,12,
		4,5,12,  12,5,13,
		5,6,13,  13,6,14,
		6,7,14,  14,7,15,
		7,0,15,  15,0,8,
	};

	TB::VBO::V_UV Navigator::vertexes[16] = {
		//外側(下の辺の左端から右回り))
		{ { -GP, -radious, -depth }, { 0.5 - 0.5*sideRatio, 0 } },
		{ { GP, -radious, -depth }, { 0.5 + 0.5*sideRatio, 0 } },
		{ { radious, -GP, -depth }, { 1, 0.5 - 0.5*sideRatio } },
		{ { radious, GP, -depth }, { 1, 0.5 + 0.5*sideRatio } },
		{ { GP, radious, -depth }, { 0.5 + 0.5*sideRatio, 1 } },
		{ { -GP, radious, -depth }, { 0.5 - 0.5*sideRatio, 1 } },
		{ { -radious, GP, -depth }, { 0, 0.5 + 0.5*sideRatio } },
		{ { -radious, -GP, -depth }, { 0, 0.5 - 0.5*sideRatio } },
		//内周(真下の頂点から右回り)
	};



	Navigator::Navigator() : texture(size, size, TB::Texture::RGBA){
		syslog(LOG_DEBUG, "Initialize Navigater");
		try{
			TB::Image image(size, size, true);

			//窓座標系のスケール取得

			//頂点生成(内側)
			for(unsigned n(0); n < 8; ++n){
				const float a(0.25 * n * M_PI);
				const float rx(sinf(a));
				const float ry(-cosf(a));
				TB::VBO::V_UV& v(vertexes[8 + n]);
				v.vertex.x = innerRadious * rx;
				v.vertex.y = innerRadious * ry;
				v.vertex.z = -depth;
				v.texture.u = 0.5 + 0.5*rx*innerRadious/radious;
				v.texture.v = 0.5 + 0.5*ry*innerRadious/radious;
			}

			//VBO確保
			vbo = TB::VBO::New(
				elementsOf(indexes), indexes,
				elementsOf(vertexes), vertexes);

			//imageに描画
			{ //NOTE:ここでブロックを作っているのはdelete imageより前にgcを消滅させるため
				TB::Image::Pen gc(image);
				TB::Image::Pen::FillColor fc(0);
				gc.Clear(fc);
				gc = fc;
				gc = TB::Image::Pen::StrokeColor(0x80000000);
				gc = areaWidth;
				gc.Arc(size*0.5, size*0.5, size*0.5 - 0.5*areaWidth, 0, 2.0 * M_PI);
			}

			//テクスチャに書き込む
			TB::Image::Raw raw(image);
			texture.Update(raw.data, 0, 0, size, size, TB::Texture::RGBA);

			//登録の類
			assert(!instance);
			instance = this;
			Core::RegisterStickies(*this);
		}
		catch(...){
			delete this;
		}
	}


	void Navigator::UpdateCenter(const TB::Vector<float, 2>& c){
		center = c;
	}


	void Navigator::DrawPoint(const TB::Vector<float, 2>& p, float d){
		TB::Vector<float, 2> point(p - center);

		//方向と距離に分離
		float distance(point.Length());
		point /= distance;

		//奥行きに対応するため距離を補正
		distance *= depth / d;

		//表示上の距離を計算
		const float w(2 * areaWidth * radious / size);
		const float i(radious - w);

		//表示領域の内側には表示しない
		if(distance <= i){
			return;
		}

		//距離を修正して方向と合成
		point *= radious - w/(distance - i + 1);

		glPointSize(3);
		glBegin(GL_POINTS);
		glVertex3f(point[0], -point[1], -depth + 0.0000001);
		glEnd();
	}



	void Navigator::DrawTransparent(){
		glAlphaFunc(GL_GREATER, 0);
		glEnable(GL_ALPHA_TEST);
		glDisable(GL_LIGHTING);
		const float r(radious);
		const float d(-depth);

		{
			TB::Texture::Binder b(texture);
			if(vbo){
				glDisable(GL_CULL_FACE);
				(*vbo).Draw();
			}else{
				glBegin(GL_TRIANGLE_STRIP);
				glTexCoord2f(0, 0);
				glVertex3f(-r, -r, d);
				glTexCoord2f(0, 1);
				glVertex3f(-r, r, d);
				glTexCoord2f(1, 0);
				glVertex3f(r, -r, d);
				glTexCoord2f(1, 1);
				glVertex3f(r, r, d);
				glEnd();
			}
		}

		/** 光点表示
		 */
		glPushMatrix();
		glRotatef(roll, 0, 0, 1);
		DrawAllPoints();
		glPopMatrix();
	}

}
