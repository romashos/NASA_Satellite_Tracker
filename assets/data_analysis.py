import matplotlib
matplotlib.use('Agg') 
import matplotlib.pyplot as plt
import matplotlib.dates as mdates
import pandas as pd
import datetime
import numpy as np
from array import *

#HIST   Elevation (subjective perceivement of the altitude of the satellite, depends on observer POV) over time
def elevation_histogram (elevation_arr):

    for value in elevation_arr: 
        round(value, 1);

    df = pd.DataFrame({
        'Value': elevation_arr
    })

    plt.figure(figsize=(10, 5))
    plt.hist(df['Value'], bins=20, edgecolor='black')
    plt.title('Histogram')
    plt.xlabel('Elevation')
    plt.ylabel('Elevation Value Occurence Frequency')
    plt.grid(True) #enable grid
    
    plt.savefig('Histogram.png')
    plt.close()

#TIME_SERIES_LAT    Latitude vs time
def lat_vs_time_graph(date_arr, lat_arr):

    date_arr = pd.to_datetime(date_arr)
    df = pd.DataFrame({'Date': date_arr, 'LatitudeVal': lat_arr})

    df.sort_values('Date', inplace=True)

    plt.figure(figsize=(10, 5))
    plt.plot(df['Date'], df['LatitudeVal'], marker='o', linestyle='-', color='b')

    plt.title('Latitude Time Series Plot')
    plt.xlabel('Date')
    plt.ylabel('Latitude')

    plt.grid(True)

    plt.xticks(rotation=45)

    plt.tight_layout()
    
    plt.savefig('Latitude_time_series.png')
    plt.close()

#TIME_SERIES_LONG   Longitude vs time
def long_vs_time_graph(date_arr, long_arr):

    date_arr = pd.to_datetime(date_arr)
    df = pd.DataFrame({'Date': date_arr, 'LongitudeVal': long_arr})

    df.sort_values('Date', inplace=True)

    # Plotting
    plt.figure(figsize=(10, 5))
    plt.plot(df['Date'], df['LongitudeVal'], marker='o', linestyle='-', color='b')

    plt.title('Longitude Time Series Plot')
    plt.xlabel('Date')
    plt.ylabel('Longitude')

    plt.grid(True)
    plt.xticks(rotation=45)

    plt.tight_layout()
    plt.savefig('Longitude_time_series.png')
    plt.close()

#ROLL_STAT          Rolling mean and Rolling std for altitude
def rolling_mean_and_std_altitude_graph (date_arr, altitude_arr):

    date_arr = pd.to_datetime(date_arr)   

    df = pd.DataFrame({
        'Date': date_arr,
        'AltitudeVal': altitude_arr
    })

    df.set_index('Date', inplace=True)
    df.sort_index(inplace=True)

    window_size = 3  # window size for rolling calculations
    df['Rolling_Mean'] = df['AltitudeVal'].rolling(window=window_size).mean()
    df['Rolling_Std'] = df['AltitudeVal'].rolling(window=window_size).std()

    plt.figure(figsize=(10, 5))

    # Plot the original values
    plt.plot(df.index, df['AltitudeVal'], label='Original Altitude', color='blue', marker='o')

    # Plot the rolling mean
    plt.plot(df.index, df['Rolling_Mean'], label=f'Rolling Mean (window={window_size})', color='red')

    # Plot the rolling std
    plt.plot(df.index, df['Rolling_Std'], label=f'Rolling Std (window={window_size})', color='green')

    # Add titles and labels
    plt.title('Altitude Rolling Mean and Standard Deviation')
    plt.xlabel('Date')
    plt.ylabel('Altitude')
    plt.legend()
    plt.grid(True)
    
    # Formats the dates to make more readable
    ax = plt.gca()
    ax.xaxis.set_major_locator(mdates.DayLocator())
    ax.xaxis.set_major_formatter(mdates.DateFormatter('%Y-%m-%d'))
    plt.xticks(rotation=90, fontsize=6)

    # Show plot
    plt.tight_layout()
    plt.savefig('Altitude_rolling_mean_std.png')
    plt.close()

#DECL_AND_ALT_CORR  Correlation between the declination angle and altitude of the satellite
def declination_altitude_correlation(date_arr, altitude_arr, declination_arr):

    date_arr = pd.to_datetime(date_arr)

    # Filter the arrays for significant changes
    filtered_indices = [i for i in range(1, len(altitude_arr)) if abs(altitude_arr[i] - altitude_arr[i - 1]) > 10]
    filtered_date_arr = [date_arr[i] for i in filtered_indices]
    filtered_altitude_arr = [altitude_arr[i] for i in filtered_indices]
    filtered_declination_arr = [declination_arr[i] for i in filtered_indices]

    df = pd.DataFrame({
        'Date': filtered_date_arr,
        'DeclinationVal': filtered_declination_arr,
        'AltitudeVal': filtered_altitude_arr
    })   
    
    df.set_index('Date', inplace=True)
    df.sort_index(inplace=True)
    
    plt.figure(figsize=(10, 5))
    plt.plot(df.index, df['AltitudeVal'], label='Altitude', marker='o', color='purple')  # No lines
    plt.plot(df.index, df['DeclinationVal'], label='Declination Angle', marker='x', color='green')  # No lines linestyle=''

    plt.title('Altitude and Declination Angle Over Time')
    plt.xlabel('Date')
    plt.ylabel('Altitude (m) / Declination Angle (degrees)')  
    plt.legend()
    plt.grid(True)

    correlation = df['AltitudeVal'].corr(df['DeclinationVal'])
    plt.text(0, 1.01, f'Correlation: {correlation:.2f}', transform=plt.gca().transAxes,
         fontsize=11, verticalalignment='bottom', horizontalalignment='left',
         bbox=dict(facecolor='white', alpha=0.5, edgecolor='none'))

    y_ticks = range(0, int(df['AltitudeVal'].max()) + 50, 50)
    plt.yticks(y_ticks)
    
    # Formats the dates to make more readable
    ax = plt.gca()
    ax.xaxis.set_major_locator(mdates.DayLocator())
    ax.xaxis.set_major_formatter(mdates.DateFormatter('%Y-%m-%d'))
    plt.xticks(rotation=90, fontsize=6)
    plt.tight_layout()

    plt.savefig('Declination_altitude_corr.png')
    plt.close()
   