from __future__ import absolute_import, division, print_function
"""
@article{liu2018pyeit,
    title={pyEIT: A python based framework for Electrical Impedance Tomography},
    author={Liu, Benyuan and Yang, Bin and Xu, Canhua and Xia, Junying and Dai, Meng and Ji, Zhenyu and You, Fusheng and Dong, Xiuzhen and Shi, Xuetao and Fu, Feng},
    journal={SoftwareX},
    volume={7},
    pages={304--308},
    year={2018},
    publisher={Elsevier}
}
"""
import matplotlib.pyplot as plt
from matplotlib import cm
import numpy as np
import pyeit.eit.jac as jac
from pyeit.eit.fem import EITForward
from pyeit.eit.interp2d import sim2pts
import pyeit.eit.protocol as protocol
import pyeit.mesh as mesh
from pyeit.mesh.shape import thorax
from pyeit.mesh.wrapper import PyEITAnomaly_Circle
import time
import requests

# Replace with the ESP32's IP address from Serial Monitor
ESP32_IP = "192.168.5.1"

n_el = 16  # nb of electrodes
b0 = [-1.0,-1.0] # Bottom left corner of mesh
b1 = [1.0,1.0] # Top right corner of mesh

def setup(n_el,BL_corner,TR_corner):
    """Place n_el electrodes equally spaced on the perimeter of a square.
    
    Parameters
    ----------
    n_el : int
        Number of electrodes
    BL_corner : list
        bottom left corner coords of square
    TR_corner : list
        top right corner coords of square
    
    Returns
    -------
    mesh_obj
        EIT mesh object
    protocol_obj
        EIT protocol settings
    """
    # create EIT mesh
    mesh_obj = mesh.create(n_el, h0=0.1, fd= lambda pts: mesh.shape.rectangle(pts,BL_corner,TR_corner))
    
    # create EIT conditions
    protocol_obj = protocol.create(n_el, dist_exc=1, step_meas=1, parser_meas="std")
    return mesh_obj, protocol_obj

def simEIT(mesh_obj, protocol_obj,anomalyList:list):
    """Simulate an artificial EIT forward problem with anolalies
    
    Parameters
    ----------
    mesh_obj : mesh_obj
        mesh used by the EIT analysis
    protocol_obj : protocol_obj
        protocol the EIT analysis will use
    anomalyList : list
        list of anomaly_type objects to be included in the forward problem
    
    Returns
    -------
    delta_perm
        EIT mesh object
    v0
        baseline voltage readings
    v1
        voltage readings with included anomalies
    """
    mesh_new = None

    # Concatinate all anomalies into the mesh
    for anomaly in anomalyList:
        if mesh_new is not None:
            mesh_new = mesh.set_perm(mesh_new, anomaly=anomaly)
            print("Added Anomaly")
            continue
        else:
            mesh_new = mesh.set_perm(mesh_obj, anomaly=anomaly)
            print("Created Anomaly")

    delta_perm = mesh_new.perm - mesh_obj.perm

    # run forward problem using original mesh to establish simulated v0 values
    fwd = EITForward(mesh_obj, protocol_obj)
    v0 = fwd.solve_eit()
    # run forward problem using anomolized mesh to find simulated v1
    v1 = fwd.solve_eit(perm=mesh_new.perm)
    
    return delta_perm,v0,v1

def initializeEIT(mesh_obj, protocol_obj):
    """Initialize EIT inverse problem
    
    Parameters
    ----------
    mesh_obj : mesh_obj
        mesh used by the EIT analysis
    protocol_obj : protocol_obj
        protocol the EIT analysis will use
    
    Returns
    -------
    eit
        JAC object that is used to analyze the mesh
    """
    eit = jac.JAC(mesh_obj, protocol_obj)
    eit.setup(p=0.2, lamb=0.0001, method="kotre", perm=1, jac_normalized=True)

    return eit

def analyze(pts, tri, v0, v, eit):
    """solve inverse EIT problem
    
    Parameters
    ----------
    pts
        nodes in the mesh
    tri
        elements of the eit mesh
    v0
        baseline voltages
    v1
        measured voltages
    eit
        eit object used for analysis
    
    Returns
    -------
    ds_n
        normalized voltage deltas for all points
    """
    ds = eit.solve(v, v0, normalize=True)
    ds_n = sim2pts(pts, tri, np.real(ds))

    return ds_n

# def retreiveData():
#     # Placeholder loading example data measurements
#     with open("synth.csv","r") as file:
#             values = []
#             for line in file.readlines():
#                 values.append(float(line))
#             v2 = np.asarray(values)
#             return v2
#     pass

def read_flags_from_esp():
    """read communication flags from the ESP
    
    Parameters
    ----------
    none
    
    Returns
    -------
    flags
        dictionary of all flags
    """
    url = f"http://{ESP32_IP}/flags"
    resp = requests.get(url, timeout=.5)
    resp.raise_for_status()  # raise if error
    text = resp.text
    print(text)
    lines = text.splitlines()

    flags = dict()
    
    for line in lines:
        strings = line.split(",")
        flags[strings[0]] = strings[1]
        
    return flags


def read_data_from_esp():
    """read data from the ESP
    
    Parameters
    ----------
    none
    
    Returns
    -------
    values
        list of all voltages recorded by esp32
    """
    url = f"http://{ESP32_IP}/data"
    resp = requests.get(url, timeout=.5)
    resp.raise_for_status()  # raise if error

    # resp.text is a single string with CSV content
    text = resp.text
    print(text)
    lines = text.splitlines()
    values = lines[0].split(",")
    print(values)
    values = [float(x) for x in values[1:]] # First string is a label

    return values

def findCentroid(x, y, ds_n):
    """finds centroid of any major anomalies detected in the analysis
    
    Parameters
    ----------
    x
        x values
    y
        y values
    ds_n
        voltage differences from eit analysis
    
    Returns
    -------
    xbar
        centroid for x values
    ybar
        centroid for y values
    """
    xbar = 0
    ybar = 0
    total = 0

    for i,val in enumerate(ds_n):
        # Square each value
        val **=2
        # Sum the weighted position values
        xbar += x[i]*val
        ybar += y[i]*val
        # Sum the total value
        total += val
    
    # divide by the total value to get the average position
    xbar = xbar/total
    ybar = ybar/total
    print(f"x_bar: {xbar}, y_bar: {ybar}")
    return xbar, ybar

def send_values(xbar, ybar):
    """finds centroid of any major anomalies detected in the analysis
    
    Parameters
    ----------
    xbar
        x values
    ybar
        y values
    ds_n
        voltage differences from eit analysis
    
    Returns
    -------
    none
    """
    url = f"http://{ESP32_IP}/set"
    params = {
        "val1": xbar,
        "val2": ybar
    }

    try:
        response = requests.get(url, params=params, timeout=0.5)
        print("Status code:", response.status_code)
        print("Response text:", response.text)
    except requests.exceptions.RequestException as e:
        print("Error talking to ESP32:", e)

def send_flg(flagName,value):
    url = f"http://{ESP32_IP}/flag"
    params = {
        flagName: value,
    }

    try:
        response = requests.get(url, params=params, timeout=0.5)
        print("Status code:", response.status_code)
        print("Response text:", response.text)
    except requests.exceptions.RequestException as e:
        print("Error talking to ESP32:", e)

# plot initial graph
def plotEITGraphs(mesh_obj, tri, x, y, ds_n,obj1=None,obj2=None):
    if obj1 == None:
        fig, axes = plt.subplots(1,1,figsize=(8, 8), constrained_layout=False)
        fig.set_size_inches(11, 4)
        im = axes.tripcolor(x, y, tri, ds_n, shading="flat")
        for i, e in enumerate(mesh_obj.el_pos):
            axes.annotate(str(i + 1), xy=(x[e], y[e]), color="r")
        axes.set_aspect("equal")
        fig.colorbar(im, ax=axes)#.ravel().tolist())
        plt.show(block=False)
    else:
        fig = obj1[0]
        axes = obj1[1]
        im = obj1[2]
        axes.remove()
        fig.canvas.draw()
        fig.canvas.flush_events()
        im = axes.tripcolor(x, y, tri, ds_n, shading="flat")
        fig.colorbar(im, ax=axes)
        plt.pause(0.005)
    
    obj1 = [fig,axes,im]

    # ax = axes[0]
    # im = ax.tripcolor(x, y, tri, np.real(delta_perm), shading="flat")
    # ax.set_aspect("equal")

    # plot EIT reconstruction
    


    # Plot the surface  
    if obj2 == None:
        fig2, axes2 = plt.subplots(subplot_kw={"projection": "3d"})
        surfplot = axes2.plot_trisurf(x, y, tri,ds_n, vmin=ds_n.min() * 2, cmap=cm.Blues)
        plt.show(block=False)
    else:
        fig2 = obj2[0]
        axes2 = obj2[1]
        surfplot = obj2[2]
        axes.remove()
        fig.canvas.draw()
        fig.canvas.flush_events()
        surfplot = axes2.plot_trisurf(x, y, tri,ds_n, vmin=ds_n.min() * 2, cmap=cm.Blues)
        plt.pause(0.005)

    obj2 = [fig2,axes2,surfplot]

    return obj1,obj2

mesh_obj, protocol_obj = setup(n_el,b0,b1)

anomalyList = [PyEITAnomaly_Circle(center=[0.25, 0.5], r=.2, perm=5000.0),
            PyEITAnomaly_Circle(center=[-0.25, -0.5], r=.2, perm=500.0)]
delta_perm, v0, v1 = simEIT(mesh_obj, protocol_obj, anomalyList)

# v2 = retreiveData()

# extract node, element, alpha
pts = mesh_obj.node
tri = mesh_obj.element
x, y = pts[:, 0], pts[:, 1]
eit = initializeEIT(mesh_obj,protocol_obj)
ds_n = analyze(pts, tri, v0, v1, eit)

xbar,ybar = findCentroid(x, y, ds_n)



figure1, figure2 = plotEITGraphs(mesh_obj, tri, x, y, ds_n)

voltages = []
oldV = []
while True:
    flags = read_flags_from_esp()

    if flags["initializeFLG"]:
        V0 = read_data_from_esp()
        oldV = V0
        print(f"Gathered {len(V0)} baseline data points")
        send_flg("initializeFLG",False)
        continue

    elif flags["readFLG"]:
        voltages = read_data_from_esp()
        if voltages == oldV:
            continue
        print(f"Gathered {len(voltages)} data points")
        oldV = voltages

        ds_n = analyze(pts, tri, V0, voltages, eit)

        xbar,ybar = findCentroid(x, y, ds_n)
        send_values(xbar,ybar)

        figure1, figure2 = plotEITGraphs(mesh_obj, tri, x, y, ds_n,figure1,figure2)
        continue

    else:
        time.sleep(0.1)

    # surfplot.remove()
    # fig.canvas.draw()
    # fig.canvas.flush_events()
    # plt.pause(0.005)
    # send_values(0.52, 0.99)

    # surfplot = ax2.plot_trisurf(x, y, tri,ds_n, vmin=ds_n.min() * 2, cmap=cm.Blues)
    # fig.canvas.draw()
    # fig.canvas.flush_events()
    # plt.pause(0.005)