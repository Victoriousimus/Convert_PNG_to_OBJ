#include <iostream>
#include <fstream>
#include <iomanip> // Необходима для работы с setprecision
#include "stbi_png.h"

class CMapParcer {
	private:
		struct SPoint {
			i_4b id; f_4b x; f_4b y; f_4b z;
			SPoint() : x(0), y(0), z(0), id(0){}
			SPoint(f_4b _x, f_4b _y, f_4b _z) : x(_x), y(_y), z(_z), id(0) {}
		};
		struct SHmap {
			private:
				SPoint** map{nullptr};
				Bool empty{true};
				i_4b size{ 0 };
			public:
				SHmap(i_4b _size) {
					map = nullptr;
					empty = true;
					size = _size;
					map = new SPoint*[size];
					for (i_4b i = 0; i < size; ++i) {
						map[i] = new SPoint[size];
					}
					empty = false;
				}
				~SHmap() {
					for (i_4b i = 0; i < size; ++i) {
						delete[] map[i];
					}
					delete[] map;
				}
				SPoint* p(i_4b _x, i_4b _y) {
					if (empty) return nullptr;
					if (_x > size && _x < 0 && _y > size && _y < 0) return nullptr;
					return &map[_x][_y];
				}
		};
		stbi_png::CImageRGB image_;
		SHmap*** hmap_;
		i_4b regSize;
		i_4b regCountX;
		i_4b regCountY;
		Bool is_bilded_;

		__forceinline void set_vert(i_4b x,i_4b y, f_4b pix_size) {
			i_4b pX = x % regSize;
			i_4b pY = y % regSize;
			u_1b color[3]{ 0 };
			SPoint& vert = *(hmap_[y / regSize][x / regSize]->p(pY, pX));
			vert.id = image_.getPix(x, y, &color);
			vert.x = pix_size * static_cast<f_4b>(pX);
			vert.y = pix_size * static_cast<f_4b>(pY);
			vert.z = pix_size * (
				(static_cast<f_4b>(color[2]) / 256.0f) +
				(static_cast<f_4b>(color[1]) / 512.0f) +
				(static_cast<f_4b>(color[0]) / 1024.0f));
		}

		__forceinline void set_side(i_4b x, i_4b y, f_4b pix_size) {
			if ((x % regSize) >0 || (y % regSize) > 0) return;
			SHmap& map = *(hmap_[y / regSize][x / regSize]);
			SPoint* v;
			i_4b max_X = x + regSize;
			i_4b max_Y = y + regSize;
			u_1b color[3]{ 0 };
			for (i_4b i = 0; i <= regSize; ++i) {
				v = map.p(i, regSize);
				v->id = image_.getPix(max_X, y + i, &color);
				v->x = pix_size * static_cast<f_4b>(regSize);
				v->y = pix_size * static_cast<f_4b>(i);
				v->z = pix_size * (
					(static_cast<f_4b>(color[2]) / 256.0f) +
					(static_cast<f_4b>(color[1]) / 512.0f) +
					(static_cast<f_4b>(color[0]) / 1024.0f));
				v = map.p(regSize, i);
				v->id = image_.getPix(x + i, max_Y, &color);
				v->x = pix_size * static_cast<f_4b>(i);
				v->y = pix_size * static_cast<f_4b>(regSize);
				v->z = pix_size * (
					(static_cast<f_4b>(color[2]) / 256.0f) +
					(static_cast<f_4b>(color[1]) / 512.0f) +
					(static_cast<f_4b>(color[0]) / 1024.0f));
			}
		}

	public:
		CMapParcer() : is_bilded_(false), hmap_(nullptr), regSize(0), regCountX(0), regCountY(0) {}
		~CMapParcer() {
			if (!is_bilded_) return;
			i_4b region_count_X = image_.getSizeX() / regSize;
			i_4b region_count_Y = image_.getSizeX() / regSize;
			for (i_4b i = 0; i < region_count_X; ++i) {
				for (i_4b j = 0; j < region_count_Y; ++j) {
					delete[] hmap_[i][j];
				}
				delete[] hmap_[i];
			}
			delete[] hmap_; 
			image_.free(); 
		}
		i_4b load(c_wrd file_name) {
			return image_.load(file_name);
		}
		i_4b bild(i_4b region_size, f_4b pix_size) {
			if (is_bilded_) return -10;
			regSize = region_size;
			regCountX = image_.getSizeX() / regSize;
			regCountY = image_.getSizeY() / regSize;
			if (regSize < 1) return -1;
			if (pix_size < 0) return -2;
			if (image_.getSizeX() < 1 || image_.getSizeY() < 1) return -3;
			hmap_ = new SHmap**[regCountY];
			for (i_4b x, y = 0; y < regCountY; ++y) {
				hmap_[y] = new SHmap * [regCountX];
				for (x = 0; x < regCountX; ++x) {
					hmap_[y][x] = new SHmap(regSize + 1); //+1
				}
			}
			i_4b Pix_X{}, Pix_Y{};
			while(Pix_Y < image_.getSizeY()) {
				set_side(Pix_X, Pix_Y, pix_size);
				set_vert(Pix_X, Pix_Y, pix_size);
				if( ++Pix_X >= image_.getSizeX()) {
					++Pix_Y; Pix_X = 0;
				}
			}
			is_bilded_ = true;
			return 0;
		}
		i_4b save_obj() {
			if (!is_bilded_) return -1;
			std::ofstream obj_File;
			t_sym fname[] = "results/hmap_x000y000.obj";
			for (i_4b y, x = 0; x < regCountX; ++x) {
				for (y = 0; y < regCountY; ++y) {
					sprintf_s(fname, sizeof(fname), "results/hmap_x%03dy%03d.obj", x, y);
					obj_File.open(fname); fname[21] = '\0';
					obj_File << "# HmapBilder" << std::endl;
					obj_File << "# Bild_png" << std::endl;
					obj_File << "o " << &fname[9] << std::endl;
					obj_File << std::fixed;
					for (i_4b yy, xx = 0; xx <= regSize; ++xx) {
						for (yy = 0; yy <= regSize; ++yy) {
							SPoint& vert = *(hmap_[x][y]->p(xx,yy));
							obj_File 
								<< "v " 
								<< std::setprecision(6) << vert.x << ' '
								<< std::setprecision(6) << vert.z << ' '
								<< std::setprecision(6) << vert.y << ' ' << std::endl;
						}
					}
					obj_File << "s 1" << std::endl;
					for (i_4b xx,yy = 0; yy < regSize; ++yy) {
						for (xx = 0; xx < regSize; ++xx) {
							obj_File << "f " 
								<< (((regSize+1) * (yy + 1) ) + xx + 1 ) << ' '
								<< (((regSize+1) *	yy		) + xx + 1 ) << ' '
								<< (((regSize+1) *	yy		) + xx + 2 )  << std::endl;
							obj_File << "f "
								<< (((regSize+1) *		 yy ) + xx + 2 ) << ' '
								<< (((regSize+1) * (yy + 1) ) + xx + 2 ) << ' '
								<< (((regSize+1) * (yy + 1)	) + xx + 1 ) << std::endl; 
						}
					}

					obj_File.close();
				}
			}
			return 0;
		}

		i_4b  print() {
			if (!is_bilded_) { std::cout << "\n\t\t!!! MAP IS NOT BILD !!!\n\n"; return -1; }
			for (i_4b y, x = 0; x < regCountX; ++x) {
				for (y = 0; y < regCountY; ++y) {
					t_sym reg_pos[] = "[ 000 x 000 ]";
					sprintf_s(reg_pos, sizeof(reg_pos), "[ %03d x %03d ]", x, y);
					std::cout << reg_pos << std::endl;
					for (i_4b yy, xx = 0; xx <= regSize; ++xx) {
						for (yy = 0; yy <= regSize; ++yy) {
							SPoint& vert = *(hmap_[x][y]->p(xx, yy));
							t_sym id_pos[] = "[000]";
							sprintf_s(id_pos, sizeof(id_pos), "[%03d]", vert.id);
							std::cout << id_pos;
						}
						std::cout << std::endl;
						std::cout << std::endl;
					}
					std::cout << std::endl << std::endl << std::endl;
				}
			}
			return 0;
		}
};

struct SVertx {
	i_4b id; f_4b x; f_4b y; f_4b z;
	SVertx() : x(0), y(0), z(0), id(0) {}
};
struct {
	c_wrd file_name;
	i_4b LOD_stride;
	f_4b pix_size;
	i_4b mapX;
	i_4b mapY;
} convert_parameters;
i_4b static wrd_comand(t_sym(*wrd)[128]) {
	i_4b cmd = 0;

	if ((*wrd[0] == 'e' || *wrd[0] == 'E')
		&& (*wrd[1] == 'x' || *wrd[1] == 'X')
		&& (*wrd[2] == 'i' || *wrd[2] == 'I')
		&& (*wrd[3] == 't' || *wrd[3] == 'T')) return -1;

	for (int i = 0; i < 124; ++i) {
		if (*wrd[i] == '\0') break;
		if (*wrd[i] == '-' && *wrd[i + 2] == ' ') {
			if (*wrd[i + 1] == 's') {
				for (int j = i; j < 124; ++j) {
					*wrd[j] = *wrd[j + 3];
					if (*wrd[j + 3] == '\0') break;
				}
				cmd += 1;
			}
			if (*wrd[i + 1] == 'v') {
				for (int j = i; j < 124; ++j) {
					*wrd[j] = *wrd[j + 3];
					if (*wrd[j + 3] == '\0') break;
				}
				cmd += 10;
			}
		}
	}
	return cmd;
}
i_4b Simple_Convert_PNG_to_OBJ() {
	stbi_png::CImageRGB image_;
	std::ofstream obj_File;
	t_sym fname[] = "map_results/hmap_LOD00_x000y000.obj";
	u_1b color[3]{ 0 };
	SVertx vert;
	i_4b size = image_.load(convert_parameters.file_name);
	sprintf_s(fname, sizeof(fname), 
		"map_results/hmap_LOD%02d_x%03dy%03d.obj", 
		convert_parameters.LOD_stride, 
		convert_parameters.mapX, 
		convert_parameters.mapY );

	if (size < 1) return -1;
	obj_File.open(fname);
	std::cout << "BILD_FILE:  " << fname << std::endl;
	fname[27] = '\0';
	obj_File << "# HmapBilder" << std::endl;
	obj_File << "# Bild_png" << std::endl;
	obj_File << "o " << &fname[9] << std::endl;
	obj_File << std::fixed;

	i_4b out_step;
	std::cout << "\n\nBILD_FILE_VERTEX:  " << fname
		<< "\n0%_________________________________________________________100%"
		<< std::endl;

	out_step = size / 62;
	for (i_4b x, y = 0; y < size; ++y) {
		for (x = 0; x < size; ++x) {
			vert.id = image_.getPix(x, y, &color);
			vert.x = convert_parameters.pix_size * static_cast<f_4b>(x);
			vert.y = convert_parameters.pix_size * static_cast<f_4b>(y);
			vert.z = convert_parameters.pix_size * (
				 static_cast<f_4b>(color[2]) +
				(static_cast<f_4b>(color[1]) / 256.0f) +
				(static_cast<f_4b>(color[0]) / 512.0f) );
			obj_File
				<< "v "
				<< std::setprecision(6) << vert.x << ' '
				<< std::setprecision(6) << vert.z << ' '
				<< std::setprecision(6) << vert.y << ' ' << std::endl;
		}
		if(y % out_step == 0) std::cout << '|';
	}
	std::cout << "\n\nBILD_FILE_INDEX:  " << fname
		<< "\n0%_________________________________________________________100%"
		<< std::endl;;

	obj_File << "s 1" << std::endl;

	out_step = (size-2) / 62;
	for (i_4b xx, yy = 0; yy < size-2; ++yy) {
		for (xx = 0; xx < size-2; ++xx) {
			obj_File << "f "
				<< (((size ) * (yy + 1)) + xx + 1) << ' '
				<< (((size ) * yy) + xx + 1) << ' '
				<< (((size ) * yy) + xx + 2) << std::endl;
			obj_File << "f "
				<< (((size ) *		yy ) + xx + 2) << ' '
				<< (((size ) * (yy + 1)) + xx + 2) << ' '
				<< (((size ) * (yy + 1)) + xx + 1) << std::endl;
		}
		if (yy % out_step == 0) std::cout << '|';
	}

	obj_File.close();

	std::cout << "\n\n\n" << std::endl;;
	return 0;
}

u_4b main(u_4b argc, t_wrd argv[]) {
	std::cout
		<< "\n-------------------------------------------------------------\n\t\t SATART_PROGRAM \n"
		<< std::endl;
	CMapParcer ParcMap;
	t_sym inp_txt[128]{ 0 };
	i_4b command{ 0 };
	i_4b valid{ 0 };

	convert_parameters.mapX = 0;
	convert_parameters.mapX = 0;
	convert_parameters.pix_size = 0.5f;
	convert_parameters.LOD_stride = 0;

	if (argv[1]) {
		std::cout << "\n> FILE NAME: " << argv[1] << std::endl;
		convert_parameters.file_name = argv[1];
		Simple_Convert_PNG_to_OBJ();
	}
	while (true) {
		std::cout << "\n> WRITE FILE NAME: ";
		std::cin >> inp_txt;
		command = wrd_comand(&inp_txt);
		if (command < 0) break;
		convert_parameters.file_name = inp_txt;
		Simple_Convert_PNG_to_OBJ();
		system("pause");
		inp_txt[0] = '\0';
		std::cout << "\n";
	}

	std::cout << "\n\n\t\t  END_PROGRAM \n";
	system("pause");
	return 0;
}
