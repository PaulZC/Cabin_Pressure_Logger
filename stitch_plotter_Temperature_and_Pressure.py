# Plots stitched csv data from Adalogger

# Column 0 = HH:MM:SS DD/MM/YYYY
# Column 1 = MPL3115A2 Pressure (Pa)
# Column 2 = MPL3115A2 Temperature (C)
# Column 3 = Battery Voltage (V)

# Import numpy and matplotlib
import numpy as np
import matplotlib.dates as mdates
import matplotlib.pyplot as plt

# Read the data from the stitch file, converts the times/dates into mdates
times,pressures,temperatures,voltages = np.loadtxt('stitched.csv', \
    delimiter=',', unpack=True, usecols=(0,1,2,3), \
    converters={0:mdates.strpdate2num('%H:%M:%S %d/%m/%Y')})

f, (ax1, ax2, ax3) = plt.subplots(3, sharex=True) # Create three subplots
ax1.plot(times, voltages) # Add the battery voltage to axis 1
ax2.plot(times, temperatures) # Add the temperatures to axis 2
ax3.plot(times, pressures) # Add the pressures to axis 3
f.subplots_adjust(hspace=0) # Close-pack the three axes
plt.setp([a.get_xticklabels() for a in f.axes[:-1]], visible=False)

ax3.xaxis.set_major_formatter(mdates.DateFormatter('%H:%M:%S')) # Show the time along the x axis
plt.xlabel('Time') # Label the x axis

ax1.set_title('Adalogger M0 + DS3231 + MPL3115A2') # Give the chart a title

# Turn on the axis grids
ax1.grid(True) 
ax2.grid(True)
ax3.grid(True)

plt.xticks(rotation=45) # Slant the times at 45 degrees
plt.tight_layout()
plt.show() # Plot the chart
    
