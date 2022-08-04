///
/// OpenCV VERSION 4.2.0
///	OS: Windows 10
/// Program Language: c++
///
#include <vector>
#include <iostream>
#include <algorithm>	//round
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgcodecs/imgcodecs.hpp>
#include <opencv2/imgcodecs/legacy/constants_c.h>

//#define DEBUG			// �D�����_�h�O��
#define alpha 0.1		// �B���L�j��

using namespace cv;
using namespace std;

double eqm(Mat &, Mat &);
double psnr(Mat &, Mat &);
void dct512(Mat &);
void showImage(string, Mat, int, int);
double correlation(Mat &, Mat &);

int main(int argc, char* argv[])
{
	Mat block;					//�Ȧs�ϰ�

	// �H 8bit �Ƕ��������J��l�Ϥ��P�B���L�Ϥ�
	Mat originalImage = imread("bridge_g_512x512.bmp", CV_LOAD_IMAGE_GRAYSCALE);
	Mat watermarkImage = imread("NFU_WM.bmp", CV_LOAD_IMAGE_GRAYSCALE);

	// ��ܭ쥻���Ϥ��H�ίB���L�Ϥ�
	showImage("��l�Ϥ�", originalImage, 0, 0);
	showImage("��l�B���L�Ϥ�", watermarkImage, 0, 543);


	// �ƻs�@����Ϥ��ýվ㦨���w�j�p
	Mat dctImage = originalImage.clone();
	resize(dctImage, dctImage, Size(512, 512));
	dctImage.convertTo(dctImage, CV_32F, 1.0 / 255.0);	// DCT �n�D�ϥ� 32bit �Ƕ�
	Mat wmrkImage = watermarkImage.clone();
	resize(wmrkImage, wmrkImage, Size(64, 64));
	wmrkImage.convertTo(wmrkImage, CV_32F, 1.0 / 255.0);			// DCT �n�D�ϥ� 32bit �Ƕ�

	dct512(dctImage);
#ifdef DEBUG
	showImage("�Ϥ�DCT�ഫ���G", dctImage, 513, 0);
#endif // DEBUG

	// �O�J�B���L, �p�G�B���L�����Ȥ����զ�, �h�ϰϤ�����(5,5),�]�N�O�����C��ϰ�,�K�[�B���L������ * alpha(default: 0.1)
	for (int i = 0; i < 512; i += 8)
	{
		float w;	//�Ȧs�B���L pixel
		for (int j = 0; j < 512; j += 8)
		{
			block = dctImage(Rect(i, j, 8, 8));
			w = (float)wmrkImage.at<float>(i / 8, j / 8);
			if (w < 1) {	// �D�զ�
				block.at<float>(5, 5) = (float)block.at<float>(5, 5) + alpha * w;	// f'(x, y) = f(x, y) + alpha * w(u, v) 
#ifdef DEBUG
				cout << "w = " << w << " alpha * w = " << alpha * w << endl;
#endif // DEBUG

			}
			idct(block, block);	// ���F�W�[�{������Ĳv, �b�o�N����DCT �f�ഫ
		}
	}

	showImage("�B���L�O�J���G�Ϥ�", dctImage, 513, 0);		// ��ܴO�J���G�Ϥ�
	dctImage.convertTo(dctImage, CV_8UC1, 255.0);			// �N�Ϥ��ন 8bit �Ƕ�
	imwrite("dctImage.bmp", dctImage);	//�O�s�Ϥ�
	cout << "(b). PSNR: " << psnr(originalImage, dctImage) << "%" << endl;
	// PSNR OpenCV ����(�L round)
	//originalImage.convertTo(originalImage, CV_32FC1);
	//dctImage.convertTo(dctImage, CV_32FC1);
	//cout << "(b). PSNR: " << PSNR(originalImage, dctImage) << endl;


	// ���X�O�J�B���L, �N��i�ϰ�DCT����X�t���Ȩòզ��B���L
	Mat fetchWatImg(Size(64, 64), CV_32F, Scalar(255));	// �O�s���X���B���L
	Mat orgImage = originalImage.clone();
	Mat damagedWatImage = imread("dctImage_damaged.bmp", CV_LOAD_IMAGE_GRAYSCALE);
	showImage("�D��}�a���Ϥ�", damagedWatImage, 1026, 0);
	Mat block2;											// �Ȧs�ϰ�
	// ��l�Ϥ�DCT
	resize(orgImage, orgImage, Size(512, 512));
	orgImage.convertTo(orgImage, CV_32F, 1.0 / 255.0);	// DCT �n�D�ϥ� 32bit �Ƕ�
	dct512(orgImage);
	// �l�`�Ϥ�DCT
	resize(damagedWatImage, damagedWatImage, Size(512, 512));
	damagedWatImage.convertTo(damagedWatImage, CV_32F, 1.0 / 255.0);	// DCT �n�D�ϥ� 32bit �Ƕ�
	dct512(damagedWatImage);
	for (int i = 0; i < 512; i += 8)
	{
		float w;	//�Ȧs�B���L pixel
		for (int j = 0; j < 512; j += 8)
		{
			// ���o�϶�
			block = orgImage(Rect(i, j, 8, 8));
			block2 = damagedWatImage(Rect(i, j, 8, 8));

			if (block.at<float>(5, 5) != block2.at<float>(5, 5)) {
				fetchWatImg.at<float>(i / 8, j / 8) = (block2.at<float>(5, 5) - block.at<float>(5, 5)) / alpha; // w(u, v) = (f'(x, y) - f(x, y)) / alpha
#ifdef DEBUG
				cout << (block2.at<float>(5, 5) - block.at<float>(5, 5)) / alpha << endl;
#endif // DEBUG
			}
		}
	}

	showImage("���X���B���L", fetchWatImg, 0, 635);
	//�p��NC
	resize(fetchWatImg, fetchWatImg, Size(64, 64));
	fetchWatImg.convertTo(fetchWatImg, CV_32F, 1.0 / 255.0);			// DCT �n�D�ϥ� 32bit �Ƕ�
	cout << "c.2 NC: " << correlation(wmrkImage, fetchWatImg) << endl;
	fetchWatImg.convertTo(fetchWatImg, CV_8UC1, 255.0);					// �N�Ϥ��ন 8bit �Ƕ�
	imwrite("fetchedWatImage.bmp", dctImage);							//�O�s���X���B���L�Ϥ�

	cout << "�{�����浲��, �󵲪G�����I��������䵲���{��, �_�h�N�b60���۰������{��!" << endl;
	waitKey(6000000);
	destroyAllWindows();
	return 0;
}

// Mean squared error
double eqm(Mat & img1, Mat & img2)
{
	img1.convertTo(img1, CV_32FC1);			// �N�Ϥ��ন 32bit �Ƕ��~�i�H�ন���
	img2.convertTo(img2, CV_32FC1);			// �N�Ϥ��ন 32bit �Ƕ��~�i�H�ন���

	int i, j;
	float eqm = 0;
	int height = img1.rows;
	int width = img1.cols;

	// �C�ӹ����� |I1(i,j) - I2(i,j)|
	for (i = 0; i < height; i++)
		for (j = 0; j < width; j++)
			eqm += (img1.at<float>(i, j) - img2.at<float>(i, j)) * (img1.at<float>(i, j) - img2.at<float>(i, j));

	img1.convertTo(img1, CV_8UC1);			// �N�Ϥ��٭�
	img2.convertTo(img2, CV_8UC1);			// �N�Ϥ��٭�
	eqm /= height * width;
	return eqm;
}

/**
 * �p�� PSNR (OPENCV)
 *
 * @param I1 �Ĥ@�i�Ϥ�
 * @param I2 �ĤG�i�Ϥ�
 * @return psnr
 */
double psnr(Mat & I1, Mat & I2)
{
	double relt = (10 * log10((255 * 255) / eqm(I1, I2)));
	return isinf(relt) ? 100 : relt;
}

/**
 * �s�}������ܹϤ�
 *
 * @param title �������D
 * @param image �n��ܪ��Ϥ�
 * @param pos_x ���� x
 * @param pos_y ���� y
 */
void showImage(string title, Mat image, int pos_x = 0, int pos_y = 0) {
	namedWindow(title);					//�]�m�������D
	moveWindow(title, pos_x, pos_y);	//�]�m������m
	imshow(title, image);				//��ܹϤ�
}

/**
 * �N512*512���ϰ�DCT
 */
void dct512(Mat & dctImage) {
	Mat block;
	// �N�Ϥ����Φ�8x8���ϰϨíӧO����DCT�ഫ
	for (int i = 0; i < 512; i += 8)
	{
		for (int j = 0; j < 512; j += 8)
		{
			block = dctImage(Rect(i, j, 8, 8));
#ifdef DEBUG
			showImage("A", block, 121, 700);
#endif // DEBUG
			dct(block, block);				//DCT �ഫ
#ifdef DEBUG
			showImage("B", block, 242, 700);
			waitKey(500);
#endif // DEBUG
		}
	}
}

/**
 * �p�� NC ��
 * @param I1 �Ĥ@�i��
 * @param I2 �ĤG�i��
 * @return nc value
 */
double correlation(Mat &I1, Mat &I2) {
	float up = 0.0, down = 0.0;
	for (int i = 0; i < I1.rows; i++)
	{
		for (int j = 0; j < I1.cols; j++)
		{
			up += I1.at<float>(i, j) * I2.at<float>(i, j);
			down += I1.at<float>(i, j) * I1.at<float>(i, j);
		}
	}
	return up/down;
}