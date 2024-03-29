#include "../interface/Stacker.h"
#include "../../Styles/Styles.h"
// take histogram, get vector of uncertainties, for each entry draw line, or at least for largest ones

/* 
draw canvas, call uncertainty histograms

*/

void Stacker::drawAllUncertaintyImpacts() {
    setUncertaintyImpactStyle();
    gROOT->ForceStyle();
    std::vector<std::string> uncToDraw = {"ttvNJetsUnc_AddJets", "ttvNJetsUnc_AddBJets", "qcdScale", "isrShape", "fsrShape"}; // ttvNJetsUnc_AddJets
    for (auto histogramID : histogramVec) {
        histogramID->setPrintToFile(false);
        drawUncertaintyImpacts(histogramID, uncToDraw);
    }
}

void Stacker::drawUncertaintyImpacts(Histogram* hist, std::vector<std::string>& uncToDraw) {
    TString name = hist->getID() + "_unc";
    TCanvas* canvas = new TCanvas(name);
    TPad* pad = new TPad(name, name, 0., 0., 1., 1.);

    gStyle->SetPalette(kRainBow);
    gStyle->SetHistLineWidth(2.);
    auto cols = TColor::GetPalette();

    canvas->Draw();
    canvas->cd();
    pad->Draw();
    pad->cd();

    // generate vector now with uncertainties in the correct way that i need
    // idk what to do now
    std::vector<TH1D*> nominalHists = processes->CreateHistogramAllProcesses(hist);
    std::map<std::string, std::pair<TH1D*, TH1D*>> variations = processes->UpAndDownHistograms(hist, nominalHists);

    TH1D* nominal = sumVector(nominalHists);

    TLegend* legend = new TLegend(0.2, 0.8, 0.93, 0.92);
    legend->SetNColumns(3);

    double max=0.;
    double min=0.98;    

    int colIndex=0;
    int step = (cols.GetSize() - 1) / (uncToDraw.size() - 1);

    TH1D* memup = nullptr;

    std::map<std::string, std::string> naming = {
        {"ttvNJetsUnc_AddJets", "ttW + jets"}, 
        {"ttvNJetsUnc_AddBJets", "ttW + HF"}, 
        {"qcdScale", "QCD Scale"}, 
        {"pdfShapeVar", "PDF"}, 
        {"isrShape", "ISR"}, 
        {"fsrShape", "FSR"}};


    std::map<std::string, int> colors = {
        {"ttvNJetsUnc_AddJets", 633}, 
        {"ttvNJetsUnc_AddBJets", 419}, 
        {"qcdScale", 407}, 
        {"pdfShapeVar", 800}, 
        {"isrShape", 906}, 
        {"fsrShape", 882}};

    for (auto it : uncToDraw) {
        //std::cout << it;
        std::pair<TH1D*, TH1D*> upAndDown = variations[it];
        
        // uncertainty / nominal
        TH1D* upVar = upAndDown.first;
        TH1D* downVar = upAndDown.second;

        upVar->Divide(nominal);
        downVar->Divide(nominal);
        if (memup == nullptr) memup = upVar;

        if (std::max(upVar->GetMaximum(), downVar->GetMaximum()) > max) {
            max = std::max(upVar->GetMaximum(), downVar->GetMaximum());
        }


        for (int i=1; i < upVar->GetNbinsX()+1; i++) {
            if (std::min(upVar->GetBinContent(i), downVar->GetBinContent(i)) < min && std::min(upVar->GetBinContent(i), downVar->GetBinContent(i)) > 0.5) {
                min = std::min(upVar->GetBinContent(i), downVar->GetBinContent(i));
            }
        }
        // set style
        upVar->UseCurrentStyle();
        downVar->UseCurrentStyle();
        // no fill color
        upVar->SetLineColor(colors[it]);
        downVar->SetLineColor(colors[it]);
        downVar->SetLineStyle(2);

        // add to legend
        legend->AddEntry(upVar, naming[it].c_str());

        // draw in optimal way
        // use SAME option
        upVar->Draw("HIST SAME");
        downVar->Draw("HIST SAME");

        colIndex += step;
    }
    //auto it = variations[uncToDraw[0]];
    memup->SetMaximum(max * 1.2);
    memup->SetMinimum(min * 0.95); 

    TLine* line = new TLine(nominal->GetBinLowEdge(1), 1., nominal->GetXaxis()->GetBinUpEdge(nominal->GetNbinsX()), 1.);
    line->Draw("SAME");

    legend->Draw();

    pad->Update();
    pad->Modified();

    std::string fullPath = pathToOutput;
    if (runT2B) {
        std::string id = hist->getID().Data();
        fullPath += getChannel(id);
        fullPath += "/";
    }

    canvas->Print(fullPath + hist->getID() + ".png");
    canvas->Print(fullPath + hist->getID() + ".pdf");

}