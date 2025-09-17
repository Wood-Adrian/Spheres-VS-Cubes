#include "obj3d.hpp"

//TODO: this function is extremely hardcoded, but then again only a very specific obj is being considered right now
Obj3d::Obj3d(const std::string& filePath) {
	std::ifstream objStream(filePath);
	std::string inLine;
	std::string token;

	std::vector<guVector> vertexBuffer;
	std::vector<guVector> normalBuffer;
	std::vector<TexCoord> textureBuffer;

	std::vector<float> numberBuffer;

	while (std::getline(objStream, inLine)) {

		token = inLine.substr(0, inLine.find(' '));
		inLine.erase(0, inLine.find(' ') + 1);

		if (token.length() < 1) continue;

		if (token[0] == 'v') {
			while (inLine.length() > 0) {
				size_t nextSpace = inLine.find(' ');
				std::string nextNumberStr = inLine.substr(0, nextSpace);
				inLine.erase(0, nextSpace);	//so that the whole line is erased if nextSpace is max(size_t), i.e. to remove (just) the token
				inLine.erase(0, 1);			//to actually erase the next space in the line
				numberBuffer.push_back(std::stof(nextNumberStr));
			}

			if (token == "v") {
				vertexBuffer.push_back({ numberBuffer[0], numberBuffer[1], numberBuffer[2] });
			}
			else if (token == "vn") {
				normalBuffer.push_back({ numberBuffer[0], numberBuffer[1], numberBuffer[2] });
			}
			else if (token == "vt") {
				textureBuffer.push_back({ numberBuffer[0], numberBuffer[1] });
			}
			else {
				//what
			}
			numberBuffer.clear();
		}
		//TODO: this feels incredibly bodged
		else if (token == "f") {
			int indexes[9]{};

			//f 1/2/3 4/5/6 7/8/9	<- format of the f(ace) section in a .obj
			//  1/2/3 is vertexBuffer[0] / textureBuffer[1] / normalBuffer[2] etc.
			for (int i = 0; i < 3; i++) {
				size_t nextSpace = inLine.find(' ');
				std::string vertexInfo = inLine.substr(0, nextSpace);
				inLine.erase(0, nextSpace);
				inLine.erase(0, 1);

				for (int j = 0; j < 3; j++) {
					size_t nextSlash = vertexInfo.find('/');
					std::string vertexNumber = vertexInfo.substr(0, nextSlash);
					vertexInfo.erase(0, nextSlash);
					vertexInfo.erase(0, 1);

					indexes[(3 * i) + j] = (std::stoi(vertexNumber) - 1);
				}
			}

			//Face face = {	vertexBuffer[indexes[0]], vertexBuffer[indexes[3]], vertexBuffer[indexes[6]],
			//				normalBuffer[indexes[2]], normalBuffer[indexes[5]], normalBuffer[indexes[8]],
			//				textureBuffer[indexes[1]], textureBuffer[indexes[4]], textureBuffer[indexes[7]]
			//	};
			Face face{};
			face.vtxa = vertexBuffer[indexes[0]];
			face.vtxb = vertexBuffer[indexes[3]];
			face.vtxc = vertexBuffer[indexes[6]];
			face.nrma = normalBuffer[indexes[2]];
			face.nrmb = normalBuffer[indexes[5]];
			face.nrmc = normalBuffer[indexes[8]];
			face.txa = textureBuffer[indexes[1]];
			face.txb = textureBuffer[indexes[4]];
			face.txc = textureBuffer[indexes[7]];

			faces.push_back(face);
		}
		else {
			continue;
		}
	}
	vertexBuffer.clear();
	normalBuffer.clear();
	textureBuffer.clear();
}

Obj3d::~Obj3d() {
	faces.clear();
}


void Obj3d::DrawObject() {
	
	GX_Begin(GX_TRIANGLES, GX_VTXFMT0, faces.size() * 3); {

		for (size_t i = 0; i < faces.size(); i++) {

			GX_Position3f32(faces[i].vtxa.x, faces[i].vtxa.y, faces[i].vtxa.z);
			GX_Normal3f32(faces[i].nrma.x, faces[i].nrma.y, faces[i].nrma.z);
			GX_Color1u32(RGBA(0xff, 0xff, 0xff, 0xff));
			GX_TexCoord2f32(faces[i].txa.x, faces[i].txa.y);

			GX_Position3f32(faces[i].vtxb.x, faces[i].vtxb.y, faces[i].vtxb.z);
			GX_Normal3f32(faces[i].nrmb.x, faces[i].nrmb.y, faces[i].nrmb.z);
			GX_Color1u32(RGBA(0xff, 0xff, 0xff, 0xff));
			GX_TexCoord2f32(faces[i].txb.x, faces[i].txb.y);

			GX_Position3f32(faces[i].vtxc.x, faces[i].vtxc.y, faces[i].vtxc.z);
			GX_Normal3f32(faces[i].nrmc.x, faces[i].nrmc.y, faces[i].nrmc.z);
			GX_Color1u32(RGBA(0xff, 0xff, 0xff, 0xff));
			GX_TexCoord2f32(faces[i].txc.x, faces[i].txc.y);
		}

	} GX_End();
}