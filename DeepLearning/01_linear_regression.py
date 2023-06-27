''' 简单线性回归问题 '''
import torch
import torch.nn as nn
import numpy as np
import matplotlib.pyplot as plt
from torch.autograd import Variable

# 确定运算设备 cpu/gpu
if torch.cuda.is_available():
    device = "cuda"
else:
    device = "cpu"

input_size = 1
output_size = 1
learning_rate = 0.001

# xtrain 生成矩阵数据
xtrain = np.array([[2.3], [4.4], [3.7], [6.1], [7.3], [2.1],[5.6],
                    [7.7], [8.7], [4.1], [6.7], [6.1], [7.5], [2.1], [7.2], [5.6], [5.7],
                    [7.7], [3.1]], dtype=np.float32)

# ytrain 生成矩阵数据
ytrain = np.array([[3.7], [4.76], [4.], [7.1], [8.6], [3.5],[5.4],
                    [7.6], [7.9], [5.3], [7.3], [7.5], [8.5], [3.2], [8.7], [6.4], [6.6],
                    [7.9], [5.3]], dtype=np.float32)

#画散点图
plt.figure()
plt.scatter(xtrain,ytrain)

plt.xlabel('xtrain')
plt.ylabel('ytrain')

plt.show()

# 构建线性回归模型
class LinearRegression(nn.Module):
    def __init__(self, input_size, output_size):
        super(LinearRegression, self).__init__()
        self.linear = nn.Linear(input_size, output_size)

    def forward(self, x):
        out = self.linear(x)
        return out

model = LinearRegression(1,1).to(device)

#定义 criterion 误差函数，总平方误差
criterion = nn.MSELoss()

optimizer = torch.optim.SGD(model.parameters(), lr=learning_rate)

# 训练 1000 次
num_epochs = 1000

for epoch in range(num_epochs):
    inputs = Variable(torch.from_numpy(xtrain)).to(device)
    target = Variable(torch.from_numpy(ytrain)).to(device)

    optimizer.zero_grad()
    outputs = model(inputs)

    # 前向传播
    loss = criterion(outputs, target)
    # 计算loss
    loss.backward()

    optimizer.step()

    if(epoch + 1) % 50 == 0:
        print('Epoch [%d/%d], Loss:%.4f' %(epoch+1, num_epochs, loss.item()))


model.eval()
predicted = model(Variable(torch.from_numpy(xtrain)).to(device)).cpu().data.numpy()
plt.plot(xtrain, ytrain, 'ro')
plt.plot(xtrain, predicted, label ='predict')
plt.legend()
plt.show()


