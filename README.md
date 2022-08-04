# DCT-Watermark
此專題採用 DCT(discrete cosine transform) 之頻率域(frequency domain) 進行不可視浮水印嵌入。

利用此種浮水印嵌入法，即可以有效的保護圖像著作權之所有人，可有效保障作者的權益。

## 需求
GCC >= 10.4
OpenCV >= v4.2.0

## DEMO
### 圖片嵌入浮水印之結果
![image](https://user-images.githubusercontent.com/29170077/182869426-74b55be9-0af6-4438-bb08-c8b958ceba46.png)

### 嵌入浮水印前、後之圖像差異性(峰值訊噪比，PSNR)
![image](https://user-images.githubusercontent.com/29170077/182869488-bddf3125-f877-419c-a54e-59c7924cd7fc.png)

### 嵌入浮水印後的圖片遭破壞，並提取嵌入之浮水印
![image](https://user-images.githubusercontent.com/29170077/182870068-a3e144fc-c4e8-41f8-8230-d3720f95349b.png)
