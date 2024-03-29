#include "../interface/Stacker.h"

void Stacker::printAll2DHistograms() {
    isRatioPlot = false;
    isSignalLine = false;
    for (auto histogramID : histogramVec2D) {
        if (! onlyDC || (onlyDC && histogramID->getPrintToFile())) {
            print2DHistogram(histogramID);
        }
    }
}

void Stacker::print2DHistogram(Histogram2D* hist) {
    // 2 goals: print straight up normal 2D hists to outfile
    // outfile hists: per process
    // outfig: ratio S/sqrt(B) en S/B (both with total stat error on background?)
    
    TString histID = hist->getID();

    THStack* histStack = new THStack(histID, histID);
    TLegend* legend = getLegend();
    std::vector<TH2D*> histVec = processes->fill2DStack(histStack, histID, legend);

    TCanvas* canv = getCanvas(histID);
    canv->Draw();
    canv->cd();
    TPad* pad = getPad(histID);
    pad->Draw();
    pad->cd();
    gStyle->SetHistLineWidth(0);
    gStyle->SetHistFillStyle(1001);

    //gStyle->SetMarkerSize(1.5);
    histStack->Draw("box");

    stackSettingsPostDraw(pad, histStack, hist, histVec[0], nullptr);
    std::string xlabel = histStack->GetYaxis()->GetTitle();
    std::string ylabel = histStack->GetXaxis()->GetTitle();

    histStack->GetYaxis()->SetTitle(replace(xlabel, "_", " ").c_str());
    histStack->GetXaxis()->SetTitle(replace(ylabel, "_", " ").c_str());
    pad->Update();
    pad->Modified();

    legend->Draw();

    TLatex* info = getDatasetInfo(pad);

    std::string fullPath = pathToOutput;
    if (runT2B) {
        std::string id = histID.Data();
        fullPath += getChannel(id);
        /*
        if (! boost::filesystem::exists(fullPath)) {
            boost::filesystem::create_directory(fullPath);
        }*/

        fullPath += "/";
    }

    canv->Print(fullPath + histID + ".png");
    canv->Print(fullPath + histID + ".pdf");

    delete info;

}