# %%
import csv
import json
import os
import pickle
from os import path
import sys
sys.path.insert(0,'.')

from .utils import *
from .dataset import ALMCData
from .classifierUtils import *
from .scoring import *
from .tsp import solveRouteClustered

from dask import bag as db

import joblib

from dask.diagnostics.progress import ProgressBar
def runMain(dataPath, outFolder):
    Routes = ALMCData.load(dataPath,score_filter='High Medium', caching=False, want_times=True, want_packages=True)
    Depots = list(set([r.station_code for r in Routes]))
    print('comstruct combi ok')
    for depot in Depots:
        buildClassifier(depot,Routes,outFolder)
    print('comstruct combi ok')


def buildClassifier(depot,Routes, outFolder):
    depotHQRRoutes = list(filter(isInStation(depot), Routes))
    AreaClf = getAreaClf(depotHQRRoutes)
    RegionClf = getRegionClf(depotHQRRoutes)

    RegionPickleFile = '-'.join(['Region', depot]) + '.pkl'
    RegionPath = path.join(outFolder, RegionPickleFile)

    AreaPickleFile = '-'.join(['Area', depot]) + '.pkl'
    AreaPath = path.join(outFolder, AreaPickleFile)

    joblib.dump(AreaClf,AreaPath)
    joblib.dump(RegionClf,RegionPath)

def applyMain(savepath,dataPath, outFolder):

    print('saved path is ', savepath)
    print('data path is ', dataPath)
    print('out folder is', outFolder)
    testRoutes = ALMCData.load(dataPath, caching=False,want_times=True, want_packages=True)
    Depots = list(set([r.station_code for r in testRoutes]))
    result = [masterApply(savepath,d, testRoutes) for d in Depots]

    proposed = {}
    for d in result:
        if d != None:
            proposed.update(d)
    outfile = outFolder +'/proposed_sequences.json'
    with open(outfile, 'w+') as fp:
        json.dump(proposed,fp)

def masterApply(outFolder,station, testRoutes):
    depotHQRRoutes = list(filter(isInStation(station), testRoutes))

    RegionPickleFile = '-'.join(['Region', station]) + '.pkl'
    if os.path.exists(RegionPickleFile):
        RegionPath = path.join(outFolder, RegionPickleFile)

        AreaPickleFile = '-'.join(['Area', station]) + '.pkl'
        AreaPath = path.join(outFolder, AreaPickleFile)

        AreaClf = joblib.load(AreaPath)
        RegionClf = joblib.load(RegionPath)
    else:
        RegionClf = None
        AreaClf = None
    b = db.from_sequence(depotHQRRoutes)
    result = b.map(lambda r: solveClusterATSP(r, RegionClf, AreaClf))
    with ProgressBar():
        sequences = result.compute()
    outDict = {}
    for route, sequence in zip(depotHQRRoutes,sequences):
        seqDict = {}
        for idx,stop in enumerate(route.stops):
            position = sequence.index(idx)
            seqDict[stop.id] = position
        outDict[route.id] = {'proposed': seqDict}
    return outDict

def solveClusterATSP(route,_RegionCLF, _AreaCLF ):
    # if _RegionCLF == None:
    #     regionDirection = True
    #     AreaDirection = True
    # else:
    regionDirection, AreaDirection = getZoneAndAreaDirection(route, _RegionCLF, _AreaCLF)
    bestDist = 1000000000
    bestSeq = []
    for a in [True,False]:
        for i in [True, False]:
            zoneidsequence, zoneidLength, zoneIDList = getSequence(route, regionDirection, a, i)
            optimalSequence, distance = solveRouteClustered(route, route.times, zoneidsequence, zoneidLength)
            if a == AreaDirection:
                   distance = distance * 0.98
            if distance < bestDist:
                bestSeq = optimalSequence
                bestDist = distance
    return bestSeq



def isInStation(stationID):
    return lambda route: route.station_code == stationID
