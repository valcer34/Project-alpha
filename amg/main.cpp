#include <atlbase.h>
#include <msxml6.h>
#include <comutil.h>
#include <atlimage.h>
#include <fstream>
#include <future>
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include "asciipng.h"
using namespace std;

//string setting_url = "https://encrypted-tbn0.gstatic.com/images?q=tbn:ANd9GcQd6d_4H0OlGY3T3AdcUFYeJABuDt76dMkBBWR_Y_8XC54FTBOT";
//
//
int setting_width = 78;
unsigned long getPixelAVG(const CImage& image, int x, int y)
{
	auto pixel = image.GetPixel(x, y);
	auto b = (pixel >> 16) & 0xff;
	auto g = (pixel >> 8) & 0xff;
	auto r = pixel & 0xff;
	return (r * 2126 + g * 7152 + b * 722) / 10000;
}

unsigned long getPixelAVGFromASCII(int x, int y)
{
	auto pixel = asciipngdata[y * 760 + x];
	auto b = (pixel >> 16) & 0xff;
	auto g = (pixel >> 8) & 0xff;
	auto r = pixel & 0xff;
	return (r * 2126 + g * 7152 + b * 722) / 10000;
}

struct Char
{
	char c;
	union
	{
		float b[9];
		struct
		{
			float _0, _1, _2, _3, _4, _5, _6, _7, _8;
		};
	};
};

vector<Char> chars;

float getScore(const Char& ch, float* b)
{
	float score = 0.f;
		score += abs(b[0] - ch.b[0]) *6.f;//6 12 6 12 48 12 6 12 6
		score += abs(b[1] - ch.b[1]) *12.f;
		score += abs(b[2] - ch.b[2]) *6.f ;
		score += abs(b[3] - ch.b[3]) *12.f;
		score += abs(b[4] - ch.b[4]) *48.f;
		score += abs(b[5] - ch.b[5]) *12.f;
		score += abs(b[6] - ch.b[6])*6.f;
		score += abs(b[7] - ch.b[7])*12.f;
		score += abs(b[8] - ch.b[8])*6.f;
	return score;
}
void generateData()
{
	map<float, vector<char>> countToC;
	float maxB[9] = { 0 };
	for (auto c = ' '; c <= '~'; ++c)
	{
		auto offset = c - ' ';
		offset *= 8;
		int count[9] = { 0 };

		vector<float> brightness;

		
			// Top left
			for (auto y = 0; y < 3; ++y)
			for (auto x = offset; x < offset + 2; ++x)
			if (getPixelAVGFromASCII(x, y) > 150) ++count[0];

			// Top
			for (auto y = 0; y < 3; ++y)
			for (auto x = offset + 2; x < offset + 6; ++x)
			if (getPixelAVGFromASCII(x, y) > 150) ++count[1];

			// Top right
			for (auto y = 0; y < 3; ++y)
			for (auto x = offset + 6; x < offset + 8; ++x)
			if (getPixelAVGFromASCII(x, y) > 150) ++count[2];

			// Left
			for (auto y = 3; y < 9; ++y)
			for (auto x = offset + 0; x < offset + 2; ++x)
			if (getPixelAVGFromASCII(x, y) > 150) ++count[3];

			// Center
			for (auto y = 2; y < 10; ++y)
			for (auto x = offset + 1; x < offset + 7; ++x)
			if (getPixelAVGFromASCII(x, y) > 150) ++count[4];

			// Right
			for (auto y = 3; y < 9; ++y)
			for (auto x = offset + 6; x < offset + 8; ++x)
			if (getPixelAVGFromASCII(x, y) > 150) ++count[5];

			// Bottom left
			for (auto y = 9; y < 12; ++y)
			for (auto x = offset + 0; x < offset + 2; ++x)
			if (getPixelAVGFromASCII(x, y) > 150) ++count[6];

			// Bottom
			for (auto y = 9; y < 12; ++y)
			for (auto x = offset + 2; x < offset + 6; ++x)
			if (getPixelAVGFromASCII(x, y) > 150) ++count[7];

			// Bottom right
			for (auto y = 9; y < 12; ++y)
			for (auto x = offset + 6; x < offset + 8; ++x)
			if (getPixelAVGFromASCII(x, y) > 150) ++count[8];

			brightness = {
				(float)count[0] ,
				(float)count[1] ,
				(float)count[2] ,
				(float)count[3] ,
				(float)count[4] ,
				(float)count[5] ,
				(float)count[6] ,
				(float)count[7] ,
				(float)count[8] ,
			};
		

		for (int i = 0; i < 9; ++i)
		{
			maxB[i] = max<>(maxB[i], brightness[i]);
		}

		chars.push_back({ c, brightness[0], brightness[1], brightness[2], brightness[3], brightness[4], brightness[5], brightness[6], brightness[7] });
	}

	for (auto &ch : chars)
	{
		for (int i = 0; i < 9; ++i)
		{
			ch.b[i] /= maxB[i];
		}
	}
}

int main(int argc, const char* argv[])
{
	string setting_url;
	cout << "Input url-image(if==0-default):";
	cin >> setting_url;
	if (setting_url == "0")setting_url = "https://encrypted-tbn0.gstatic.com/images?q=tbn:ANd9GcQd6d_4H0OlGY3T3AdcUFYeJABuDt76dMkBBWR_Y_8XC54FTBOT";
	CoInitialize(nullptr);
	srand((unsigned int)time(0));
	generateData();
	HRESULT hr;
	CComPtr<IXMLHTTPRequest> request;
	hr = request.CoCreateInstance(CLSID_XMLHTTP60);
	hr = request->open(_bstr_t("GET"),_bstr_t(setting_url.c_str()),_variant_t(VARIANT_FALSE),_variant_t(),_variant_t());
	auto ret = async(launch::async, [&]
	{HRESULT hr;
	hr = request->send(_variant_t());
	});
	ret.wait();
	//get status - 200 if success
	long status;
	hr = request->get_status(&status);
	if (status==200)
	{
		// load image data (if URL points to an image)
		VARIANT responseVariant;
		hr = request->get_responseStream(&responseVariant);
		IStream* stream = (IStream*)responseVariant.punkVal;
		CImage image;
		image.Load(stream);
		stream->Release();

		auto w = image.GetWidth();
		auto h = image.GetHeight();
		float ratio = (float)w / (float)setting_width;
		auto ratioh = ratio * (12.f / 8.f);
		int sw = (int)((float)w / ratio);
		int sh = (int)((float)h / ratioh);

		unsigned long bAvg = 0;
		float fAvg = 0.f;
		
			for (int j = 0; j < image.GetHeight(); ++j)
			{
				for (int i = 0; i < image.GetWidth(); ++i)
				{
					bAvg += getPixelAVG(image, i, j);
				}
			}
			fAvg = (float)(bAvg / (image.GetWidth() * image.GetHeight()) ) / 255.f;
			fAvg *= 1.35f;
		

		for (int j = 0; j < sh; ++j)
		{
			for (int i = 0; i < sw; ++i)
			{
				auto ix = (int)((float)i * ratio);
				auto iy = (int)((float)j * ratioh);
				auto toix = (int)((float)(i + 1) * ratio);
				auto toiy = (int)((float)(j + 1) * ratioh);
				auto sizex = toix - ix;
				auto sizey = toiy - iy;
				float b[9] = { 0 };

					// Top left
					{
						unsigned long avg = 0, cnt = 0;
						
							for (int jj = iy; jj < iy + max<>(1, (sizey * 3 / 12)); ++jj)
							for (int ii = ix; ii < ix + max<>(1, (sizex * 2 / 8)); ++ii)
							{
								avg += getPixelAVG(image, ii, jj);
								++cnt;
							}
							if (cnt) avg /= cnt;
						b[0] = (float)avg / 255.f;
					}

					// Top
					{
						unsigned long avg = 0, cnt = 0;
						
							for (int jj = iy; jj < iy + max<>(1, (sizey * 3 / 12)); ++jj)
							for (int ii = ix + max<>(1, (sizex * 2 / 8)); ii < ix + max<>(1, (sizex * 2 / 8)) + max<>(1, (sizex * 4 / 8)); ++ii)
							{
								avg += getPixelAVG(image, ii, jj);
								++cnt;
							}
							if (cnt) avg /= cnt;
						
						b[1] = (float)avg / 255.f;
					}

					// Top right
					{
						unsigned long avg = 0, cnt = 0;
						
							for (int jj = iy; jj < iy + max<>(1, (sizey * 3 / 12)); ++jj)
							for (int ii = ix + max<>(1, (sizex * 2 / 8)) + max<>(1, (sizex * 4 / 8)); ii < ix + sizex; ++ii)
							{
								avg += getPixelAVG(image, ii, jj);
								++cnt;
							}
							if (cnt) avg /= cnt;
						
						b[2] = (float)avg / 255.f;
					}

					// Left
					{
						unsigned long avg = 0, cnt = 0;
						
							for (int jj = iy + max<>(1, (sizey * 3 / 12)); jj < iy + max<>(1, (sizey * 3 / 12)) + max<>(1, (sizey * 6 / 12)); ++jj)
							for (int ii = ix; ii < ix + max<>(1, (sizex * 2 / 8)); ++ii)
							{
								avg += getPixelAVG(image, ii, jj);
								++cnt;
							}
							if (cnt) avg /= cnt;
						
						b[3] = (float)avg / 255.f;
					}

					// Center
					{
						unsigned long avg = 0, cnt = 0;
						
							for (int jj = iy + max<>(1, (sizey * 2 / 12)); jj < iy + max<>(1, (sizey * 2 / 12)) + max<>(1, (sizey * 8 / 12)); ++jj)
							for (int ii = ix + max<>(1, (sizex * 1 / 8)); ii < ix + max<>(1, (sizex * 1 / 8)) + max<>(1, (sizex * 6 / 8)); ++ii)
							{
								avg += getPixelAVG(image, ii, jj);
								++cnt;
							}
							if (cnt) avg /= cnt;
						
						b[4] = (float)avg / 255.f;
					}

					// Right
					{
						unsigned long avg = 0, cnt = 0;
						
							for (int jj = iy + max<>(1, (sizey * 3 / 12)); jj < iy + max<>(1, (sizey * 3 / 12)) + max<>(1, (sizey * 6 / 12)); ++jj)
							for (int ii = ix + max<>(1, (sizex * 2 / 8)) + max<>(1, (sizex * 4 / 8)); ii < ix + sizex; ++ii)
							{
								avg += getPixelAVG(image, ii, jj);
								++cnt;
							}
							if (cnt) avg /= cnt;
						
						b[5] = (float)avg / 255.f;
					}

					// Bottom left
					{
						unsigned long avg = 0, cnt = 0;
						
							for (int jj = iy + max<>(1, (sizey * 3 / 12)) + max<>(1, (sizey * 6 / 12)); jj < iy + sizey; ++jj)
							for (int ii = ix; ii < ix + max<>(1, (sizex * 2 / 8)); ++ii)
							{
								avg += getPixelAVG(image, ii, jj);
								++cnt;
							}
							if (cnt) avg /= cnt;
						
						b[6] = (float)avg / 255.f;
					}

					// Bottom
					{
						unsigned long avg = 0, cnt = 0;
						
							for (int jj = iy + max<>(1, (sizey * 3 / 12)) + max<>(1, (sizey * 6 / 12)); jj < iy + sizey; ++jj)
							for (int ii = ix + max<>(1, (sizex * 2 / 8)); ii < ix + max<>(1, (sizex * 2 / 8)) + max<>(1, (sizex * 4 / 8)); ++ii)
							{
								avg += getPixelAVG(image, ii, jj);
								++cnt;
							}
							if (cnt) avg /= cnt;
						
						b[7] = (float)avg / 255.f;
					}

					// Bottom right
					{
						unsigned long avg = 0, cnt = 0;
						
							for (int jj = iy + max<>(1, (sizey * 3 / 12)) + max<>(1, (sizey * 6 / 12)); jj < iy + sizey; ++jj)
							for (int ii = ix + max<>(1, (sizex * 2 / 8)) + max<>(1, (sizex * 4 / 8)); ii < ix + sizex; ++ii)
							{
								avg += getPixelAVG(image, ii, jj);
								++cnt;
							}
							if (cnt) avg /= cnt;
						
						b[8] = (float)avg / 255.f;
					}
	


					for (auto &_b : b)
						_b = pow(_b, 1.f + (fAvg - .5f));

					
					char c = ' ';

					float bestScore = 10000.f;
					for (auto &ch : chars)
					{
						auto score = getScore(ch, b);
						if (score < bestScore)
						{
							c = ch.c;
							bestScore = score;
						}
					}

					cout << c;
				
				
			}
			cout << endl;
		}
	}//end
	else cout << "false image this url";
	system("pause");
	return 0;
}
