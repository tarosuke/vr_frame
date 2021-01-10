/** Model
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

#include <toolbox/gl/vbo.h>
#include <toolbox/gl/texture.h>
#include <toolbox/image.h>

#include <module.h>



namespace core{

	// Abstract Model
	class Model : protected ExternalModule{
	public:

		//Model有効判定
		operator bool(){ return !!vbo; };

		//
		virtual ~Model();

	protected:
		//Model用パラメタ
		template<typename T> struct Params{
			unsigned numOfIndex;
			const unsigned* index;
			unsigned numOfVertex;
			const T* vertex;
		};

		//バッファタイプ別に作るとキリがないので
		template<typename T> Model(const T& p) :
			vbo(TB::VBO::New(
				p.numOfIndex,
				p.index,
				p.numOfVertex,
				p.vertex
			)){};


		TB::VBO* const vbo;
	private:
	};

	//頂点とカラーバッファのみのModel
	class Model_C : public Model{
	public:

	protected:
		using Params = Params<TB::VBO::V_UV>;

		Model_C(const Params&, const TB::Image&);
		void Draw() override;

	private:
		TB::Image::Raw rawImage;
		TB::Texture texture;
	};


}
