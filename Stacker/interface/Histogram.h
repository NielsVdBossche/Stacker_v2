#ifndef HISTOGRAM_MANAGER_H
#define HISTOGRAM_MANAGER_H

#include <TString.h>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <map>

#include "../../Helpers/interface/ParseTools.h"
#include "../../Helpers/interface/StringTools.h"

class Histogram {
    private:
        TString id;
        std::string cleanName;
        bool printToFile = false;
        bool logScale; // 0 for lin, 1 for log
        bool drawUncertainties = false;
        std::vector<std::string>* xBinLabels = nullptr;

        std::map<TString, bool> relevance;

        int RebinFixed = 1;
        double* RebinVar = nullptr;
    public:
        Histogram(TString histID);
        Histogram(TString histID, bool requireLogScale);
        ~Histogram();

        TString getID() const {return id;};
        bool isLogscale() const {return logScale;};

        bool getDrawUncertainties() {return drawUncertainties;}
        void setDrawUncertainties(bool draw) {drawUncertainties = draw;}
        
        std::vector<std::string>* getXBinLabels() const {return xBinLabels;};

        void setXBinLabels(std::vector<std::string>& binlabels) {
            xBinLabels = new std::vector<std::string>(binlabels);
        }

        void readSettings(std::istringstream& settingLine);
        virtual void readSetting(std::string& setting, std::pair<std::string, std::string>& currSetAndVal);

        virtual bool is2D() {return false;}

        static bool searchHist(Histogram*, std::string&);

        void setPrintToFile(bool set) {printToFile = set;}
        bool getPrintToFile() {return printToFile;}
        
        std::string getCleanName() const {return cleanName;}
        void setCleanName(std::string& newCleanName) {cleanName = newCleanName;}

        void setRelevance(std::map<TString, bool> newRelevance) {relevance = newRelevance;}
        std::map<TString, bool> getRelevance() {return relevance;}
        bool isRelevant(TString& entry) {return relevance[entry];}

        bool HasRebin() const {if (RebinVar || RebinFixed != 1) return true; else return false;}
        double* GetRebinVar() {return RebinVar;}
        int GetRebin() {return RebinFixed;}
};

//bool searchHist(Histogram*, std::string&);

#endif