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

//#define DEBUG			// 非除錯否則別用
#define alpha 0.1		// 浮水印強度

using namespace cv;
using namespace std;

double eqm(Mat &, Mat &);
double psnr(Mat &, Mat &);
void dct512(Mat &);
void showImage(string, Mat, int, int);
double correlation(Mat &, Mat &);

int main(int argc, char* argv[])
{
	Mat block;					//暫存圖區

	// 以 8bit 灰階類型載入原始圖片與浮水印圖片
	Mat originalImage = imread("bridge_g_512x512.bmp", CV_LOAD_IMAGE_GRAYSCALE);
	Mat watermarkImage = imread("NFU_WM.bmp", CV_LOAD_IMAGE_GRAYSCALE);

	// 顯示原本的圖片以及浮水印圖片
	showImage("原始圖片", originalImage, 0, 0);
	showImage("原始浮水印圖片", watermarkImage, 0, 543);


	// 複製一份原圖片並調整成指定大小
	Mat dctImage = originalImage.clone();
	resize(dctImage, dctImage, Size(512, 512));
	dctImage.convertTo(dctImage, CV_32F, 1.0 / 255.0);	// DCT 要求使用 32bit 灰階
	Mat wmrkImage = watermarkImage.clone();
	resize(wmrkImage, wmrkImage, Size(64, 64));
	wmrkImage.convertTo(wmrkImage, CV_32F, 1.0 / 255.0);			// DCT 要求使用 32bit 灰階

	dct512(dctImage);
#ifdef DEBUG
	showImage("圖片DCT轉換結果", dctImage, 513, 0);
#endif // DEBUG

	// 嵌入浮水印, 如果浮水印像素值不為白色, 則圖區之像素(5,5),也就是中間顏色區域,添加浮水印像素質 * alpha(default: 0.1)
	for (int i = 0; i < 512; i += 8)
	{
		float w;	//暫存浮水印 pixel
		for (int j = 0; j < 512; j += 8)
		{
			block = dctImage(Rect(i, j, 8, 8));
			w = (float)wmrkImage.at<float>(i / 8, j / 8);
			if (w < 1) {	// 非白色
				block.at<float>(5, 5) = (float)block.at<float>(5, 5) + alpha * w;	// f'(x, y) = f(x, y) + alpha * w(u, v) 
#ifdef DEBUG
				cout << "w = " << w << " alpha * w = " << alpha * w << endl;
#endif // DEBUG

			}
			idct(block, block);	// 為了增加程式執行效率, 在這就先做DCT 逆轉換
		}
	}

	showImage("浮水印嵌入結果圖片", dctImage, 513, 0);		// 顯示嵌入結果圖片
	dctImage.convertTo(dctImage, CV_8UC1, 255.0);			// 將圖片轉成 8bit 灰階
	imwrite("dctImage.bmp", dctImage);	//保存圖片
	cout << "(b). PSNR: " << psnr(originalImage, dctImage) << "%" << endl;
	// PSNR OpenCV 版本(無 round)
	//originalImage.convertTo(originalImage, CV_32FC1);
	//dctImage.convertTo(dctImage, CV_32FC1);
	//cout << "(b). PSNR: " << PSNR(originalImage, dctImage) << endl;


	// 取出嵌入浮水印, 將兩張圖做DCT後取出差異值並組成浮水印
	Mat fetchWatImg(Size(64, 64), CV_32F, Scalar(255));	// 保存取出的浮水印
	Mat orgImage = originalImage.clone();
	Mat damagedWatImage = imread("dctImage_damaged.bmp", CV_LOAD_IMAGE_GRAYSCALE);
	showImage("遭到破壞的圖片", damagedWatImage, 1026, 0);
	Mat block2;											// 暫存圖區
	// 原始圖片DCT
	resize(orgImage, orgImage, Size(512, 512));
	orgImage.convertTo(orgImage, CV_32F, 1.0 / 255.0);	// DCT 要求使用 32bit 灰階
	dct512(orgImage);
	// 損害圖片DCT
	resize(damagedWatImage, damagedWatImage, Size(512, 512));
	damagedWatImage.convertTo(damagedWatImage, CV_32F, 1.0 / 255.0);	// DCT 要求使用 32bit 灰階
	dct512(damagedWatImage);
	for (int i = 0; i < 512; i += 8)
	{
		float w;	//暫存浮水印 pixel
		for (int j = 0; j < 512; j += 8)
		{
			// 取得圖塊
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

	showImage("取出的浮水印", fetchWatImg, 0, 635);
	//計算NC
	resize(fetchWatImg, fetchWatImg, Size(64, 64));
	fetchWatImg.convertTo(fetchWatImg, CV_32F, 1.0 / 255.0);			// DCT 要求使用 32bit 灰階
	cout << "c.2 NC: " << correlation(wmrkImage, fetchWatImg) << endl;
	fetchWatImg.convertTo(fetchWatImg, CV_8UC1, 255.0);					// 將圖片轉成 8bit 灰階
	imwrite("fetchedWatImage.bmp", dctImage);							//保存取出的浮水印圖片

	cout << "程式執行結束, 於結果視窗點擊任何按鍵結束程式, 否則將在60秒後自動關閉程式!" << endl;
	waitKey(6000000);
	destroyAllWindows();
	return 0;
}

// Mean squared error
double eqm(Mat & img1, Mat & img2)
{
	img1.convertTo(img1, CV_32FC1);			// 將圖片轉成 32bit 灰階才可以轉成方圖
	img2.convertTo(img2, CV_32FC1);			// 將圖片轉成 32bit 灰階才可以轉成方圖

	int i, j;
	float eqm = 0;
	int height = img1.rows;
	int width = img1.cols;

	// 每個像素做 |I1(i,j) - I2(i,j)|
	for (i = 0; i < height; i++)
		for (j = 0; j < width; j++)
			eqm += (img1.at<float>(i, j) - img2.at<float>(i, j)) * (img1.at<float>(i, j) - img2.at<float>(i, j));

	img1.convertTo(img1, CV_8UC1);			// 將圖片還原
	img2.convertTo(img2, CV_8UC1);			// 將圖片還原
	eqm /= height * width;
	return eqm;
}

/**
 * 計算 PSNR (OPENCV)
 *
 * @param I1 第一張圖片
 * @param I2 第二張圖片
 * @return psnr
 */
double psnr(Mat & I1, Mat & I2)
{
	double relt = (10 * log10((255 * 255) / eqm(I1, I2)));
	return isinf(relt) ? 100 : relt;
}

/**
 * 新開視窗顯示圖片
 *
 * @param title 視窗標題
 * @param image 要顯示的圖片
 * @param pos_x 視窗 x
 * @param pos_y 視窗 y
 */
void showImage(string title, Mat image, int pos_x = 0, int pos_y = 0) {
	namedWindow(title);					//設置視窗標題
	moveWindow(title, pos_x, pos_y);	//設置視窗位置
	imshow(title, image);				//顯示圖片
}

/**
 * 將512*512的圖做DCT
 */
void dct512(Mat & dctImage) {
	Mat block;
	// 將圖片分割成8x8的圖區並個別執行DCT轉換
	for (int i = 0; i < 512; i += 8)
	{
		for (int j = 0; j < 512; j += 8)
		{
			block = dctImage(Rect(i, j, 8, 8));
#ifdef DEBUG
			showImage("A", block, 121, 700);
#endif // DEBUG
			dct(block, block);				//DCT 轉換
#ifdef DEBUG
			showImage("B", block, 242, 700);
			waitKey(500);
#endif // DEBUG
		}
	}
}

/**
 * 計算 NC 值
 * @param I1 第一張圖
 * @param I2 第二張圖
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