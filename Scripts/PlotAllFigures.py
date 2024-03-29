import sys
import os
import subprocess
import time

# execute in Stacker_v2 dir. Use python3 PlotAllFigures.py with location relative to Stacker_v2 dir

eras = ["16", "16PreVFP", "16PostVFP", "17", "18"]

def PlotFigures(inputFiles, settingFile, uncertaintyFile="", observationFiles=[], additionalArguments=""):
    # set cwd to be /user/nivanden/Stacker_v2
    os.chdir('/user/nivanden/Stacker_v2/')
    baseCommand = "./ExecuteStackerOnCondor.py "
    arguments = ""
    for inputFile in inputFiles:
        arguments += inputFile + " "
    
    arguments += "SettingFiles/" + settingFile + " "

    if uncertaintyFile != "":
        arguments += "-unc UncertaintyFiles/" + uncertaintyFile + " "
    
    if len(observationFiles) != 0:
        arguments += "-RD " 
        for inputFile in observationFiles:
            arguments += inputFile + " "
    
    finalCommand = baseCommand + arguments + additionalArguments
    #print(finalCommand)
    subprocess.run(finalCommand.split())
    #os.system(finalCommand)

    # execute terminalcommand

def PlotFiguresLocal(inputFiles, settingFile, uncertaintyFile="", observationFiles=[], additionalArguments=""):
    # set cwd to be /user/nivanden/Stacker_v2
    os.chdir('/user/nivanden/Stacker_v2/')
    baseCommand = "./stacker_exec "
    arguments = ""
    for inputFile in inputFiles:
        arguments += inputFile + " "
    
    arguments += "SettingFiles/" + settingFile + " "

    if uncertaintyFile != "":
        arguments += "-unc UncertaintyFiles/" + uncertaintyFile + " "
    
    if len(observationFiles) != 0:
        arguments += "-RD " 
        for inputFile in observationFiles:
            arguments += inputFile + " "
    
    finalCommand = baseCommand + arguments + additionalArguments
    #print(finalCommand)
    subprocess.run(finalCommand.split())

def PlotSeperateEras(inputFiles, observationFiles):
    for era in eras:
        inputFilesEra = [filename for filename in inputFiles if ("20" + era in filename) or ("Data"+era in filename)]
        obsFilesEra = [filename for filename in observationFiles if ("20" + era in filename) or ("Data"+era in filename)]

        if (GetCR(inputFilesEra[0])):
            uncFile = "fullCR.txt"
            if ("DY" in inputFilesEra[0] or "TTBar" in inputFilesEra[0]):
                settingFile = "OSDL_plots.txt"
                uncFile = ""
            else:
                settingFile = "mainCRDD.txt"
        elif GetDD(inputFilesEra[0]):
            uncFile = "20" + era + ".txt"
            settingFile = "mainDD20" + era + ".txt"
        else:
            uncFile = "cuts20"+era+".txt"
            settingFile = "main.txt"


        #settingFile = GetSettingfile(inputFilesEra[0])
        PlotFigures(inputFilesEra, settingFile, uncFile, obsFilesEra)

    return

def PlotAllErasCombined(inputFiles, observationFiles):
    uncFile = "full.txt"
    if (GetCR(inputFiles[0])):
        uncFile = "fullCR.txt"
    elif not GetDD(inputFiles[0]):
        uncFile = "cutbasedTests.txt"

    settingFile = GetSettingfile(inputFiles[0])
    PlotFigures(inputFiles, settingFile, uncFile, observationFiles)
    return

def ParseInputArguments(inputArgs):
    # TODO: add settingfile overwrite
    inputfiles = []
    obsFiles = []

    # skip 1?
    inputfiles = [filename for filename in inputArgs[1:]]

    if any("-data" in filename for filename in inputfiles): 
        index = inputfiles.index("-data")
        obsFiles = inputfiles[index+1:]
        inputfiles = inputfiles[:index]

    return inputfiles, obsFiles

def GetSettingfile(inputfile):
    base = "main"
    if GetCR(inputfile):
        base += "CR"
    if GetDD(inputfile):
        base += "DD"
    base += ".txt"
    return base
    

def GetDD(inputfile):
    return any(["MCPrompt" in inputfile, "nonPrompt" in inputfile, "ChargeDD" in inputfile])
        

def GetCR(inputfile):
    if "CR" in inputfile:
        return True
    else:
        return False


if __name__ == "__main__":
    inputFiles, obsFiles = ParseInputArguments(sys.argv)

    PlotSeperateEras(inputFiles, obsFiles)
    PlotAllErasCombined(inputFiles, obsFiles)

