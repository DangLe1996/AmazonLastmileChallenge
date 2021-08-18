import csv
import fnmatch

import numpy as np
from natsort import natsorted

from .utils import getAreaID, getRegionID
import pandas as pd
from sklearn.tree import DecisionTreeClassifier



def getAreaClf(Routes):
    #get routes with only 1 zone change
    trainData = []
    for r in Routes:
        interaction = []
        firstStop = np.argsort(r.actual)[1]
        firstZoneID = r.stops[firstStop].zone_id
        firstRegion = getRegionID(firstZoneID)
        lastZoneID = ' '
        for s in np.argsort(r.actual)[1:]:
            currentZone = r.stops[s].zone_id
            currentRegion = getRegionID(currentZone)
            if currentRegion == firstRegion:
                interaction.append(currentZone)
                lastZoneID = currentZone
            else:
                break
        sourceZoneID = firstZoneID
        dstZoneID = lastZoneID
        firstStopArea = getAreaID(sourceZoneID)
        lastStopArea = getAreaID(dstZoneID)
        regionID = getRegionID(sourceZoneID)
        reverse = False
        temp = regionID.split('-')
        if firstStopArea > lastStopArea:
            reverse = True
        codeNew = int(str(hash(regionID))[1:5])
        trainData.append([temp[0],temp[1],codeNew,reverse,firstStopArea])
    trainDF = pd.DataFrame(trainData,columns  = ['Code','Number','CodeNew', 'Ascending',
                                                'Area'])
    features = ['CodeNew']
    X = trainDF[features]
    y = trainDF['Ascending']
    clf = DecisionTreeClassifier(class_weight='balanced')
    clf.fit(X,y)
    return clf


def getRegionClf(S1_Routes):
    uniqueList = []
    for testRoute in S1_Routes:
        sequence = list(np.argsort(testRoute.actual))

        regionIds = [getRegionID(testRoute.stops[s].zone_id) for s in sequence]
        values = list(pd.unique(regionIds))
        values.remove(' ')
        if len(values) > 1:
            uniqueList.append([values[0],values[-1]])
    trainData = []
    for src, dst in uniqueList:
        letter = src.split('-')[0]
        if src[0] == dst[0]:
            reverse = False
            if src > dst:
                reverse = True
            trainData.append([letter, reverse])
    trainDF = pd.DataFrame(trainData, columns=['Code', 'Ascending'])
    trainDF['CodeNew'] = [ord(x) - 64 for x in trainDF.Code]
    features = ['CodeNew']
    X = trainDF[features]
    y = trainDF['Ascending']
    clf = DecisionTreeClassifier(class_weight='balanced')
    clf.fit(X, y)
    return clf


def getRouteArea(route,regionCode,reverse = False):
    result = []
    for s in route.stops:
        if getRegionID(s.zone_id) == regionCode:
            result.append(getAreaID(s.zone_id))
    result = list(set(result))
    result = natsorted(result,  reverse=reverse)
    return result


def getRegionCode(route):
    zoneIds = [s.zone_id for s in route.stops]
    zoneIds.remove(None)
    RegionIds = [getRegionID(z) for z in zoneIds]
    RegionIds = pd.unique(RegionIds)
    R0 = pd.unique([r[0] for r in RegionIds])
    return R0, RegionIds

def getAreasFromRegion(stopZoneID,region):
    areaList = []
    for zone_id in stopZoneID:
        if getRegionID(zone_id) == region:
            areaList.append(getAreaID(zone_id))
    areaList = list(set(areaList))
    areaList.sort()
    return areaList


def getZoneNumber(zoneIDList,regionCode,AreaCode):
    result = []
    getzoneNum = lambda zoneID: zoneID.split('.')[1][:-1]
    key = regionCode +'.*' + AreaCode
    stopDict = {}

    for s in zoneIDList:
        if fnmatch.fnmatch(s,key):
            result.append(getzoneNum(s))
    result = list(set(result))
    result.sort()
    return result


def getSequence(route,RegionDirection,AreaDirection, ZoneDirection ):

    zoneIds = [s.zone_id for s in route.stops]
    zoneIds.remove(None)
    regionCodes = list(set([getRegionID(z) for z in zoneIds]))
    # print(regionCodes)
    regionCodes = natsorted(regionCodes, reverse=RegionDirection)
    usedAreas = []
    zoneIDNums = []
    RegionDict = {}
    for s in regionCodes:
        areas = getRouteArea(route, s, AreaDirection)
        if len(usedAreas) > 0:
            if usedAreas[-1] != areas[0]:
                AreaDirection = not AreaDirection
                areas = getRouteArea(route, s, AreaDirection)
        RegionDict[s] = areas
        for a in areas:
            usedAreas.append(a)
            zoneIDNums.append(getZoneNumber(zoneIds,s,a))

    zoneIDSequece = getSequenceZoneNumber(zoneIDNums, ZoneDirection)

    zoneIds = [s.zone_id for s in route.stops]

    optimalSequence = []
    zoneLength = []
    optimalSequence.append(route.depot)
    zoneLength.append(1)
    position = 0
    zoneIDList= []
    for s in regionCodes:
        areas = RegionDict[s]
        for a in areas:
            for z in zoneIDSequece[position]:
                key = f'{s}.{z}{a}'
                zoneIDList.append(key)
                indices = [i for i, x in enumerate(zoneIds) if x == key]
                optimalSequence.extend(indices)
                zoneLength.append(len(indices))
            position += 1
    optimalSequence.append(route.depot)
    zoneLength.append(1)

    return optimalSequence, zoneLength, zoneIDList

def getZoneAndAreaDirection(route,RegionClf,AreaClf):
    R0, RegionsIdsNew = getRegionCode(route)
    RegionsIdsNew = RegionsIdsNew.tolist()
    reverseRegion = False
    reverseArea = False

    # %%
    if len(RegionsIdsNew) == 1:
        codeNew = int(str(hash(RegionsIdsNew[0]))[1:5])
        reverseArea = AreaClf.predict(np.array([codeNew]).reshape(-1, 1))[0]

    elif len(R0) == 1:
        codeNew = ord(R0[0]) - 64
        reverseRegion = RegionClf.predict(np.array([codeNew]).reshape(-1, 1))[0]
        RegionsIdsNew = natsorted(RegionsIdsNew, key=lambda y: y.lower(), reverse=reverseRegion)
        region1 = RegionsIdsNew[0]
        region2 = RegionsIdsNew[1]
        stopZoneID = [s.zone_id for s in route.stops]
        stopZoneID.remove(None)
        A1 = getAreasFromRegion(stopZoneID, region1)
        A2 = getAreasFromRegion(stopZoneID, region2)
        if A1[0] == A2[0] or A1[0] == A2[-1]:
            reverseArea = True
        else:
            reverseArea = False
    # print(RegionsIdsNew)
    # print(reverseRegion)
    # print(reverseArea)
    return reverseRegion, reverseArea



def getSequenceZoneNumber(zoneidlist, direction):
    sequence = []
    result = []
    tempSeq = [i for i in zoneidlist]
    for c in tempSeq:
        if len(sequence) == 0:
            temp = sorted(c, reverse=direction)
            sequence.extend(temp)
            result.append(temp)
        else:
            if c[0] == sequence[-1]:
                sequence.extend(c)
                result.append(c)
            elif c[-1] == sequence[-1]:
                temp = sorted(c, reverse=True)
                sequence.extend(temp)
                result.append(temp)
            elif sequence[-1] == 2 and c[0] == 1:
                sequence.extend(c)
                result.append(c)
            elif len(c) == 1:
                sequence.extend(c)
                result.append(c)
            else:
                sequence.extend(c)
                result.append(c)
    return result
















