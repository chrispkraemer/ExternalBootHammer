# libraries
import numpy as np
import matplotlib.pyplot as plt
 
# set width of bars
barWidth = 0.25
 
# set heights of bars
unmodified = [1, 1, 1, 1, 1, 1]
bootVerbose = [501.7, 37.3 , 1198.5, 24.7, 1211.2, 7.7]
bootBasic = [12.3, 36.1, 848.0, 24.1, 276.2, 3.7]
bootStrategic = [6.1, 1.3, 294.3, 1.5, 8.1, 1.3]

f, (ax, ax2) = plt.subplots(2,1, sharex=True)
 
# Set position of bar on X axis
r0 = np.arange(len(unmodified))
r1 = [x + barWidth for x in r0]
r2 = [x + barWidth for x in r1]
r3 = [x + barWidth for x in r2]
 
# Make the plot
ax.bar(r0, unmodified, color='#af6d5f', width=barWidth, edgecolor='white', label='Unmodified')
ax.bar(r1, bootStrategic, color='#2d7f5e', width=barWidth, edgecolor='white', label='Strategic')
ax.bar(r2, bootBasic, color='#557f2d', width=barWidth, edgecolor='white', label='Basic')
ax.bar(r3, bootVerbose, color='#7f6d5f', width=barWidth, edgecolor='white', label='Verbose')
 
ax2.bar(r0, unmodified, color='#af6d5f', width=barWidth, edgecolor='white', label='Unmodified')
ax2.bar(r1, bootStrategic, color='#2d7f5e', width=barWidth, edgecolor='white', label='Strategic')
ax2.bar(r2, bootBasic, color='#557f2d', width=barWidth, edgecolor='white', label='Basic')
ax2.bar(r3, bootVerbose, color='#7f6d5f', width=barWidth, edgecolor='white', label='Verbose')


ax.set_ylim(250, 1250);
ax2.set_ylim(0, 40);
ax.spines.bottom.set_visible(False);
ax2.spines.top.set_visible(False);
ax.xaxis.tick_top()
ax.tick_params(labeltop=False);
ax2.xaxis.tick_bottom()

plt.subplots_adjust(wspace=0,hspace=0.1)

d = .5  # proportion of vertical to horizontal extent of the slanted line
kwargs = dict(marker=[(-1, -d), (1, d)], markersize=12,
              linestyle="none", color='k', mec='k', mew=1, clip_on=False)
ax.plot([0, 1], [0, 0], transform=ax.transAxes, **kwargs)
ax2.plot([0, 1], [1, 1], transform=ax2.transAxes, **kwargs)


# Add xticks on the middle of the group bars
plt.xlabel('group', fontweight='bold')
plt.xticks([r + barWidth+(barWidth/2) for r in range(len(bootStrategic))], ['BC', 'Blowfish', 'Cuckoo', 
'AR', 'RSA', 'CEM'])
 
# Create legend & Show graphic
plt.legend()
plt.show()
