import argparse
import json
import awkward as ak
import numpy as np
import matplotlib.pyplot as plt
import os

from src.variables.variableReader import VariableReader, Variable
from src.configuration import load_channels
from src.histogramTools import HistogramManager

import src.plotTools.figureCreator as fg
from src import generate_binning
from plotHistograms import modify_yrange_shape, generate_outputfolder, copy_index_html

import plugins.eft as eft


def parse_arguments():
    parser = argparse.ArgumentParser(description='Process command line arguments.')
    # TODO
    # add general specifiers
    # add toggles
    # need specifics for: process, maybe channel? Idk see what nominal plotting does
    # also for year, combined does not make sense here
    args = parser.parse_args()
    return args


def main_plot_EFT(variable: Variable, plotdir: str, histograms, process_info: dict):
    fig, ax_main = fg.create_singleplot()

    binning = generate_binning(variable.range, variable.nbins)
    # first plot nominal, then start adding variations

    nominal_content = np.array(ak.to_numpy(histograms[variable.name]["nominal"]))

    # pretty_name = generate_process_name("SM", info)
    nominal_weights = np.ones(len(nominal_content))
    ax_main.hist(binning[:-1], binning, weights=nominal_weights, histtype="step", color=process_info["color"], label="SM")

    eft_variations = eft.getEFTVariationsLinear()
    minim = 1.
    maxim = 1.
    for eft_var in eft_variations:
        lin_name = "EFT_" + eft_var
        quad_name = lin_name + "_" + eft_var
        current_variation = nominal_content
        current_variation += np.array(ak.to_numpy(histograms[variable.name][lin_name]["Up"]))
        current_variation += np.array(ak.to_numpy(histograms[variable.name][quad_name]["Up"]))

        current_variation /= nominal_content

        minim = min(minim, np.min(current_variation))
        maxim = max(maxim, np.min(current_variation))
        pretty_eft_name = eft_var + " = 1"
        ax_main.hist(binning[:-1], binning, weights=current_variation, histtype="step", label=pretty_eft_name)

    ax_main.set_xlim(variable.range)
    ax_main.set_ylabel("Events")
    modify_yrange_shape((minim, maxim), ax_main)
    ax_main.legend(ncols=2)

    # fix output name
    fig.savefig(os.path.join(plotdir, f"{variable.name}.png"))
    fig.savefig(os.path.join(plotdir, f"{variable.name}.pdf"))
    plt.close(fig)


if __name__ == "__main__":
    args = parse_arguments()

    # load process specifics
    # need a set of processes
    with open(args.processfile, 'r') as f:
        processinfo = json.load(f)["Processes"][args.process]

    variables = VariableReader(args.variablefile, args.variable)
    channels = load_channels(args.channelfile)
    storagepath = args.storage

    outputfolder_base = generate_outputfolder(args.years, args.outputfolder, suffix="_EFT_Variations")

    # first plot nominal, then start adding variations
    # load variables, want to do this for all processes

    # also load channels

    # contrary to plotHistograms: load uncertainties if no args.UseEFT
    # do need a selection somewhere defined for the uncertainties needed/desired
    for channel in channels:
        if args.channel is not None and channel != args.channel:
            continue

        storagepath_tmp = os.path.join(storagepath, channel)
        systematics = ["nominal"]

        systematics.extend(eft.getEFTVariationsGroomed())
        outputfolder = os.path.join(outputfolder_base, channel)
        if not os.path.exists(outputfolder):
            os.makedirs(outputfolder)
        copy_index_html(outputfolder)

        histograms = HistogramManager(storagepath_tmp, args.process, variables, systematics, args.years[0])
        histograms.load_histograms()

        for _, variable in variables.get_variable_objects().items():
            main_plot_EFT(variable, outputfolder, histograms, processinfo)

        for subchannel in channels[channel].subchannels.keys():
            storagepath_tmp = os.path.join(storagepath, channel + subchannel)
            histograms = dict()

            outputfolder = os.path.join(outputfolder_base, channel, subchannel)
            if not os.path.exists(outputfolder):
                os.makedirs(outputfolder)
            copy_index_html(outputfolder)

            for process, info in processinfo.items():
                histograms[process] = dict()
                for year in args.years:
                    histograms[process][year] = HistogramManager(storagepath_tmp, process, variables, systematics, year)
                    histograms[process][year].load_histograms()
            for _, variable in variables.get_variable_objects().items():
                main_plot_EFT(variable, outputfolder, histograms, processinfo)