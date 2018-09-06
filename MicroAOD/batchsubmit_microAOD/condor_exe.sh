PACKAGE=package.tar.gz
OUTPUTDIR=$1
OUTPUTFILENAME=$2
INPUTFILENAMES=$3
INDEX=$4
ARGS=$7

# probably need a few other args, like nEvents and xSec (or maybe not?)

echo "[wrapper] OUTPUTDIR	= " ${OUTPUTDIR}
echo "[wrapper] OUTPUTFILENAME	= " ${OUTPUTFILENAME}
echo "[wrapper] INPUTFILENAMES	= " ${INPUTFILENAMES}
echo "[wrapper] INDEX		= " ${INDEX}

echo "[wrapper] hostname  = " `hostname`
echo "[wrapper] date      = " `date`
echo "[wrapper] linux timestamp = " `date +%s`

######################
# Set up environment #
######################

export SCRAM_ARCH=slc6_amd64_gcc530
source /cvmfs/cms.cern.ch/cmsset_default.sh

# Untar
tar -xJf ${PACKAGE}

# Build
cd CMSSW_8_0_28/src/flashgg
echo "[wrapper] in directory: " ${PWD}
echo "[wrapper] attempting to build"
eval `scramv1 runtime -sh`
scramv1 b ProjectRename
scram b
eval `scramv1 runtime -sh`
cd $CMSSW_BASE/src/flashgg

echo "process.source = cms.Source(\"PoolSource\",
fileNames=cms.untracked.vstring(\"${INPUTFILENAMES}\".split(\",\"))
)

process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32( -1 ) )
" >> MicroAOD/test/microAODstd.py

echo ${ARGS//|/ }

# Create tag file
echo "[wrapper `date +\"%Y%m%d %k:%M:%S\"`] running: cmsRun -n 4 MicroAOD/test/microAODstd.py ${ARGS//|/ }"
cmsRun -n 4 MicroAOD/test/microAODstd.py ${ARGS//|/ }

echo "[wrapper] output root files are currently: "
ls -lh *.root

if [[ $OUTPUTFILENAME = *"test_skim"* ]]; then

    echo "process.source = cms.Source(\"PoolSource\",
fileNames=cms.untracked.vstring(\"file:myMicroAODOutputFile.root\")
)

process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32( -1 ) )
" >> Skimming/test/skim_cfg.py

    if [[ $ARGS = *"DoubleEG"* ]]; then
        sed -i 's/isMC = True/isMC = False/g' Skimming/test/skim_cfg.py
    fi
    
    echo "[wrapper `date +\"%Y%m%d %k:%M:%S\"`] running: cmsRun -n 4 Skimming/test/skim_cfg.py ${ARGS//|/ }"
    cmsRun -n 4 Skimming/test/skim_cfg.py ${ARGS//|/ }

fi

# Copy output
gfal-copy -p -f -t 4200 --verbose file://`pwd`/${OUTPUTFILENAME}.root gsiftp://gftp.t2.ucsd.edu/${OUTPUTDIR}/${OUTPUTFILENAME}_${INDEX}.root --checksum ADLER32
