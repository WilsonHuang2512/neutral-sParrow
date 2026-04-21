using CameraCsharp;
using System;
using System.Runtime.InteropServices;
using System.Text;

namespace KW_Camera_Color
{
    class Program
    {
        static void Main(string[] args)
        {
            CameraCls camera = new CameraCls();

            string cameraId = "127.0.0.1";
            int ret_code;

            // 连接相机
            Console.WriteLine("正在连接相机...");
            ret_code = camera.DfConnect_Csharp(cameraId);

            int width = 0;
            int height = 0;
            int channels = 0;

            if (ret_code == 0)
            {
                Console.WriteLine("相机连接成功!");
                ret_code = camera.GetCameraResolution_Csharp(out width, out height);
                Console.WriteLine("Width: {0}", width);
                Console.WriteLine("Height: {0}", height);

                ret_code = camera.DfGetCameraChannels_Csharp(out channels);
                Console.WriteLine("Channels: {0}", channels);
            }
            else
            {
                Console.WriteLine("连接相机失败!");
                Console.ReadKey();
                return;
            }

            // 分配内存保存采集结果
            StringBuilder timestamp = new StringBuilder(30);

            // 根据通道数分配不同大小的亮度图内存
            int brightnessSize = (channels == 3) ? (width * height * 3) : (width * height);
            byte[] brightnessArray = new byte[brightnessSize];
            IntPtr brightnessPtr = Marshal.AllocHGlobal(brightnessArray.Length);

            float[] depthArray = new float[width * height];
            IntPtr depthPtr = Marshal.AllocHGlobal(depthArray.Length * sizeof(float));

            float[] point_cloud_Array = new float[width * height * 3];
            IntPtr point_cloud_Ptr = Marshal.AllocHGlobal(point_cloud_Array.Length * sizeof(float));

            // 读取JSON配置
            StringBuilder config_json = new StringBuilder(20480);
            StringBuilder status_json = new StringBuilder(20480);
            int num;
            string path = "3.json";

            Console.WriteLine("正在读取配置文件...");
            camera.DfReadJson_Csharp(config_json, path);
            camera.DfSetParamJson_Csharp(config_json, status_json, out num);
            Console.WriteLine("配置文件加载成功!");

            // 采集数据
            Console.WriteLine("正在采集数据...");
            ret_code = camera.DfCaptureData_Csharp(num, timestamp);

            if (ret_code == 0)
            {
                Console.WriteLine("数据采集成功!");

                // 根据通道数选择不同的获取方式
                if (channels == 1)
                {
                    // 灰度相机
                    Console.WriteLine("检测到灰度相机，获取灰度亮度图...");
                    ret_code = camera.DfGetUndistortBrightnessData_Csharp(brightnessPtr);
                    
                    if (ret_code == 0)
                    {
                        Console.WriteLine("灰度亮度图获取成功!");
                    }
                }
                else if (channels == 3)
                {
                    // 彩色相机
                    Console.WriteLine("检测到彩色相机，获取彩色亮度图...");
                    ret_code = camera.DfGetUndistortColorBrightnessData_Csharp(brightnessPtr, Color.Bgr);
                    
                    if (ret_code == 0)
                    {
                        Console.WriteLine("彩色亮度图获取成功!");
                    }
                }
                else
                {
                    Console.WriteLine("警告: 不支持的通道数 {0}", channels);
                }

                // 获取深度图数据
                Console.WriteLine("正在获取深度图...");
                ret_code = camera.DfGetUndistortDepthDataFloat_Csharp(depthPtr);

                // 获取点云数据
                Console.WriteLine("正在获取点云数据...");
                ret_code = camera.DfGetPointcloudData_Csharp(point_cloud_Ptr);

                // 保存点云 - 根据通道数使用不同文件名
                Console.WriteLine("正在保存点云文件...");
                string ply, pcd;
                
                if (channels == 1)
                {
                    ply = "pointcloud.ply";
                    pcd = "pointcloud.pcd";
                }
                else
                {
                    ply = "color_cloud.ply";
                    pcd = "color_cloud.pcd";
                }
                
                camera.savePointcloudToPly_Csharp(point_cloud_Ptr, brightnessPtr, channels, ply);
                camera.savePointcloudToPcd_Csharp(point_cloud_Ptr, brightnessPtr, channels, pcd);
                Console.WriteLine("点云保存成功: {0}, {1}", pcd, ply);

                // 复制数据到托管数组
                Marshal.Copy(brightnessPtr, brightnessArray, 0, brightnessArray.Length);
                Marshal.Copy(depthPtr, depthArray, 0, depthArray.Length);
                Marshal.Copy(point_cloud_Ptr, point_cloud_Array, 0, point_cloud_Array.Length);

                // 释放内存
                Marshal.FreeHGlobal(brightnessPtr);
                Marshal.FreeHGlobal(depthPtr);
                Marshal.FreeHGlobal(point_cloud_Ptr);
            }
            else
            {
                Console.WriteLine("采集数据失败!");
            }

            // 断开相机
            ret_code = camera.DfDisconnect_Csharp(cameraId);
            if (ret_code == 0)
            {
                Console.WriteLine("相机已断开!");
            }

            Console.WriteLine("\n按任意键退出...");
            Console.ReadKey();
        }
    }
}