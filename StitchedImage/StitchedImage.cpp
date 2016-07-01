#include <opencv2/opencv.hpp>
#include<opencv2/nonfree/nonfree.hpp>
#include<opencv2/legacy/legacy.hpp>
#include<vector>
#include<iostream>
#include<sstream>
#include<string>
#include<time.h> 
using namespace std;
using namespace cv;
Mat Stitched(Mat img1, Mat img2) {
	Mat g1(img1, Rect(0, 0, img1.cols, img1.rows));
	Mat g2(img2, Rect(0, 0, img2.cols, img2.rows));
	cvtColor(g1, g1, CV_BGR2GRAY);
	cvtColor(g2, g2, CV_BGR2GRAY);
	SiftFeatureDetector siftdet;
	vector<KeyPoint>kp1, kp2;
	//SIFT sift;
	SiftDescriptorExtractor extractor;
	Mat descriptor1, descriptor2;
	FlannBasedMatcher matcher;
	vector<DMatch> matches, goodmatches;
	/*������������ȡ*/
	siftdet.detect(g1, kp1);
	siftdet.detect(g2, kp2);
	/* ��������������ȡ */
	extractor.compute(g1, kp1, descriptor1);
	extractor.compute(g2, kp2, descriptor2);
	/* �������������ٽ�ƥ�� */
	matcher.match(descriptor1, descriptor2, matches);
	Mat  firstmatches;
	/*������һ��ƥ��Ľ��*/
	drawMatches(img1, kp1, img2, kp2,
		matches, firstmatches, Scalar::all(-1), Scalar::all(-1),
		vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);
	imshow("first_matches", firstmatches);
	/* �������������������ֵ����Сֵ */
	double max_dist = 0; double min_dist = 1000;
	for (int i = 0; i < descriptor1.rows; i++) {
		if (matches[i].distance > max_dist) {
			max_dist = matches[i].distance;
		}
		if (matches[i].distance < min_dist) {
			min_dist = matches[i].distance;
		}
	}
	cout << "The max distance is: " << max_dist << endl;
	cout << "The min distance is: " << min_dist << endl;
	for (int i = 0; i < descriptor1.rows; i++) {
		if (matches[i].distance < 2 * min_dist) {
			goodmatches.push_back(matches[i]);
		}
	}
	Mat img_matches;
	/*�ڶ���ɸѡ��Ľ��*/
	drawMatches(img1, kp1, img2, kp2,
		goodmatches, img_matches, Scalar::all(-1), Scalar::all(-1),
		vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);
	imshow("good_matches", img_matches);
	vector<Point2f> keypoints1, keypoints2;
	for (int i = 0; i < goodmatches.size(); i++) {
		keypoints1.push_back(kp1[goodmatches[i].queryIdx].pt);
		keypoints2.push_back(kp2[goodmatches[i].trainIdx].pt);
	}
	/*���㵥Ӧ����*/
	Mat H = findHomography(keypoints1, keypoints2, CV_RANSAC);
	Mat stitchedImage;
	int mRows = img2.rows;
	if (img1.rows> img2.rows)
	{
		mRows = img1.rows;
	}
	/*�ж�ͼ������߻������ұ�*/
	int propimg1 = 0, propimg2 = 0;
	for (int i = 0; i < goodmatches.size(); i++) {
		if (kp1[goodmatches[i].queryIdx].pt.x > img1.cols / 2) {
			propimg1++;
		}
		if (kp2[goodmatches[i].trainIdx].pt.x > img2.cols / 2) {
			propimg2++;
		}
	}
	bool flag = false;
	Mat imgright;
	Mat imgleft;
	if ((propimg1 / (goodmatches.size() + 0.0)) > (propimg2 / (goodmatches.size() + 0.0))) {
		imgleft = img1.clone();
		flag = true;
	}
	else {
		imgleft = img2.clone();
		flag = false;
	}
	if (flag) {
		imgright = img2.clone();
		flag = false;
	}
	else {
		imgright = img1.clone();
	}
	/*���ϱ���õ��ұߵ�ͼ�񾭹�����Hת����stitchedImage�ж�Ӧ��λ��*/
	warpPerspective(imgright, stitchedImage, H, Size(img2.cols + img1.cols, mRows));
	/*����ߵ�ͼ��Ž���*/
	Mat half(stitchedImage, Rect(0, 0, imgleft.cols, imgleft.rows));
	imgleft.copyTo(half);
	return stitchedImage;
}
int main() {
	Mat img1 = imread("1.jpg");
	Mat  stitchedImage;
	int n;
	cout << "Dataset2" << endl;
	cout << "��������ƴ�ӵ�ͼƬ����������1С��18��" << endl;
	cin >> n;
	cout << "����ɹ�����ʼ��ʱ" << endl;
	clock_t start,finish;  
    double totaltime;
    start=clock();  
	resize(img1, img1, Size(img1.cols / 4, img1.rows / 4));
	for (int k = 2; k <= n; k++) {
		stringstream stream;
		string str;
		stream << k;
		stream >> str;
		string filename = str + ".jpg";
		cout << "����ƴ��......." << filename << endl;
		Mat img = imread(filename);
		resize(img, img, Size(img.cols / 4, img.rows / 4));
		stitchedImage = Stitched(img1, img);
		img1 = stitchedImage;
	}
	finish = clock();
	totaltime = (double)(finish - start) / CLOCKS_PER_SEC;
	cout << "ƴ�ӳɹ�" << endl;
	cout << "ƴ�ӻ�����ʱ��Ϊ��" << totaltime << "�룡" << endl;
	imshow("ResultImage", stitchedImage);
	imwrite("ResultImage.jpg", stitchedImage);
	waitKey(0);
	return 0;
}