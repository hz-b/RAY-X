import matplotlib.pyplot as plt
from matplotlib import cm
import pandas as pd
import numpy as np
import h5py
import sys
import enum


class PlotType(enum.Enum):
    """
    Type of plots/functions to use for plotting
    """
    _plotLikeRAYUI = 1
    _plotForEach = 2
    _plotForEachSubplot = 3


def importOutput(filename: str):
    """
    Import output h5 format and clean data
    """
    # file will be closed when we exit from WITH scope
    with h5py.File(filename, 'r') as h5f:
        # default, Columns are hard-coded, to be changed if necessary
        _keys = [int(i)for i in list(h5f.keys())]
        _keys.sort()
        
        _dfs= []
        for key in _keys:
            dataset = h5f[str(key)]
            _df = pd.DataFrame(dataset,columns=['Xloc', 'Yloc', 'Zloc', 'Weight', 'Xdir', 'Ydir', 'Zdir', 'Energy',
                                               'Stokes0', 'Stokes1', 'Stokes2', 'Stokes3', 'pathLength', 'order', 'lastElement', 'extraParam'])
            _dfs.append(_df)
        df = pd.concat(_dfs,axis=0) # concat once done otherwise, too memory intensive
    return df


def plotLikeRAYUI(df: pd.DataFrame, title: str):
    """
    Plot like RAY-UI in correct order
    """

    correct_order = df['extraParam'].max()  # RAY-UI order like
    ray_count = df.shape[0]
    # Weight column is _Decaprecated_. Due to incorrect weight assignment on dynamic tracing. TODO: Fix/Rewrite shader

    df = df[df['extraParam'] == correct_order]

    f = plt.figure(figsize=(10, 10))

    f.canvas.manager.set_window_title('Image Plane Footprint')
    x = df['Xloc']
    y = df['Yloc']
    plt.scatter(x, y, s=0.2, label='Ray')
    plt.xlabel('x / mm')
    plt.ylabel('y / mm')

    plt.text(plt.xlim()[1], plt.ylim()[0], "Generated by RAY-X.",
             color='grey', fontstyle='italic', fontsize='x-small', ha='right')
    plt.title(title.split('.')[0] + ' footprint')
    plt.legend([f'Ray {df.shape[0]}({ray_count})'], markerscale=4)
    plt.show()


def plotForEach(df: pd.DataFrame, title: str):
    """
    Plot all intersections on multiple figures
    """

    print("Plotting for:")
    for unique in df['extraParam'].unique():
        print(int(unique))
        temp_df = df[df['extraParam'] == unique]
        f = plt.figure(figsize=(10, 10))
        f.canvas.manager.set_window_title(
            str('Image Plane Footprint '+str(int(unique))))
        x = temp_df['Xloc']
        y = temp_df['Yloc']
        _size = 0.2
        if (temp_df.shape[0] < 50):
            _size = 5
        plt.scatter(x, y, s=_size, label='Ray')
        plt.xlabel('x / mm')
        plt.ylabel('y / mm')
        plt.text(plt.xlim()[1], plt.ylim()[0], "Generated by RAY-X.",
                 color='grey', fontstyle='italic', fontsize='x-small', ha='right')
        plt.title(title.split('.')[0] + ' footprint')
        plt.legend(['Ray'], markerscale=4)
    plt.show()

    return


def plotForEachSubplot(df: pd.DataFrame, title: str, _single_color=False):
    """
    Plots all intersections in one figure under multiple subplots
    Set _single_color to true if you want all subplots to be Blue
    """

    print("[PYTHON] Subplotting for:")
    _uniqueCount = len(df['extraParam'].unique())
    _cols = int(_uniqueCount / 2) + 1
    i = 1
    f = plt.figure(figsize=(20, 20))
    f.canvas.manager.set_window_title('Image Plane Footprint')  # Qt stuff..
    f.canvas.manager.window.showMaximized()  # Qt stuff..

    # https://matplotlib.org/stable/tutorials/colors/colormaps.html
    # If color unclear or not to your taste.
    color = 'tab20b'
    viridis_col = cm.get_cmap(color, _uniqueCount+1)
    for unique in df['extraParam'].unique():
        print('[PYTHON] '+str(int(unique)))
        temp_df = df[df['extraParam'] == unique]
        plt.subplot(2, _cols, i)
        x = temp_df['Xloc']
        y = temp_df['Yloc']
        _size = 0.2
        if (temp_df.shape[0] < 50):
            _size = 5
        if not(_single_color):
            plt.scatter(x, y, s=_size, label='Ray', color=viridis_col.colors[i])
        else:
            plt.scatter(x, y, s=_size, label='Ray')
        plt.xlabel('x / mm')
        plt.ylabel('y / mm')
        plt.text(plt.xlim()[1], plt.ylim()[0], "Generated by RAY-X.",
                 color='grey', fontstyle='italic', fontsize='x-small', ha='right')
        plt.title(str(int(unique)))
        i += 1
    plt.suptitle(title.split('.')[0] + ' footprint')
    plt.legend(['Ray'], markerscale=1)
    # plt.tight_layout()
    plt.show()


def plotOutput(filename: str, title: str, plot_type: int):
    """
    Plots output file to a matplotlib figure(s)
    """

    df = importOutput(filename)

    #TODO: Enhance
    _whichPlot = PlotType(plot_type)

    if (_whichPlot == PlotType._plotForEach):
        plotForEach(df, title)
    elif(_whichPlot == PlotType._plotLikeRAYUI):
        plotLikeRAYUI(df, title)
    elif(_whichPlot == PlotType._plotForEachSubplot):
        plotForEachSubplot(df, title)


if __name__ == '__main__':
    plotOutput('output.h5', str(sys.argv[1]), int(sys.argv[2]))
