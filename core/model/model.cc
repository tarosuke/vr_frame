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

#include <model.h>



namespace vr_core{

	Model::~Model(){
		if(vbo){
			delete vbo;
		}
	}


	Model_C::Model_C(
		const Params& p,
		const TB::Image& image) :
		Model(p),
		rawImage(image),
		texture(
			rawImage.data,
			rawImage.width,
			rawImage.height,
			rawImage.transparent ? TB::Texture::RGBA : TB::Texture::RGB){}


	void Model_C::Draw(){
		if(!vbo){
			return;
		}

		//カラーバッファのセットアップ
		TB::Texture::Binder b(texture);

		//描画
		(*vbo).Draw();
	}
}
