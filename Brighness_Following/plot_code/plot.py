import serial
import matplotlib.pyplot as plt

# 串口设置（请根据实际情况修改端口和波特率）
ser = serial.Serial('/dev/tty.usbserial-1410', 230400, timeout=1)
print("Opened port:", ser.name)

# 发送参数：Kp=0.1, Ki=0.1, 采样点数=300
command = "0.1 0.1 300\n"
ser.write(command.encode('utf-8'))
print("Sent command:", command.strip())

# 初始化数据存储
ADC_vals = []
REF_vals = []
max_samples = 900  # 采样点数上限

# 开启交互模式（interactive mode），便于实时更新图形
plt.ion()
fig, ax = plt.subplots()
adc_line, = ax.plot([], [], 'r-', label="ADC Value")  # 红色线条表示 ADC
ref_line, = ax.plot([], [], 'b-', label="Reference")  # 蓝色线条表示参考值
ax.set_xlabel("Sample Index")
ax.set_ylabel("ADC / REF Value")
ax.set_title("Real-time ADC Data with Reference")
ax.legend()

# 可根据 ADC 实际范围设置 y 轴（假设 0~1023）
ax.set_xlim(0, max_samples)
ax.set_ylim(0, 1023)

# 读取串口数据
while len(ADC_vals) < max_samples:
    data_line = ser.readline()  # 读取一行数据
    if not data_line:
        print("未收到数据，可能超时了")
        continue
    try:
        # 解码并拆分成字段
        line_str = data_line.decode('utf-8').strip()
        print("Received:", line_str)
        parts = line_str.split()
        
        # 期望的数据格式：剩余点数 ADC值 参考值
        if len(parts) == 3:
            remaining_samples = int(parts[0])
            adc_val = int(parts[1])
            ref_val = int(parts[2])
            
            # 存储数据
            ADC_vals.append(adc_val)
            REF_vals.append(ref_val)
            
            # 更新图像
            adc_line.set_data(range(len(ADC_vals)), ADC_vals)
            ref_line.set_data(range(len(REF_vals)), REF_vals)
            
            # 自适应调整 x 轴
            ax.set_xlim(0, max_samples)
            
            # 重新绘制
            plt.draw()
            plt.pause(0.01)

    except Exception as e:
        print("数据处理错误：", e)
        continue

# 关闭串口，并关闭交互模式，显示最终完整图形
ser.close()
plt.ioff()
plt.show()


