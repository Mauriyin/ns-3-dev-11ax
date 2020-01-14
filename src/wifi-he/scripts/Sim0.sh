#!/bin/bash
source spatial-reuse-functions.sh
rm results
cd ../examples
export AUTO_DELETE_SPATIAL_REUSE_OUTPUT_FILES=1

export enablePcap=0
export enableAscii=0
export rxSensitivity=-82
export standard=11ac

obssPdLevel=-82
#export NS_LOG=DynamicObssPdAlgorithm=logic

for j in 1 2 4 8 16; do
for i in 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40; do
RngRunpar=${i}
npar=${j}
Res=Sim0


export powerBackoff=0
export RngRun=${RngRunpar}
export obssPdAlgorithm=ConstantObssPdAlgorithm
export obssPdThreshold=-82
export maxAmpduSize=4900
export MCS=0
export downlink=0
export uplink=10
export duration=5
export enableRts=0
export txStartOffset=50
export enableObssPd=1
export txGain=0
export rxGain=0
export antennas=1
export maxSupportedTxSpatialStreams=1
export maxSupportedRxSpatialStreams=1
export performTgaxTimingChecks=0
export nodePositionsFile=NONE
export bw=20
export scenario=logdistance
export nBss=2
export payloadSizeUplink=1500
export payloadSizeDownlink=1500
export useIdealWifiManager=0
export r=4
export powSta=20
export powAp=20
export txRange=102
export ccaTrSta=-62
export ccaTrAp=-62
export d=20
export sigma=3.5
export bianchi=1
export n=${npar}
export enableObssPd=1
export test=${Res}_${nBss}_${n}_${MCS}_${d}_${RngRun}
run_one &
sleep 1
done

wait
done
