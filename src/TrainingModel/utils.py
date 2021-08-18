import fnmatch
from datetime import *
import itertools
from math import sqrt

import numpy as np
from sklearn.cluster import KMeans
from sklearn.metrics import confusion_matrix, balanced_accuracy_score, precision_score
from sklearn.model_selection import train_test_split

from sklearn.neighbors import KNeighborsClassifier
from sklearn.neighbors import NearestNeighbors
from sklearn import preprocessing
import pandas as pd
from sklearn.neural_network import MLPClassifier
from sklearn.tree import DecisionTreeClassifier

getRegionID = lambda zoneID: zoneID.split('.')[0] if zoneID != None else ' '
getAreaID = lambda zoneID: zoneID[-1] if zoneID != None else ' '



def isInStation(stationID):
    return lambda route: route.station_code == stationID

def fixZoneID(route):
    """
    Using 5 nearest neighbors from a stop with missing zone id to assign to that zone.
    :param route:
    """
    coordinates = []
    y = []
    badStopsIndex = []
    for index, s in enumerate(route.stops):
        if index != route.depot:
            if s.zone_id != None and len(s.zone_id.split('-')) != 2:
                badStopsIndex.append(index)
            elif s.zone_id != None and index != route.depot:
                coordinates.append((s.lat, s.lng))
                y.append(s.zone_id)
            elif s.zone_id == None:
                badStopsIndex.append(index)
    neigh = KNeighborsClassifier(n_neighbors=5)

    neigh.fit(coordinates, y)
    for i in badStopsIndex:
        s = route.stops[i]
        test = [s.lat, s.lng]
        predicted = neigh.predict([test])
        route.stops[i].zone_id = predicted[0]

def check_zone(stops):
    same = np.fromiter((src.zone_id == dst.zone_id for src in stops for dst in stops), dtype=np.int_)
    return same.reshape(len(stops), len(stops))

def check_same_cluster(stops):
    sameCluster = lambda src, dst: src.zone_id != dst.zone_id and \
                                   src.zone_id != None and dst.zone_id != None \
                                   and src.zone_id[-1] == dst.zone_id[-1] \
                                    and src.zone_id.split('.')[0] == dst.zone_id.split('.')[0]

    same = np.fromiter((sameCluster(src,dst)
                        for src in stops for dst in stops), dtype=np.int_)
    return same.reshape(len(stops), len(stops))




