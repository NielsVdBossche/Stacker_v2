#include "../interface/DatacardWriter.h"

void DatacardWriter::WriteDatacardVariations()
{
    // for each histogram: write a separate datacard -> reuse code but don't produce new outputfile.
    for (auto it : allHistograms)
    {
        WriteDatacardVariationInit(it);
    }
}

void DatacardWriter::WriteDatacardVariationInit(Histogram *histogram)
{
    std::ofstream datacardVar;
    datacardVar.open("combineFiles/Variations/Base/" + datacardName + "_" + histogram->getCleanName() + ".txt");

    datacardVar << "imax " << allHistograms.size() << std::endl;
    datacardVar << "jmax *" << std::endl;
    datacardVar << "kmax *" << std::endl;

    for (unsigned i = 0; i < 100; i++)
    {
        datacardVar << "-";
    }

    datacardVar << "shapes * * " << yearID << ".root";
    // combinefile name
    // folder structure
    datacardVar << " $CHANNEL/$PROCESS $CHANNEL/$SYSTEMATIC/$PROCESS" << std::endl;

    for (unsigned i = 0; i < 100; i++)
    {
        datacardVar << "-";
    }

    datacardVar << std::setw(20) << "bin\t";
    std::stringstream rates;
    rates << std::setw(20) << "observation\t";

    datacardVar << std::setw(15) << histogram->getCleanName() << "\t";
    rates << std::setw(15) << "-1"
          << "\t";

    datacardVar << std::endl;
    datacardVar << rates.str() << std::endl;

    for (unsigned i = 0; i < 100; i++)
    {
        datacardVar << "-";
    }

    datacardVar << std::setw(30) << "bin"
                << "\t" << std::setw(15) << "\t";
    ;

    // loop histograms,
    std::stringstream processline;
    std::stringstream processNumbers;
    std::stringstream ratesAlt;

    processline << std::setw(30) << "process"
                << "\t" << std::setw(15) << "\t";
    processNumbers << std::setw(30) << "process"
                   << "\t" << std::setw(15) << "\t";
    ratesAlt << std::setw(30) << "rate"
             << "\t" << std::setw(15) << "\t";

    // loop histograms
    Process *proc = allProc->getTail();
    int j = -1;
    while (proc)
    {
        j++;

        if (!histogram->getRelevance()[proc->getName()])
        {
            proc = proc->getPrev();
            continue;
        }

        datacardVar << std::setw(15) << histogram->getCleanName() << "\t";
        processline << std::setw(15) << proc->getName().Data() << "\t";
        processNumbers << std::setw(15) << j << "\t";
        ratesAlt << std::setw(15) << "-1"
                 << "\t";

        proc = proc->getPrev();
    }

    datacardVar << std::endl;
    datacardVar << processline.str() << std::endl;
    datacardVar << processNumbers.str() << std::endl;
    datacardVar << ratesAlt.str() << std::endl;

    for (unsigned i = 0; i < 100; i++)
    {
        datacardVar << "-";
    }


    WriteDatacardVariationUncertainties(datacardVar, allProc->getUncHead());
}

void DatacardWriter::WriteDatacardVariationUncertainties(std::ofstream& datacardVar, Uncertainty *uncertainty, bool eraSpecific)
{
    // consider switching to just 4 bools for era or just naming it erawise
    //std::cout << uncertainty->getName() << " " << uncertainty->isEraSpecific() << eraSpecific << std::endl;
    if (uncertainty->isEraSpecific() && !eraSpecific && uncertainty->isBothEraAndFull())
    {
        WriteDatacardVariationUncertainties(datacardVar, uncertainty, true);
    }
    else if (!uncertainty->isBothEraAndFull() && uncertainty->isEraSpecific() && !eraSpecific)
    {
        WriteDatacardVariationUncertainties(datacardVar, uncertainty, true);
        if (uncertainty->getNext() != nullptr)
            WriteDatacardVariationUncertainties(datacardVar, uncertainty->getNext());
        return;
    }
    if (uncertainty->is1718Specific() && !eraSpecific)
        WriteDatacardVariation1718Uncertainties(datacardVar, uncertainty);

    // write name
    std::string name = uncertainty->getName();

    std::vector<TString> relevantProcesses = uncertainty->getRelevantProcesses();

    unsigned k = 1;
    if (!uncertainty->getCorrelatedAmongProcesses())
        k = relevantProcesses.size();

    for (unsigned j = 0; j < k; j++)
    {
        std::string tempName = name;
        if (k > 1)
            tempName += relevantProcesses[j].Data();
        if (eraSpecific)
            tempName += yearID;
        datacardVar << std::setw(30) << tempName << "\t";

        double errorValue = 1.0;

        if (uncertainty->isFlat())
        {
            datacardVar << std::setw(15) << "lnN"
                        << "\t";
            if (eraSpecific)
                errorValue = uncertainty->getFlatRateEra();
            else
                errorValue = uncertainty->getFlatRateAll();
        }
        else
        {
            datacardVar << std::setw(15) << "shape"
                        << "\t";
            uncertainty->setOutputName(tempName);
        }

        for (unsigned i = 0; i < allHistograms.size(); i++)
        {
            std::stringstream interString;

            Process *proc = allProc->getTail();
            while (proc)
            {
                TString currentName = proc->getName();
                if (!allHistograms[i]->isRelevant(currentName))
                {
                    proc = proc->getPrev();
                    continue;
                }
                if (!containsProcess(relevantProcesses, currentName) || (k != 1 && currentName != relevantProcesses[j]))
                {
                    interString << std::setw(15) << "-"
                                << "\t";
                    proc = proc->getPrev();
                    continue;
                }
                interString << std::setw(15) << std::setprecision(5) << errorValue << "\t";

                proc = proc->getPrev();
            }

            // loop over number of histograms we consider
            datacardVar << interString.str();
        }
        datacardVar << std::endl;
    }

    if (eraSpecific)
        return;
    else if (uncertainty->getNext() != nullptr)
        WriteDatacardVariationUncertainties(datacardVar, uncertainty->getNext());
    else
        datacardVar << "* autoMCStats 0 1 1" << std::endl;

}

void DatacardWriter::WriteDatacardVariation1718Uncertainties(std::ofstream& datacardVar, Uncertainty *uncertainty)
{
    std::string name = uncertainty->getName() + "1718";

    std::vector<TString> relevantProcesses = uncertainty->getRelevantProcesses();

    unsigned k = 1;
    if (!uncertainty->getCorrelatedAmongProcesses())
        k = relevantProcesses.size();

    for (unsigned j = 0; j < k; j++)
    {
        std::string tempName = name;
        if (k > 1)
            tempName += relevantProcesses[j].Data();
        datacardVar << std::setw(30) << tempName << "\t";

        double errorValue = 1.0;

        if (uncertainty->isFlat())
        {
            datacardVar << std::setw(15) << "lnN"
                        << "\t";
            errorValue = uncertainty->getFlatRate1718();
        }
        else
        {
            datacardVar << std::setw(15) << "shape"
                        << "\t";
        }

        for (unsigned i = 0; i < allHistograms.size(); i++)
        {
            std::stringstream interString;

            Process *proc = allProc->getTail();
            while (proc)
            {
                TString currentName = proc->getName();
                if (!allHistograms[i]->isRelevant(currentName))
                {
                    proc = proc->getPrev();
                    continue;
                }
                if (!containsProcess(relevantProcesses, currentName) || (k != 1 && currentName != relevantProcesses[j]))
                {
                    interString << std::setw(15) << "-"
                                << "\t";
                    proc = proc->getPrev();
                    continue;
                }
                interString << std::setw(15) << std::setprecision(5) << errorValue << "\t";

                proc = proc->getPrev();
            }

            // loop over number of histograms we consider
            datacardVar << interString.str();
        }
        datacardVar << std::endl;
    }
}
