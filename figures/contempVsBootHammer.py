
# libraries
import numpy as np
import matplotlib.pyplot as plt
 
# set width of bars
barWidth = 0.25
 
# set heights of bars
unmodified = [1, 1, 1, 1, 1, 1]
alpaca_undo = [1.9, 1.6, 1.6, 1.1, 1.1, 2.3] 
alpaca_redo = [3.3, 2.9, 2.3, 1.2, 1.3, 4.1]
chain = [12.2, 10.9, 13.0, 2.0, 10.6, 10.2]
dino = [11.7, 7.9, 38.0, 1.3, 4.0, 4.3]
ratchet = [2.6, 6.4, 6.8, 1.6, 7.7, 6.9]
#bootVerbose = [501.7, 37.3 , 1198.5, 24.7, 1211.2, 7.7]
#bootBasic = [12.3, 36.1, 848.0, 24.1, 276.2, 3.7]
bootStrategic = [6.1, 1.3, 294.3, 1.5, 8.1, 1.3]


f, (axtop, ax, ax2) = plt.subplots(3,1, sharex=True, figsize = (6,6), gridspec_kw={'height_ratios': [1,1, 3]})
 
# Set position of bar on X axis
#r0 = np.arange(len(unmodified))
r0 = [1, 3.5, 6, 8.5, 11, 13.5]
r1 = [x + barWidth for x in r0]
r2 = [x + barWidth for x in r1]
r3 = [x + barWidth for x in r2]
r4 = [x + barWidth for x in r3]
r5 = [x + barWidth for x in r4]
r6 = [x + barWidth for x in r5]
 
unmodColor = '#e6f7ff'
bootColor = '#339966'
undoColor = '#b3e6ff'
redoColor = '#80d4ff'
chainColor = '#4dc3ff'
dinoColor = '#1ab2ff'
ratColor = '#0099e6'


# Make the plot

axtop.bar(r0, unmodified, color=unmodColor, width=barWidth, edgecolor='black', label='Unmodified')
axtop.bar(r1, bootStrategic, color=bootColor, width=barWidth, edgecolor='black', label='BootHammer')
axtop.bar(r2, alpaca_undo, color=undoColor, width=barWidth, edgecolor='black', label='Alpaca-undo')
axtop.bar(r3, alpaca_redo, color=redoColor, width=barWidth, edgecolor='black', label='Alpaca-redo')
axtop.bar(r4, chain, color=chainColor, width=barWidth, edgecolor='black', label='Chain')
axtop.bar(r5, dino, color=dinoColor, width=barWidth, edgecolor='black', label='Dino')
axtop.bar(r6, ratchet, color=ratColor, width=barWidth, edgecolor='black', label='Ratchet')



ax.bar(r0, unmodified, color=unmodColor, width=barWidth, edgecolor='black', label='Unmodified')
ax.bar(r1, bootStrategic, color=bootColor, width=barWidth, edgecolor='black', label='BootHammer')
ax.bar(r2, alpaca_undo, color=undoColor, width=barWidth, edgecolor='black', label='Alpaca-undo')
ax.bar(r3, alpaca_redo, color=redoColor, width=barWidth, edgecolor='black', label='Alpaca-redo')
ax.bar(r4, chain, color=chainColor, width=barWidth, edgecolor='black', label='Chain')
ax.bar(r5, dino, color=dinoColor, width=barWidth, edgecolor='black', label='Dino')
ax.bar(r6, ratchet, color=ratColor, width=barWidth, edgecolor='black', label='Ratchet')
 
ax2.bar(r0, unmodified, color=unmodColor, width=barWidth, edgecolor='black', label='Unmodified')
ax2.bar(r1, bootStrategic, color=bootColor, width=barWidth, edgecolor='black', label='BootHammer')
ax2.bar(r2, alpaca_undo, color=undoColor, width=barWidth, edgecolor='black', label='Alpaca-undo')
ax2.bar(r3, alpaca_redo, color=redoColor, width=barWidth, edgecolor='black', label='Alpaca-redo')
ax2.bar(r4, chain, color=chainColor, width=barWidth, edgecolor='black', label='Chain')
ax2.bar(r5, dino, color=dinoColor, width=barWidth, edgecolor='black', label='Dino')
ax2.bar(r6, ratchet, color=ratColor, width=barWidth, edgecolor='black', label='Ratchet')

axtop.set_ylim(275, 300)
ax.set_ylim(30, 45);
ax2.set_ylim(0, 15);
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
ax.legend(loc = 'lower right')
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
ax2.yaxis.set_label_coords(-0.075, 1.0)
#plt.xlabel('Benchmarks', fontweight='bold')
#plt.ylabel('Normalized Overhead', fontweight='bold')
#plt.xlabel.coords(5,0)
plt.xticks([r + barWidth for r in r2], ['BC', 'Blowfish', 'Cuckoo', 
'AR', 'RSA', 'CEM'])
 
axtop.set_title("Strategic BootHammer vs State-of-the-Art", x=0.5, y = 1.1)
# Create legend & Show graphic
#plt.legend(loc='upper left')
plt.show()
