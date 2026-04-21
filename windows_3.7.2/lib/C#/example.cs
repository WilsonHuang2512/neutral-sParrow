using CameraCsharp;
using System;
using System.Runtime.InteropServices;
using System.Text;

namespace KW_Camera_Color
{
    class Program
    {
        // 打印标定参数
        static void PrintCalibration(CameraCsharp.CalibrationParam calib)
        {
            Console.WriteLine("\nIntrinsic:");
            for (int r = 0; r < 3; r++)
            {
                for (int c = 0; c < 3; c++)
                    Console.Write("{0:F6}\t", calib.intrinsic[3 * r + c]);
                Console.WriteLine();
            }

            Console.WriteLine("\nDistortion:");
            for (int c = 0; c < 5; c++)
                Console.Write("{0:F6}\t", calib.distortion[c]);
            Console.WriteLine();

            Console.WriteLine("\nExtrinsic:");
            for (int r = 0; r < 4; r++)
            {
                for (int c = 0; c < 4; c++)
                    Console.Write("{0:F6}\t", calib.extrinsic[4 * r + c]);
                Console.WriteLine();
            }
        }

        // 保存BGR亮度图为BMP文件
        static void SaveBgrAsBmp(IntPtr bgrPtr, int width, int height, string filename)
        {
            byte[] bgr = new byte[width * height * 3];
            Marshal.Copy(bgrPtr, bgr, 0, bgr.Length);

            int rowStride = width * 3;
            int fileSize = 54 + rowStride * height;

            byte[] header = new byte[54];
            header[0] = (byte)'B'; header[1] = (byte)'M';
            header[2] = (byte)(fileSize); header[3] = (byte)(fileSize >> 8);
            header[4] = (byte)(fileSize >> 16); header[5] = (byte)(fileSize >> 24);
            header[10] = 54;
            header[14] = 40;
            header[18] = (byte)(width); header[19] = (byte)(width >> 8);
            header[20] = (byte)(width >> 16); header[21] = (byte)(width >> 24);
            int negH = -height;
            header[22] = (byte)(negH); header[23] = (byte)(negH >> 8);
            header[24] = (byte)(negH >> 16); header[25] = (byte)(negH >> 24);
            header[26] = 1;
            header[28] = 24;

            // 交换R和B通道，确保图像颜色正确
            byte[] pixels = new byte[rowStride * height];
            for (int i = 0; i < width * height; i++)
            {
                pixels[i * 3 + 0] = bgr[i * 3 + 2]; // B <- R
                pixels[i * 3 + 1] = bgr[i * 3 + 1]; // G
                pixels[i * 3 + 2] = bgr[i * 3 + 0]; // R <- B
            }

            using (var fs = new System.IO.FileStream(filename, System.IO.FileMode.Create))
            {
                fs.Write(header, 0, 54);
                fs.Write(pixels, 0, pixels.Length);
            }
            Console.WriteLine("Saved: " + filename);
        }

        static void Main(string[] args)
        {
            CameraCls camera = new CameraCls();

            string cameraId = "127.0.0.1";
            int ret_code;

            // 连接相机
            ret_code = camera.DfConnect_Csharp(cameraId);

            int width = 0, height = 0, channels = 0;

            if (ret_code == 0)
            {
                // 必须连接相机成功后，才可获取相机分辨率
                camera.GetCameraResolution_Csharp(out width, out height);
                Console.WriteLine("Width: {0}  Height: {1}", width, height);

                camera.DfGetCameraChannels_Csharp(out channels);
                Console.WriteLine("Channels: {0}", channels);
            }
            else
            {
                Console.WriteLine("Connect Camera Error!");
                Console.ReadKey();
                return;
            }

            // 获取相机标定参数
            CameraCsharp.CalibrationParam calib;
            ret_code = camera.DfGetCalibrationParam_Csharp(out calib);
            if (0 == ret_code)
            {
                PrintCalibration(calib);
            }
            else
            {
                Console.WriteLine("Get Calibration Param Error!");
            }

            // 分配内存保存采集结果
            StringBuilder timestamp = new StringBuilder(30);
            int brightnessSize = (channels == 3) ? (width * height * 3) : (width * height);
            IntPtr brightnessPtr = Marshal.AllocHGlobal(brightnessSize);
            IntPtr depthPtr = Marshal.AllocHGlobal(width * height * sizeof(float));
            IntPtr pcdPtr = Marshal.AllocHGlobal(width * height * 3 * sizeof(float));

            // 设置参数 - 使用JSON配置文件
            StringBuilder config_json = new StringBuilder(20480);
            StringBuilder status_json = new StringBuilder(20480);
            int num;
            camera.DfReadJson_Csharp(config_json, "3.json");
            camera.DfSetParamJson_Csharp(config_json, status_json, out num);

            // 采集数据
            ret_code = camera.DfCaptureData_Csharp(num, timestamp);

            if (0 == ret_code)
            {
                if (channels == 1)
                {
                    // 获取灰度亮度图数据
                    ret_code = camera.DfGetUndistortBrightnessData_Csharp(brightnessPtr);
                    if (0 == ret_code) Console.WriteLine("Get Brightness!");
                }
                else if (channels == 3)
                {
                    // 获取彩色亮度图数据并保存 - 使用BGR格式（点云颜色正确）
                    ret_code = camera.DfGetUndistortColorBrightnessData_Csharp(brightnessPtr, Color.Bgr);
                    if (0 == ret_code)
                    {
                        Console.WriteLine("Get color Brightness!");
                        SaveBgrAsBmp(brightnessPtr, width, height, "bright.bmp");
                    }
                }

                // 获取深度图数据
                ret_code = camera.DfGetUndistortDepthDataFloat_Csharp(depthPtr);
                if (0 == ret_code) Console.WriteLine("Get Depth!");

                // 获取点云数据并保存
                ret_code = camera.DfGetPointcloudData_Csharp(pcdPtr);
                if (0 == ret_code)
                {
                    string ply = (channels == 3) ? "color_cloud.ply" : "pointcloud.ply";
                    string pcd = (channels == 3) ? "color_cloud.pcd" : "pointcloud.pcd";
                    camera.savePointcloudToPly_Csharp(pcdPtr, brightnessPtr, channels, ply);
                    camera.savePointcloudToPcd_Csharp(pcdPtr, brightnessPtr, channels, pcd);
                    Console.WriteLine("Get Pointcloud!");
                }
            }
            else
            {
                Console.WriteLine("Capture Data Error!");
            }

            // 释放内存
            Marshal.FreeHGlobal(brightnessPtr);
            Marshal.FreeHGlobal(depthPtr);
            Marshal.FreeHGlobal(pcdPtr);

            // 断开相机
            camera.DfDisconnect_Csharp(cameraId);

            Console.WriteLine("\n按任意键退出...");
            Console.ReadKey();
        }
    }
}