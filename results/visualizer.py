#
# This file is part of the JKQ QMAP library which is released under the MIT license.
# See file README.md or go to https://github.com/lucasberent/qsatencoder for more information.
#

import csv
import math
import statistics

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import scipy.optimize as opt
from mpl_toolkits.axes_grid1.inset_locator import InsetPosition, mark_inset


def plotEC():
    filenameEC = ''

    fig, ax = plt.subplots()
    ax.axis('tight')
    ax.axis('off')

    df = pd.read_json(filenameEC)['benchmarks']
    nrRows = len(df)
    eqIdx = math.floor(nrRows / 2)
    rows = []

    for row in range(eqIdx):
        rows.append(
            [df[row]['nrOfQubits'],
             df[row]['numGates'],
             (df[row]['preprocTime'] + df[row]['satConstructionTime']) / 1000,
             (df[row]['solvingTime']) / 1000,
             int(df[row]['z3map']['sat conflicts']) if 'sat conflicts' in df[row]['z3map'] else int(0)
             ])

    ueqIdx = len(df) - eqIdx
    rows2 = []
    for row in range(ueqIdx, len(df), 1):
        rows2.append([
            df[row]['nrOfQubits'],
            df[row]['numGates'],
            (df[row]['preprocTime'] + df[row]['satConstructionTime']) / 1000,
            (df[row]['solvingTime']) / 1000,
            int(df[row]['z3map']['sat conflicts']) if 'sat conflicts' in df[row]['z3map'] else int(0)
        ])
    outfile = open('', 'w')
    writer = csv.writer(outfile)
    for i in range(len(rows)):
        writer.writerow(rows[i] + rows2[i])


def logFunc(x, a, b):
    return a * np.log2(x) + b


def LinFun(x, a, b):
    return np.multiply(a, x) + b


def SqrtFun(x, a, b):
    return a * np.sqrt(x) + b


def QuadFun(x, a, b, c):
    return a * np.square(x) + np.multiply(x, b) + c


def ConstFun(x, a, b):
    return a * np.multiply(x, 0) + b


# preprocessing+satconstruction time in nr of qubits
def plotScaling():
    filenameQB = ''
    filenameQB1 = ''
    filenameQB2 = ''
    filenameQB3 = ''
    filenameCS = ''
    filenameCS1 = ''
    filenameCS2 = ''
    filenameCS3 = ''

    fig, ax = plt.subplots(figsize=(12, 8), layout='constrained', nrows=2, ncols=2)
    dfQB = pd.read_json(filenameQB)['benchmarks']  # 10 depth
    dfQB1 = pd.read_json(filenameQB1)['benchmarks']  # 50 depth
    dfQB2 = pd.read_json(filenameQB2)['benchmarks']  # 250 depth
    dfQB3 = pd.read_json(filenameQB3)['benchmarks']  # 1000 depth
    dfCS = pd.read_json(filenameCS)['benchmarks']  # 5 qubits
    dfCS1 = pd.read_json(filenameCS1)['benchmarks']  # 20 qubits
    dfCS2 = pd.read_json(filenameCS2)['benchmarks']  # 65 qubits
    dfCS3 = pd.read_json(filenameCS3)['benchmarks']  # 127 qubits

    dataQBtime = []
    dataQBtime1 = []
    dataQBtime2 = []
    dataQBtime3 = []
    dataCStime = []
    dataCStime1 = []
    dataCStime2 = []
    dataCStime3 = []
    dataQBclauses = []
    dataQBclauses1 = []
    dataQBclauses2 = []
    dataQBclauses3 = []
    dataCSclauses = []
    dataCSclauses1 = []
    dataCSclauses2 = []
    dataCSclauses3 = []
    xDataCS = []
    xDataQB = []
    stepsize = 10  # stepsize needs to be adapted to benchmarking runs

    for i in range(0, len(dfQB), stepsize):
        xDataQB.append(dfQB[i]['nrOfQubits'])

    for i in range(0, len(dfCS), stepsize):
        xDataCS.append(dfCS[i]['numGates'])

    # get y preproc time and nr clauses for qubit scaling
    for row in range(0, len(dfQB), stepsize):
        time = []
        time1 = []
        time2 = []
        time3 = []
        clauses = []
        clauses1 = []
        clauses2 = []
        clauses3 = []
        for i in range(0, stepsize, 1):
            time.append(dfQB[row + i]['preprocTime'] + dfQB[row + i]['satConstructionTime'])
            time1.append(dfQB1[row + i]['preprocTime'] + dfQB1[row + i]['satConstructionTime'])
            time2.append(dfQB2[row + i]['preprocTime'] + dfQB2[row + i]['satConstructionTime'])
            time3.append(dfQB3[row + i]['preprocTime'] + dfQB3[row + i]['satConstructionTime'])

            twocls = dfQB[row + i]['z3map']['sat mk clause 2ary'] if 'sat mk clause 2ary' in dfQB[row + i][
                'z3map'] else 0
            twocls1 = dfQB1[row + i]['z3map']['sat mk clause 2ary'] if 'sat mk clause 2ary' in dfQB1[row + i][
                'z3map'] else 0
            twocls2 = dfQB2[row + i]['z3map']['sat mk clause 2ary'] if 'sat mk clause 2ary' in dfQB2[row + i][
                'z3map'] else 0
            twocls3 = dfQB3[row + i]['z3map']['sat mk clause 2ary'] if 'sat mk clause 2ary' in dfQB3[row + i][
                'z3map'] else 0

            threecls = dfQB[row + i]['z3map']['sat mk clause 3ary'] if 'sat mk clause 3ary' in dfQB[row + i][
                'z3map'] else 0
            threecls1 = dfQB1[row + i]['z3map']['sat mk clause 3ary'] if 'sat mk clause 3ary' in dfQB1[row + i][
                'z3map'] else 0
            threecls2 = dfQB2[row + i]['z3map']['sat mk clause 3ary'] if 'sat mk clause 3ary' in dfQB2[row + i][
                'z3map'] else 0
            threecls3 = dfQB3[row + i]['z3map']['sat mk clause 3ary'] if 'sat mk clause 3ary' in dfQB3[row + i][
                'z3map'] else 0

            ncls = dfQB[row + i]['z3map']['sat mk clause nary'] if 'sat mk clause nary' in dfQB[row + i]['z3map'] else 0
            ncls1 = dfQB1[row + i]['z3map']['sat mk clause nary'] if 'sat mk clause nary' in dfQB1[row + i][
                'z3map'] else 0
            ncls2 = dfQB2[row + i]['z3map']['sat mk clause nary'] if 'sat mk clause nary' in dfQB2[row + i][
                'z3map'] else 0
            ncls3 = dfQB3[row + i]['z3map']['sat mk clause nary'] if 'sat mk clause nary' in dfQB3[row + i][
                'z3map'] else 0

            clauses.append(twocls + threecls + ncls)
            clauses1.append(twocls1 + threecls1 + ncls1)
            clauses2.append(twocls2 + threecls2 + ncls2)
            clauses3.append(twocls3 + threecls3 + ncls3)

        dataQBtime.append(statistics.mean(time))
        dataQBtime1.append(statistics.mean(time1))
        dataQBtime2.append(statistics.mean(time2))
        dataQBtime3.append(statistics.mean(time3))
        dataQBclauses.append(statistics.mean(clauses))
        dataQBclauses1.append(statistics.mean(clauses1))
        dataQBclauses2.append(statistics.mean(clauses2))
        dataQBclauses3.append(statistics.mean(clauses3))

    # circuit size scaling data
    for row in range(0, len(dfCS), stepsize):
        time = []
        time1 = []
        time2 = []
        time3 = []
        clauses = []
        clauses1 = []
        clauses2 = []
        clauses3 = []
        for i in range(0, stepsize, 1):
            time.append(dfCS[row + i]['preprocTime'] + dfCS[row + i]['satConstructionTime'])
            time1.append(dfCS1[row + i]['preprocTime'] + dfCS1[row + i]['satConstructionTime'])
            time2.append(dfCS2[row + i]['preprocTime'] + dfCS2[row + i]['satConstructionTime'])
            time3.append(dfCS3[row + i]['preprocTime'] + dfCS3[row + i]['satConstructionTime'])

            twocls = dfCS[row + i]['z3map']['sat mk clause 2ary'] if 'sat mk clause 2ary' in dfCS[row + i][
                'z3map'] else 0
            twocls1 = dfCS1[row + i]['z3map']['sat mk clause 2ary'] if 'sat mk clause 2ary' in dfCS1[row + i][
                'z3map'] else 0
            twocls2 = dfCS2[row + i]['z3map']['sat mk clause 2ary'] if 'sat mk clause 2ary' in dfCS2[row + i][
                'z3map'] else 0
            twocls3 = dfCS3[row + i]['z3map']['sat mk clause 2ary'] if 'sat mk clause 2ary' in dfCS3[row + i][
                'z3map'] else 0

            threecls = dfCS[row + i]['z3map']['sat mk clause 3ary'] if 'sat mk clause 3ary' in dfCS[row + i][
                'z3map'] else 0
            threecls1 = dfCS1[row + i]['z3map']['sat mk clause 3ary'] if 'sat mk clause 3ary' in dfCS1[row + i][
                'z3map'] else 0
            threecls2 = dfCS2[row + i]['z3map']['sat mk clause 3ary'] if 'sat mk clause 3ary' in dfCS2[row + i][
                'z3map'] else 0
            threecls3 = dfCS3[row + i]['z3map']['sat mk clause 3ary'] if 'sat mk clause 3ary' in dfCS3[row + i][
                'z3map'] else 0

            ncls = dfCS[row + i]['z3map']['sat mk clause nary'] if 'sat mk clause nary' in dfCS[row + i]['z3map'] else 0
            ncls1 = dfCS1[row + i]['z3map']['sat mk clause nary'] if 'sat mk clause nary' in dfCS1[row + i][
                'z3map'] else 0
            ncls2 = dfCS2[row + i]['z3map']['sat mk clause nary'] if 'sat mk clause nary' in dfCS2[row + i][
                'z3map'] else 0
            ncls3 = dfCS3[row + i]['z3map']['sat mk clause nary'] if 'sat mk clause nary' in dfCS3[row + i][
                'z3map'] else 0

            clauses.append(twocls + threecls + ncls)
            clauses1.append(twocls1 + threecls1 + ncls1)
            clauses2.append(twocls2 + threecls2 + ncls2)
            clauses3.append(twocls3 + threecls3 + ncls3)
        dataCStime.append(statistics.mean(time))
        dataCStime1.append(statistics.mean(time1))
        dataCStime2.append(statistics.mean(time2))
        dataCStime3.append(statistics.mean(time3))
        dataCSclauses.append(statistics.mean(clauses))
        dataCSclauses1.append(statistics.mean(clauses1))
        dataCSclauses2.append(statistics.mean(clauses2))
        dataCSclauses3.append(statistics.mean(clauses3))

    ax[0, 0].plot(xDataQB, dataQBtime, 'o', label='Depth 10', color='b', alpha=0.2)
    ax[0, 0].plot(xDataQB, dataQBtime1, 's', label='Depth 50', color='r', alpha=0.2)
    ax[0, 0].plot(xDataQB, dataQBtime2, 'v', label='Depth 250', color='m', alpha=0.2)
    ax[0, 0].plot(xDataQB, dataQBtime3, 'd', label='Depth 1000', color='g', alpha=0.2)
    optimizedParameters, pcov = opt.curve_fit(QuadFun, xDataQB, dataQBtime)
    optimizedParameters1, pcov1 = opt.curve_fit(QuadFun, xDataQB, dataQBtime1)
    optimizedParameters2, pcov2 = opt.curve_fit(QuadFun, xDataQB, dataQBtime2)
    optimizedParameters3, pcov3 = opt.curve_fit(QuadFun, xDataQB, dataQBtime3)
    func = str(math.ceil(optimizedParameters[1])) + 'x^2+' + str(math.ceil(optimizedParameters[0]))
    func1 = str(math.ceil(optimizedParameters1[1])) + 'x^2+' + str(math.ceil(optimizedParameters1[0]))
    func2 = str(math.ceil(optimizedParameters2[1])) + 'x^2+' + str(math.ceil(optimizedParameters2[0]))
    func3 = str(math.ceil(optimizedParameters3[1])) + 'x^2+' + str(math.ceil(optimizedParameters3[0]))
    print('Param for 0,0 fits:' + func + ',', func1 + ',' + func2 + ',' + func3)
    ax[0, 0].plot(xDataQB, QuadFun(xDataQB, *optimizedParameters), color='b')
    ax[0, 0].plot(xDataQB, QuadFun(xDataQB, *optimizedParameters1), color='r')
    ax[0, 0].plot(xDataQB, QuadFun(xDataQB, *optimizedParameters2), color='m')
    ax[0, 0].plot(xDataQB, QuadFun(xDataQB, *optimizedParameters3), color='g')
    ax[0, 0].legend()
    ax[0, 0].legend(loc=4)
    ax[0, 0].set_yscale('log')
    ax[0, 0].set(xlabel='Number of Qubits')
    ax[0, 0].set(ylabel='Time(ms)')

    ax[0, 1].plot(xDataQB, dataQBclauses, 'o', label='Depth 10', color='b', alpha=0.2)
    ax[0, 1].plot(xDataQB, dataQBclauses1, 's', label='Depth 50', color='r', alpha=0.2)
    ax[0, 1].plot(xDataQB, dataQBclauses2, 'v', label='Depth 250', color='m', alpha=0.2)
    ax[0, 1].plot(xDataQB, dataQBclauses3, 'd', label='Depth 1000', color='g', alpha=0.2)
    optimizedParameters, pcov = opt.curve_fit(ConstFun, xDataQB, dataQBclauses)
    optimizedParameters1, pcov1 = opt.curve_fit(ConstFun, xDataQB, dataQBclauses1)
    optimizedParameters2, pcov2 = opt.curve_fit(ConstFun, xDataQB, dataQBclauses2)
    optimizedParameters3, pcov3 = opt.curve_fit(ConstFun, xDataQB, dataQBclauses3)
    func = str(math.ceil(optimizedParameters[1])) + '+' + str(math.ceil(optimizedParameters[0]))
    func1 = str(math.ceil(optimizedParameters1[1])) + '+' + str(math.ceil(optimizedParameters1[0]))
    func2 = str(math.ceil(optimizedParameters2[1])) + '+' + str(math.ceil(optimizedParameters2[0]))
    func3 = str(math.ceil(optimizedParameters3[1])) + '+' + str(math.ceil(optimizedParameters3[0]))
    print('Param for 0,1 fits:' + func + ',', func1 + ',' + func2 + ',' + func3)
    ax[0, 1].plot(xDataQB, ConstFun(xDataQB, *optimizedParameters), color='b')
    ax[0, 1].plot(xDataQB, ConstFun(xDataQB, *optimizedParameters1), color='r')
    ax[0, 1].plot(xDataQB, ConstFun(xDataQB, *optimizedParameters2), color='m')
    ax[0, 1].plot(xDataQB, ConstFun(xDataQB, *optimizedParameters3), color='g')
    ax[0, 1].legend()
    ax[0, 1].set_yscale('log')
    ax[0, 1].set(xlabel='Number of Qubits')
    ax[0, 1].set(ylabel='Number of Clauses')

    order = np.argsort(xDataCS)
    xDataCS = np.array(xDataCS)[order]
    dataCStime = np.array(dataCStime)[order]
    dataCStime1 = np.array(dataCStime1)[order]
    dataCStime2 = np.array(dataCStime2)[order]
    dataCStime3 = np.array(dataCStime3)[order]
    dataCSclauses = np.array(dataCSclauses)[order]
    dataCSclauses1 = np.array(dataCSclauses1)[order]
    dataCSclauses2 = np.array(dataCSclauses2)[order]
    dataCSclauses3 = np.array(dataCSclauses3)[order]

    ax[1, 0].plot(xDataCS, dataCStime, 'o', label='5 Qubits', color='b', alpha=0.2)
    ax[1, 0].plot(xDataCS, dataCStime1, 's', label='20 Qubits', color='r', alpha=0.2)
    ax[1, 0].plot(xDataCS, dataCStime2, 'v', label='65 Qubits', color='m', alpha=0.2)
    ax[1, 0].plot(xDataCS, dataCStime3, 'd', label='127 Qubits', color='g', alpha=0.2)

    optimizedParameters, pcov = opt.curve_fit(LinFun, xDataCS, dataCStime)
    optimizedParameters1, pcov1 = opt.curve_fit(LinFun, xDataCS, dataCStime1)
    optimizedParameters2, pcov2 = opt.curve_fit(LinFun, xDataCS, dataCStime2)
    optimizedParameters3, pcov3 = opt.curve_fit(LinFun, xDataCS, dataCStime3)
    func = str(math.ceil(optimizedParameters[1])) + 'x+' + str(math.ceil(optimizedParameters[0]))
    func1 = str(math.ceil(optimizedParameters1[1])) + 'x+' + str(math.ceil(optimizedParameters1[0]))
    func2 = str(math.ceil(optimizedParameters2[1])) + 'x+' + str(math.ceil(optimizedParameters2[0]))
    func3 = str(math.ceil(optimizedParameters3[1])) + 'x+' + str(math.ceil(optimizedParameters3[0]))
    print('Param for 1,0 fits:' + func + ',', func1 + ',' + func2 + ',' + func3)
    ax[1, 0].plot(xDataCS, LinFun(xDataCS, *optimizedParameters), color='b')
    ax[1, 0].plot(xDataCS, LinFun(xDataCS, *optimizedParameters1), color='r')
    ax[1, 0].plot(xDataCS, LinFun(xDataCS, *optimizedParameters2), color='m')
    ax[1, 0].plot(xDataCS, LinFun(xDataCS, *optimizedParameters3), color='g')
    ax[1, 0].legend()
    ax[1, 0].set_yscale('log')
    ax[1, 0].set(xlabel='Number of Gates')
    ax[1, 0].set(ylabel='Time(ms)')

    ax[1, 1].plot(xDataCS, dataCSclauses, 'o', label='5 Qubits', color='b', alpha=0.2)
    ax[1, 1].plot(xDataCS, dataCSclauses1, 's', label='20 Qubits', color='r', alpha=0.2)
    ax[1, 1].plot(xDataCS, dataCSclauses2, 'v', label='65 Qubits', color='m', alpha=0.2)
    ax[1, 1].plot(xDataCS, dataCSclauses3, 'd', label='127 Qubits', color='g', alpha=0.2)
    optimizedParameters, pcov = opt.curve_fit(LinFun, xDataCS, dataCSclauses)
    optimizedParameters1, pcov1 = opt.curve_fit(LinFun, xDataCS, dataCSclauses1)
    optimizedParameters2, pcov2 = opt.curve_fit(LinFun, xDataCS, dataCSclauses2)
    optimizedParameters3, pcov3 = opt.curve_fit(LinFun, xDataCS, dataCSclauses3)
    func = str(math.ceil(optimizedParameters[1])) + 'x+' + str(math.ceil(optimizedParameters[0]))
    func1 = str(math.ceil(optimizedParameters1[1])) + 'x+' + str(math.ceil(optimizedParameters1[0]))
    func2 = str(math.ceil(optimizedParameters2[1])) + 'x+' + str(math.ceil(optimizedParameters2[0]))
    func3 = str(math.ceil(optimizedParameters3[1])) + 'x+' + str(math.ceil(optimizedParameters3[0]))
    print('Param for 1,1 fits:' + func + ',', func1 + ',' + func2 + ',' + func3)
    ax[1, 1].plot(xDataCS, LinFun(xDataCS, *optimizedParameters), color='b')
    ax[1, 1].plot(xDataCS, LinFun(xDataCS, *optimizedParameters1), color='r')
    ax[1, 1].plot(xDataCS, LinFun(xDataCS, *optimizedParameters2), color='m')
    ax[1, 1].plot(xDataCS, LinFun(xDataCS, *optimizedParameters3), color='g')
    ax[1, 1].legend()
    ax[1, 1].set_yscale('log')
    ax[1, 1].set(xlabel='Number of Gates')
    ax[1, 1].set(ylabel='Number of Clauses')

    plt.show()


def linfunc(x, k, a):
    return x * k + a


def constant4Fun(x, y):
    return np.full(x.shape, 4 ^ y)

# preprocessing+satconstruction time in nr of qubits
def plotGenerators():
    filename = ''
    filename1 = ''
    filename2 = ''
    xData = []
    xData1 = []
    xData2 = []
    yData = []
    yData1 = []
    yData2 = []

    df = pd.read_json(filename)['benchmarks']
    df1 = pd.read_json(filename1)['benchmarks']
    df2 = pd.read_json(filename2)['benchmarks']
    fig, ax = plt.subplots(figsize=(10, 5), layout='constrained')
    stepsize = 1  # needs to be adapted to benchmarks

    for i in range(0, len(df), stepsize):
        xData.append(df[i]['numGates'])
    for i in range(0, len(df1), stepsize):
        xData1.append(df1[i]['numGates'])
    for i in range(0, len(df2), stepsize):
        xData2.append(df2[i]['numGates'])

    for row in range(0, len(df), stepsize):
        tmp = []
        for i in range(0, stepsize, 1):
            tmp.append(df[row + i]['numGenerators'])
        yData.append(statistics.mean(tmp))

    for row in range(0, len(df1), stepsize):
        tmp = []
        for i in range(0, stepsize, 1):
            tmp.append(df1[row + i]['numGenerators'])
        yData1.append(statistics.mean(tmp))

    for row in range(0, len(df2), stepsize):
        tmp = []
        for i in range(0, stepsize, 1):
            tmp.append(df2[row + i]['numGenerators'])
        yData2.append(statistics.mean(tmp))

    order = np.argsort(xData)
    order1 = np.argsort(xData1)
    order2 = np.argsort(xData2)

    xData = np.array(xData)[order]
    xData1 = np.array(xData1)[order1]
    xData2 = np.array(xData2)[order2]

    yData = np.array(yData)[order]
    yData1 = np.array(yData1)[order1]
    yData2 = np.array(yData2)[order2]

    ax.plot(xData, yData, 'o', label='1 Qubit', alpha=0.2, color='b')
    ax.plot(xData1, yData1, 'd', label='2 Qubits', alpha=0.2, color='r')
    ax.plot(xData2, yData2, 's', label='3 Qubits', alpha=0.2, color='m')
    optimizedParameters, pcov = opt.curve_fit(QuadFun, xData, yData)
    optimizedParameters1, pcov2 = opt.curve_fit(QuadFun, xData1, yData1)
    optimizedParameters2, pcov2 = opt.curve_fit(QuadFun, xData2, yData2)
    func = str(math.ceil(optimizedParameters[1])) + 'x^2+' + str(math.ceil(optimizedParameters[0]))
    ax.plot(xData1, QuadFun(xData1, *optimizedParameters1), color='r')
    ax.plot(xData2, QuadFun(xData2, *optimizedParameters2), color='m')

    ax2 = plt.axes([0, 0, 1, 1])
    ip = InsetPosition(ax, [0.5, 0.2, 0.3, 0.3])
    ax2.set_axes_locator(ip)
    mark_inset(ax, ax2, loc1=2, loc2=4, fc="none", ec='0.5')
    expData = list(range(0, 50))
    ax.plot(expData, np.power(2, expData), label='2^|G|', color='g', )
    ax2.plot(expData[:7], np.power(2, expData[:7]), label='2^|G|', color='g', )
    ax2.plot(xData, yData, 'o', alpha=0.2, color='b')
    ax2.plot(xData1[:50], yData1[:50], 'd', alpha=0.2, color='r')
    ax2.plot(xData2[:50], yData2[:50], 's', alpha=0.2, color='m')
    ax2.plot(xData1[:100], QuadFun(xData1[:100], *optimizedParameters1), color='r')
    ax2.plot(xData2[:100], QuadFun(xData2[:100], *optimizedParameters2), color='m')
    ax2.set_xlim(right=50)
    ax2.set_ylim(top=100)
    ax.set_ylim(top=1000)
    ax.set_xlim(right=1650)
    ax.legend()
    ax.set_yscale('log')
    ax.set_xlabel('Number of Gates')
    ax.set_ylabel('Number of Generators')
    ax.legend(loc=4)
    plt.show()


plotScaling()
plotGenerators()
plotEC()
