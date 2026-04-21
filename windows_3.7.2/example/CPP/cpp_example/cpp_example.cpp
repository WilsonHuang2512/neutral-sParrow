#include <iostream>
#include <string.h>
#include "xcamera.h"
#include "enumerate.h"
#include <opencv2/opencv.hpp>
using namespace CAMERA;

int main()
{
	int ret_code = 0;

	// 创建相机
	XCamera* p_camera = (XCamera*)createXCamera();

	// 连接相机
	ret_code = p_camera->connect("127.0.0.1");

	int width = 0, height = 0;
	int channels = 1;

	if (0 == ret_code)
	{
		// 必须连接相机成功后，才可获取相机分辨率
		ret_code = p_camera->getCameraResolution(&width, &height);
		std::cout << "Width: " << width << "    Height: " << height << std::endl;

		ret_code = p_camera->getCameraChannels(&channels);
		std::cout << "channels: " << channels << std::endl;
	}
	else
	{
		std::cout << "Connect Camera Error!";
		return -1;
	}

	// 分配内存保存采集结果
	float* point_cloud_data = (float*)malloc(sizeof(float) * width * height * 3);
	memset(point_cloud_data, 0, sizeof(float) * width * height * 3);

	float* depth_data = (float*)malloc(sizeof(float) * width * height);
	memset(depth_data, 0, sizeof(float) * width * height);

	char* timestamp_data = (char*)malloc(sizeof(char) * 30);
	memset(timestamp_data, 0, sizeof(char) * 30);

	unsigned char* brightness_data = (unsigned char*)malloc(sizeof(unsigned char) * width * height);
	memset(brightness_data, 0, sizeof(unsigned char) * width * height);

	unsigned char* color_brightness_data = (unsigned char*)malloc(sizeof(unsigned char) * width * height * 3);
	memset(color_brightness_data, 0, sizeof(unsigned char) * width * height * 3);

	if (0 == ret_code)
	{
		// 设置参数 - 使用JSON配置文件
		char status_json[20480];
		char config_json[20480];
		int num = 0;
		p_camera->readJson(config_json, "3.json");
		p_camera->setParamJson(config_json, status_json, num);

		ret_code = p_camera->captureData(num, timestamp_data);

		if (0 == ret_code)
		{
			if (1 == channels)
			{
				// 获取灰度亮度图数据并保存
				ret_code = p_camera->getBrightnessData(brightness_data);
				if (0 == ret_code)
				{
					cv::Mat bright = cv::Mat(height, width, CV_8U, brightness_data);
					cv::imwrite("bright.bmp", bright);
				}
				
				// 获取深度图数据并保存
				ret_code = p_camera->getDepthData(depth_data);
				if (0 == ret_code)
				{
					cv::Mat depth = cv::Mat(height, width, CV_32F, depth_data);
					cv::imwrite("depth.tiff", depth);
					std::cout << "Get Depth!" << std::endl;
				}

				// 获取点云数据并保存
				ret_code = p_camera->getPointcloudData(point_cloud_data);
				if (0 == ret_code)
				{
					std::cout << "Get Pointcloud!" << std::endl;
					const char* pcd = "pointcloud.pcd";
					const char* ply = "pointcloud.ply";
					p_camera->savePointcloudToPcd(point_cloud_data, brightness_data, channels, pcd);
					p_camera->savePointcloudToPly(point_cloud_data, brightness_data, channels, ply);
				}
			}
			else if (3 == channels)
			{
				// 获取彩色亮度图数据并保存 - 使用BGR格式
				ret_code = p_camera->getColorBrightnessData(color_brightness_data, Color::Bgr);
				if (0 == ret_code)
				{
					cv::Mat bright = cv::Mat(height, width, CV_8UC3, color_brightness_data);
					cv::imwrite("bright.bmp", bright);
					std::cout << "Get color Brightness!" << std::endl;
				}
				
				// 获取深度图数据并保存
				ret_code = p_camera->getDepthData(depth_data);
				if (0 == ret_code)
				{
					cv::Mat depth = cv::Mat(height, width, CV_32F, depth_data);
					cv::imwrite("depth.tiff", depth);
					std::cout << "Get Depth!" << std::endl;
				}

				// 获取点云数据并保存
				ret_code = p_camera->getPointcloudData(point_cloud_data);
				if (0 == ret_code)
				{
					std::cout << "Get Pointcloud!" << std::endl;
					const char* pcd = "color_cloud.pcd";
					const char* ply = "color_cloud.ply";
					p_camera->savePointcloudToPcd(point_cloud_data, color_brightness_data, channels, pcd);
					p_camera->savePointcloudToPly(point_cloud_data, color_brightness_data, channels, ply);
				}
			}
			std::cout << "Capture Success!" << std::endl;
		}
		else
		{
			std::cout << "Capture Data Error!" << std::endl;
		}
	}

	// 释放内存
	free(brightness_data);
	free(color_brightness_data);
	free(depth_data);
	free(point_cloud_data);
	free(timestamp_data);

	// 断开相机
	p_camera->disconnect("127.0.0.1");

	destroyXCamera(p_camera);

	return 0;
}