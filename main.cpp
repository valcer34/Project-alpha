#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <comutil.h>
#include <atlimage.h>
#include <msxml6.h>
#include <future>
#include <atlbase.h>
#include <msxml6.h>

#include "asciipng.h"

using namespace std;

//petrik start
unsigned long getPixelAVG(const CImage& image, int x, int y)
{
	auto pixel = image.GetPixel(x, y);
	//�������� �������� � rgb � 2-�� � ��������� � 10-��� � �������
	auto b = (pixel >> 16) & 0xff;//24 ������;
	auto g = (pixel >> 8) & 0xff;
	auto r = pixel & 0xff;
	return (r * 2126 + g * 7152 + b * 722) / 10000;//(100)^2
}
unsigned long getPixelAVGFromASCII(int x, int y)
{
	auto pixel = asciipngdata[y * 760 + x];
	//�������� �������� � rgb � 2-�� � ��������� � 10-��� � �������
	auto b = (pixel >> 16) & 0xff;
	auto g = (pixel >> 8) & 0xff;
	auto r = pixel & 0xff;
	return (r * 2126 + g * 7152 + b * 722) / 10000;//(100)^2
}

struct Char
{
	char c;
	float b[9];
};

vector<Char> chars;
float getScore(const Char& ch, float* b)
{
	float score = 0.f;
	//6 12 6 12 48 12 6 12 6 - ������������ ��������� � ��������� ������� �������
	//������� ����� ������ ���������� � ��������� ������� �� ���������
		score += abs(b[0] - ch.b[0])*6.f;
		score += abs(b[1] - ch.b[1])*12.f;
		score += abs(b[2] - ch.b[2])*6.f;
		score += abs(b[3] - ch.b[3])*12.f;
		score += abs(b[4] - ch.b[4])*48.f;
		score += abs(b[5] - ch.b[5])*12.f;
		score += abs(b[6] - ch.b[6])*6.f;
		score += abs(b[7] - ch.b[7])*12.f;
		score += abs(b[8] - ch.b[8])*6.f;
	return score;
}
//petrik end
//Aheichyk begin
void generateData()
{
	int ofst = 0;
	float maxB[9] = { 0 };
	for (auto c = ' '; c <= '~'; ++c, ++ofst)//��������� ���������� ��������(95)
	{	
		auto offset = ofst;//������� �������� ������
		offset *= 8;//����� ������ ����������� �������
		int count[9] = { 0 };//����� ���������
		vector<float> brightness;
		//�������� ��������� �������� �� ���������
			// Top left(3 �� 2)
			for (auto y = 0; y < 3; ++y)
			for (auto x = offset; x < offset + 2; ++x)
			if (getPixelAVGFromASCII(x, y) > 150) ++count[0];// ����������� ������ (�� ������� ��� ������ ������ �� x)

			// Top(3 �� 4)
			for (auto y = 0; y < 3; ++y)
			for (auto x = offset + 2; x < offset + 6; ++x)
			if (getPixelAVGFromASCII(x, y) > 150) ++count[1];

			// Top right(3 �� 2)
			for (auto y = 0; y < 3; ++y)
			for (auto x = offset + 6; x < offset + 8; ++x)
			if (getPixelAVGFromASCII(x, y) > 150) ++count[2];

			// Left(6 �� 2)
			for (auto y = 3; y < 9; ++y)
			for (auto x = offset ; x < offset + 2; ++x)
			if (getPixelAVGFromASCII(x, y) > 150) ++count[3];

			// Center(6 �� 4)
			for (auto y = 3; y < 9; ++y)
			for (auto x = offset + 2; x < offset + 6; ++x)
			if (getPixelAVGFromASCII(x, y) > 150) ++count[4];

			// Right(6 �� 2)
			for (auto y = 3; y < 9; ++y)
			for (auto x = offset + 6; x < offset + 8; ++x)
			if (getPixelAVGFromASCII(x, y) > 150) ++count[5];

			// Bottom left(3 �� 2)
			for (auto y = 9; y < 12; ++y)
			for (auto x = offset; x < offset + 2; ++x)
			if (getPixelAVGFromASCII(x, y) > 150) ++count[6];

			// Bottom(3 �� 4)
			for (auto y = 9; y < 12; ++y)
			for (auto x = offset + 2; x < offset + 6; ++x)
			if (getPixelAVGFromASCII(x, y) > 150) ++count[7];

			// Bottom right(3 �� 2)
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
			maxB[i] = max(maxB[i],brightness[i]);//���� �������� �� ���� ��������� �������� (�������)
		}

		chars.push_back({ c, brightness[0], brightness[1], brightness[2], brightness[3], brightness[4], brightness[5], brightness[6], brightness[7], brightness[8] });
	}

	for (Char &ch : chars)
	{
		for (int i = 0; i < 9; ++i)
		{
			ch.b[i] /= maxB[i];//��������� ��������(1 = max)
		}
	}
}

int main(int argc, const char* argv[])
{
	string setting_url;
	for (;;){
	cout << "Input url-image(if==0,00,000-default):";
	cin >> setting_url;
	if (setting_url == "000")setting_url = "https://3dnews.ru/assets/external/illustrations/2018/02/01/964971/theres-one-simple-reason-nintendo-is-bringing-super-mario-to-the-iphone-first.jpg";
	if (setting_url == "00")setting_url = "https://encrypted-tbn0.gstatic.com/images?q=tbn:ANd9GcReB4CKrXDUHl-Omrh_DCKMiINT6tH1j1sWV_-penT71iE6sLg_";
	if (setting_url == "0")setting_url = "https://encrypted-tbn0.gstatic.com/images?q=tbn:ANd9GcQd6d_4H0OlGY3T3AdcUFYeJABuDt76dMkBBWR_Y_8XC54FTBOT";
	CoInitialize(nullptr);//�������������� ����� com
	 generateData();//���������� ������� ������� ��������� �������� (����������� �������� � ���������� �� ����� � �������� ������������)
	HRESULT hr;//�������� ���������� ��������� ������
	//���������� ����� � ���-�������
	CComPtr<IXMLHTTPRequest> request;
	hr = request.CoCreateInstance(CLSID_XMLHTTP60); // ������� ��������� xml-������� 
	hr = request->open(_bstr_t("GET"), _bstr_t(setting_url.c_str()), _variant_t(VARIANT_FALSE), _variant_t(), _variant_t());//������ �������� �������
	auto ret = async(launch::async,[&]{HRESULT hr;hr = request->send(_variant_t());});//���������� ��������� ������� request->send � ��������� ������
	ret.wait();//���� �� ����������� - ����������� �����
	//�������� ������ - 200 � ������ ������
	long status;//������ ���������
	hr = request->get_status(&status);
	if (status == 200)
	{
		// ��������� ������ ����������� (���� URL ��������� �� �����������)
		VARIANT responseVariant;//������ ������, ������� � �������� ��������� ���������� ��������� ����, ������� �� ����� ���������
		hr = request->get_responseStream(&responseVariant);//��������� ��������� ������
		IStream* stream = (IStream*)responseVariant.punkVal;//����������� ������
		CImage image;//���� ��������
		image.Load(stream);
		stream->Release();

		int setting_width = 78;
		auto w = image.GetWidth();
		auto h = image.GetHeight();
		float ratio = (float)w / (float)setting_width;//��������� �������� �������� � ����������� ��������
		auto ratioh = ratio * (1.5f);//��������� ������ �� ratio
		int sw = (int)((float)w / ratio);//����������� ������ 
		int sh = (int)((float)h / ratioh);//����������� ������
		//Aheichyk end

		//Petrik start
		unsigned long bAvg = 0;//����� ������� ����������
		float fAvg = 0.f;//����� �������� ������� � ascii

		for (int j = 0; j < image.GetHeight(); ++j)
		{
			for (int i = 0; i < image.GetWidth(); ++i)
			{
				bAvg += getPixelAVG(image, i, j);
			}
		}
		fAvg = (bAvg / (image.GetWidth() * image.GetHeight())) / 255.f;//�������������� � ���������� �������� ascii
		fAvg = (fAvg*1.35f)+0.5f;//����� ������ ������� 
	
		//��������� ���������� � ������ � ����������� � ������
		for (int j = 0; j < sh; ++j)
		{
			for (int i = 0; i < sw; ++i)
			{
				auto ix = (int)(i * ratio);
				auto iy = (int)(j * ratioh);
				auto toix = (int)((i + 1) * ratio);
				auto toiy = (int)((j + 1) * ratioh);
				auto sizex = toix - ix;
				auto sizey = toiy - iy;
				float b[9] = { 0 };//����� ���������

				// Top left
				{
					unsigned long avg = 0, cnt = 0;

					for (int jj = iy; jj < iy + max(1, (sizey * 1/4)); ++jj)
					for (int ii = ix; ii < ix + max(1, (sizex * 1/4)); ++ii)
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

					for (int jj = iy; jj < iy + max(1, (sizey * 1 / 4)); ++jj)
					for (int ii = ix + max(1, (sizex * 1 / 4)); ii < ix + max(1, (sizex * 1 / 4)) + max(1, (sizex * 1 / 2)); ++ii)
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

					for (int jj = iy; jj < iy + max(1, (sizey * 1 / 4)); ++jj)
					for (int ii = ix + max(1, (sizex * 1 /4)) + max(1, (sizex * 1 / 2)); ii < ix + sizex; ++ii)
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

					for (int jj = iy + max(1, (sizey * 1 / 4)); jj < iy + max(1, (sizey * 1 / 4)) + max(1, (sizey * 1/ 2)); ++jj)
					for (int ii = ix; ii < ix + max(1, (sizex * 1 / 4)); ++ii)
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

					for (int jj = iy + max(1, (sizey * 1 / 6)); jj < iy + max(1, (sizey * 1 / 6)) + max(1, (sizey * 2 / 3)); ++jj)
					for (int ii = ix + max(1, (sizex * 1 / 8)); ii < ix + max(1, (sizex * 1 / 8)) + max(1, (sizex * 3 / 4)); ++ii)
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

					for (int jj = iy + max(1, (sizey * 1 / 4)); jj < iy + max(1, (sizey * 1 / 4)) + max(1, (sizey * 1 / 2)); ++jj)
					for (int ii = ix + max(1, (sizex * 1 / 4)) + max(1, (sizex * 1 / 2)); ii < ix + sizex; ++ii)
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

					for (int jj = iy + max(1, (sizey * 1 / 4)) + max(1, (sizey * 1 / 2)); jj < iy + sizey; ++jj)
					for (int ii = ix; ii < ix + max(1, (sizex * 1 / 4)); ++ii)
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
					for (int jj = iy + max(1, (sizey * 1/ 4)) + max(1, (sizey * 1 / 2)); jj < iy + sizey; ++jj)
					for (int ii = ix + max(1, (sizex * 1 /4)); ii < ix + max(1, (sizex * 1 / 4)) + max(1, (sizex * 1 / 2)); ++ii)
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

					for (int jj = iy + max(1, (sizey * 1 / 4)) + max(1, (sizey * 1 / 2)); jj < iy + sizey; ++jj)
					for (int ii = ix + max(1, (sizex * 1 / 4)) + max(1, (sizex * 1 / 2)); ii < ix + sizex; ++ii)
					{
						avg += getPixelAVG(image, ii, jj);
						++cnt;
					}
					if (cnt) avg /= cnt;
					b[8] = (float)avg / 255.f;
				}
				for (float &_b : b) _b = pow(_b, fAvg );//�������� ���������� �������
				char c = ' ';
				float bestScore = 10000.f;
				for (Char &ch : chars)//��������� �� ���������� ������
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
		system("pause");
		return 0;
	}//Petrik end
	else cout << "False url: this not image"<<endl;
}
}
