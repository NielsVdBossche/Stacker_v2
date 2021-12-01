#include "../interface/Uncertainty.h"

Uncertainty::Uncertainty(std::string& name, bool flat, bool corrProcess, bool eraSpec, std::vector<TString>& processes) : 
    name(name), flat(flat), correlated(corrProcess), eraSpecific(eraSpec), relevantProcesses(processes) {

    nameUp = name + "Up";
    nameDown = name + "Down";
}


TH1D* Uncertainty::getUncertainty(TString& histogramID, Process* head, std::vector<TH1D*>& histVec) {
    if (flat) {
        return getFlatUncertainty(histogramID, head, histVec);
    } else {
        return getShapeUncertainty(histogramID, head, histVec);
    }
}


TH1D* Uncertainty::getShapeUncertainty(TString& histogramID, Process* head, std::vector<TH1D*>& histVec) {
    // Loop processes, ask to add stuff

    // create one final TH1D to return
    Process* current = head;
    TH1D* ret = new TH1D(*histVec[0]);
    ret->SetName(histogramID + name);
    ret->SetTitle(histogramID + name);


    //correlated case : linearly add up and down variations
    std::vector<double> varDown(histVec[0]->GetNbinsX() - 2, 0.);
    std::vector<double> varUp(histVec[0]->GetNbinsX() - 2, 0.);

    //uncorrelated case : quadratically add the maximum of the up and down variations
    std::vector<double> var(histVec[0]->GetNbinsX() - 2, 0.);

    int histCount = 0;
    int procCount = 0;

    while (current) {
        // TODO: check if uncertainty needs this process, otherwise continue and put stuff to next one;
        if (current->getName() != relevantProcesses[procCount]) {
            histCount++;
            current = current->getNext();
        }

        TH1D* histNominal = histVec[histCount];
        TH1D* histUp = current->getHistogramUncertainty(nameUp, histogramID);
        TH1D* histDown = current->getHistogramUncertainty(nameDown, histogramID);

        // do stuff
        // anyway
        for (int bin = 1; bin < histNominal->GetNbinsX() - 1; bin++) {

            double nominalContent = histNominal->GetBinContent( bin );
            double downVariedContent = histUp->GetBinContent( bin );
            double upVariedContent = histDown->GetBinContent( bin );
            double down = fabs( downVariedContent - nominalContent );
            double up = fabs( upVariedContent - nominalContent );

            //uncorrelated case : 
            if(! correlated ){
                double variation = std::max( down, up );
                var[bin - 1] += variation*variation;
            
            //correlated case :     
            } else {
                varDown[bin - 1] += down;
                varUp[bin - 1] += up;
            }

        }

        current = current->getNext();
        histCount++;
        procCount++;
    } 

    for (int bin = 1; bin < histVec[0]->GetNbinsX() - 1; bin++) {
        //correlated case :
        if(correlated ){
            var[bin - 1] = std::max( varDown[bin - 1], varUp[bin - 1] );
            var[bin - 1] = var[bin - 1] * var[bin - 1];
        }

        ret->SetBinContent(bin, var[bin - 1]);
    }

    // writeout uncertainty

    return ret;
    
    // set return value

}

TH1D* Uncertainty::getFlatUncertainty(TString& histogramID, Process* head, std::vector<TH1D*>& histVec) {
    Process* current = head;
    //int histCount = 0;

    TH1D* ret = new TH1D(*histVec[0]);
    ret->SetName(histogramID + name);
    ret->SetTitle(histogramID + name);

    std::vector<double> var(histVec[0]->GetNbinsX() - 2, 0.);
    int procCount = 0;

    for (unsigned histCount = 0; histCount < histVec.size(); histCount++) {
        if (current->getName() != relevantProcesses[procCount]) {
            current = current->getNext();
            continue;
        }

        for (int bin = 1; bin < histVec[0]->GetNbinsX() - 1; bin++) {

            if (correlated) {
                double binContent = histVec[histCount]->GetBinContent( bin );
                double variation = binContent * (flatUncertainty - 1.);
                var[bin - 1] += variation;
            } else {
                double variation = histVec[histCount]->GetBinContent( bin )*( flatUncertainty - 1. );
                var[bin - 1] += variation * variation;
            }
        }

        current = current->getNext();
        procCount++;
    }

    for (int bin = 1; bin < histVec[0]->GetNbinsX() - 1; bin++) {
        //correlated case :
        if (correlated) ret->SetBinContent(bin, var[bin - 1] * var[bin - 1]);
        else ret->SetBinContent(bin, var[bin - 1]);
    }

    return ret;
}