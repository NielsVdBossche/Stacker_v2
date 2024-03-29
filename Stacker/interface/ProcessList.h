#ifndef PROCESSLIST_H
#define PROCESSLIST_H

#include "Process.h"
#include "ProcessSet.h"
#include "Uncertainty.h"
#include "Histogram.h"

#include "../../Helpers/interface/thTools.h"
#include <TString.h>
#include <TFile.h>
#include <THStack.h>

class ProcessList {
    private:
        std::vector<TString> allProcessNames;
        Process* head = nullptr;
        Process* tail = nullptr;

        Uncertainty* headUnc = nullptr;
        Uncertainty* tailUnc = nullptr;

        bool verbose = false;
        bool veryVerbose = false;

    public:
        ProcessList() = default;
        ~ProcessList();

        Process* addProcess(TString& name, int color, TFile* inputfile, TFile* outputfile, bool signal, bool data, bool oldStuff);
        Process* addProcess(TString& name, int color, std::vector<TFile*>& inputfiles, TFile* outputfile, bool signal, bool data, bool oldStuff);
        
        Process* addProcess(TString& name, std::vector<TString>& procNames, int color, TFile* inputfile, TFile* outputfile, bool signal, bool data, bool oldStuff);
        Process* addProcess(TString& name, std::vector<TString>& procNames, int color, std::vector<TFile*>& inputfiles, TFile* outputfile, bool signal, bool data, bool oldStuff);

        Uncertainty* addUncertainty(std::string& name, bool flat, bool envelope, bool corrProcess, bool eraSpec, std::vector<TString>& processes, TFile* outputfile);
        
        Uncertainty* getUncHead() {return headUnc;}

        Process* getHead() {return head;}
        Process* getTail() {return tail;}
        std::vector<TString> getAllProcessNames() {return allProcessNames;};
        std::vector<Process*> getAllProcess();

        std::vector<TH1D*> fillStack(THStack* stack, Histogram* hist, TLegend* legend, TFile* outfile, std::vector<TH1D*>* signalHistograms, TH1D** sysUnc);
        std::map<TString, bool> printHistograms(Histogram* hist, TFile* outfile, bool isData, Process* dataProc);
        std::vector<TH2D*> fill2DStack(THStack* stack, TString& histogramID, TLegend* legend);

        std::map<std::string, std::pair<TH1D*, TH1D*>> UpAndDownHistograms(Histogram* hist, std::vector<TH1D*>& nominalHists);
        std::vector<TH1D*> CreateHistogramAllProcesses(Histogram* hist);

        void setVerbosity(bool verbosity) {verbose = verbosity;}
        void setVeryVerbosity(bool verbosity) {veryVerbose = verbosity;}


};

#endif