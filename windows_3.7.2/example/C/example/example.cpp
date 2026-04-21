// example.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream> 
#include <string.h>
#include "open_cam3d.h"

int main()
{
	/*****************************************************************************************************/
	int ret_code = 0;

	//连接相机 
	ret_code = DfConnect("127.0.0.1");

	int width = 0, height = 0;
	int channels = 1;

	if (0 == ret_code)
	{
		//必须连接相机成功后，才可获取相机分辨率
		ret_code = DfGetCameraResolution(&width, &height);
		std::cout << "Width: " << width << "    Height: " << height << std::endl;

		ret_code =DfGetCameraChannels(&channels);
		std::cout << "channels: " << channels << std::endl;
	}
	else
	{
		std::cout << "Connect Camera Error!";
		return -1;
	}

	//分配内存保存采集结果
	float* point_cloud_data = (float*)malloc(sizeof(float) * width * height * 3);
	memset(point_cloud_data, 0, sizeof(float) * width * height * 3);

	float* height_map_data = (float*)malloc(sizeof(float) * width * height);
	memset(height_map_data, 0, sizeof(float) * width * height);

	float* depth_data = (float*)malloc(sizeof(float) * width * height);
	memset(depth_data, 0, sizeof(float) * width * height);

	char* timestamp_data = (char*)malloc(sizeof(char) * 30);
	memset(timestamp_data, 0, sizeof(char) * 30);

	unsigned char* brightness_data = (unsigned char*)malloc(sizeof(unsigned char) * width * height);
	memset(brightness_data, 0, sizeof(unsigned char) * width * height);

	unsigned char* color_brightness_data = (unsigned char*)malloc(sizeof(unsigned char) * width * height * 3);
	memset(color_brightness_data, 0, sizeof(unsigned char) * width * height * 3);

	int capture_num = 0;

	if (0 == ret_code)
	{
		//set patam json
		char status_json[20480];
		char config_json[20480];
		int num = 0;
		DfReadJson(config_json, "3.json");
		DfSetParamJson(config_json, status_json, num);

		ret_code = DfCaptureData(num, timestamp_data);

		if (0 == ret_code)
		{
			if (1 == channels)
			{
				//获取亮度图数据并保存
				ret_code = DfGetBrightnessData(brightness_data);
				if (0 == ret_code)
				{
					std::cout << "Get Brightness!" << std::endl;
				}
				//获取深度图数据并保存
				ret_code = DfGetDepthDataFloat(depth_data);

				if (0 == ret_code)
				{
					std::cout << "Get Depth!" << std::endl;
				}

				//获取点云数据并保存（2选1）
				ret_code = DfGetPointcloudData(point_cloud_data);
				if (0 == ret_code)
				{
					std::cout << "Get Pointcloud!" << std::endl;
					const char* pcd = "1.pcd";
					const char* ply = "1.ply";
					savePointcloudToPcd(point_cloud_data, brightness_data, channels, pcd);
					savePointcloudToPly(point_cloud_data, brightness_data, channels, ply);
				}
			}
			else if (3 == channels)
			{
				//获取亮度图数据并保存
				ret_code = DfGetColorBrightnessData(color_brightness_data, Color::Rgb);
				if (0 == ret_code)
				{
					std::cout << "Get color Brightness!" << std::endl;
				}
				//获取深度图数据并保存
				ret_code = DfGetDepthDataFloat(depth_data);

				if (0 == ret_code)
				{
				
					std::cout << "Get Depth!" << std::endl;
				}

				//获取点云数据并保存（2选1）
				ret_code = DfGetPointcloudData(point_cloud_data);
				if (0 == ret_code)
				{
					std::cout << "Get Pointcloud!" << std::endl;
					const char* pcd = "point_cloud.pcd";
					const char* ply = "point_cloud.ply";
					savePointcloudToPcd(point_cloud_data, color_brightness_data, channels, pcd);
					savePointcloudToPly(point_cloud_data, color_brightness_data, channels, ply);
				}
			}
			capture_num++;
			std::cout << "Capture num: " << capture_num << std::endl;

		}
		else
		{
			std::cout << "Capture Data Error!" << std::endl;
		}

	}

	free(brightness_data);
	free(depth_data);
	free(point_cloud_data);
	free(height_map_data);
	free(timestamp_data);

	DfDisconnect("127.0.0.1");

}


