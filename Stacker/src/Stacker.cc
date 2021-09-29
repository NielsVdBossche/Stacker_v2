#include "../interface/Stacker.h"

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <functional>

Stacker::Stacker(const char* rootFilename, std::string& settingFile) {
    // Constructer should either call parser or take output of parser
    // Constructer should build processlist and set its own settings

    // Processlist should be built based on:
    // - Directory's in root file: set of directories for all categories,
    //   under these a set of directories related to different processes/years 
    //   with all correctly normalized histograms belonging to this category
    // - Some component telling which directory in the file belongs with which process
    // - Order + colors defined in settings -> from this it sounds best to use parser inside of constructor
    //   OR build processlist in parser
    // 

    TH1::AddDirectory(false);
    setTDRStyle();

    inputfile = new TFile(rootFilename, "read");
    processes = new ProcessList();

    std::ifstream infile(settingFile);
    std::string line;

    if (! infile.is_open()) {
        std::cout << "ERROR: Settingsfile not found" << std::endl;
        exit(1);
    }

    while (getline(infile, line)) {
        // IF CONTAINS # or only whitespace: ignore line
        std::cout << line << std::endl;
        if(! considerLine(&line)){
            continue;
        }
        
        if (line.find("BROAD SETTINGS") != std::string::npos || line.find("HISTOGRAMS") != std::string::npos) {
            std::cout << "Parsing BROAD SETTINGS" << std::endl;
            break;
        }
        // first, a processname is read, from this, a processelement is built and for this processname a set of directories is achieved
        std::istringstream stream(line);
        
        std::string processName;
        std::string colorString;
        std::string type;
        stream >> processName >> colorString >> type;
        
        // Color_t currentColor = std::stoi(colorString);
        TString processNameAlt(processName);

        bool signal = false;
        bool data = false;
        if (type.find('S') != std::string::npos) {
            signal = true;
        } else if (type.find('D') != std::string::npos) {
            data = true;
        } else if (type.find('B') == std::string::npos) {
            std::cerr << "Error: type identifier '" << type << "' unknown" << std::endl;
            exit(1);
        }

        processes->addProcess(processNameAlt, std::stoi(colorString), inputfile, signal, data);
    }

    while (getline(infile, line)) {
        if (!considerLine(&line)) {
            continue;
        }

        if (line.find("HISTOGRAMS") != std::string::npos) {
            std::cout << "Parsing HISTOGRAMS" << std::endl;
            break;
        }

        std::pair<std::string, std::string> currSetAndVal = splitSettingAndValue(line);

        if (currSetAndVal.first == "Lumi") {
            setLumi(currSetAndVal.second);
        } else if (currSetAndVal.first == "Drawopt") {
            setDrawOpt(currSetAndVal.second);
        }
        // Set gen settings
    }

    // walk in inputfile to first process, check all histogram names   
    inputfile->cd("Nominal");
    gDirectory->cd(processes->getHead()->getName());
    gDirectory->cd(gDirectory->GetListOfKeys()->At(0)->GetName());

    TList* histogramsAvailable = gDirectory->GetListOfKeys();

    for (auto const&& obj : *histogramsAvailable) {
        Histogram* hist = new Histogram(TString(obj->GetName()));
        histogramVec.push_back(hist);
    }

    while (getline(infile, line)) {
        if (!considerLine(&line)) {
            continue;
        }
        // Set histograms to check

        std::string histID;
        std::istringstream stream(line);

        stream >> histID;
        
        std::vector<Histogram*>::iterator it;
        it = std::find_if(histogramVec.begin(), histogramVec.end(), std::bind(Histogram::searchHist, std::placeholders::_1, histID));

        if (it == histogramVec.end()) {
            std::cout << histID << " not found!" << std::endl;
            continue;
        } else {
            std::cout << "FOUND " << histID << std::endl;
        }

        // Manage settings. Call function in histogram class maybe to parse and fix setting.
        (*it)->readSettings(stream);

    }
    outputfile = new TFile("Combinefile.root", "recreate");
}

Stacker::~Stacker() {
    inputfile->Close();
    outputfile->Close();

    delete inputfile;
    delete outputfile;
}

void Stacker::printAllHistograms() {
    for (auto histogramID : histogramVec) {
        printHistogram(histogramID);
    }
}

void Stacker::printHistogram(Histogram* hist) {
    TString histID = hist->getID();

    THStack* histStack = new THStack(histID, histID);
    TLegend* legend = getLegend();
    std::vector<TH1D*> histVec = processes->fillStack(histStack, histID, legend, outputfile);

    stackSettingsPreDraw(histStack, histVec);

    TCanvas* canv = getCanvas(histID);
    canv->Draw();
    canv->cd();
    TPad* pad = getPad(histID);
    pad->Draw();
    pad->cd();

    histStack->Draw(drawOpt.c_str());

    stackSettingsPostDraw(pad, histStack, hist, histVec[0]);

    pad->Update();
    pad->Modified();

    legend->Draw();

    TLatex* info = getDatasetInfo(pad);

    canv->Print("Output/" + histID + ".png");
}