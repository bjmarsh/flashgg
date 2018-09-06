import sys, os
import time
import itertools
import numpy
import json

from metis.Sample import DBSSample
from metis.CondorTask import CondorTask
from metis.StatsParser import StatsParser

job_tag = "test"
exec_path = "condor_exe.sh"
tar_path = "package.tar.xz"
hadoop_path = "flashgg/MicroAOD/test"

DOSKIM = True

def getArgs(pid, dataset):
    args = "processType={0} datasetName={1}".format(pid, dataset.split("/")[1])
    return args

dsdefs = []

job_jsons = ["datasets_RunIISummer16.json"]
for js in job_jsons:
    jobs = json.load(open(js))
    for pid in jobs["processes"]:
        fpo = jobs["processes"][pid]["filesPerOutput"]
        for ds in jobs["processes"][pid]["datasets"]:
            args = getArgs(pid, ds)                
            dsdefs.append((ds, fpo, args))

total_summary = {}
while True:
    allcomplete = True
    for ds,fpo,args in dsdefs[:]:
        sample = DBSSample( dataset=ds )
        task = CondorTask(
                sample = sample,
                open_dataset = False,
                files_per_output = fpo,
                output_name = "test_skim.root" if DOSKIM else "myMicroAODOutputFile.root",
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
