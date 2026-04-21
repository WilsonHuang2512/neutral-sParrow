import camera
import numpy as np

# 连接相机
ip = '127.0.0.1'
ret = camera.DfConnect(ip)

# 获取相机分辨率和通道数
width = np.zeros(1, dtype=np.int32)
height = np.zeros(1, dtype=np.int32)
channels = np.zeros(1, dtype=np.int32)
camera.DfGetCameraResolution(width, height)
camera.DfGetCameraChannels(channels)
print('通道数', channels[0])
print("height:", height[0])
print("width:", width[0])

width = int(width[0])
height = int(height[0])
channels = int(channels[0])

# 读取并设置JSON配置
cfg = camera.DfReadJson("3.json")
status, maxnum = camera.DfSetParamJson(cfg)

# 采集数据
camera.DfCaptureData(maxnum, "time")

# 获取点云数据
pointcloud_data = np.zeros((width * height * 3,), dtype=np.float32)
camera.DfGetPointcloudData(pointcloud_data)

# 获取深度图
depth_data = np.zeros((height, width), dtype=np.float32)
camera.DfGetDepthDataFloat(depth_data)

# 根据通道数选择不同的亮度图获取方式
if channels == 1:
    # 灰度相机
    print("检测到灰度相机，获取灰度亮度图...")
    brightness_data = np.zeros((height, width), dtype=np.uint8)
    camera.DfGetBrightnessData(brightness_data)
    
    # 保存灰度点云
    camera.savePointcloudToPcd(pointcloud_data, brightness_data, channels, "pointcloud.pcd")
    camera.savePointcloudToPly(pointcloud_data, brightness_data, channels, "pointcloud.ply")
    print("灰度点云已保存")
    
elif channels == 3:
    # 彩色相机
    print("检测到彩色相机，获取彩色亮度图...")
    color_data = np.zeros((height, width, 3), dtype=np.uint8)
    camera.DfGetColorBrightnessData(color_data, camera.XemaColor.Bgr)
    
    # 保存彩色点云
    camera.savePointcloudToPcd(pointcloud_data, color_data, channels, "color_cloud.pcd")
    camera.savePointcloudToPly(pointcloud_data, color_data, channels, "color_cloud.ply")
    print("彩色点云已保存")
    
else:
    print(f"警告: 不支持的通道数 {channels}")

# 断开相机
finish = camera.DfDisconnect(ip)
if(finish == 0):
   print('相机已断开')