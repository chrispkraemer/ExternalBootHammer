# libraries
import numpy as np
import matplotlib.pyplot as plt
 
# set width of bars
barWidth = 0.25
 
# set heights of bars
#unmodified = [1, 1, 1, 1, 1, 1]
#bootStrategic = [6.1, 1.3, 294.3, 1.5, 8.1, 1.3]
 
unmodified = [1, 1, 1, 1, 1]

bootStrategic = [6.1, 1.3, 1.5, 8.1, 1.3]
# Set position of bar on X axis
r0 = np.arange(len(unmodified))
r3 = [x + barWidth for x in r0]
 
# Make the plot
plt.bar(r0, unmodified, color='#af6d5f', width=barWidth, edgecolor='white', label='Unmodified')
plt.bar(r3, bootStrategic, color='#2d7f5e', width=barWidth, edgecolor='white', label='Strategic')
 
# Add xticks on the middle of the group bars
plt.xlabel('Benchmark', fontweight='bold')
#plt.xticks([r + barWidth+0.25 for r in range(len(bootStrategic))], ['BC', 'Blowfish', 'Cuckoo', 
#'AR', 'RSA', 'CEM'])
 
plt.xticks([r + (barWidth/2) for r in range(len(bootStrategic))], ['BC', 'Blowfish',  
'AR', 'RSA', 'CEM'])
# Create legend & Show graphic
plt.legend()
plt.show()
