// Alexander Decker
// Assignment 1
// CSCI 5607

#include <iostream>
#include <string>
#include <string.h>
#include <fstream>
#include <stdlib.h>
#include <time.h>
#include <sstream>
#include <vector>
#include <cmath>
#include <float.h>
#include <algorithm>

using namespace std;

double PI = 3.14159;
bool parallel = false;

// ** Data types **
struct vector3 {
	vector3() {}
	vector3(double ix, double iy, double iz) {
		x = ix; y = iy; z = iz;
	}
	double x, y, z;
	double getLength() { return sqrt((x * x) + (y * y) + (z * z)); }
};

vector<vector3> verticies;
vector<vector3> surfaceNormals;
vector<vector3> textCoords;

struct light {
	int w;
	float x, y, z, r, g, b;
};

struct color {
	color() {}
	color(int r, int g, int b) {red = r; green = g; blue = b;}
	int red, blue, green;
}; 

struct texture {
	int height, width;
	vector<color> image;
};

vector<texture> textures;

struct mtlcolor {
	int Odr, Odg, Odb, Osr, Osg, Osb;
	float ka, kd, ks, n, op, ior;
};

struct sphere {
	int id, text_ind;
	double cx, cy, cz, r;
	mtlcolor col;
};

struct window_size {
	int height, width;
};

struct triangle {
	triangle() {
		vector3 zero = vector3(0,0,0);
		n1 = zero; n2 = zero; n3 = zero;
		zero = vector3(-1,-1,0);
		vt1 = zero; vt2 = zero; vt3 = zero;
	}
	vector3 v1, v2, v3, n1, n2, n3, vt1, vt2, vt3;
	int id, text_ind;
	mtlcolor col;
};

vector<triangle> triangles;

struct file_contents {
	file_contents() {
		size.width = 0;
		size.height = 0;
		eye = vector3(DBL_MAX, DBL_MAX, DBL_MAX);
		viewdir = vector3(DBL_MAX, DBL_MAX, DBL_MAX);
		updir = vector3(DBL_MAX, DBL_MAX, DBL_MAX);
		fov = DBL_MAX;
		bkgcolor.red = -1;
	}

	window_size size;
	vector3 eye, viewdir, updir;
	double fov;
	color bkgcolor;
	vector<sphere> spheres;
	vector<light> lights;
};

color background;

// ** Vector arithmetic functions **

// Dot two matricies
double dot(vector3 v1, vector3 v2) {
	return (v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z);
}

// Negate a vector
vector3 neg(vector3 v1) {
	vector3 ret (v1.x * -1.0, v1.y * -1.0, v1.z * -1.0);
	return ret;
}

// Scale a vector by a constant
vector3 scale(vector3 v1, double c) {
	vector3 ret(1.0 * v1.x * c, 1.0 * v1.y * c, 1.0 * v1.z * c);
	return ret;
}

// Normalize a matrix
vector3 normalize(vector3 v) {
	double length = sqrt((v.x * v.x) + (v.y * v.y) + (v.z * v.z));
	vector3 ret(v.x / length, v.y / length, v.z / length);

	if (length != 0) {
		return ret;
	} else {
		return v;
	}
}

// Cross two matricies
vector3 cross(vector3 v1, vector3 v2) {
	vector3 ret(((1.0 * v1.y * v2.z) - (1.0 * v1.z * v2.y)),
				((1.0 * v1.z * v2.x) - (1.0 * v1.x * v2.z)), 
				((1.0 * v1.x * v2.y) - (1.0 * v1.y * v2.x)));

	return ret;
}

// Subtract a matrix from another
vector3 sub(vector3 v1, vector3 v2) {
	vector3 ret(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);
	return ret;
}

// Add two vectors
vector3 add(vector3 v1, vector3 v2) {
	vector3 ret(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);
	return ret;
}

// Checks if value is a number
bool is_num (string input) {
	double num = 0;
	istringstream strg(input);
		if ((strg >> num).fail()) {
    		return false;
		}
	return true;
}

// Function to check if file is empty
bool file_empty (string filename) {
	ifstream image_file(filename);

	image_file.seekg(0, ios::end);

	if (image_file.tellg() == 0) {
		cout << "Input file is empty, Exiting...";
		return true;
	}
	image_file.close();

	return false;
}

// ** Parsing functions **

// Parse texture
texture parse_texture(string filename) {
	texture ret;
	istringstream info(filename);
	string temp;
	info >> temp; info >> temp;
	
	ifstream texture_file(temp);

	if (!texture_file.good()) {
		cout << "Specified texture file does not exist... Exiting!" << endl;
		exit(1); }

	// Check if file is empty
	if (file_empty(filename)) {
		cout << "Specified texture file is empty... Exiting!" << endl;
		exit(1); }

	color setter;
	string s;
	bool header = true;

	while (getline(texture_file, s)) {
		istringstream info(s);
		string keyword;

		if (header) {
			info >> keyword;
			info >> keyword;
			ret.width = stoi(keyword);
			info >> keyword;
			ret.height = stoi(keyword);
			header = false; }
		else {
			info >> keyword;
			setter.red = stoi(keyword);
			info >> keyword;
			setter.green = stoi(keyword);
			info >> keyword;
			setter.blue = stoi(keyword);
			ret.image.push_back(setter);
		}
	}
	return ret;

	texture_file.close();
}

// Parse sphere info from a string and a color
sphere parse_sphere(string s, mtlcolor c) {
	istringstream info(s);
	string temp;
	info >> temp; info >> temp;
	sphere ret;
	ret.col = c;
    
	ret.cx = stod(temp);
	if (!is_num(temp)) {cout << "Sphere cx must be a number Exiting..." << endl; exit(1);}
	info >> temp;
	ret.cy = stod(temp);
	if (!is_num(temp)) {cout << "Sphere cy must be a number Exiting..." << endl; exit(1);}
	info >> temp;
	ret.cz = stod(temp);
	if (!is_num(temp)) {cout << "Sphere cz must be a number Exiting..." << endl; exit(1);}
	info >> temp;
	ret.r = stod(temp);

	return ret;
}

// Parse height and width from string
window_size parse_imsize(string s) {
	istringstream info(s);
	string temp;
	info >> temp; info >> temp;
	window_size ret;
	
	if (!is_num(temp)) {cout << "Width must be a number, Exiting..." << endl; exit(1);}
	ret.width = stod(temp);
	if (ret.width <= 0) {cout << "Width must be at least 1" << endl; exit(1);}
	info >> temp;
	if (!is_num(temp)) {cout << "Height must be a number, Exiting..." << endl; exit(1);}
	ret.height = stod(temp);
	if (ret.height <= 0) {cout << "Height must be at least 1" << endl; exit(1);}

	return ret;
}

// Parse light from string
light parse_light(string s) {
	istringstream info(s);
	string temp;
	info >> temp; info >> temp;
	light ret;
	
	if (!is_num(temp)) {cout << "Light x position must be a number, Exiting..." << endl; exit(1);}
	ret.x = stod(temp);
	info >> temp;
	if (!is_num(temp)) {cout << "Light y position must be a number, Exiting..." << endl; exit(1);}
	ret.y = stod(temp);
	info >> temp;
	if (!is_num(temp)) {cout << "Light z position must be a number, Exiting..." << endl; exit(1);}
	ret.z = stod(temp);
	info >> temp;
	if (!is_num(temp)) {cout << "Light w must be a number, Exiting..." << endl; exit(1);}
	ret.w = stoi(temp);
	info >> temp;
	if (!is_num(temp)) {cout << "Colors must have numerical values, Exiting..." << endl; exit(1);}
	ret.r = stod(temp);
	if ( ret.r > 1 || ret.r < 0) {cout << "Colors must have value between 1 and 0" << endl; exit(1);}
	info >> temp;
	if (!is_num(temp)) {cout << "Colors must have numerical values, Exiting..." << endl; exit(1);}
	ret.g = stod(temp);
	if ( ret.g > 1 || ret.g < 0) {cout << "Colors must have value between 1 and 0" << endl; exit(1);}
	info >> temp;
	if (!is_num(temp)) {cout << "Colors must have numerical values, Exiting..." << endl; exit(1);}
	ret.b = stod(temp);
	if ( ret.b > 1 || ret.b < 0) {cout << "Colors must have value between 1 and 0" << endl; exit(1);}

	if (ret.w == 0 && vector3(ret.x, ret.y, ret.z).getLength() == 0) {
		cout << "Directional light cannot be 0 vector, Exiting..." << endl;
		exit(1);
	}

	return ret;
}

// Parses a string into an rgb color object
color parse_bg_color(string s) {
	istringstream info(s);
	string temp;
	info >> temp; info >> temp;
	color ret;
	
	if (!is_num(temp)) {cout << "Colors must have numerical values, Exiting..." << endl; exit(1);}
	ret.red = round(stod(temp) * 255);
	if ( ret.red > 255 || ret.red < 0) {cout << "Colors must have value between 1 and 0" << endl; exit(1);}
	info >> temp;
	if (!is_num(temp)) {cout << "Colors must have numerical values, Exiting..." << endl; exit(1);}
	ret.green = round(stod(temp) * 255);
	if ( ret.green > 255 || ret.green < 0.0) {cout << "Colors must have value between 1 and 0" << endl; exit(1);}
	info >> temp;
	if (!is_num(temp)) {cout << "Colors must have numerical values, Exiting..." << endl; exit(1);}
	ret.blue = round(stod(temp) * 255);
	if ( ret.blue > 255.0 || ret.blue < 0.0) {cout << "Colors must have value between 1 and 0" << endl; exit(1);}

	return ret;
}

// parse mtl color from a string
mtlcolor parse_mtl_color(string s) {
	istringstream info(s);
	string temp;
	info >> temp; info >> temp;
	mtlcolor ret;
	
	if (!is_num(temp)) {cout << "Colors must have numerical values, Exiting..." << endl; exit(1);}
	ret.Odr = round(stod(temp) * 255);
	if ( ret.Odr > 255 || ret.Odr < 0) {cout << "Colors must have value between 1 and 0" << endl; exit(1);}
	info >> temp;
	if (!is_num(temp)) {cout << "Colors must have numerical values, Exiting..." << endl; exit(1);}
	ret.Odg = round(stod(temp) * 255);
	if ( ret.Odg > 255 || ret.Odg < 0) {cout << "Colors must have value between 1 and 0" << endl; exit(1);}
	info >> temp;
	if (!is_num(temp)) {cout << "Colors must have numerical values, Exiting..." << endl; exit(1);}
	ret.Odb = round(stod(temp) * 255);
	if ( ret.Odb > 255 || ret.Odb < 0) {cout << "Colors must have value between 1 and 0" << endl; exit(1);}
	info >> temp;
	if (!is_num(temp)) {cout << "Colors must have numerical values, Exiting..." << endl; exit(1);}
	ret.Osr = round(stod(temp) * 255);
	if ( ret.Osr > 255 || ret.Osr < 0) {cout << "Colors must have value between 1 and 0" << endl; exit(1);}
	info >> temp;
	if (!is_num(temp)) {cout << "Colors must have numerical values, Exiting..." << endl; exit(1);}
	ret.Osg = round(stod(temp) * 255);
	if ( ret.Osg > 255 || ret.Osg < 0) {cout << "Colors must have value between 1 and 0" << endl; exit(1);}
	info >> temp;
	if (!is_num(temp)) {cout << "Colors must have numerical values, Exiting..." << endl; exit(1);}
	ret.Osb = round(stod(temp) * 255);
	if ( ret.Osb > 255 || ret.Osb < 0) {cout << "Colors must have value between 1 and 0" << endl; exit(1);}
	info >> temp;
	if (!is_num(temp)) {cout << "Ka must have a numerical value, Exiting..." << endl; exit(1);}
	ret.ka = stod(temp);
	info >> temp;
	if (!is_num(temp)) {cout << "Kd must have a numerical value, Exiting..." << endl; exit(1);}
	ret.kd = stod(temp);
	info >> temp;
	if (!is_num(temp)) {cout << "Ks must have a numerical value, Exiting..." << endl; exit(1);}
	ret.ks = stod(temp);
	info >> temp;
	if (!is_num(temp)) {cout << "N must have a numerical value, Exiting..." << endl; exit(1);}
	ret.n = stoi(temp);
	info >> temp;
	if (!is_num(temp)) {cout << "Opacity must have a numerical value, Exiting..." << endl; exit(1);}
	ret.op = stod(temp);
	info >> temp;
	if (!is_num(temp)) {cout << "Index of refraction must have a numerical value, Exiting..." << endl; exit(1);}
	ret.ior = stod(temp);

	return ret;
}

// Parses a string into a vector3 object
vector3 parse_vector(string s) {
	istringstream info(s);
	vector3 ret;
	string temp;
	info >> temp; info >> temp;
	
	if (!is_num(temp)) {cout << "Vectors must have numerical values, Exiting..." << endl; exit(1);}
	ret.x = stod(temp);
	info >> temp;
	if (!is_num(temp)) {cout << "Vectors must have numerical values, Exiting..." << endl; exit(1);}
	ret.y = stod(temp);
	info >> temp;
	if (!is_num(temp)) {cout << "Vectors must have numerical values, Exiting..." << endl; exit(1);}
	ret.z = stod(temp);

	return ret;
}

//gets a triangle
triangle parse_triangle(string s) {
	istringstream info(s);
	triangle ret;
	string temp;
	info >> temp; info >> temp;

	if (temp.find("//") != std::string::npos) {
    	ret.v1 = verticies.at(stoi(temp.substr(0, temp.find("//"))) - 1);
    	temp.erase(0, temp.find("//") + 2);
    	ret.n1 = surfaceNormals.at(stoi(temp) -1);
    	info >> temp;
    	ret.v2 = verticies.at(stoi(temp.substr(0, temp.find("//"))) - 1);
    	temp.erase(0, temp.find("//") + 2);
    	ret.n2 = surfaceNormals.at(stoi(temp) - 1);
    	info >> temp;
    	ret.v3 = verticies.at(stoi(temp.substr(0, temp.find("//"))) - 1);
    	temp.erase(0, temp.find("//") + 2);
    	ret.n3 = surfaceNormals.at(stoi(temp) - 1);
    	ret.vt1.x = DBL_MAX;
	}
	else if(temp.find("/") != std::string::npos) {
		ret.v1 = verticies.at(stoi(temp.substr(0, temp.find("/"))) - 1);
    	temp.erase(0, temp.find("/") + 1);
    	ret.vt1 = textCoords.at(stoi(temp.substr(0, temp.find("/"))) - 1);
    	temp.erase(0, temp.find("/") + 1);
    	ret.n1 = surfaceNormals.at(stoi(temp) - 1);
    	info >> temp;
    	ret.v2 = verticies.at(stoi(temp.substr(0, temp.find("/"))) - 1);
    	temp.erase(0, temp.find("/") + 1);
    	ret.vt2 = textCoords.at(stoi(temp.substr(0, temp.find("/"))) - 1);
    	temp.erase(0, temp.find("/") + 1);
    	ret.n2 = surfaceNormals.at(stoi(temp) - 1);
    	info >> temp;
    	ret.v3 = verticies.at(stoi(temp.substr(0, temp.find("/"))) - 1);
    	temp.erase(0, temp.find("/") + 1);
    	ret.vt3 = textCoords.at(stoi(temp.substr(0, temp.find("/"))) - 1);
    	temp.erase(0, temp.find("/") + 1);
    	ret.n3 = surfaceNormals.at(stoi(temp) - 1);
	}
	else {
		ret.v1 = verticies.at(stoi(temp) - 1); info >> temp;
		ret.v2 = verticies.at(stoi(temp) - 1); info >> temp;
		ret.v3 = verticies.at(stoi(temp) - 1);
		ret.n1 = vector3(0,0,0);
		ret.vt1.x = DBL_MAX;
	}
	return ret;
}

// Function to parse input file
file_contents parse_file(string filename) {
	ifstream image_file(filename);

	file_contents image_info;

	// Check if file exists
	if (!image_file.good()) {
		cout << "Specified file does not exist... Exiting!" << endl;
		exit(1); }

	// Check if file is empty
	if (file_empty(filename)) {
		cout << "Specified file is empty... Exiting!" << endl;
		exit(1); }

	string s;
	mtlcolor cur_col;
	cur_col.Odr = -1;
	int id = 0;
	int texture_index = -1;
	
	while (getline(image_file, s)) {
		istringstream info(s);
		string keyword;
		string fov;

		info >> keyword;

		// Parse file info by keyword
		if (keyword == "f") {
			triangle tempTri = parse_triangle(info.str());
			tempTri.id = id++;
			tempTri.text_ind = texture_index;
			tempTri.col = cur_col;
			triangles.push_back(tempTri);
		}
		else if (keyword == "v") {
			verticies.push_back(parse_vector(info.str()));
		}
		else if (keyword == "vt") {
			textCoords.push_back(parse_vector(info.str()));
		}
		else if (keyword == "vn") {
			vector3 i = parse_vector(info.str());
			if (i.getLength() == 0) {
				cout << "Invalid vector entered: length 0, exiting" << endl;
				exit(1);
			}
			surfaceNormals.push_back(i);
		}
		else if (keyword == "parallel") {
			parallel = true;
		}
		else if (keyword == "texture") {
			textures.push_back(parse_texture(info.str()));
			texture_index++;
		}
		else if (keyword == "eye") {
			if (image_info.eye.x != DBL_MAX) 
				{cout << "Two eye locations in input file, taking the later one..." << endl;}
			image_info.eye = parse_vector(info.str());
		}
		else if (keyword == "viewdir") {
			if (image_info.viewdir.x != DBL_MAX)
				{cout << "Two view directions in input file, taking the later one..." << endl;}
			vector3 i = parse_vector(info.str());
			if (i.getLength() == 0) {
				cout << "Invalid vector entered: length 0, exiting" << endl;
				exit(1);
			}
			image_info.viewdir = i;
		}
		else if (keyword == "updir") {
			if (image_info.updir.x != DBL_MAX) 
				{cout << "Two up directions in input file, taking the later one..." << endl;}
			vector3 i = parse_vector(info.str());
			if (i.getLength() == 0) {
				cout << "Invalid vector entered: length 0, exiting" << endl;
				exit(1);
			}
			image_info.updir = i;
		}
		else if (keyword == "fovh") {
			info >> fov;
			if (image_info.fov != DBL_MAX) 
				{cout << "Two field of view params in input file, taking the later one..." << endl;}
			if (stod(fov) < 0 || stod(fov) > 180)
				{cout << "Invalid field of view, must be between 0 and 180, Exiting..." << endl; exit(1);}
			image_info.fov = stod(fov);
		}
		else if (keyword == "light") {
			image_info.lights.push_back(parse_light(info.str()));
		}
		else if (keyword == "imsize") {
			if (image_info.size.width != 0) {cout << "Two imsize params in input file, taking the later one..." << endl;}
			image_info.size = parse_imsize(info.str());
		}
		else if (keyword == "bkgcolor") {
			if (image_info.bkgcolor.red != -1) {cout << "Two imsize params in input file, taking the later one..." << endl;}
			image_info.bkgcolor = parse_bg_color(info.str());
		}
		else if (keyword == "mtlcolor") {
			cur_col = parse_mtl_color(info.str());
		}
		else if (keyword == "sphere") {
			if (cur_col.Odr == -1) {
				cout << "Sphere declared before mtlcolor is set, Exiting..." << endl;
				exit(1);
			}
			sphere sp = parse_sphere(info.str() , cur_col); sp.id = id++; sp.text_ind = texture_index;
			image_info.spheres.push_back(sp);
		}
		else {
			
		}	
	}
	return image_info;
}

// Detect if there is an object between the point and the light
double shadow(vector3 o, vector3 dir, vector<sphere> sp, int id, double lightDist, vector<triangle> tris) {

	double ret = 1;

	for (sphere s: sp) {
		if (s.id != id) {
			double A = dot(dir, dir);
			vector3 temp((o.x-s.cx), (o.y-s.cy), (o.z-s.cz));
			double B = dot(scale(temp, 2.0), dir);
			double C = dot(temp, temp) - (s.r * s.r); 

			double discrim = (B*B) - (4.0 * A * C);

			if (discrim == 0) {
				double tempt = (-1.0 * B) / (2.0 * A);

				if (tempt > 0 && tempt < lightDist) {
					ret *= (1 - s.col.op);
				} 
			} else if (discrim > 0) {
				double tempt  = ((-1.0 * B) + sqrt(discrim)) / (2.0 * A);
				double tempt2 = ((-1.0 * B) - sqrt(discrim)) / (2.0 * A);

				if (tempt > 0 && tempt < lightDist) {
					ret *= (1 - s.col.op);
				}
				if (tempt2 > 0 && tempt2 < lightDist) {
					ret *= (1 - s.col.op);
				}
			}
		}
	}
	for (triangle tri: tris) {
		// Barycentric sadness
		if (tri.id != id) {
			float a = tri.v1.x - tri.v2.x; float b = tri.v1.y - tri.v2.y; float c = tri.v1.z - tri.v2.z; float d = tri.v1.x - tri.v3.x;
			float e = tri.v1.y - tri.v3.y; float f = tri.v1.z - tri.v3.z; float g = dir.x;               float h = dir.y;         float i = dir.z;
			float j = tri.v1.x - o.x;    float k = tri.v1.y - o.y;    float l = tri.v1.z - o.z;    float m = (e*i) - (h*f);
			float n = (g*f) - (d*i);       float o = (d*h) - (e*g);       float p = (a*k) - (j*b);       float q = (j*c) - (a*l); float r = (b*l) - (k*c);
			float s = (a*m) + (b*n) + (c*o);
			float beta  =  ((j*m) + (k*n) + (l*o))/ s;
			float gamma =  ((i*p) + (h*q) + (g*r))/ s;
			double tempt = (-1.0) * (((f*p) + (e*q) + (d*r))/s);
			// Check if there is a triangle intersect
			if(gamma > 0 && gamma < 1 && beta > 0 && beta < (1 - gamma)) {
				if (tempt > 0 && tempt < lightDist) {
					ret *= (1 - tri.col.op);
				}
			}
		}
	}

	return ret;
}

color getColor(vector3 ray, vector3 eye, vector<sphere> spheres, vector<light> lights, vector<triangle> tris, int recDepth, int id);

// Calc Phong Illum.
color calc_phong_sphere(sphere s, vector3 ray, vector3 eye, vector<light> lights, double t, vector<sphere> sph, vector<triangle> tris, int recDepth) {
    color ret;
    mtlcolor scol = s.col;
    vector3 insectPt = add(eye, scale(ray, t));
    vector3 N = normalize(sub(insectPt, vector3(s.cx, s.cy, s.cz)));

    // Calculate new Odr if the object happens to be textured
    if (s.text_ind > -1) { 
    	float phi   = acos(N.x);
    	float theta = atan2(N.z, N.y);

    	float v = phi / PI;
    	float u = (theta + PI) / (2 * PI);   	

    	int i = (int) (0.5 + u * (textures.at(s.text_ind).width - 1));
    	int j = (int) (0.5 + v * (textures.at(s.text_ind).height - 1));

    	scol.Odr = textures.at(s.text_ind).image.at(i + textures.at(s.text_ind).width * j).red;
    	scol.Odb = textures.at(s.text_ind).image.at(i + textures.at(s.text_ind).width * j).blue;
    	scol.Odg = textures.at(s.text_ind).image.at(i + textures.at(s.text_ind).width * j).green;
    }

    // Init colors with diffuse color
    ret.red = (scol.ka * scol.Odr);
    ret.blue = (scol.ka * scol.Odb);
    ret.green = (scol.ka * scol.Odg);
	
	for (light l: lights) {
		double lDist = DBL_MAX;
		// calculate L for directional light 
		vector3 L = normalize(neg(vector3(l.x, l.y, l.z)));

		// Change L and light distance if the light is a point light
		if (l.w == 1) { 
			L = normalize(sub(vector3(l.x, l.y, l.z), insectPt));
			lDist = sub(vector3(l.x, l.y, l.z), insectPt).getLength();
		 } 

		double S = shadow(insectPt, L, sph, s.id, lDist, tris);

		vector3 V = normalize(sub(eye, insectPt));
		vector3 H = normalize(scale(add(L , V) , (1.0 / (add(L,V).getLength()))));

		// Add the calculated diffuse and specular colors
		ret.red   +=  ((l.r) * S * ((scol.kd * scol.Odr * max(0.0,(dot(N,L)))) + (scol.ks * scol.Osr * (pow (max(0.0,dot(N,H)), scol.n)))));
		ret.green +=  ((l.g) * S * ((scol.kd * scol.Odg * max(0.0,(dot(N,L)))) + (scol.ks * scol.Osg * (pow (max(0.0,dot(N,H)), scol.n)))));
		ret.blue  +=  ((l.b) * S * ((scol.kd * scol.Odb * max(0.0,(dot(N,L)))) + (scol.ks * scol.Osb * (pow (max(0.0,dot(N,H)), scol.n)))));

		if (recDepth > 0)
		{
			recDepth--;
			float refAngle = dot(ray, N); 

			vector3 refVector = sub(ray, scale(N ,2 * refAngle));

			float F0 = ((s.col.ior-1)/(1+s.col.ior)) * ((s.col.ior-1)/(1+s.col.ior));
			//cout << F0 << endl;
			float FR = F0 + ((1 - F0) * pow(1-cos(refAngle), 5));
			
			color R = getColor(refVector, insectPt, sph, lights, tris, recDepth, s.id);

			ret.red   += (FR * (R.red * 1.0));
			ret.green += (FR * (R.green * 1.0));
			ret.blue  += (FR * (R.blue * 1.0)); 

			//N = neg(N);
			ray = neg(normalize(ray));

			// refraction code
			//ray inside the sphere
			vector3 refInside = add(scale(neg(N),  sqrt(1 - ((pow(1.0/s.col.ior, 2)) * (1 - pow (dot(N, ray) , 2))))), scale(sub(scale(N, dot(N, ray)), ray), (1.0/s.col.ior)));

			refInside = normalize(refInside);

			double A = dot(refInside, refInside);
			vector3 temp((insectPt.x-s.cx), (insectPt.y-s.cy), (insectPt.z-s.cz));
			double B = dot(scale(temp, 2.0), refInside);
			double C = dot(temp, temp) - (s.r * s.r); 

			double discrim = (B*B) - (4.0 * A * C);

			vector3 exitPt;

			if (discrim == 0) {
				double tempt = (-1.0 * B) / (2.0 * A);
				exitPt = insectPt;
				
			} else if (discrim > 0) {
				double tempt  = ((-1.0 * B) + sqrt(discrim)) / (2.0 * A);
				double tempt2 = ((-1.0 * B) - sqrt(discrim)) / (2.0 * A);

				exitPt = add(insectPt, scale(refInside , max(tempt,tempt2)));
			}

			vector3 sphereCenter = vector3(s.cx, s.cy, s.cz);

			vector3 exitN = normalize(sub(sphereCenter, exitPt)); // not sure if this should be negated or not

			//exitN = neg(exitN);

			double dist = sqrt( pow((exitPt.x - insectPt.x),2)
							  + pow((exitPt.y - insectPt.y),2) 
							  + pow((exitPt.z - insectPt.z),2));

			refInside = neg(refInside);

			// ray on the other side of the sphere
			vector3 refOutside = add(scale(neg(exitN),  sqrt(1 - ((pow(s.col.ior/1.0, 2)) * (1 - pow (dot(exitN, refInside) , 2))))), scale(sub(scale(exitN, dot(exitN, refInside)), refInside), (s.col.ior/1.0)));

			refOutside = normalize(refOutside);

			color T;

			// TIR Case
			if ((1 - (pow(((1 / s.col.ior)  * sin(refAngle)) , 2)) )  < 0 || refAngle > asin(s.col.ior / 1.0)) {
				T.red = 0;
				T.blue = 0;
				T.green = 0;
			} else {
					T = getColor(refOutside, exitPt, sph, lights, tris, recDepth, s.id);
				}

			ret.red += ((1 - FR) * (1 - s.col.op) * T.red * 1.0);
			ret.green += ((1 - FR) * (1 - s.col.op) * T.green * 1.0);
			ret.blue += ((1 - FR) * (1 - s.col.op) * T.blue * 1.0); 

			/*ret.red += ((1 - FR) * (exp(-1 * s.col.op * dist)) * T.red * 1.0);
			ret.green += ((1 - FR) * (exp(-1 * s.col.op * dist)) * T.green * 1.0);
			ret.blue += ((1 - FR) * (exp(-1 * s.col.op * dist)) * T.blue * 1.0); */

		}
		
	}
	ret.red = min(255,ret.red);
	ret.green = min(255,ret.green);
	ret.blue = min(255,ret.blue);
	return ret;
}

// Calculste phong color for triangle
color calc_phong_triangle(triangle t, vector3 dir, vector3 eye, vector<light> lights, vector<sphere> sph,
						  vector<triangle> tris, vector3 insectPt, float u, float v, vector3 N, int recDepth) {
	color ret;
	mtlcolor tcol = t.col;

	if (t.vt1.x != DBL_MAX) { 
    	int i = (int) (0.5 + u * (textures.at(t.text_ind).width - 1));
    	int j = (int) (0.5 + v * (textures.at(t.text_ind).height - 1));

    	tcol.Odr = textures.at(t.text_ind).image.at(i + textures.at(t.text_ind).width * j).red;
    	tcol.Odb = textures.at(t.text_ind).image.at(i + textures.at(t.text_ind).width * j).blue;
    	tcol.Odg = textures.at(t.text_ind).image.at(i + textures.at(t.text_ind).width * j).green;
    }

    ret.red = (tcol.ka * tcol.Odr);
    ret.blue = (tcol.ka * tcol.Odb);
    ret.green = (tcol.ka * tcol.Odg);

    for (light l: lights) {
		double lDist = DBL_MAX;
		// calculate L for directional light 
		vector3 L = normalize(neg(vector3(l.x, l.y, l.z)));

		// Change L and light distance if the light is a point light
		if (l.w == 1) { 
			L = normalize(sub(vector3(l.x, l.y, l.z), insectPt));
			lDist = sub(vector3(l.x, l.y, l.z), insectPt).getLength();
		} 
		// Check if the point is in shadow

		vector3 V = normalize(sub(eye, insectPt));
		vector3 H = normalize(scale(add(L , V) , (1.0 / (add(L,V).getLength()))));

		double S = shadow(insectPt, L, sph, t.id, lDist, tris);

		// Add the calculated diffuse and specular colors
		ret.red   += ((l.r) * ((tcol.kd * S * tcol.Odr * max(0.0,(dot(N,L)))) + (tcol.ks * tcol.Osr * (pow (max(0.0,dot(N,H)), tcol.n)))));
		ret.green += ((l.g) * ((tcol.kd * S * tcol.Odg * max(0.0,(dot(N,L)))) + (tcol.ks * tcol.Osg * (pow (max(0.0,dot(N,H)), tcol.n)))));
		ret.blue  += ((l.b) * ((tcol.kd * S * tcol.Odb * max(0.0,(dot(N,L)))) + (tcol.ks * tcol.Osb * (pow (max(0.0,dot(N,H)), tcol.n)))));

		if (recDepth > 0)
		{
			recDepth--;
			float refAngle = dot(dir, N); 

			vector3 refVector = sub(dir, scale(N ,2 * refAngle));

			float F0 = ((t.col.ior-1)/(1+t.col.ior)) * ((t.col.ior-1)/(1+t.col.ior));
			//cout << F0 << endl;
			float FR = F0 + (1 - F0) * ((1-cos(refAngle)) * (1-cos(refAngle)) * (1-cos(refAngle)) * (1-cos(refAngle)) * (1-cos(refAngle)));
			
			color R = getColor(refVector, insectPt, sph, lights, tris, recDepth, t.id);

			ret.red   += (int) (FR * (R.red * 1.0));
			ret.green += (int) (FR * (R.green * 1.0));
			ret.blue  += (int) (FR * (R.blue * 1.0));

			vector3 ray = neg(dir);

			vector3 refractVec = add(scale(neg(N),  sqrt(1 - ((pow(1.0/t.col.ior, 2)) * (1 - pow (dot(N, ray) , 2))))), scale(sub(scale(N, dot(N, ray)), ray), (1.0/t.col.ior)));

			color T = getColor(refractVec, insectPt, sph, lights, tris, recDepth, t.id);

			ret.red   += ((1 - FR) * (1 - t.col.op) * T.red * 1.0);
			ret.blue  += ((1 - FR) * (1 - t.col.op) * T.blue * 1.0);
			ret.green += ((1 - FR) * (1 - t.col.op) * T.green * 1.0);

		}
		
	}
	return ret;
}

// Calc color
color getColor(vector3 ray, vector3 eye, vector<sphere> spheres, vector<light> lights, vector<triangle> tris, int recDepth, int id) {
	// Init local variables
	double t = DBL_MAX;
	double tempt, tempt2;
	color ret;
	ret.red = -1;

	for (sphere s: spheres) {
		if (s.id != id) { // detect self collision
			double A = dot(ray, ray);
			vector3 temp((eye.x-s.cx), (eye.y-s.cy), (eye.z-s.cz));
			double B = dot(scale(temp, 2.0), ray);
			double C = dot(temp, temp) - (s.r * s.r); 

			// Calculate discriminant
			double discrim = (B*B) - (4.0 * A * C);

			// Single hit case
			if (discrim == 0) {
				// Calculate single intersect t
				tempt = (-1.0 * B) / (2.0 * A);

				if (tempt > 0 && tempt < t) {
					t = tempt;
					ret = calc_phong_sphere(s, ray, eye, lights, t, spheres, tris, recDepth);
				} 
			// Double hit case
			} else if (discrim > 0) {
				// Calculate both intersect t
				tempt  = ((-1.0 * B) + sqrt(discrim)) / (2.0 * A);
				tempt2 = ((-1.0 * B) - sqrt(discrim)) / (2.0 * A);

				if (tempt > 0 && tempt < t) {
					t = tempt;
					ret = calc_phong_sphere(s, ray, eye, lights, t, spheres, tris, recDepth);
				}
				if (tempt2 > 0 && tempt2 < t) {
					t = tempt2;
					ret = calc_phong_sphere(s, ray, eye, lights, t, spheres, tris, recDepth);
				}
			}
		}
	}
	for (triangle tri: tris) {
		if (tri.id != id) { // Detect self collision
			// Barycentric sadness
			float a = tri.v1.x - tri.v2.x; float b = tri.v1.y - tri.v2.y; float c = tri.v1.z - tri.v2.z; float d = tri.v1.x - tri.v3.x;
			float e = tri.v1.y - tri.v3.y; float f = tri.v1.z - tri.v3.z; float g = ray.x;               float h = ray.y;         float i = ray.z;
			float j = tri.v1.x - eye.x;    float k = tri.v1.y - eye.y;    float l = tri.v1.z - eye.z;    float m = (e*i) - (h*f);
			float n = (g*f) - (d*i);       float o = (d*h) - (e*g);       float p = (a*k) - (j*b);       float q = (j*c) - (a*l); float r = (b*l) - (k*c);
			float s = (a*m) + (b*n) + (c*o);
			float beta  =  ((j*m) + (k*n) + (l*o))/ s;
			float gamma =  ((i*p) + (h*q) + (g*r))/ s;
			float alpha = 1 - (beta + gamma);
			tempt = (-1.0) * (((f*p) + (e*q) + (d*r))/s);
			vector3 sect = add(eye, scale(ray, tempt));
			// Check if there is a triangle intersect
			if(gamma > 0 && gamma < 1 && beta > 0 && beta < (1 - gamma)) {
				if (tempt > 0 && tempt < t) {
					if (tri.n1.x == 0 && tri.n1.y == 0 && tri.n1.z == 0) {
						ret.red = tri.col.Odr;
						ret.green = tri.col.Odg;
						ret.blue = tri.col.Odb;
					} else {
						t = tempt;
						float u = (tri.vt1.x * alpha) + (tri.vt2.x * beta) + (tri.vt3.x * gamma);
						float v = (tri.vt1.y * alpha) + (tri.vt2.y * beta) + (tri.vt3.y * gamma);
						vector3 pointNorm = normalize(add(add(scale(tri.n1,alpha), scale(tri.n2,beta)), scale(tri.n3,gamma)));
						ret = calc_phong_triangle(tri, ray, eye, lights, spheres, tris, sect, u, v, pointNorm, recDepth);
					}
				}
			}
		}
	}
	if (ret.red == -1 && id != -1) {
		ret = background;
	}
	return ret;		
}

// Draws the output image
void draw_image(file_contents image_info) {
 	// Load file info
	vector3 eye = image_info.eye;
	vector3 viewdir = image_info.viewdir;
	vector3 updir = image_info.updir;
	double fov = image_info.fov;
	int pixwidth = image_info.size.width;
	int pixheight = image_info.size.height;
	color bkgcolor = image_info.bkgcolor;
	background = image_info.bkgcolor;
	vector<sphere> spheres = image_info.spheres;
	vector<light> lights = image_info.lights;

	// Calculate u and v
	vector3 u = normalize(cross( viewdir, updir));
	vector3 v = normalize(cross( u, viewdir));

	// Window eidth and height
	double ww = 2 * tan((fov/2.0) * (PI/180.0));
	double aspect = (pixwidth / pixheight);
	double wh = ww / aspect;

	// Init normal viewdir
	vector3 n = normalize(viewdir);

	// Calculate needed Corner points
	vector3 ul = sub( add( add (eye, n), scale(v, wh/2.0)), scale(u, ww/2.0));
	vector3 ur = add( add( add (eye, n), scale(v, wh/2.0)), scale(u, ww/2.0));
	vector3 ll = sub( sub( add (eye, n), scale(v, wh/2.0)), scale(u, ww/2.0));

	// Calc delta vectors
	vector3 deltah = scale( sub (ur, ul), (1.0 / (pixwidth - 1)));
	vector3 deltav = scale( sub (ll, ul), (1.0 / (pixheight - 1))); 

	vector<color> pixbuffer;

	//Draw Image
	for (int i = 0; i < pixheight; i++) {
		for (int j = 0; j < pixwidth; j++) {
			// Calculate screen point
			vector3 pixpoint = add( add( ul , scale( deltah , j)), scale( deltav, i));

			// Calculate the direction vector to the screen point
			vector3 pixvector = normalize(sub(pixpoint, eye));


			color pixcolor = bkgcolor;

			// Determine color of ray.
			if (!parallel) {pixcolor = getColor(pixvector, eye, spheres, lights, triangles, 10, -1); } //Cast perspective ray
			else {pixcolor = getColor(viewdir, pixpoint, spheres, lights, triangles, 10, -1); }        //Cast paralell ray

			// Print backroungd color if invalid color returned, otherwise print returned color
			if (pixcolor.red != -1) {
				pixbuffer.push_back(pixcolor);
			} else {			
				pixbuffer.push_back(bkgcolor);
			}
		}
	}
	// Create header
	ofstream outf("image.ppm");
	outf << "P3 " << " " << pixwidth << " " << pixheight << endl;
	outf << "255" << endl;

	for (color c: pixbuffer) {
		outf << c.red << " " << c.green << " " << c.blue << endl;
	}
	outf.close();
}

// Main
int main(int argc, char** argv) {

	if (argc != 2) {
		cout << "Incorrect number of arguments. Command should be of the form \"Raytracer <image_info_file>\", Exiting..." << endl;
		return 1;
	}
	// Get parsed file contents
	file_contents image_info = parse_file(argv[1]);
	// Draw image
	draw_image(image_info);

	return 0;
}