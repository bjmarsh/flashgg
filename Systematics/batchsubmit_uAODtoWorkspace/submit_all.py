import sys, os
import time
import itertools
import numpy
import json

from metis.Sample import DirectorySample
from metis.CondorTask import CondorTask
from metis.StatsParser import StatsParser

job_tag = "flashgg_metis_test"
exec_path = "condor_exe.sh"
tar_path = "package.tar.gz"
hadoop_path = "flashgg/metis_test"

uAOD_indir = "/hadoop/cms/store/user/bemarsh/flashgg/MicroAOD_skim/2016_skim_v2/"

def getArgs(pid):
    args = "processId={0} maxEvents=-1".format(pid)

    if "tth_" in pid:
        args += " useAAA=0 doFiducial=False tthTagsOnly=False puTarget=2.39e+05,8.38e+05,2.31e+06,3.12e+06,4.48e+06,6e+06,7e+06,1.29e+07,3.53e+07,7.87e+07,1.77e+08,3.6e+08,6.03e+08,8.77e+08,1.17e+09,1.49e+09,1.76e+09,1.94e+09,2.05e+09,2.1e+09,2.13e+09,2.15e+09,2.13e+09,2.06e+09,1.96e+09,1.84e+09,1.7e+09,1.55e+09,1.4e+09,1.24e+09,1.09e+09,9.37e+08,7.92e+08,6.57e+08,5.34e+08,4.27e+08,3.35e+08,2.58e+08,1.94e+08,1.42e+08,1.01e+08,6.9e+07,4.55e+07,2.88e+07,1.75e+07,1.02e+07,5.64e+06,2.99e+06,1.51e+06,7.32e+05,3.4e+05,1.53e+05,6.74e+04,3.05e+04,1.52e+04,8.98e+03,6.5e+03,5.43e+03,4.89e+03,4.52e+03,4.21e+03,3.91e+03,3.61e+03,3.32e+03,3.03e+03,2.75e+03,2.47e+03,2.21e+03,1.97e+03,1.74e+03,1.52e+03,1.32e+03,1.14e+03,983,839"

    if "Data" in pid:
        args += " useAAA=0 doFiducial=False tthTagsOnly=False lumiMask={0}/src/flashgg/MetaData/work/jsons/Cert_271036-284044_13TeV_23Sep2016ReReco_Collisions16_JSON.txt".format(os.environ["CMSSW_BASE"])

    return args

dsdefs = []

job_jsons = ["data_jobs.json","tth_sig_jobs.json"]
for js in job_jsons:
    jobs = json.load(open(js))
    for pid in jobs["processes"]:
    # for pid in ["tth_125"]:
        for dsdef in jobs["processes"][pid]:
            ds = dsdef["ds"]
            fpo = dsdef["filesPerOutput"]
            args = getArgs(pid)
            indir = os.path.join(uAOD_indir, ds.strip("/"))
            dsdefs.append((ds, indir, fpo, args))

total_summary = {}
while True:
    allcomplete = True
    for ds,loc,fpo,args in dsdefs:
        sample = DirectorySample( dataset=ds, location=loc )
        files = [f.name for f in sample.get_files()]
        # print(files)
        task = CondorTask(
                sample = sample,
                open_dataset = False,
                flush = True,
                files_per_output = fpo,
                output_name = "output.root",
                tag = job_tag,
                executable = exec_path,
                tarfile = tar_path,
                condor_submit_params = {"sites" : "T2_US_UCSD"},
                special_dir = hadoop_path,
                arguments = args.replace(" ","|")
                )
        task.process()
        allcomplete = allcomplete and task.complete()
        # save some information for the dashboard
        total_summary[ds] = task.get_task_summary()
    # parse the total summary and write out the dashboard
    StatsParser(data=total_summary, webdir="~/public_html/dump/metis/").do()
    os.system("chmod -R 755 ~/public_html/dump/metis")
    if allcomplete:
        print ""
        print "Job={} finished".format(job_tag)
        print ""
        break
    print "Sleeping 300 seconds ..."
    time.sleep(300)
