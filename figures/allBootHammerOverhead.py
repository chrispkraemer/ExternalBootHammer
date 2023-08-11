
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


f, (axtop, ax, ax2) = plt.subplots(3,1, sharex=True, figsize = (6,6), gridspec_kw={'height_ratios': [1,1, 3]})
 
# Set position of bar on X axis
#r0 = np.arange((len(unmodified))+1.0)
value = 1.25
x = 1
y = x + value 
z = y + value 
xa = z + value 
xb = xa + value 
xc = xb + value 
r0 = [x, y, z, xa, xb, xc]
r1 = [x + barWidth for x in r0]
r2 = [x + barWidth for x in r1]
r3 = [x + barWidth for x in r2]
r4 = [x + barWidth for x in r3]
r5 = [x + barWidth for x in r4]
r6 = [x + barWidth for x in r5]
 
unmodColor = '#ecf9f2'
bootColor = '#339966'
basicColor = '#26734d'
verboseColor = '#133926'


# Make the plot

axtop.bar(r0, unmodified, color=unmodColor, width=barWidth, edgecolor='black', label='Unmodified')
axtop.bar(r1, bootStrategic, color=bootColor, width=barWidth, edgecolor='black', label='Strategic')

axtop.bar(r2, bootBasic, color=basicColor, width=barWidth, edgecolor='black', label='Basic')

axtop.bar(r3, bootVerbose, color=verboseColor, width=barWidth, edgecolor='black', label='Verbose')

ax.bar(r0, unmodified, color=unmodColor, width=barWidth, edgecolor='black', label='Unmodified')
ax.bar(r1, bootStrategic, color=bootColor, width=barWidth, edgecolor='black', label='Strategic')
ax.bar(r2, bootBasic, color=basicColor, width=barWidth, edgecolor='black', label='Basic')
ax.bar(r3, bootVerbose, color=verboseColor, width=barWidth, edgecolor='black', label='Verbose')
 
ax2.bar(r0, unmodified, color=unmodColor, width=barWidth, edgecolor='black', label='Unmodified')
ax2.bar(r1, bootStrategic, color=bootColor, width=barWidth, edgecolor='black', label='Strategic')
ax2.bar(r2, bootBasic, color=basicColor, width=barWidth, edgecolor='black', label='Basic')
ax2.bar(r3, bootVerbose, color=verboseColor, width=barWidth, edgecolor='black', label='Verbose')

axtop.set_ylim(800, 1300)
ax.set_ylim(200, 600);
ax2.set_ylim(0, 40);
ax.spines.bottom.set_visible(False);
axtop.spines.bottom.set_visible(False);
ax2.spines.top.set_visible(False);
ax.spines.top.set_visible(False);
ax.xaxis.set_tick_params(bottom=False)
axtop.xaxis.set_tick_params(top=False)
axtop.xaxis.set_tick_params(bottom=False)
#ax2.xaxis.tick_top()
#axtop.xaxis.tick_top()
ax.tick_params(labeltop=False);
axtop.tick_params(labeltop=False);
ax2.xaxis.tick_bottom()
#axtop.xaxis.tick_bottom()

plt.subplots_adjust(wspace=0,hspace=0.12)
axtop.legend(loc = 'upper left')
#plt.get_legend().remove()

d = .5  # proportion of vertical to horizontal extent of the slanted line
kwargs = dict(marker=[(-1, -d), (1, d)], markersize=12,
              linestyle="none", color='k', mec='k', mew=1, clip_on=False)
ax.plot([0, 1], [0, 0], transform=ax.transAxes, **kwargs)
ax2.plot([0, 1], [1, 1], transform=ax2.transAxes, **kwargs)
ax.plot([0, 1], [1, 1], transform=ax.transAxes, **kwargs)
axtop.plot([0, 1], [0, 0], transform=axtop.transAxes, **kwargs)

#plt.title("Normalized BootHammer Runtimes", loc= "center")
# Add xticks on the middle of the group bars
#ax2.set_xlabel('Benchmarks')
ax2.xaxis.set_label_coords(0.48, -0.2)
ax2.set_ylabel('Runtime (normalied)')
ax2.yaxis.set_label_coords(-0.08, 1.0)
#plt.xlabel('Benchmarks', fontweight='bold')
#plt.ylabel('Normalized Overhead', fontweight='bold')
#plt.xlabel.coords(5,0)
plt.xticks([r +barWidth/2 for r in r1], ['BC', 'Blowfish', 'Cuckoo', 
'AR', 'RSA', 'CEM'])
 
axtop.set_title("Normalized BootHammer Runtimes", x=0.5, y = 1.1)
# Create legend & Show graphic
#plt.legend(loc='upper left')
plt.show()
