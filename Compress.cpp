#include "pch.h"
#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream> 
#include <math.h>
#include <string>
#include <stdlib.h>  
#include <filesystem>
#include <string>
#include <regex>
#include <map>
#include <algorithm> 
#include <fstream>


using namespace cv;
using namespace std;
namespace fs = filesystem;
struct dimension {
	string name;
	int height;
	int width;
};
vector<dimension> resolutions;

void fillResolutions() {
	dimension R4K = {"4K", 3840, 2160};
	resolutions.push_back(R4K);
	dimension R3K = {"3K", 3000, 2000 };
	resolutions.push_back(R3K);
	dimension R2K = {"2K", 2048, 1080 };
	resolutions.push_back(R2K);
	dimension R720p = {"720p", 1280, 720 };
	resolutions.push_back(R720p);
	dimension R480p = {"480p", 544, 480 };
	resolutions.push_back(R480p);
}

std::vector<std::string> explode(std::string const & s, char delim)
{
	std::vector<std::string> result;
	std::istringstream iss(s);

	for (std::string token; std::getline(iss, token, delim); )
	{
		result.push_back(std::move(token));
	}

	return result;
}

void copyFile(string source, string destination)
{
	std::ifstream src(source, std::ios::binary);
	std::ofstream dst(destination, std::ios::binary);
	dst << src.rdbuf();
}

Mat applyFilter(string path, string filter, int intensity) {
	Mat image = imread(path);
	Mat imageFiltered;
	if (filter.compare("blur") == 0){
		if (intensity > 0)
			blur(image, imageFiltered, Size(intensity, intensity));
		else
			imageFiltered = image;
	}

	return imageFiltered;
}

bool lowerQuality() {

	regex number("(.+)(_O.tif)");
	string basePath = "images/";
	string originalPath = "images/originals/";

	for (auto& p : fs::directory_iterator(originalPath)) {
		string filePath = p.path().u8string();
		string fileName = explode(explode(filePath, '/').back(),'.')[0];

		if (regex_match(filePath, number)) {
			Mat imageOriginal = imread(filePath);
			
			
			for (int i = 0; i <= 100; i += 20) {
				cout << "Compressing image " << fileName << " of " << i << "%" << endl;
				std::vector<int> params;
				params.push_back(CV_IMWRITE_JPEG_QUALITY);
				params.push_back(100 - i + i%100); 
				string format = (i==0) ? ".tif" : ".jpg";
				string newPath = basePath + "compressed/"+to_string(i)+"/" + fileName + format;

				if (i == 0)
					copyFile(filePath, newPath);
				else
					imwrite(newPath, imageOriginal, params);

				for (int intensity = 10; intensity <= 80; intensity += 10) {
					cout << "Blurring image " << fileName << " with intensity " << intensity << endl;
					Mat imageFiltered = applyFilter(newPath, "blur", intensity);
					string newPathFiltered = basePath + "compressed/" + to_string(i) + "/" + explode(fileName, '_')[0] + "_B" + to_string(intensity/10) + format;
					imwrite(newPathFiltered, imageFiltered);
				}
				
			}

			for (dimension resolution : resolutions){
				cout << "Resizing image " << fileName << " to " << resolution.name << endl;
				Size size(resolution.height, resolution.width);
				Mat resizedImage;
				resize(imageOriginal, resizedImage, size);
				string newPath = basePath + "resized/" + resolution.name + "/" + fileName + ".tif";

				if (resolution.name.compare("4K") == 0) 
					copyFile(filePath, newPath);
				else
					imwrite(newPath, resizedImage);

				for (int intensity = 10; intensity <= 80; intensity += 10) {
					cout << "Blurring image " << fileName << " with intensity " << intensity << endl;
					Mat imageFiltered = applyFilter(newPath, "blur", intensity);
					string newPathFiltered = basePath + "resized/" + resolution.name + "/" + explode(fileName, '_')[0] + "_B" + to_string(intensity / 10) + ".tif";
					imwrite(newPathFiltered, imageFiltered);
				}
			}
			
			
			
	
		}
	}
	return true;
}
void cleanDirectories() {
	uintmax_t n = fs::remove_all("images/compressed");
	n = fs::remove_all("images/resized");
	fs::create_directories("images/resized");
	fs::create_directories("images/compressed");
	for (int i = 0; i <= 100; i += 20){
		fs::create_directories("images/compressed/"+to_string(i));
	}
	for (dimension resolution : resolutions) {
		fs::create_directories("images/resized/" + resolution.name);
	}
}
int main()
{	

	fillResolutions();
	while (1) {
		int answer;
		cout << "What do you want to do? Generate(1)/Clean(2)/Exit(3)" << endl;
		cin >> answer;

		if (answer == 1) {
			lowerQuality();
			cout << "Images generated successfully" << endl;
		}
		else if (answer == 2){
			cleanDirectories();
			cout << "Directory cleaned" << endl;
		}else break;
	}
	return 0;
}
